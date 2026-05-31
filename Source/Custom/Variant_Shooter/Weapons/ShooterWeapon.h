// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 *  Base class for a simple first person shooter weapon
 *  Provides both first person and third person perspective meshes
 *  Handles ammo and firing logic
 *  Interacts with the weapon owner through the ShooterWeaponHolder interface
 */
UCLASS(abstract)
class CUSTOM_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	/** First person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Third person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

protected:

	/** Cast pointer to the weapon owner */
	IShooterWeaponHolder* WeaponOwner;

	/** Type of projectiles this weapon will shoot */
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** Number of bullets in a magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 10;

	/** Number of bullets in the current magazine */
	int32 CurrentBullets = 0;

	/** Destroyed enemies required before this grenade launcher fires three projectiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0))
	int32 TripleShotEnemyRequirement = 10;

	/** Destroyed enemies required before this grenade launcher fires five projectiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0))
	int32 FiveShotEnemyRequirement = 12;

	/** Destroyed enemies required before projectiles use the upgraded enemy pull force. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0))
	int32 StrongPullEnemyRequirement = 15;

	/** Projectile count before grenade launcher shot-count upgrades are unlocked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 1, ClampMax = 25))
	int32 DefaultProjectileCount = 1;

	/** Projectile count after TripleShotEnemyRequirement destroyed enemies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 1, ClampMax = 25))
	int32 TripleShotProjectileCount = 3;

	/** Projectile count after FiveShotEnemyRequirement destroyed enemies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 1, ClampMax = 25))
	int32 FiveShotProjectileCount = 5;

	/** Yaw angle between projectiles when upgraded shots fire multiple grenades. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0.0, ClampMax = 45.0, Units = "Degrees"))
	float SpreadAngle = 5.0f;

	/** Pull force assigned to spawned grenade projectiles before the strong-pull upgrade unlocks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0.0))
	float DefaultProjectilePullForce = 2200.0f;

	/** Pull force assigned to spawned grenade projectiles after StrongPullEnemyRequirement is reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grenade Launcher Upgrades", meta = (ClampMin = 0.0))
	float UpgradedProjectilePullForce = 6000.0f;

	/** Current projectile count selected from destroyed enemy upgrade thresholds. */
	int32 CurrentProjectileCount = 1;

	/** Current projectile pull force selected from destroyed enemy upgrade thresholds. */
	float CurrentProjectilePullForce = 2200.0f;

	/** Tracks whether the triple-shot upgrade unlock log has already fired. */
	bool bTripleShotUpgradeUnlocked = false;

	/** Tracks whether the five-shot upgrade unlock log has already fired. */
	bool bFiveShotUpgradeUnlocked = false;

	/** Tracks whether the strong-pull upgrade unlock log has already fired. */
	bool bStrongPullUpgradeUnlocked = false;
	
	/** Animation montage to play when firing this weapon */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** AnimInstance class to set for the first person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** AnimInstance class to set for the third person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	/** Cone half-angle for variance while aiming */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	/** Amount of firing recoil to apply to the owner */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	/** Name of the first person muzzle socket where projectiles will spawn */
	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	/** Distance ahead of the muzzle that bullets will spawn at */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	/** Minimum safe distance ahead of the muzzle for grenade projectile spawns. */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MinimumProjectileSpawnOffset = 100.0f;

	/** If true, this weapon will automatically fire at the refire rate */
	UPROPERTY(EditAnywhere, Category="Refire")
	bool bFullAuto = false;

	/** Time between shots for this weapon. Affects both full auto and semi auto modes */
	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	/** Game time of last shot fired, used to enforce refire rate on semi auto */
	float TimeOfLastShot = 0.0f;

	/** If true, the weapon is currently firing */
	bool bIsFiring = false;

	/** Timer to handle full auto refiring */
	FTimerHandle RefireTimer;

	/** Cast pawn pointer to the owner for AI perception system interactions */
	TObjectPtr<APawn> PawnOwner;

	/** Loudness of the shot for AI perception system interactions */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	/** Max range of shot AI perception noise */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 3000.0f;

	/** Tag to apply to noise generated by shooting this weapon */
	UPROPERTY(EditAnywhere, Category="Perception")
	FName ShotNoiseTag = FName("Shot");

public:	

	/** Constructor */
	AShooterWeapon();

protected:
	
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay Cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Called when the weapon's owner is destroyed */
	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	/** Activates this weapon and gets it ready to fire */
	void ActivateWeapon();

	/** Deactivates this weapon */
	void DeactivateWeapon();

	/** Start firing this weapon */
	void StartFiring();

	/** Refreshes grenade launcher upgrades from the owner's existing destroyed enemy count. */
	void RefreshGrenadeLauncherUpgrades(int32 DestroyedEnemyCount);

	/** Stop firing this weapon */
	void StopFiring();

protected:

	/** Fire the weapon */
	virtual void Fire();

	/** Called when the refire rate time has passed while shooting semi auto weapons */
	void FireCooldownExpired();

	/** Fire one or more projectiles towards the target location. */
	virtual void FireProjectile(const FVector& TargetLocation);

	/** Calculates the spawn transform for projectiles shot by this weapon. */
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;

	/** Calculates the spawn transform for one projectile in a multi-projectile spread. */
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation, float YawOffsetDegrees) const;

	/** Returns the yaw offset for the projectile at the given index so spreads stay centered on the aim direction. */
	float CalculateProjectileYawOffset(int32 ProjectileIndex, int32 ProjectileCount) const;

	/** Applies grenade launcher upgrade state from the owner's existing destroyed enemy count. */
	void UpdateGrenadeLauncherUpgrades(int32 DestroyedEnemyCount);

public:

	/** Returns the first person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** Returns the third person mesh */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** Returns the first person anim instance class */
	const TSubclassOf<UAnimInstance>& GetFirstPersonAnimInstanceClass() const;

	/** Returns the third person anim instance class */
	const TSubclassOf<UAnimInstance>& GetThirdPersonAnimInstanceClass() const;

	/** Returns the magazine size */
	int32 GetMagazineSize() const { return MagazineSize; };

	/** Returns the current bullet count */
	int32 GetBulletCount() const { return CurrentBullets; }
};
