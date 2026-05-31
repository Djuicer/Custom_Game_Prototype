// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGameplayUI.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "ShooterGameplayUI"

void UShooterGameplayUI::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshAllUI();
}

void UShooterGameplayUI::SetCurrentWave(int32 NewWave)
{
	CurrentWave = NewWave;
	RefreshWaveText();
}

void UShooterGameplayUI::SetEnemiesRemaining(int32 NewRemaining)
{
	EnemiesRemaining = NewRemaining;
	RefreshEnemiesRemainingText();
}

void UShooterGameplayUI::SetScore(int32 NewScore)
{
	Score = NewScore;
	RefreshScoreText();
}

void UShooterGameplayUI::SetPlayerHealth(float NewCurrentHealth, float NewMaxHealth)
{
	CurrentHealth = NewCurrentHealth;
	MaxHealth = NewMaxHealth;
	RefreshHealthWidgets();
}

void UShooterGameplayUI::RefreshAllUI()
{
	RefreshWaveText();
	RefreshEnemiesRemainingText();
	RefreshScoreText();
	RefreshHealthWidgets();
}

void UShooterGameplayUI::RefreshWaveText()
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(LOCTEXT("WaveFormat", "Wave: {0}"), FText::AsNumber(CurrentWave)));
	}
}

void UShooterGameplayUI::RefreshEnemiesRemainingText()
{
	if (EnemiesRemainingText)
	{
		EnemiesRemainingText->SetText(FText::Format(LOCTEXT("EnemiesRemainingFormat", "Enemies Remaining: {0}"), FText::AsNumber(EnemiesRemaining)));
	}
}

void UShooterGameplayUI::RefreshScoreText()
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFormat", "Score: {0}"), FText::AsNumber(Score)));
	}
}

void UShooterGameplayUI::RefreshHealthWidgets()
{
	if (HealthText)
	{
		HealthText->SetText(FText::Format(
			LOCTEXT("HealthFormat", "Health: {0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(CurrentHealth)),
			FText::AsNumber(FMath::RoundToInt(MaxHealth))));
	}

	if (HealthBar)
	{
		const float HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
		HealthBar->SetPercent(HealthPercent);
	}
}

#undef LOCTEXT_NAMESPACE
