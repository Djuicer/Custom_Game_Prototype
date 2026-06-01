// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StickyCylinderExplosive.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStickyCooldownStartedDelegate, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStickyCooldownUpdatedDelegate, float, Remaining, float, Duration, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStickyCooldownFinishedDelegate);

/**
 * Throwable sticky cylinder explosive and Blueprint-friendly ability owner.
 *
 * ShooterCharacter creates one hidden instance as the gameplay ability. That instance owns
 * activation/cooldown state and spawns visible projectile instances of this class when used.
 * Projectile instances keep the existing sticky/detonation behavior.
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
	float ExplosionForce = 2200.0f;

	/** Damage dealt only to enemy actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion", meta = (ClampMin = 0.0))
	float DamageAmount = 50.0f;

	/** Optional Blueprint actor spawned when this explosive detonates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion")
	TSubclassOf<AActor> ExplosionEffectClass;

	/** Small upward bias so launched characters pop away from the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Explosion")
	float UpwardLaunchBias = 0.25f;

	/** Offset from impact point along the surface normal to avoid z-fighting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Stick", meta = (ClampMin = 0.0, Units = "cm"))
	float StickSurfaceOffset = 8.0f;

	/** Cooldown seconds applied by the ability instance after a successful activation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Cooldown", meta = (ClampMin = 0.0, Units = "s"))
	float CooldownDuration = 3.0f;

	/** How often the ability instance broadcasts cooldown progress. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Cooldown", meta = (ClampMin = 0.01, Units = "s"))
	float CooldownUpdateInterval = 0.05f;

	/** Projectile actor class spawned by the ability instance. Defaults to this native class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sticky Explosive|Ability")
	TSubclassOf<AStickyCylinderExplosive> ProjectileClass;

	/** Distance in front of the first-person camera used to spawn the sticky explosive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Ability", meta = (ClampMin = 0.0, Units = "cm"))
	float SpawnDistance = 100.0f;

	/** Yaw angle between each sticky cylinder in a multi-throw fan. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Ability", meta = (ClampMin = 0.0, Units = "deg"))
	float CylinderSpreadAngle = 10.0f;

	/** Sideways spacing between each sticky cylinder spawn point in a multi-throw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sticky Explosive|Ability", meta = (ClampMin = 0.0, Units = "cm"))
	float CylinderSpawnSideOffset = 18.0f;

	/** Whether this explosive has already stuck to a surface. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sticky Explosive|Stick")
	bool bIsStuck = false;

	/** Whether detonation was requested before the explosive stuck to a surface. */
	bool bPendingDetonation = false;

	/** Whether this explosive has already detonated. */
	bool bHasDetonated = false;

	/** True while this ability instance is cooling down. */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Sticky Explosive|Cooldown")
	bool bIsOnCooldown = false;

	/** World time when the current cooldown ends. */
	UPROPERTY(Transient)
	float CooldownEndTime = 0.0f;

	/** Active sticky explosives spawned by this ability instance. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AStickyCylinderExplosive>> ActiveStickyExplosives;

	/** Last active sticky explosive spawned by this ability instance. */
	UPROPERTY(Transient)
	TObjectPtr<AStickyCylinderExplosive> ActiveStickyExplosive;

	FTimerHandle CooldownTimerHandle;

public:
	AStickyCylinderExplosive();

	/** Broadcast when a successful activation starts cooldown. */
	UPROPERTY(BlueprintAssignable, Category = "Sticky Explosive|Cooldown")
	FStickyCooldownStartedDelegate OnCooldownStarted;

	/** Broadcast periodically while cooldown is active. Percent is 1 at cooldown start and 0 when ready. */
	UPROPERTY(BlueprintAssignable, Category = "Sticky Explosive|Cooldown")
	FStickyCooldownUpdatedDelegate OnCooldownUpdated;

	/** Broadcast when the ability becomes ready again. */
	UPROPERTY(BlueprintAssignable, Category = "Sticky Explosive|Cooldown")
	FStickyCooldownFinishedDelegate OnCooldownFinished;

	/** Returns true when this ability instance is allowed to activate. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive|Ability")
	bool CanActivateAbility() const;

	/** Attempts to activate the ability. Cooldown starts only after explosives are successfully spawned. */
	UFUNCTION(BlueprintCallable, Category = "Sticky Explosive|Ability")
	void ActivateAbility();

	/** Returns true while this ability instance is cooling down. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive|Cooldown")
	bool IsOnCooldown() const;

	/** Remaining cooldown seconds, or 0 when ready. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive|Cooldown")
	float GetCooldownRemaining() const;

	/** Configured cooldown duration in seconds. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive|Cooldown")
	float GetCooldownDuration() const { return CooldownDuration; }

	/** Cooldown progress from 1 at cooldown start to 0 when ready. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive|Cooldown")
	float GetCooldownPercent() const;

	/** Requests detonation for every active projectile spawned by this ability instance. */
	UFUNCTION(BlueprintCallable, Category = "Sticky Explosive|Ability")
	void DetonateActiveExplosives();

	/** Launches this projectile instance in the supplied direction. */
	void LaunchInDirection(const FVector& Direction);

	/** Requests detonation, or detonates immediately if this projectile is already stuck. */
	UFUNCTION(BlueprintCallable, Category = "Sticky Explosive")
	void RequestDetonation();

	/** Detonates once when stuck, or records a pending detonation request while airborne. */
	UFUNCTION(BlueprintCallable, Category = "Sticky Explosive")
	void Detonate();

	/** Returns true after this projectile has stuck to a valid surface. */
	UFUNCTION(BlueprintPure, Category = "Sticky Explosive")
	bool IsStuck() const { return bIsStuck; }

	/** Returns the collision component. */
	UCapsuleComponent* GetCollisionComponent() const { return CollisionComponent; }

	/** Returns the visible cylinder mesh. */
	UStaticMeshComponent* GetCylinderMesh() const { return CylinderMesh; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

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

	/** Spawns projectile instances for a successful ability activation. */
	bool SpawnAbilityProjectiles();

	/** Starts cooldown on this ability instance. */
	void StartCooldown();

	/** Broadcasts the current cooldown state and finishes when time expires. */
	void HandleCooldownTick();

	/** Finishes cooldown and returns this ability instance to ready. */
	void FinishCooldown();

	/** Clears the active sticky explosive reference when a spawned projectile is destroyed. */
	UFUNCTION()
	void HandleActiveStickyExplosiveDestroyed(AActor* DestroyedActor);

	/** Stops movement and attaches/aligned the cylinder to the hit surface. */
	void StickToSurface(const FHitResult& Hit);

	/** Spawns the optional Blueprint explosion effect at the current actor transform. */
	void SpawnExplosionEffect();

	/** Applies launch/impulse push and enemy-only damage around the explosive. */
	void ApplyExplosionEffects();

	/** Returns true if the actor should receive enemy-only explosive damage. */
	bool IsDamageableEnemy(AActor* Actor) const;
};
