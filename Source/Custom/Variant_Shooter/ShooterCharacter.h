// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class AStickyCylinderExplosive;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class AUltimate;
class UAbilityCooldownWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class CUSTOM_API AShooterCharacter : public ACustomCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()
	
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** Optional Enhanced Input action for the Shift ability. Shift is also bound directly in C++. */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* ThrowStickyExplosiveAction;

	/** Optional Enhanced Input action for detonating the sticky explosive. Right Mouse Button is also bound directly in C++. */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* DetonateStickyExplosiveAction;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

	/** Current HP remaining to this character */
	float CurrentHP = 0.0f;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	/** Actor tag to grant this character when it dies */
	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapons")
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;


	/**
	 * Widget Blueprint class for the Shift ability cooldown UI.
	 * Assign your Widget Blueprint derived from UAbilityCooldownWidget on the Shooter Character Blueprint.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UAbilityCooldownWidget> AbilityWidgetClass;

	/** Runtime instance of the ability cooldown widget created in BeginPlay. */
	UPROPERTY(BlueprintReadOnly, Category="UI")
	TObjectPtr<UAbilityCooldownWidget> AbilityWidget;

	/** Sticky explosive ability class to instantiate. Uses the native cylinder explosive if left unset. */
	UPROPERTY(EditDefaultsOnly, Category="Sticky Explosive")
	TSubclassOf<AStickyCylinderExplosive> StickyExplosiveClass;

	/** Runtime ability instance that owns activation and cooldown state for Shift. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Sticky Explosive")
	TObjectPtr<AStickyCylinderExplosive> StickyCylinderExplosiveAbility;


	/** Lifetime number of enemy actors that have reported death/destruction to this player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ultimate")
	int32 DestroyedEnemyCount = 0;

	/** Enemy actors already credited, so duplicate Blueprint/native death notifications cannot double-charge Ultimate. */
	TSet<TWeakObjectPtr<AActor>> CountedDestroyedEnemies;

	/** Current Ultimate charge. The UI displays UltimateCharge / UltimateMaxCharge as 0% to 100%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ultimate", meta = (ClampMin = "0.0"))
	float UltimateCharge = 0.0f;

	/** Charge required before Q can activate the Ultimate. Keep this at 10 for one charge per enemy, or use 100 for percentage-style tuning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ultimate", meta = (ClampMin = "1.0"))
	float UltimateMaxCharge = 10.0f;

	/** Charge granted by RegisterDestroyedEnemy for each unique destroyed enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ultimate", meta = (ClampMin = "0.0"))
	float UltimateChargePerEnemyDestroyed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Switching")
	TSubclassOf<AUltimate> UltimateCharacterClass;

	void DoSwitchToUltimate();

	void RefreshUltimateWidget();

public:

	/** Bullet count updated delegate */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** Damaged delegate */
	FDamagedDelegate OnDamaged;
	
	void HideForUltimateMode();
	void RestoreAfterUltimateMode();

public:

	/** Constructor */
	AShooterCharacter();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles aim inputs from either controls or UI interfaces */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Handles move inputs from either controls or UI interfaces */
	virtual void DoMove(float Right, float Forward)  override;

	/** Handles jump start inputs from either controls or UI interfaces */
	virtual void DoJumpStart()  override;

	/** Handles jump end inputs from either controls or UI interfaces */
	virtual void DoJumpEnd()  override;

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

	/** Handles the Shift ability input by activating the StickyCylinderExplosive ability. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void HandleAbilityPressed();

	/** Activates the StickyCylinderExplosive ability. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrowStickyExplosive();

	/** Requests detonation for active explosives owned by the StickyCylinderExplosive ability. */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoDetonateStickyExplosive();

	/** Returns true when Shift can throw another explosive cylinder. */
	UFUNCTION(BlueprintPure, Category="Sticky Explosive")
	bool CanThrowExplosiveCylinder() const;

	/** Returns seconds remaining before another explosive cylinder can be thrown. */
	UFUNCTION(BlueprintPure, Category="Sticky Explosive")
	float GetExplosiveCylinderCooldownRemaining() const;

	/** Records exactly one enemy death/destruction notification. Exposed for enemy Blueprint death hooks if needed. */
	UFUNCTION(BlueprintCallable, Category="Ultimate")
	void RegisterDestroyedEnemy(AActor* DestroyedEnemy);

	/** Lifetime number of enemies destroyed. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	int32 GetDestroyedEnemyCount() const { return DestroyedEnemyCount; }

	/** Current Ultimate charge. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	float GetUltimateCharge() const { return UltimateCharge; }

	/** Maximum Ultimate charge needed before Q can activate the Ultimate. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	float GetUltimateMaxCharge() const { return UltimateMaxCharge; }

	/** Returns true when Ultimate charge is full. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	bool IsUltimateReady() const;

	/** Returns Ultimate charge from 0 to 1 for UI progress bars. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	float GetUltimateChargePercent() const;

	/** Adds raw Ultimate charge and refreshes the UI. Safe to call from Blueprint. */
	UFUNCTION(BlueprintCallable, Category="Ultimate")
	void AddUltimateCharge(float Amount);

	/** Simple Blueprint hook for enemy Blueprint/death logic. Call this when an enemy is destroyed. */
	UFUNCTION(BlueprintCallable, Category="Ultimate")
	void AddUltimateChargeFromEnemyDestroyed(float ChargeAmount);

	/** Attempts to activate Ultimate from the current charge. Returns true only when activation succeeds and charge is reset. */
	UFUNCTION(BlueprintCallable, Category="Ultimate")
	bool TryActivateUltimate();

	/** Backwards-compatible helper for older Blueprint graphs that checked enemy-count Ultimate charge. */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	int32 GetUltimateEnemyCharge() const { return FMath::RoundToInt(UltimateCharge); }

	/** Backwards-compatible helper for older Blueprint graphs. Prefer IsUltimateReady(). */
	UFUNCTION(BlueprintPure, Category="Ultimate")
	bool IsUltimateCharged() const { return IsUltimateReady(); }

public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:

	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Called when this character's HP is depleted */
	void Die();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();

public:

	/** Returns true if the character is dead */
	bool IsDead() const;
};
