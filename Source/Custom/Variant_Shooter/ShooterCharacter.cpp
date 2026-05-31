// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "StickyCylinderExplosive.h"
#include "Enemy.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "Ultimate.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	StickyExplosiveClass = AStickyCylinderExplosive::StaticClass();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;

	if (DefaultWeaponClass)
	{
		AddWeaponClass(DefaultWeaponClass);
	}

	// Allow the first-person camera to look further down
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			// Negative = look up limit
			PC->PlayerCameraManager->ViewPitchMin = -80.0f;

			// Positive = look down limit
			PC->PlayerCameraManager->ViewPitchMax = 89.0f;
		}
	}
	
	// update the HUD
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	// Press Q to switch from ShooterCharacter to Ultimate
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AShooterCharacter::DoSwitchToUltimate);

	// Sticky explosive fallback bindings so the ability is immediately testable without asset changes.
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AShooterCharacter::DoThrowStickyExplosive);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AShooterCharacter::DoDetonateStickyExplosive);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);

		if (ThrowStickyExplosiveAction)
		{
			EnhancedInputComponent->BindAction(ThrowStickyExplosiveAction, ETriggerEvent::Started, this, &AShooterCharacter::DoThrowStickyExplosive);
		}

		if (DetonateStickyExplosiveAction)
		{
			EnhancedInputComponent->BindAction(DetonateStickyExplosiveAction, ETriggerEvent::Started, this, &AShooterCharacter::DoDetonateStickyExplosive);
		}
	}
}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoAim(Yaw, Pitch);
	}
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoMove(Right, Forward);
	}
}

void AShooterCharacter::DoJumpStart()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpStart();
	}
}

void AShooterCharacter::DoJumpEnd()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpEnd();
	}
}

void AShooterCharacter::DoStartFiring()
{
	// fire the current weapon
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons to switch between
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else
		{
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::DoThrowStickyExplosive()
{
	if (IsDead() || !GetWorld())
	{
		return;
	}

	if (!CanThrowExplosiveCylinder())
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("Explosive Cylinder throw blocked by cooldown. Remaining: %.2fs"),
			GetExplosiveCylinderCooldownRemaining()
		);
		return;
	}

	const TArray<TObjectPtr<AStickyCylinderExplosive>> StickyExplosivesToDestroy = ActiveStickyExplosives;
	for (AStickyCylinderExplosive* StickyExplosive : StickyExplosivesToDestroy)
	{
		if (IsValid(StickyExplosive))
		{
			StickyExplosive->Destroy();
		}
	}
	ActiveStickyExplosives.Reset();
	ActiveStickyExplosive = nullptr;

	TSubclassOf<AStickyCylinderExplosive> ExplosiveClass = StickyExplosiveClass;
	if (!ExplosiveClass)
	{
		ExplosiveClass = AStickyCylinderExplosive::StaticClass();
	}

	const int32 CylinderCount = DestroyedEnemyCount >= 10 ? 5 : (DestroyedEnemyCount >= 5 ? 3 : 1);
	const UCameraComponent* FirstPersonCamera = GetFirstPersonCameraComponent();
	const FRotator BaseRotation = FirstPersonCamera ? FirstPersonCamera->GetComponentRotation() : GetControlRotation();
	const FVector BaseLocation = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation();
	const FVector RightVector = BaseRotation.RotateVector(FVector::RightVector);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 CylinderIndex = 0; CylinderIndex < CylinderCount; ++CylinderIndex)
	{
		const float SpreadStep = static_cast<float>(CylinderIndex) - (static_cast<float>(CylinderCount - 1) * 0.5f);
		const float AngleOffset = SpreadStep * CylinderSpreadAngle;
		const FRotator SpreadRotation = BaseRotation + FRotator(0.0f, AngleOffset, 0.0f);
		const FVector SpreadDirection = SpreadRotation.Vector();
		const FVector SpawnLocation = BaseLocation
			+ SpreadDirection * StickyExplosiveSpawnDistance
			+ RightVector * (SpreadStep * CylinderSpawnSideOffset);

		AStickyCylinderExplosive* SpawnedExplosive = GetWorld()->SpawnActor<AStickyCylinderExplosive>(
			ExplosiveClass,
			SpawnLocation,
			SpreadRotation,
			SpawnParams
		);

		if (SpawnedExplosive)
		{
			SpawnedExplosive->OnDestroyed.AddDynamic(this, &AShooterCharacter::HandleActiveStickyExplosiveDestroyed);
			SpawnedExplosive->LaunchInDirection(SpreadDirection);
			ActiveStickyExplosives.Add(SpawnedExplosive);
			ActiveStickyExplosive = SpawnedExplosive;
		}
	}

	if (!ActiveStickyExplosives.IsEmpty())
	{
		NextExplosiveCylinderThrowTime = GetWorld()->GetTimeSeconds() + ExplosiveCylinderCooldown;
	}
}

void AShooterCharacter::DoDetonateStickyExplosive()
{
	if (IsDead())
	{
		return;
	}

	const TArray<TObjectPtr<AStickyCylinderExplosive>> StickyExplosivesToDetonate = ActiveStickyExplosives;
	for (AStickyCylinderExplosive* StickyExplosive : StickyExplosivesToDetonate)
	{
		if (IsValid(StickyExplosive))
		{
			StickyExplosive->RequestDetonation();
		}
	}
}

void AShooterCharacter::HandleActiveStickyExplosiveDestroyed(AActor* DestroyedActor)
{
	ActiveStickyExplosives.Remove(Cast<AStickyCylinderExplosive>(DestroyedActor));

	if (DestroyedActor == ActiveStickyExplosive)
	{
		ActiveStickyExplosive = ActiveStickyExplosives.IsEmpty() ? nullptr : ActiveStickyExplosives.Last();
	}
}

bool AShooterCharacter::CanThrowExplosiveCylinder() const
{
	return GetExplosiveCylinderCooldownRemaining() <= 0.0f;
}

float AShooterCharacter::GetExplosiveCylinderCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, NextExplosiveCylinderThrowTime - World->GetTimeSeconds());
}

void AShooterCharacter::RegisterDestroyedEnemy(AActor* DestroyedEnemy)
{
	if (!DestroyedEnemy || (!DestroyedEnemy->IsA<AEnemy>() && !DestroyedEnemy->IsA<AShooterNPC>()))
	{
		return;
	}

	const TWeakObjectPtr<AActor> DestroyedEnemyKey(DestroyedEnemy);
	if (CountedDestroyedEnemies.Contains(DestroyedEnemyKey))
	{
		return;
	}

	CountedDestroyedEnemies.Add(DestroyedEnemyKey);

	++DestroyedEnemyCount;
	++UltimateEnemyCharge;

	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->SetDestroyedEnemyCount(DestroyedEnemyCount);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy destroyed: %s | Total destroyed: %d | Ultimate charge: %d/%d"),
		*GetNameSafe(DestroyedEnemy),
		DestroyedEnemyCount,
		UltimateEnemyCharge,
		EnemiesRequiredForUltimate
	);
}

bool AShooterCharacter::ConsumeUltimateCharge()
{
	if (!IsUltimateCharged())
	{
		return false;
	}

	UltimateEnemyCharge -= EnemiesRequiredForUltimate;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Ultimate charge consumed. Remaining charge: %d/%d"),
		UltimateEnemyCharge,
		EnemiesRequiredForUltimate
	);
	return true;
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	// stub
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;
}

void AShooterCharacter::Die()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// grant the death tag to the character
	Tags.Add(DeathTag);
		
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable controls
	DisableInput(nullptr);

	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// call the BP handler
	BP_OnDeath();

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}

bool AShooterCharacter::IsDead() const
{
	// the character is dead if their current HP drops to zero
	return CurrentHP <= 0.0f;
}

void AShooterCharacter::DoSwitchToUltimate()
{
	if (IsDead())
	{
		return;
	}

	if (!IsUltimateCharged())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ultimate is not charged yet. Charge: %d/%d"),
			UltimateEnemyCharge,
			EnemiesRequiredForUltimate
		);
		return;
	}

	if (!UltimateCharacterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UltimateCharacterClass is not set on ShooterCharacter."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoSwitchToUltimate failed: Shooter has no PlayerController."));
		return;
	}

	const float SpawnDistance = 300.0f;

	const FVector SpawnLocation =
		GetActorLocation() +
		GetActorForwardVector() * SpawnDistance;

	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerController;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AUltimate* NewUltimate = World->SpawnActor<AUltimate>(
		UltimateCharacterClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewUltimate)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Ultimate character."));
		return;
	}

	if (!ConsumeUltimateCharge())
	{
		NewUltimate->Destroy();
		return;
	}

	// Store the exact shooter that pressed Q.
	NewUltimate->SetReturnShooterCharacter(this);

	// Hide old shooter but keep it alive at its current location.
	HideForUltimateMode();

	// Keep the same view direction when switching into Ultimate.
	PlayerController->SetControlRotation(GetControlRotation());

	// Possess Ultimate.
	PlayerController->Possess(NewUltimate);

	// Make sure input is active for Ultimate.
	PlayerController->SetIgnoreLookInput(false);
	PlayerController->SetIgnoreMoveInput(false);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Switched to Ultimate. Old shooter kept alive: %s | New pawn: %s"),
		*GetName(),
		*GetNameSafe(PlayerController->GetPawn())
	);
}

void AShooterCharacter::HideForUltimateMode()
{
	GetCharacterMovement()->StopMovementImmediately();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// Stop actor tick while hidden.
	SetActorTickEnabled(false);

	// Stop first-person and third-person character mesh animation ticks.
	if (GetFirstPersonMesh())
	{
		GetFirstPersonMesh()->SetVisibility(false, true);
		GetFirstPersonMesh()->SetComponentTickEnabled(false);
		GetFirstPersonMesh()->bPauseAnims = true;
	}

	if (GetMesh())
	{
		GetMesh()->SetVisibility(false, true);
		GetMesh()->SetComponentTickEnabled(false);
		GetMesh()->bPauseAnims = true;
	}

	// Stop weapon meshes and weapon animation Blueprint ticks.
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (IsValid(Weapon))
		{
			Weapon->SetActorHiddenInGame(true);
			Weapon->SetActorEnableCollision(false);
			Weapon->SetActorTickEnabled(false);

			if (Weapon->GetFirstPersonMesh())
			{
				Weapon->GetFirstPersonMesh()->SetVisibility(false, true);
				Weapon->GetFirstPersonMesh()->SetComponentTickEnabled(false);
				Weapon->GetFirstPersonMesh()->bPauseAnims = true;
			}

			if (Weapon->GetThirdPersonMesh())
			{
				Weapon->GetThirdPersonMesh()->SetVisibility(false, true);
				Weapon->GetThirdPersonMesh()->SetComponentTickEnabled(false);
				Weapon->GetThirdPersonMesh()->bPauseAnims = true;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Shooter hidden for Ultimate mode: %s"), *GetName());
}

void AShooterCharacter::RestoreAfterUltimateMode()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// Restore movement component state.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetComponentTickEnabled(true);
		MoveComp->Activate(true);
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->StopMovementImmediately();
	}

	// Restore input and camera control.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();

		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		EnableInput(PC);

		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -80.0f;
			PC->PlayerCameraManager->ViewPitchMax = 89.0f;
		}
	}

	if (GetFirstPersonMesh())
	{
		GetFirstPersonMesh()->SetVisibility(true, true);
		GetFirstPersonMesh()->SetComponentTickEnabled(true);
		GetFirstPersonMesh()->bPauseAnims = false;
	}

	if (GetMesh())
	{
		GetMesh()->SetVisibility(true, true);
		GetMesh()->SetComponentTickEnabled(true);
		GetMesh()->bPauseAnims = false;
	}

	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (IsValid(Weapon))
		{
			Weapon->SetActorHiddenInGame(false);
			Weapon->SetActorEnableCollision(true);
			Weapon->SetActorTickEnabled(true);

			if (Weapon->GetFirstPersonMesh())
			{
				Weapon->GetFirstPersonMesh()->SetVisibility(true, true);
				Weapon->GetFirstPersonMesh()->SetComponentTickEnabled(true);
				Weapon->GetFirstPersonMesh()->bPauseAnims = false;
			}

			if (Weapon->GetThirdPersonMesh())
			{
				Weapon->GetThirdPersonMesh()->SetVisibility(true, true);
				Weapon->GetThirdPersonMesh()->SetComponentTickEnabled(true);
				Weapon->GetThirdPersonMesh()->bPauseAnims = false;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Shooter restored after Ultimate mode: %s"), *GetName());
}