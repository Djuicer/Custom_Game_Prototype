// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class CUSTOM_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

	int32 GameplayDestroyedEnemyCount = 0;
	int32 GameplayRemainingEnemiesInWave = 0;
	int32 GameplayCurrentWave = 1;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Updates the gameplay HUD with the lifetime number of destroyed enemies. */
	void SetDestroyedEnemyCount(int32 NewCount);

	/** Updates the gameplay HUD with the number of enemies remaining in the active wave. */
	void SetRemainingEnemiesInWave(int32 NewCount);

	/** Updates the gameplay HUD with the active wave number. */
	void SetCurrentWave(int32 NewWave);

	/** Returns the active shooter UI created by this game mode. */
	UShooterUI* GetShooterUI() const { return ShooterUI; }
};
