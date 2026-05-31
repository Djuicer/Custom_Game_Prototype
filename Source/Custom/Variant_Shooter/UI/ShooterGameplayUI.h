// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterGameplayUI.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * Reusable gameplay HUD widget for displaying wave, enemy, score, and health values.
 */
UCLASS(BlueprintType, Blueprintable)
class CUSTOM_API UShooterGameplayUI : public UUserWidget
{
	GENERATED_BODY()

public:
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

	/** Optional text block named ScoreText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> ScoreText;

	/** Optional text block named HealthText in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UTextBlock> HealthText;

	/** Optional progress bar named HealthBar in the Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|Gameplay UI|Widgets")
	TObjectPtr<UProgressBar> HealthBar;

	/** Latest current wave value. */
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 CurrentWave = 0;

	/** Latest enemies remaining value. */
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 EnemiesRemaining = 0;

	/** Latest score or destroyed enemy count value. */
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	int32 Score = 0;

	/** Latest current player health value. */
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float CurrentHealth = 0.0f;

	/** Latest maximum player health value. */
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Gameplay UI")
	float MaxHealth = 0.0f;

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshWaveText();
	void RefreshEnemiesRemainingText();
	void RefreshScoreText();
	void RefreshHealthWidgets();
};
