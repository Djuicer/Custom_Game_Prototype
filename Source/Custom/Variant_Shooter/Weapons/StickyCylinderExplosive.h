// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StickyCylinderExplosive.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

/**
 * Throwable sticky cylinder explosive for the shooter character.
 * It flies as a projectile, sticks to the first valid blocking surface, and detonates on command.
 */
UCLASS()
class CUSTOM_API AStickyCylinderExplosive : public AActor
{
	GENERATED_BODY()

	/** Collision used for projectile hits and explosion origin. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CollisionComponent;

	/** Visible cylinder body. Assign a custom cylinder mesh in Blueprint if desired. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* CylinderMesh;

	/** Moves the explosive while it is airborne. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

protected:
	/** Explosion radius for push and enemy-only damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion", meta = (ClampMin = 0.0, Units = "cm"))
	float ExplosionRadius = 450.0f;

	/** Base explosion launch/impulse force. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion", meta = (ClampMin = 0.0))
	float ExplosionForce = 1600.0f;

	/** Damage dealt only to enemy actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion", meta = (ClampMin = 0.0))
	float DamageAmount = 50.0f;

	/** Small upward bias so launched characters pop away from the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion")
	float UpwardLaunchBias = 0.25f;

	/** Offset from impact point along the surface normal to avoid z-fighting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Stick", meta = (ClampMin = 0.0, Units = "cm"))
	float StickSurfaceOffset = 8.0f;

	/** Whether this explosive has already stuck to a surface. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sticky Explosive|Stick")
	bool bIsStuck = false;

	/** Whether this explosive has already detonated. */
	bool bHasDetonated = false;

public:
	AStickyCylinderExplosive();

	/** Launches the explosive in the supplied direction. */
	void LaunchInDirection(const FVector& Direction);

	/** Detonates the explosive if it is stuck and has not already detonated. */
	UFUNCTION(BlueprintCallable, Category = "Sticky Explosive")
	void Detonate();

	/** Returns true after the explosive has stuck to a valid surface. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive")
	bool IsStuck() const { return bIsStuck; }

	/** Returns the collision component. */
	UCapsuleComponent* GetCollisionComponent() const { return CollisionComponent; }

	/** Returns the visible cylinder mesh. */
	UStaticMeshComponent* GetCylinderMesh() const { return CylinderMesh; }

protected:
	virtual void BeginPlay() override;

	virtual void NotifyHit(
		UPrimitiveComponent* MyComp,
		AActor* Other,
		UPrimitiveComponent* OtherComp,
		bool bSelfMoved,
		FVector HitLocation,
		FVector HitNormal,
		FVector NormalImpulse,
		const FHitResult& Hit
	) override;

	/** Stops movement and attaches/aligned the cylinder to the hit surface. */
	void StickToSurface(const FHitResult& Hit);

	/** Applies launch/impulse push and enemy-only damage around the explosive. */
	void ApplyExplosionEffects();

	/** Returns true if the actor should receive enemy-only explosive damage. */
	bool IsDamageableEnemy(AActor* Actor) const;
};
