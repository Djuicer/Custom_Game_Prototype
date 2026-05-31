// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGameplayUI.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Variant_Shooter/ShooterCharacter.h"

#define LOCTEXT_NAMESPACE "ShooterGameplayUI"

void UShooterGameplayUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ShooterCharacter.IsValid())
	{
		if (APawn* OwningPawn = GetOwningPlayerPawn())
		{
			InitializeWithPlayer(Cast<AShooterCharacter>(OwningPawn));
		}
	}

	RefreshUI();
}

void UShooterGameplayUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshUI();
}

void UShooterGameplayUI::InitializeWithPlayer(AShooterCharacter* InPlayer)
{
	ShooterCharacter = InPlayer;
	RefreshUI();
}

void UShooterGameplayUI::RefreshUI()
{
	AShooterCharacter* Player = ShooterCharacter.Get();
	if (!IsValid(Player))
	{
		RefreshAllUI();
		return;
	}

	CurrentHealth = Player->GetCurrentHealth();
	MaxHealth = Player->GetMaxHealth();
	Score = Player->GetDestroyedEnemyCount();
	UltimateCharge = Player->GetUltimateEnemyCharge();
	UltimateRequired = Player->GetEnemiesRequiredForUltimate();
	UltimatePercent = Player->GetUltimateCharge();
	bUltimateReady = Player->IsUltimateReady();
	ExplosiveCooldownRemaining = Player->GetExplosiveCylinderCooldownRemaining();
	ExplosiveCooldownPercent = Player->GetExplosiveCylinderCooldownPercent();
	GrenadeLauncherUpgradeLevel = Player->GetGrenadeLauncherUpgradeLevel();
	ExplosiveCylinderCount = Player->GetExplosiveCylinderCount();
	CurrentWave = Player->GetCurrentWave();
	EnemiesRemaining = Player->GetEnemiesRemainingInWave();
	EnemiesAlive = Player->GetEnemiesAliveInWave();

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
	RefreshEnemiesAliveText();
	RefreshScoreText();
	RefreshHealthWidgets();
	RefreshUltimateWidgets();
	RefreshExplosiveCooldownWidgets();
	RefreshGrenadeLauncherWidgets();
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

void UShooterGameplayUI::RefreshEnemiesAliveText()
{
	if (EnemiesAliveText)
	{
		EnemiesAliveText->SetText(FText::Format(LOCTEXT("EnemiesAliveFormat", "Enemies Alive: {0}"), FText::AsNumber(EnemiesAlive)));
	}
}

void UShooterGameplayUI::RefreshScoreText()
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFormat", "Destroyed: {0}"), FText::AsNumber(Score)));
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

void UShooterGameplayUI::RefreshUltimateWidgets()
{
	if (UltimateText)
	{
		UltimateText->SetText(FText::Format(
			LOCTEXT("UltimateFormat", "Ultimate: {0} / {1}"),
			FText::AsNumber(UltimateCharge),
			FText::AsNumber(UltimateRequired)));
	}

	if (UltimateBar)
	{
		UltimateBar->SetPercent(UltimatePercent);
	}

	if (UltimateReadyText)
	{
		UltimateReadyText->SetText(bUltimateReady ? LOCTEXT("UltimateReady", "Ultimate Ready") : LOCTEXT("UltimateNotReady", "Ultimate Charging"));
	}
}

void UShooterGameplayUI::RefreshExplosiveCooldownWidgets()
{
	if (ExplosiveCooldownText)
	{
		const FText CooldownText = ExplosiveCooldownRemaining <= 0.0f
			? LOCTEXT("ExplosiveReady", "Cylinder: Ready")
			: FText::Format(LOCTEXT("ExplosiveCooldownFormat", "Cylinder: {0}s"), FText::AsNumber(ExplosiveCooldownRemaining));
		ExplosiveCooldownText->SetText(CooldownText);
	}

	if (ExplosiveCooldownBar)
	{
		ExplosiveCooldownBar->SetPercent(ExplosiveCooldownPercent);
	}
}

void UShooterGameplayUI::RefreshGrenadeLauncherWidgets()
{
	if (GrenadeLauncherLevelText)
	{
		GrenadeLauncherLevelText->SetText(FText::Format(
			LOCTEXT("GrenadeLauncherLevelFormat", "Grenade Lv {0} ({1} cylinders)"),
			FText::AsNumber(GrenadeLauncherUpgradeLevel),
			FText::AsNumber(ExplosiveCylinderCount)));
	}
}

#undef LOCTEXT_NAMESPACE
