// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

class UTextBlock;
class UVerticalBox;

/**
 *  Simple scoreboard UI for a first person shooter game
 */
UCLASS(abstract)
class CUSTOM_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Allows Blueprint to update score sub-widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);

	/** Updates all wave/gameplay counter text values at once. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void UpdateGameplayStats(int32 InDestroyedEnemyCount, int32 InEnemiesRemaining, int32 InWaveNumber);

	/** Updates the destroyed enemy score text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void UpdateDestroyedEnemyCount(int32 InDestroyedEnemyCount);

	/** Updates the current wave remaining-enemy text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void UpdateEnemiesRemaining(int32 InEnemiesRemaining);

	/** Updates the current wave number text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void UpdateWaveNumber(int32 InWaveNumber);

protected:

	virtual void NativeConstruct() override;

	/** Optional Blueprint-bound score text. Created automatically if the Widget Blueprint does not provide one. */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	/** Optional Blueprint-bound enemies-remaining text. Created automatically if the Widget Blueprint does not provide one. */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemiesRemainingText;

	/** Optional Blueprint-bound wave text. Created automatically if the Widget Blueprint does not provide one. */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveText;

private:

	int32 DestroyedEnemyCount = 0;
	int32 EnemiesRemaining = 0;
	int32 WaveNumber = 1;

	void EnsureGameplayStatsLayout();
	void ConfigureStatText(UTextBlock* TextBlock) const;
	void RefreshText();
};
