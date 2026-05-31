// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterUI.h"

#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "ShooterUI"

void UShooterUI::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshWaveText();
}

void UShooterUI::SetDestroyedEnemyCount(int32 NewCount)
{
	DestroyedEnemyCount = FMath::Max(0, NewCount);
	RefreshWaveText();
}

void UShooterUI::SetRemainingEnemiesInWave(int32 NewCount)
{
	RemainingEnemiesInWave = FMath::Max(0, NewCount);
	RefreshWaveText();
}

void UShooterUI::SetCurrentWave(int32 NewWave)
{
	CurrentWave = FMath::Max(1, NewWave);
	RefreshWaveText();
}

void UShooterUI::RefreshWaveText()
{
	if (DestroyedEnemyCountText)
	{
		DestroyedEnemyCountText->SetText(FText::Format(LOCTEXT("DestroyedEnemyCountFormat", "Score: {0}"), DestroyedEnemyCount));
	}

	if (RemainingEnemiesText)
	{
		RemainingEnemiesText->SetText(FText::Format(LOCTEXT("RemainingEnemiesFormat", "Enemies Remaining: {0}"), RemainingEnemiesInWave));
	}

	if (WaveNumberText)
	{
		WaveNumberText->SetText(FText::Format(LOCTEXT("WaveNumberFormat", "Wave: {0}"), CurrentWave));
	}
}

#undef LOCTEXT_NAMESPACE
