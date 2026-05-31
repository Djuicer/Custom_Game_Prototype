// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the UI
	if (ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToViewport(0);
			ShooterUI->SetDestroyedEnemyCount(0);
			ShooterUI->SetRemainingEnemiesInWave(0);
			ShooterUI->SetCurrentWave(1);
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
	}
}

void AShooterGameMode::SetDestroyedEnemyCount(int32 NewCount)
{
	if (ShooterUI)
	{
		ShooterUI->SetDestroyedEnemyCount(NewCount);
	}
}

void AShooterGameMode::SetRemainingEnemiesInWave(int32 NewCount)
{
	if (ShooterUI)
	{
		ShooterUI->SetRemainingEnemiesInWave(NewCount);
	}
}

void AShooterGameMode::SetCurrentWave(int32 NewWave)
{
	if (ShooterUI)
	{
		ShooterUI->SetCurrentWave(NewWave);
	}
}

void AShooterGameMode::SetWaveInfo(int32 NewWave, int32 RemainingEnemies)
{
	if (ShooterUI)
	{
		ShooterUI->SetCurrentWave(NewWave);
		ShooterUI->SetRemainingEnemiesInWave(RemainingEnemies);
	}
}
