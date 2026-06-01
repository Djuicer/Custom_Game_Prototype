// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityCooldownWidget.h"

#include "Variant_Shooter/Weapons/StickyCylinderExplosive.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UAbilityCooldownWidget::UAbilityCooldownWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAbilityCooldownWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshFromAbility();
}

void UAbilityCooldownWidget::NativeDestruct()
{
	UnbindFromAbility();
	Super::NativeDestruct();
}

void UAbilityCooldownWidget::BindToAbility(AStickyCylinderExplosive* Ability)
{
	if (BoundAbility == Ability)
	{
		RefreshFromAbility();
		return;
	}

	UnbindFromAbility();
	BoundAbility = Ability;

	if (BoundAbility)
	{
		BoundAbility->OnCooldownStarted.AddDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownStarted);
		BoundAbility->OnCooldownUpdated.AddDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownUpdated);
		BoundAbility->OnCooldownFinished.AddDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownFinished);
	}

	RefreshFromAbility();
}

void UAbilityCooldownWidget::UnbindFromAbility()
{
	if (BoundAbility)
	{
		BoundAbility->OnCooldownStarted.RemoveDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownStarted);
		BoundAbility->OnCooldownUpdated.RemoveDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownUpdated);
		BoundAbility->OnCooldownFinished.RemoveDynamic(this, &UAbilityCooldownWidget::HandleAbilityCooldownFinished);
		BoundAbility = nullptr;
	}

	ApplyReadyVisuals();
}

bool UAbilityCooldownWidget::IsCooldownActive() const
{
	return BoundAbility && BoundAbility->IsOnCooldown();
}

float UAbilityCooldownWidget::GetCooldownRemaining() const
{
	return BoundAbility ? BoundAbility->GetCooldownRemaining() : 0.0f;
}

float UAbilityCooldownWidget::GetCooldownDuration() const
{
	return BoundAbility ? BoundAbility->GetCooldownDuration() : 0.0f;
}

void UAbilityCooldownWidget::UpdateUltimateCharge(float ChargePercent, bool bIsReady)
{
	const float ClampedPercent = FMath::Clamp(ChargePercent, 0.0f, 1.0f);

	if (UltimateChargeBar)
	{
		UltimateChargeBar->SetPercent(ClampedPercent);
	}

	if (UltimateChargeText)
	{
		const int32 DisplayPercent = FMath::RoundToInt(ClampedPercent * 100.0f);
		UltimateChargeText->SetText(FText::Format(NSLOCTEXT("AbilityCooldownWidget", "UltimateChargePercent", "{0}%"), FText::AsNumber(DisplayPercent)));
		UltimateChargeText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (UltimateReadyText)
	{
		UltimateReadyText->SetText(NSLOCTEXT("AbilityCooldownWidget", "UltimateReady", "READY"));
		UltimateReadyText->SetVisibility(bIsReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (UltimateIcon)
	{
		UltimateIcon->SetColorAndOpacity(bIsReady ? UltimateReadyIconTint : UltimateChargingIconTint);
	}

	OnUltimateChargeUpdated(ClampedPercent, bIsReady);
}

void UAbilityCooldownWidget::UpdateScore(int32 NewScore)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(NSLOCTEXT("AbilityCooldownWidget", "ScoreFormat", "Score: {0}"), FText::AsNumber(NewScore)));
		ScoreText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UAbilityCooldownWidget::UpdateRemainingEnemies(int32 NewRemainingEnemies)
{
	if (RemainingEnemiesText)
	{
		RemainingEnemiesText->SetText(FText::Format(NSLOCTEXT("AbilityCooldownWidget", "RemainingEnemiesFormat", "Enemies Left: {0}"), FText::AsNumber(NewRemainingEnemies)));
		RemainingEnemiesText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UAbilityCooldownWidget::UpdateWaveNumber(int32 NewWaveNumber)
{
	if (WaveText)
	{
		WaveText->SetText(FText::Format(NSLOCTEXT("AbilityCooldownWidget", "WaveFormat", "Wave: {0}"), FText::AsNumber(NewWaveNumber)));
		WaveText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UAbilityCooldownWidget::UpdateWaveHUD(int32 NewScore, int32 NewRemainingEnemies, int32 NewWaveNumber)
{
	UpdateScore(NewScore);
	UpdateRemainingEnemies(NewRemainingEnemies);
	UpdateWaveNumber(NewWaveNumber);
}

void UAbilityCooldownWidget::HandleAbilityCooldownStarted(float Duration)
{
	const float Remaining = BoundAbility ? BoundAbility->GetCooldownRemaining() : Duration;
	const float Percent = BoundAbility ? BoundAbility->GetCooldownPercent() : 1.0f;
	ApplyCooldownVisuals(Remaining, Duration, Percent);
	OnCooldownStarted(Duration);
}

void UAbilityCooldownWidget::HandleAbilityCooldownUpdated(float Remaining, float Duration, float Percent)
{
	ApplyCooldownVisuals(Remaining, Duration, Percent);
	OnCooldownTick(Remaining, Duration, Percent);
}

void UAbilityCooldownWidget::HandleAbilityCooldownFinished()
{
	ApplyReadyVisuals();
	OnCooldownFinished();
}

void UAbilityCooldownWidget::ApplyReadyVisuals()
{
	if (AbilityIcon)
	{
		AbilityIcon->SetColorAndOpacity(ReadyIconTint);
	}

	ClearCooldownText();
}

void UAbilityCooldownWidget::ApplyCooldownVisuals(float Remaining, float, float)
{
	if (AbilityIcon)
	{
		AbilityIcon->SetColorAndOpacity(CooldownIconTint);
	}

	UpdateCooldownText(Remaining);
}

void UAbilityCooldownWidget::RefreshFromAbility()
{
	if (BoundAbility && BoundAbility->IsOnCooldown())
	{
		ApplyCooldownVisuals(
			BoundAbility->GetCooldownRemaining(),
			BoundAbility->GetCooldownDuration(),
			BoundAbility->GetCooldownPercent()
		);
		return;
	}

	ApplyReadyVisuals();
}

void UAbilityCooldownWidget::UpdateCooldownText(float Remaining) const
{
	if (!CooldownText)
	{
		return;
	}

	const int32 DisplaySeconds = FMath::CeilToInt(Remaining);
	CooldownText->SetText(FText::AsNumber(DisplaySeconds));
	CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UAbilityCooldownWidget::ClearCooldownText() const
{
	if (CooldownText)
	{
		CooldownText->SetText(FText::GetEmpty());
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}
