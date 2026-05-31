// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;
class UShooterHUDWidget;
class AEnemySpawner;

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract, config="Game")
class CUSTOM_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn (legacy HUD). */
	UPROPERTY(EditAnywhere, Category="Shooter|UI|Legacy")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** If true, spawn the legacy bullet/health HUD. Disabled by default so the clean HUD is the only gameplay HUD. */
	UPROPERTY(EditAnywhere, Category="Shooter|UI|Legacy")
	bool bSpawnLegacyBulletCounterUI = false;

	/** Type of clean gameplay HUD widget to spawn. Uses the native C++ UMG layout if no Blueprint subclass is assigned. */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterHUDWidget> ShooterHUDWidgetClass;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Pointer to the clean gameplay HUD widget. */
	UPROPERTY()
	TObjectPtr<UShooterHUDWidget> ShooterHUDWidget;

	/** Wave spawner currently driving the HUD. */
	UPROPERTY()
	TObjectPtr<AEnemySpawner> BoundEnemySpawner;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	UFUNCTION()
	void OnPawnHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void OnDestroyedEnemyCountChanged(int32 DestroyedEnemyCount);

	UFUNCTION()
	void OnWaveChanged(int32 CurrentWave);

	UFUNCTION()
	void OnEnemiesRemainingChanged(int32 EnemiesRemaining);

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	void CreateShooterHUD();
	void BindShooterHUDToSpawner();
	void InitializeShooterHUDFromPawn(AShooterCharacter* ShooterCharacter);
};
