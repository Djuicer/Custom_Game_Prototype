// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the first person mesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// create the third person mesh
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	// subscribe to the owner's destroyed delegate
	GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	// cast the weapon owner
	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// fill the first ammo clip
	CurrentBullets = MagazineSize;

	// initialize grenade launcher upgrade state from defaults and the owner's existing destroyed enemy count
	CurrentProjectileCount = FMath::Max(1, DefaultProjectileCount);
	CurrentProjectilePullForce = DefaultProjectilePullForce;
	if (const AShooterCharacter* ShooterOwner = Cast<AShooterCharacter>(GetOwner()))
	{
		UpdateGrenadeLauncherUpgrades(ShooterOwner->GetDestroyedEnemyCount());
	}

	// attach the meshes to the owner
	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// ensure this weapon is destroyed when the owner is destroyed
	Destroy();
}

void AShooterWeapon::ActivateWeapon()
{
	// unhide this weapon
	SetActorHiddenInGame(false);

	// notify the owner
	WeaponOwner->OnWeaponActivated(this);
}

void AShooterWeapon::DeactivateWeapon()
{
	// ensure we're no longer firing this weapon while deactivated
	StopFiring();

	// hide the weapon
	SetActorHiddenInGame(true);

	// notify the owner
	WeaponOwner->OnWeaponDeactivated(this);
}

void AShooterWeapon::RefreshGrenadeLauncherUpgrades(int32 DestroyedEnemyCount)
{
	UpdateGrenadeLauncherUpgrades(DestroyedEnemyCount);
}

void AShooterWeapon::StartFiring()
{
	// raise the firing flag
	bIsFiring = true;

	// check how much time has passed since we last shot
	// this may be under the refire rate if the weapon shoots slow enough and the player is spamming the trigger
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > RefireRate)
	{
		// fire the weapon right away
		Fire();

	} else {

		// if we're full auto, schedule the next shot
		if (bFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeSinceLastShot, false);
		}

	}
}

void AShooterWeapon::StopFiring()
{
	// lower the firing flag
	bIsFiring = false;

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Fire()
{
	// ensure the player still wants to fire. They may have let go of the trigger
	if (!bIsFiring)
	{
		return;
	}

	if (const AShooterCharacter* ShooterOwner = Cast<AShooterCharacter>(GetOwner()))
	{
		UpdateGrenadeLauncherUpgrades(ShooterOwner->GetDestroyedEnemyCount());
	}
	
	// fire a projectile at the target
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// update the time of our last shot
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// make noise so the AI perception system can hear us
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// are we full auto?
	if (bFullAuto)
	{
		// schedule the next shot
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	} else {

		// for semi-auto weapons, schedule the cooldown notification
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);

	}
}

void AShooterWeapon::FireCooldownExpired()
{
	// notify the owner
	WeaponOwner->OnSemiWeaponRefire();
}

void AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	if (!ProjectileClass || !GetWorld())
	{
		return;
	}

	// spawn all projectiles selected by the current grenade launcher upgrade tier
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	APawn* ShooterPawn = Cast<APawn>(GetOwner());
	SpawnParams.Owner = ShooterPawn ? static_cast<AActor*>(ShooterPawn) : GetOwner();
	SpawnParams.Instigator = ShooterPawn;

	const int32 ProjectileCount = FMath::Max(1, CurrentProjectileCount);
	for (int32 ProjectileIndex = 0; ProjectileIndex < ProjectileCount; ++ProjectileIndex)
	{
		const float YawOffset = CalculateProjectileYawOffset(ProjectileIndex, ProjectileCount);
		const FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation, YawOffset);

		AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);
		if (Projectile)
		{
			Projectile->SetEnemyPullForce(CurrentProjectilePullForce);
		}
	}

	// play the firing montage once per trigger pull
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// add recoil once per trigger pull
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// consume one ammo round per trigger pull, even when upgrades spawn multiple projectiles
	--CurrentBullets;

	// if the clip is depleted, reload it
	if (CurrentBullets <= 0)
	{
		CurrentBullets = MagazineSize;
	}

	// update the weapon HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	return CalculateProjectileSpawnTransform(TargetLocation, 0.0f);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation, float YawOffsetDegrees) const
{
	// find the muzzle location
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// calculate the spawn location safely ahead of the muzzle so spread grenades do not spawn inside the shooter
	const FVector AimDirection = (TargetLocation - MuzzleLoc).GetSafeNormal();
	const FVector SpawnLoc = MuzzleLoc + (AimDirection * FMath::Max(MuzzleOffset, MinimumProjectileSpawnOffset));

	// find the aim rotation vector while applying some variance to the target
	FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));
	AimRot.Yaw += YawOffsetDegrees;

	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

float AShooterWeapon::CalculateProjectileYawOffset(int32 ProjectileIndex, int32 ProjectileCount) const
{
	const float CenteredIndex = static_cast<float>(ProjectileIndex) - ((static_cast<float>(ProjectileCount) - 1.0f) * 0.5f);
	return CenteredIndex * SpreadAngle;
}

void AShooterWeapon::UpdateGrenadeLauncherUpgrades(int32 DestroyedEnemyCount)
{
	CurrentProjectileCount = FMath::Max(1, DefaultProjectileCount);
	CurrentProjectilePullForce = DefaultProjectilePullForce;

	if (DestroyedEnemyCount >= TripleShotEnemyRequirement)
	{
		CurrentProjectileCount = FMath::Max(1, TripleShotProjectileCount);

		if (!bTripleShotUpgradeUnlocked)
		{
			bTripleShotUpgradeUnlocked = true;
			UE_LOG(LogTemp, Warning, TEXT("Grenade launcher upgrade unlocked: %d-shot at %d destroyed enemies."), CurrentProjectileCount, DestroyedEnemyCount);
		}
	}

	if (DestroyedEnemyCount >= FiveShotEnemyRequirement)
	{
		CurrentProjectileCount = FMath::Max(1, FiveShotProjectileCount);

		if (!bFiveShotUpgradeUnlocked)
		{
			bFiveShotUpgradeUnlocked = true;
			UE_LOG(LogTemp, Warning, TEXT("Grenade launcher upgrade unlocked: %d-shot at %d destroyed enemies."), CurrentProjectileCount, DestroyedEnemyCount);
		}
	}

	if (DestroyedEnemyCount >= StrongPullEnemyRequirement)
	{
		CurrentProjectilePullForce = UpgradedProjectilePullForce;

		if (!bStrongPullUpgradeUnlocked)
		{
			bStrongPullUpgradeUnlocked = true;
			UE_LOG(LogTemp, Warning, TEXT("Grenade launcher upgrade unlocked: strong pull force %.1f at %d destroyed enemies."), CurrentProjectilePullForce, DestroyedEnemyCount);
		}
	}
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}
