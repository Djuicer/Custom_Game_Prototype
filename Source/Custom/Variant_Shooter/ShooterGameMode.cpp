// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "EnemySpawner.h"
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
			ShooterUI->UpdateGameplayStats(0, 0, 1);
		}
	}

	TArray<AActor*> EnemySpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawner::StaticClass(), EnemySpawners);
	for (AActor* SpawnerActor : EnemySpawners)
	{
		AEnemySpawner* EnemySpawner = Cast<AEnemySpawner>(SpawnerActor);
		if (!EnemySpawner)
		{
			continue;
		}

		EnemySpawner->OnWaveStatsUpdated.RemoveDynamic(this, &AShooterGameMode::OnWaveStatsUpdated);
		EnemySpawner->OnWaveStatsUpdated.AddDynamic(this, &AShooterGameMode::OnWaveStatsUpdated);
		OnWaveStatsUpdated(EnemySpawner->GetCurrentWave(), EnemySpawner->GetEnemiesRemainingInCurrentWave());
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

void AShooterGameMode::SetDestroyedEnemyCount(int32 DestroyedEnemyCount)
{
	if (ShooterUI)
	{
		ShooterUI->UpdateDestroyedEnemyCount(DestroyedEnemyCount);
	}
}

void AShooterGameMode::OnWaveStatsUpdated(int32 CurrentWave, int32 EnemiesRemaining)
{
	if (ShooterUI)
	{
		ShooterUI->UpdateEnemiesRemaining(EnemiesRemaining);
		ShooterUI->UpdateWaveNumber(CurrentWave);
	}
}
