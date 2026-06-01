// Copyright Epic Games, Inc. All Rights Reserved.

#include "StickyCylinderExplosive.h"

#include "Enemy.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AStickyCylinderExplosive::AStickyCylinderExplosive()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision Component"));
	CollisionComponent->InitCapsuleSize(18.0f, 32.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	CylinderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cylinder Mesh"));
	CylinderMesh->SetupAttachment(CollisionComponent);
	CylinderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (DefaultCylinderMesh.Succeeded())
	{
		CylinderMesh->SetStaticMesh(DefaultCylinderMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 2200.0f;
	ProjectileMovement->MaxSpeed = 2200.0f;
	ProjectileMovement->ProjectileGravityScale = 3.50f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

}

void AStickyCylinderExplosive::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* InstigatorPawn = GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(InstigatorPawn, true);
	}
}

void AStickyCylinderExplosive::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

bool AStickyCylinderExplosive::CanActivateAbility() const
{
	return !IsOnCooldown() && !bHasDetonated;
}

void AStickyCylinderExplosive::ActivateAbility()
{
	if (!CanActivateAbility())
	{
		return;
	}

	if (SpawnAbilityProjectiles())
	{
		StartCooldown();
	}
}

bool AStickyCylinderExplosive::IsOnCooldown() const
{
	return bIsOnCooldown && GetCooldownRemaining() > 0.0f;
}

float AStickyCylinderExplosive::GetCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (!World || !bIsOnCooldown)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, CooldownEndTime - World->GetTimeSeconds());
}

float AStickyCylinderExplosive::GetCooldownPercent() const
{
	if (CooldownDuration <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetCooldownRemaining() / CooldownDuration, 0.0f, 1.0f);
}

void AStickyCylinderExplosive::DetonateActiveExplosives()
{
	const TArray<TObjectPtr<AStickyCylinderExplosive>> StickyExplosivesToDetonate = ActiveStickyExplosives;
	for (AStickyCylinderExplosive* StickyExplosive : StickyExplosivesToDetonate)
	{
		if (IsValid(StickyExplosive))
		{
			StickyExplosive->RequestDetonation();
		}
	}
}

bool AStickyCylinderExplosive::SpawnAbilityProjectiles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		OwningPawn = GetInstigator();
	}

	if (!OwningPawn)
	{
		return false;
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

	TSubclassOf<AStickyCylinderExplosive> ExplosiveClass = ProjectileClass;
	if (!ExplosiveClass)
	{
		ExplosiveClass = GetClass();
	}

	const AShooterCharacter* ShooterOwner = Cast<AShooterCharacter>(OwningPawn);
	const int32 DestroyedEnemyCount = ShooterOwner ? ShooterOwner->GetDestroyedEnemyCount() : 0;
	const int32 CylinderCount = DestroyedEnemyCount >= 10 ? 5 : (DestroyedEnemyCount >= 5 ? 3 : 1);
	const UCameraComponent* FirstPersonCamera = ShooterOwner ? ShooterOwner->GetFirstPersonCameraComponent() : nullptr;
	const FRotator BaseRotation = FirstPersonCamera ? FirstPersonCamera->GetComponentRotation() : OwningPawn->GetControlRotation();
	const FVector BaseLocation = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : OwningPawn->GetActorLocation();
	const FVector RightVector = BaseRotation.RotateVector(FVector::RightVector);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningPawn;
	SpawnParams.Instigator = OwningPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 CylinderIndex = 0; CylinderIndex < CylinderCount; ++CylinderIndex)
	{
		const float SpreadStep = static_cast<float>(CylinderIndex) - (static_cast<float>(CylinderCount - 1) * 0.5f);
		const float AngleOffset = SpreadStep * CylinderSpreadAngle;
		const FRotator SpreadRotation = BaseRotation + FRotator(0.0f, AngleOffset, 0.0f);
		const FVector SpreadDirection = SpreadRotation.Vector();
		const FVector SpawnLocation = BaseLocation
			+ SpreadDirection * SpawnDistance
			+ RightVector * (SpreadStep * CylinderSpawnSideOffset);

		AStickyCylinderExplosive* SpawnedExplosive = World->SpawnActor<AStickyCylinderExplosive>(
			ExplosiveClass,
			SpawnLocation,
			SpreadRotation,
			SpawnParams
		);

		if (SpawnedExplosive)
		{
			SpawnedExplosive->OnDestroyed.AddDynamic(this, &AStickyCylinderExplosive::HandleActiveStickyExplosiveDestroyed);
			SpawnedExplosive->LaunchInDirection(SpreadDirection);
			ActiveStickyExplosives.Add(SpawnedExplosive);
			ActiveStickyExplosive = SpawnedExplosive;
		}
	}

	return !ActiveStickyExplosives.IsEmpty();
}

void AStickyCylinderExplosive::StartCooldown()
{
	UWorld* World = GetWorld();
	if (!World || CooldownDuration <= 0.0f)
	{
		FinishCooldown();
		return;
	}

	bIsOnCooldown = true;
	CooldownEndTime = World->GetTimeSeconds() + CooldownDuration;

	OnCooldownStarted.Broadcast(CooldownDuration);
	OnCooldownUpdated.Broadcast(GetCooldownRemaining(), CooldownDuration, GetCooldownPercent());

	World->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&AStickyCylinderExplosive::HandleCooldownTick,
		FMath::Max(0.01f, CooldownUpdateInterval),
		true
	);
}

void AStickyCylinderExplosive::HandleCooldownTick()
{
	if (GetCooldownRemaining() <= 0.0f)
	{
		FinishCooldown();
		return;
	}

	OnCooldownUpdated.Broadcast(GetCooldownRemaining(), CooldownDuration, GetCooldownPercent());
}

void AStickyCylinderExplosive::FinishCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	const bool bWasOnCooldown = bIsOnCooldown;
	bIsOnCooldown = false;
	CooldownEndTime = 0.0f;

	if (bWasOnCooldown)
	{
		OnCooldownUpdated.Broadcast(0.0f, CooldownDuration, 0.0f);
		OnCooldownFinished.Broadcast();
	}
}

void AStickyCylinderExplosive::HandleActiveStickyExplosiveDestroyed(AActor* DestroyedActor)
{
	ActiveStickyExplosives.Remove(Cast<AStickyCylinderExplosive>(DestroyedActor));

	if (DestroyedActor == ActiveStickyExplosive)
	{
		ActiveStickyExplosive = ActiveStickyExplosives.IsEmpty() ? nullptr : ActiveStickyExplosives.Last();
	}
}

void AStickyCylinderExplosive::LaunchInDirection(const FVector& Direction)
{
	const FVector LaunchDirection = Direction.GetSafeNormal();
	SetActorRotation(LaunchDirection.Rotation());
	ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
}

void AStickyCylinderExplosive::NotifyHit(
	UPrimitiveComponent* MyComp,
	AActor* Other,
	UPrimitiveComponent* OtherComp,
	bool bSelfMoved,
	FVector HitLocation,
	FVector HitNormal,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (bIsStuck || bHasDetonated)
	{
		return;
	}

	if (!Other || Other == this || Other == GetInstigator())
	{
		return;
	}

	if (!OtherComp || OtherComp == CollisionComponent || OtherComp->GetOwner() == this)
	{
		return;
	}

	StickToSurface(Hit);
}

void AStickyCylinderExplosive::StickToSurface(const FHitResult& Hit)
{
	UPrimitiveComponent* HitComponent = Hit.GetComponent();

	if (!HitComponent || HitComponent == CollisionComponent || HitComponent->GetOwner() == this)
	{
		return;
	}

	bIsStuck = true;

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	const FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
	const FVector StuckLocation = Hit.ImpactPoint + SurfaceNormal * StickSurfaceOffset;
	const FRotator StuckRotation = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();

	SetActorLocationAndRotation(StuckLocation, StuckRotation);

	AttachToComponent(
		HitComponent,
		FAttachmentTransformRules::KeepWorldTransform,
		Hit.BoneName
	);

	if (bPendingDetonation)
	{
		Detonate();
	}
}

void AStickyCylinderExplosive::RequestDetonation()
{
	if (bHasDetonated)
	{
		return;
	}

	if (!bIsStuck)
	{
		bPendingDetonation = true;
		return;
	}

	Detonate();
}

void AStickyCylinderExplosive::Detonate()
{
	if (bHasDetonated)
	{
		return;
	}

	if (!bIsStuck)
	{
		bPendingDetonation = true;
		return;
	}

	bHasDetonated = true;
	bPendingDetonation = false;
	SpawnExplosionEffect();
	ApplyExplosionEffects();
	Destroy();
}

void AStickyCylinderExplosive::SpawnExplosionEffect()
{
	if (!ExplosionEffectClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	World->SpawnActor<AActor>(
		ExplosionEffectClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams
	);
}

void AStickyCylinderExplosive::ApplyExplosionEffects()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector ExplosionCenter = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape ExplosionShape;
	ExplosionShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StickyCylinderExplosion), false);
	QueryParams.AddIgnoredActor(this);

	World->OverlapMultiByObjectType(
		Overlaps,
		ExplosionCenter,
		FQuat::Identity,
		ObjectParams,
		ExplosionShape,
		QueryParams
	);

	TSet<AActor*> AffectedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* AffectedActor = Overlap.GetActor();
		UPrimitiveComponent* AffectedComponent = Overlap.GetComponent();

		if (!AffectedActor || AffectedActor == this || AffectedActors.Contains(AffectedActor))
		{
			continue;
		}

		AffectedActors.Add(AffectedActor);

		const FVector TargetLocation = AffectedActor->GetActorLocation();
		const float Distance = FVector::Distance(ExplosionCenter, TargetLocation);
		const float DistanceAlpha = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.0f, 1.0f);
		const float ForceScale = FMath::Max(0.1f, DistanceAlpha);

		FVector PushDirection = (TargetLocation - ExplosionCenter).GetSafeNormal();
		if (PushDirection.IsNearlyZero())
		{
			PushDirection = FVector::UpVector;
		}

		PushDirection = (PushDirection + FVector::UpVector * UpwardLaunchBias).GetSafeNormal();
		const FVector LaunchVelocity = PushDirection * ExplosionForce * ForceScale;

		if (ACharacter* Character = Cast<ACharacter>(AffectedActor))
		{
			Character->LaunchCharacter(LaunchVelocity, true, true);
		}
		else if (AffectedComponent && AffectedComponent->IsSimulatingPhysics())
		{
			AffectedComponent->AddImpulseAtLocation(LaunchVelocity * AffectedComponent->GetMass(), ExplosionCenter);
		}

		if (IsDamageableEnemy(AffectedActor))
		{
			UGameplayStatics::ApplyDamage(
				AffectedActor,
				DamageAmount,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);
		}
	}
}

bool AStickyCylinderExplosive::IsDamageableEnemy(AActor* Actor) const
{
	if (!Actor || Actor == GetInstigator())
	{
		return false;
	}

	if (const ACharacter* Character = Cast<ACharacter>(Actor))
	{
		if (Character->IsPlayerControlled())
		{
			return false;
		}
	}

	return Actor->IsA<AEnemy>() || Actor->IsA<AShooterNPC>();
}
