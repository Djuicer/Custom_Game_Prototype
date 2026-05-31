// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterUI.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/SlateFontInfo.h"

#define LOCTEXT_NAMESPACE "ShooterUI"

void UShooterUI::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureGameplayHUDWidgets();
	RefreshGameplayHUDText();
}

void UShooterUI::SetDestroyedEnemyCount(int32 NewCount)
{
	DestroyedEnemyCount = FMath::Max(0, NewCount);
	RefreshGameplayHUDText();
}

void UShooterUI::SetRemainingEnemiesInWave(int32 NewCount)
{
	RemainingEnemiesInWave = FMath::Max(0, NewCount);
	RefreshGameplayHUDText();
}

void UShooterUI::SetCurrentWave(int32 NewWave)
{
	CurrentWave = FMath::Max(1, NewWave);
	RefreshGameplayHUDText();
}

void UShooterUI::EnsureGameplayHUDWidgets()
{
	if (ScoreText && EnemiesRemainingText && WaveText)
	{
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	if (!GameplayHUDContainer)
	{
		GameplayHUDContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GameplayHUDContainer"));
		AddGameplayHUDContainerToRoot();
	}

	if (!ScoreText)
	{
		ScoreText = CreateHUDTextBlock(TEXT("ScoreText"));
		GameplayHUDContainer->AddChildToVerticalBox(ScoreText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}

	if (!EnemiesRemainingText)
	{
		EnemiesRemainingText = CreateHUDTextBlock(TEXT("EnemiesRemainingText"));
		GameplayHUDContainer->AddChildToVerticalBox(EnemiesRemainingText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}

	if (!WaveText)
	{
		WaveText = CreateHUDTextBlock(TEXT("WaveText"));
		GameplayHUDContainer->AddChildToVerticalBox(WaveText);
	}
}

void UShooterUI::RefreshGameplayHUDText()
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFormat", "Score: {0}"), FText::AsNumber(DestroyedEnemyCount)));
	}

	if (EnemiesRemainingText)
	{
		EnemiesRemainingText->SetText(FText::Format(LOCTEXT("EnemiesRemainingFormat", "Enemies Remaining: {0}"), FText::AsNumber(RemainingEnemiesInWave)));
	}

	if (WaveText)
	{
		WaveText->SetText(FText::Format(LOCTEXT("WaveFormat", "Wave: {0}"), FText::AsNumber(CurrentWave)));
	}
}

UTextBlock* UShooterUI::CreateHUDTextBlock(FName WidgetName) const
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowColorAndOpacity(FLinearColor::Black);
	TextBlock->SetShadowOffset(FVector2D(1.5f, 1.5f));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = 24;
	TextBlock->SetFont(FontInfo);

	return TextBlock;
}

void UShooterUI::AddGameplayHUDContainerToRoot()
{
	if (!WidgetTree || !GameplayHUDContainer)
	{
		return;
	}

	UWidget* ExistingRoot = WidgetTree->RootWidget;
	if (!ExistingRoot)
	{
		UCanvasPanel* CanvasRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("GameplayHUDRoot"));
		WidgetTree->RootWidget = CanvasRoot;
		ExistingRoot = CanvasRoot;
	}

	UPanelWidget* RootPanel = Cast<UPanelWidget>(ExistingRoot);
	if (!RootPanel)
	{
		UCanvasPanel* CanvasRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("GameplayHUDRoot"));
		WidgetTree->RootWidget = CanvasRoot;
		RootPanel = CanvasRoot;
	}

	RootPanel->AddChild(GameplayHUDContainer);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GameplayHUDContainer->Slot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(24.0f, 24.0f));
	}
}

#undef LOCTEXT_NAMESPACE
