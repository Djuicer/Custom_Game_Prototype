// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterProjectile.h"

#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

AShooterProjectile::AShooterProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the collision component and assign it as the root
	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));

	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// create the projectile movement component. No need to attach it because it's not a Scene Component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bShouldBounce = true;

	// set the default damage type
	HitDamageType = UDamageType::StaticClass();
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// ignore the pawn that shot this projectile
	CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
}

void AShooterProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the destruction timer
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void AShooterProjectile::NotifyHit(
	class UPrimitiveComponent* MyComp,
	AActor* Other,
	class UPrimitiveComponent* OtherComp,
	bool bSelfMoved,
	FVector HitLocation,
	FVector HitNormal,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	// Ignore if we've already hit something else
	if (bHit)
	{
		return;
	}

	// If the projectile hits the player character/model, do not explode.
	if (ACharacter* HitCharacter = Cast<ACharacter>(Other))
	{
		if (HitCharacter->IsPlayerControlled())
		{
			// Make the projectile ignore the player from now on
			CollisionComponent->IgnoreActorWhenMoving(HitCharacter, true);

			if (OtherComp)
			{
				OtherComp->IgnoreActorWhenMoving(this, true);
			}

			return;
		}
	}

	bHit = true;

	// Disable collision on the projectile
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Make AI perception noise
	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	if (bExplodeOnHit)
	{
		// Apply explosion damage centered on the projectile
		ExplosionCheck(GetActorLocation());
	}
	else
	{
		// Single hit projectile. Process the collided actor
		ProcessHit(Other, OtherComp, Hit.ImpactPoint, -Hit.ImpactNormal);
	}

	// Pass control to BP for any extra effects
	BP_OnProjectileHit(Hit);

	// Check if we should schedule deferred destruction of the projectile
	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DestructionTimer,
			this,
			&AShooterProjectile::OnDeferredDestruction,
			DeferredDestructionTime,
			false
		);
	}
	else
	{
		// Destroy the projectile right away
		Destroy();
	}
}

void AShooterProjectile::ExplosionCheck(const FVector& ExplosionCenter)
{
	// Do a sphere overlap check to look for nearby actors to damage
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (!bDamageOwner)
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		ExplosionCenter,
		FQuat::Identity,
		ObjectParams,
		OverlapShape,
		QueryParams
	);

	TArray<AActor*> DamagedActors;

	for (const FOverlapResult& CurrentOverlap : Overlaps)
	{
		AActor* OverlappedActor = CurrentOverlap.GetActor();

		if (!OverlappedActor)
		{
			continue;
		}

		// Avoid affecting the same actor multiple times
		if (DamagedActors.Contains(OverlappedActor))
		{
			continue;
		}

		DamagedActors.Add(OverlappedActor);

		ACharacter* HitCharacter = Cast<ACharacter>(OverlappedActor);

		// Only affect Character classes
		if (!HitCharacter)
		{
			continue;
		}

		const FVector CharacterLocation = HitCharacter->GetActorLocation();

		FVector AffectDirection;
		float LaunchStrength = 0.0f;

		if (HitCharacter->IsPlayerControlled())
		{
			// Player: push AWAY from the explosion
			AffectDirection = CharacterLocation - ExplosionCenter;
			LaunchStrength = PlayerPushForce;
		}
		else
		{
			// Enemy / AI character: pull TOWARD the explosion
			AffectDirection = ExplosionCenter - CharacterLocation;
			LaunchStrength = EnemyPullForce;
		}

		AffectDirection = AffectDirection.GetSafeNormal();
		
		AffectDirection += FVector(0.0f, 0.0f, 0.15f);
		AffectDirection.Normalize();

		ProcessHit(
			HitCharacter,
			CurrentOverlap.GetComponent(),
			CharacterLocation,
			AffectDirection,
			LaunchStrength
		);
	}
}

void AShooterProjectile::ProcessHit(
	AActor* HitActor,
	UPrimitiveComponent* HitComp,
	const FVector& HitLocation,
	const FVector& HitDirection,
	float LaunchStrength
)
{
	ACharacter* HitCharacter = Cast<ACharacter>(HitActor);


	if (!HitCharacter)
	{
		return;
	}


	if (LaunchStrength <= 0.0f)
	{
		LaunchStrength = PhysicsForce;
	}

	const FVector LaunchVelocity = HitDirection * LaunchStrength;

	HitCharacter->LaunchCharacter(
		LaunchVelocity,
		true,   
		true    
	);

	// Damage enemy after launching it
	if (AEnemy* Enemy = Cast<AEnemy>(HitCharacter))
	{
		Enemy->DealDamage(HitDamage);
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	// destroy this actor
	Destroy();
}
