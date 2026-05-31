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

protected:
	/** Optional text widgets that can be supplied by the Widget Blueprint. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemiesRemainingText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveText;

	/** Runtime-created fallback container used when the Widget Blueprint has not added the HUD text yet. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> GameplayHUDContainer;

	int32 DestroyedEnemyCount = 0;
	int32 RemainingEnemiesInWave = 0;
	int32 CurrentWave = 1;

	virtual void NativeConstruct() override;

	void EnsureGameplayHUDWidgets();
	void RefreshGameplayHUDText();
	UTextBlock* CreateHUDTextBlock(FName WidgetName) const;
	void AddGameplayHUDContainerToRoot();

public:
	/** Allows Blueprint to update score sub-widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);

	/** Updates the visible destroyed-enemy score text. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void SetDestroyedEnemyCount(int32 NewCount);

	/** Updates the visible count of enemies remaining in the current wave. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void SetRemainingEnemiesInWave(int32 NewCount);

	/** Updates the visible current wave number. */
	UFUNCTION(BlueprintCallable, Category="Shooter|HUD")
	void SetCurrentWave(int32 NewWave);
};
