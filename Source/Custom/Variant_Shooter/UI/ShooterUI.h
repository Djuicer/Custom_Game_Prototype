// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

class UTextBlock;

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

	/** Sets the lifetime destroyed enemy count and refreshes any bound text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Wave")
	void SetDestroyedEnemyCount(int32 NewCount);

	/** Sets how many enemies remain in the current wave and refreshes any bound text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Wave")
	void SetRemainingEnemiesInWave(int32 NewCount);

	/** Sets the current wave number and refreshes any bound text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Wave")
	void SetCurrentWave(int32 NewWave);

	/** Lifetime number of destroyed enemies. */
	UFUNCTION(BlueprintPure, Category="Shooter|Wave")
	int32 GetDestroyedEnemyCount() const { return DestroyedEnemyCount; }

	/** Number of enemies left to defeat in the current wave. */
	UFUNCTION(BlueprintPure, Category="Shooter|Wave")
	int32 GetRemainingEnemiesInWave() const { return RemainingEnemiesInWave; }

	/** Current wave number. */
	UFUNCTION(BlueprintPure, Category="Shooter|Wave")
	int32 GetCurrentWave() const { return CurrentWave; }

protected:

	/** UUserWidget interface */
	virtual void NativeConstruct() override;

	/** Optional TextBlock for showing the destroyed enemy score, e.g. "Score: 12". */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category="Shooter|Wave")
	TObjectPtr<UTextBlock> DestroyedEnemyCountText;

	/** Optional TextBlock for showing remaining enemies, e.g. "Enemies Remaining: 8". */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category="Shooter|Wave")
	TObjectPtr<UTextBlock> RemainingEnemiesText;

	/** Optional TextBlock for showing current wave, e.g. "Wave: 3". */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category="Shooter|Wave")
	TObjectPtr<UTextBlock> WaveNumberText;

	/** Blueprint-readable score value for manual bindings. */
	UPROPERTY(BlueprintReadOnly, Category="Shooter|Wave")
	int32 DestroyedEnemyCount = 0;

	/** Blueprint-readable remaining enemy value for manual bindings. */
	UPROPERTY(BlueprintReadOnly, Category="Shooter|Wave")
	int32 RemainingEnemiesInWave = 0;

	/** Blueprint-readable current wave value for manual bindings. */
	UPROPERTY(BlueprintReadOnly, Category="Shooter|Wave")
	int32 CurrentWave = 1;

private:

	/** Refreshes any optional bound TextBlocks from the stored values. */
	void RefreshWaveText();
};
