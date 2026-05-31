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

	/** Type of legacy scoreboard UI widget to spawn. Disabled by default because AShooterPlayerController owns the clean HUD. */
	UPROPERTY(EditAnywhere, Category="Shooter|Legacy UI")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** If true, spawn the legacy ShooterUI scoreboard in addition to the clean gameplay HUD. */
	UPROPERTY(EditAnywhere, Category="Shooter|Legacy UI")
	bool bSpawnLegacyShooterUI = false;

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
};
