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

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Updates the displayed destroyed enemy score. */
	void SetDestroyedEnemyCount(int32 NewCount);

	/** Updates the displayed remaining enemies for the active wave. */
	void SetRemainingEnemiesInWave(int32 NewCount);

	/** Updates the displayed current wave number. */
	void SetCurrentWave(int32 NewWave);

	/** Updates both displayed wave values together. */
	void SetWaveInfo(int32 NewWave, int32 RemainingEnemies);

	/** Returns the spawned shooter UI, if one is available. */
	UShooterUI* GetShooterUI() const { return ShooterUI; }
};
