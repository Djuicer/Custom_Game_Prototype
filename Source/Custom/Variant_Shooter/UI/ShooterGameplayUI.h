// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterGameplayUI.generated.h"

class AShooterCharacter;
class UProgressBar;
class UTextBlock;

/**
 * Reusable gameplay HUD widget for displaying live shooter gameplay values.
 *
 * The owning ShooterCharacter passes itself through InitializeWithPlayer when the
 * widget is created. Blueprint widgets can either bind directly to the exposed
 * values/getters or name their TextBlock/ProgressBar children as listed below
 * and let RefreshUI populate them.
 */
UCLASS(BlueprintType, Blueprintable)
class CUSTOM_API UShooterGameplayUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Stores the gameplay data source used by this widget. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void InitializeWithPlayer(AShooterCharacter* InPlayer);

	/** Refreshes all cached values from the stored player reference and updates bound widgets. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void RefreshUI();

	/** Returns the currently assigned shooter character, if valid. */
	UFUNCTION(BlueprintPure, Category = "Shooter|Gameplay UI")
	AShooterCharacter* GetShooterCharacter() const { return ShooterCharacter.Get(); }

	/** Updates the displayed current wave number. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void SetCurrentWave(int32 NewWave);

	/** Updates the displayed number of enemies remaining in the current wave. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void SetEnemiesRemaining(int32 NewRemaining);

	/** Updates the displayed score or destroyed enemy count. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void SetScore(int32 NewScore);

	/** Updates the displayed player health and health bar percent. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void SetPlayerHealth(float NewCurrentHealth, float NewMaxHealth);

	/** Refreshes every bound widget from the stored values. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Gameplay UI")
	void RefreshAllUI();

	/** Optional text block named WaveText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> WaveText;

	/** Optional text block named EnemiesRemainingText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> EnemiesRemainingText;

	/** Optional text block named EnemiesAliveText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> EnemiesAliveText;

	/** Optional text block named ScoreText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> ScoreText;

	/** Optional text block named HealthText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> HealthText;

	/** Optional progress bar named HealthBar in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UProgressBar> HealthBar;

	/** Optional text block named UltimateText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> UltimateText;

	/** Optional progress bar named UltimateBar in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UProgressBar> UltimateBar;

	/** Optional text block named UltimateReadyText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> UltimateReadyText;

	/** Optional text block named ExplosiveCooldownText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> ExplosiveCooldownText;

	/** Optional progress bar named ExplosiveCooldownBar in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UProgressBar> ExplosiveCooldownBar;

	/** Optional text block named GrenadeLauncherLevelText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> GrenadeLauncherLevelText;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 CurrentWave = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 EnemiesRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 EnemiesAlive = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float CurrentHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float MaxHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 UltimateCharge = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 UltimateRequired = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float UltimatePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	bool bUltimateReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float ExplosiveCooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float ExplosiveCooldownPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 GrenadeLauncherUpgradeLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 ExplosiveCylinderCount = 1;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AShooterCharacter> ShooterCharacter;

	void RefreshWaveText();
	void RefreshEnemiesRemainingText();
	void RefreshEnemiesAliveText();
	void RefreshScoreText();
	void RefreshHealthWidgets();
	void RefreshUltimateWidgets();
	void RefreshExplosiveCooldownWidgets();
	void RefreshGrenadeLauncherWidgets();
};
