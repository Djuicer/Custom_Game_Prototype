// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the UI
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ShooterUIClass)
		{
			ShooterUI = CreateWidget<UShooterUI>(PlayerController, ShooterUIClass);
			if (ShooterUI)
			{
				ShooterUI->AddToViewport(0);
				ShooterUI->SetDestroyedEnemyCount(GameplayDestroyedEnemyCount);
				ShooterUI->SetRemainingEnemiesInWave(GameplayRemainingEnemiesInWave);
				ShooterUI->SetCurrentWave(GameplayCurrentWave);
			}
		}
	}
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);

	// update the UI
	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
		ShooterUI->SetDestroyedEnemyCount(GameplayDestroyedEnemyCount);
	}
}


void AShooterGameMode::SetDestroyedEnemyCount(int32 NewCount)
{
	GameplayDestroyedEnemyCount = FMath::Max(0, NewCount);
	if (ShooterUI)
	{
		ShooterUI->SetDestroyedEnemyCount(GameplayDestroyedEnemyCount);
	}
}

void AShooterGameMode::SetRemainingEnemiesInWave(int32 NewCount)
{
	GameplayRemainingEnemiesInWave = FMath::Max(0, NewCount);
	if (ShooterUI)
	{
		ShooterUI->SetRemainingEnemiesInWave(GameplayRemainingEnemiesInWave);
	}
}

void AShooterGameMode::SetCurrentWave(int32 NewWave)
{
	GameplayCurrentWave = FMath::Max(1, NewWave);
	if (ShooterUI)
	{
		ShooterUI->SetCurrentWave(GameplayCurrentWave);
	}
}
