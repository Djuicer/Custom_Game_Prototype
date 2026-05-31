// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterUI.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateColor.h"

void UShooterUI::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureGameplayStatsLayout();
	RefreshText();
}

void UShooterUI::UpdateGameplayStats(int32 InDestroyedEnemyCount, int32 InEnemiesRemaining, int32 InWaveNumber)
{
	DestroyedEnemyCount = FMath::Max(0, InDestroyedEnemyCount);
	EnemiesRemaining = FMath::Max(0, InEnemiesRemaining);
	WaveNumber = FMath::Max(1, InWaveNumber);

	RefreshText();
}

void UShooterUI::UpdateDestroyedEnemyCount(int32 InDestroyedEnemyCount)
{
	DestroyedEnemyCount = FMath::Max(0, InDestroyedEnemyCount);
	RefreshText();
}

void UShooterUI::UpdateEnemiesRemaining(int32 InEnemiesRemaining)
{
	EnemiesRemaining = FMath::Max(0, InEnemiesRemaining);
	RefreshText();
}

void UShooterUI::UpdateWaveNumber(int32 InWaveNumber)
{
	WaveNumber = FMath::Max(1, InWaveNumber);
	RefreshText();
}

void UShooterUI::EnsureGameplayStatsLayout()
{
	if (!WidgetTree)
	{
		return;
	}

	ScoreText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ScoreText")));
	EnemiesRemainingText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("EnemiesRemainingText")));
	WaveText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("WaveText")));

	UVerticalBox* StatsBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("ShooterStatsBox")));
	if (!StatsBox)
	{
		UWidget* ExistingRoot = WidgetTree->RootWidget;
		UOverlay* OverlayRoot = Cast<UOverlay>(ExistingRoot);

		if (!OverlayRoot)
		{
			OverlayRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ShooterUIRootOverlay"));
			WidgetTree->RootWidget = OverlayRoot;

			if (ExistingRoot)
			{
				UOverlaySlot* ExistingRootSlot = OverlayRoot->AddChildToOverlay(ExistingRoot);
				if (ExistingRootSlot)
				{
					ExistingRootSlot->SetHorizontalAlignment(HAlign_Fill);
					ExistingRootSlot->SetVerticalAlignment(VAlign_Fill);
				}
			}
		}

		StatsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShooterStatsBox"));
		UOverlaySlot* StatsSlot = OverlayRoot->AddChildToOverlay(StatsBox);
		if (StatsSlot)
		{
			StatsSlot->SetHorizontalAlignment(HAlign_Left);
			StatsSlot->SetVerticalAlignment(VAlign_Top);
			StatsSlot->SetPadding(FMargin(24.0f, 24.0f, 0.0f, 0.0f));
		}
	}

	if (!ScoreText)
	{
		ScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScoreText"));
		if (UVerticalBoxSlot* Slot = StatsBox->AddChildToVerticalBox(ScoreText))
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
	}

	if (!EnemiesRemainingText)
	{
		EnemiesRemainingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EnemiesRemainingText"));
		if (UVerticalBoxSlot* Slot = StatsBox->AddChildToVerticalBox(EnemiesRemainingText))
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
	}

	if (!WaveText)
	{
		WaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaveText"));
		StatsBox->AddChildToVerticalBox(WaveText);
	}

	ConfigureStatText(ScoreText);
	ConfigureStatText(EnemiesRemainingText);
	ConfigureStatText(WaveText);
}

void UShooterUI::ConfigureStatText(UTextBlock* TextBlock) const
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = 28;
	TextBlock->SetFont(FontInfo);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
}

void UShooterUI::RefreshText()
{
	if (!ScoreText || !EnemiesRemainingText || !WaveText)
	{
		EnsureGameplayStatsLayout();
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(NSLOCTEXT("ShooterUI", "ScoreFormat", "Score: {0}"), DestroyedEnemyCount));
	}

	if (EnemiesRemainingText)
	{
		EnemiesRemainingText->SetText(FText::Format(NSLOCTEXT("ShooterUI", "EnemiesRemainingFormat", "Enemies Remaining: {0}"), EnemiesRemaining));
	}

	if (WaveText)
	{
		WaveText->SetText(FText::Format(NSLOCTEXT("ShooterUI", "WaveFormat", "Wave: {0}"), WaveNumber));
	}
}
