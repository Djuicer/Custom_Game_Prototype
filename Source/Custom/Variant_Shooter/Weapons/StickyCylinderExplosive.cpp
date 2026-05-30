// Copyright Epic Games, Inc. All Rights Reserved.

#include "StickyCylinderExplosive.h"

#include "Enemy.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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

	if (bIsStuck || bHasDetonated || !Other || Other == this || Other == GetInstigator())
	{
		return;
	}

	StickToSurface(Hit);
}

void AStickyCylinderExplosive::StickToSurface(const FHitResult& Hit)
{
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

	if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
	}

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
	ApplyExplosionEffects();
	Destroy();
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
