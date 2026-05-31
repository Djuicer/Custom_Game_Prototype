// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * Clean gameplay HUD for the shooter variant.
 *
 * The widget creates a simple UMG layout in C++ so it can be used directly or
 * as the parent class for a Widget Blueprint such as WBP_ShooterHUD.
 */
UCLASS(Blueprintable)
class CUSTOM_API UShooterHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Updates the displayed wave number. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|HUD")
	void SetCurrentWave(int32 NewWave);

	/** Updates the displayed enemies remaining in the active wave. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|HUD")
	void SetEnemiesRemaining(int32 NewRemaining);

	/** Updates the displayed score / destroyed enemy count. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|HUD")
	void SetScore(int32 NewScore);

	/** Updates the displayed player health text and health bar. */
	UFUNCTION(BlueprintCallable, Category = "Shooter|HUD")
	void SetPlayerHealth(float CurrentHealth, float MaxHealth);

protected:
	virtual void NativeConstruct() override;

	/** Optional text block bindings for Widget Blueprint subclasses. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|HUD")
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|HUD")
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|HUD")
	TObjectPtr<UTextBlock> EnemiesRemainingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|HUD")
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Shooter|HUD")
	TObjectPtr<UProgressBar> HealthBar;

private:
	int32 CurrentWave = 1;
	int32 EnemiesRemaining = 0;
	int32 Score = 0;
	float CurrentHealth = 100.0f;
	float MaxHealth = 100.0f;

	void BuildDefaultLayout();
	void RefreshAllText();
};
