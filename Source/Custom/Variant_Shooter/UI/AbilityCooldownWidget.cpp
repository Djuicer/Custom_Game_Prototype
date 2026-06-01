// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilityCooldownWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAbilityCooldownWidget::UAbilityCooldownWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAbilityCooldownWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyReadyVisuals();
}

void UAbilityCooldownWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	Super::NativeDestruct();
}

bool UAbilityCooldownWidget::StartCooldown(float NewCooldownDuration)
{
	if (bIsCooldownActive || NewCooldownDuration <= 0.0f)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bIsCooldownActive = true;
	CooldownDuration = NewCooldownDuration;
	CooldownRemaining = NewCooldownDuration;
	CooldownEndTime = World->GetTimeSeconds() + NewCooldownDuration;

	ApplyCooldownVisuals();
	OnCooldownStarted(CooldownDuration);
	OnCooldownTick(CooldownRemaining, CooldownDuration);

	World->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&UAbilityCooldownWidget::HandleCooldownTimerTick,
		FMath::Max(0.01f, CooldownUpdateInterval),
		true
	);

	return true;
}

void UAbilityCooldownWidget::ResetCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	bIsCooldownActive = false;
	CooldownDuration = 0.0f;
	CooldownRemaining = 0.0f;
	CooldownEndTime = 0.0f;
	ApplyReadyVisuals();
}

void UAbilityCooldownWidget::HandleCooldownTimerTick()
{
	if (!bIsCooldownActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		FinishCooldown();
		return;
	}

	CooldownRemaining = FMath::Max(0.0f, CooldownEndTime - World->GetTimeSeconds());

	if (CooldownRemaining <= 0.0f)
	{
		FinishCooldown();
		return;
	}

	ApplyCooldownVisuals();
	OnCooldownTick(CooldownRemaining, CooldownDuration);

}

void UAbilityCooldownWidget::FinishCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	bIsCooldownActive = false;
	CooldownDuration = 0.0f;
	CooldownRemaining = 0.0f;
	CooldownEndTime = 0.0f;
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

void UAbilityCooldownWidget::ApplyCooldownVisuals()
{
	if (AbilityIcon)
	{
		AbilityIcon->SetColorAndOpacity(CooldownIconTint);
	}

	UpdateCooldownText();
}

void UAbilityCooldownWidget::UpdateCooldownText() const
{
	if (!CooldownText)
	{
		return;
	}

	const int32 DisplaySeconds = FMath::CeilToInt(CooldownRemaining);
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
