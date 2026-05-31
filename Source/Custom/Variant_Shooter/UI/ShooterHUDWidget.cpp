// Copyright Epic Games, Inc. All Rights Reserved.

#include "Variant_Shooter/UI/ShooterHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateColor.h"

void UShooterHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildDefaultLayout();
	RefreshAllText();
}

void UShooterHUDWidget::SetCurrentWave(int32 NewWave)
{
	CurrentWave = FMath::Max(1, NewWave);
	RefreshAllText();
}

void UShooterHUDWidget::SetEnemiesRemaining(int32 NewRemaining)
{
	EnemiesRemaining = FMath::Max(0, NewRemaining);
	RefreshAllText();
}

void UShooterHUDWidget::SetScore(int32 NewScore)
{
	Score = FMath::Max(0, NewScore);
	RefreshAllText();
}

void UShooterHUDWidget::SetPlayerHealth(float NewCurrentHealth, float NewMaxHealth)
{
	MaxHealth = FMath::Max(0.0f, NewMaxHealth);
	CurrentHealth = MaxHealth > 0.0f ? FMath::Clamp(NewCurrentHealth, 0.0f, MaxHealth) : 0.0f;
	RefreshAllText();
}

void UShooterHUDWidget::BuildDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget || (HealthText && WaveText && EnemiesRemainingText && ScoreText))
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UVerticalBox* StatBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatBox"));
	UCanvasPanelSlot* StatBoxSlot = RootCanvas->AddChildToCanvas(StatBox);
	StatBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	StatBoxSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	StatBoxSlot->SetPosition(FVector2D(32.0f, 32.0f));
	StatBoxSlot->SetAutoSize(true);

	const FSlateColor TextColor(FLinearColor::White);

	HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
	HealthText->SetColorAndOpacity(TextColor);
	FSlateFontInfo HealthFont = HealthText->GetFont();
	HealthFont.Size = 28;
	HealthText->SetFont(HealthFont);
	UVerticalBoxSlot* HealthTextSlot = StatBox->AddChildToVerticalBox(HealthText);
	HealthTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));

	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.85f, 0.2f, 1.0f));
	UVerticalBoxSlot* HealthBarSlot = StatBox->AddChildToVerticalBox(HealthBar);
	HealthBarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	WaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaveText"));
	WaveText->SetColorAndOpacity(TextColor);
	FSlateFontInfo WaveFont = WaveText->GetFont();
	WaveFont.Size = 26;
	WaveText->SetFont(WaveFont);
	UVerticalBoxSlot* WaveTextSlot = StatBox->AddChildToVerticalBox(WaveText);
	WaveTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));

	EnemiesRemainingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EnemiesRemainingText"));
	EnemiesRemainingText->SetColorAndOpacity(TextColor);
	FSlateFontInfo EnemiesFont = EnemiesRemainingText->GetFont();
	EnemiesFont.Size = 26;
	EnemiesRemainingText->SetFont(EnemiesFont);
	UVerticalBoxSlot* EnemiesRemainingSlot = StatBox->AddChildToVerticalBox(EnemiesRemainingText);
	EnemiesRemainingSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));

	ScoreText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScoreText"));
	ScoreText->SetColorAndOpacity(TextColor);
	FSlateFontInfo ScoreFont = ScoreText->GetFont();
	ScoreFont.Size = 26;
	ScoreText->SetFont(ScoreFont);
	StatBox->AddChildToVerticalBox(ScoreText);
}

void UShooterHUDWidget::RefreshAllText()
{
	if (HealthText)
	{
		HealthText->SetText(FText::Format(NSLOCTEXT("ShooterHUD", "HealthFormat", "Health: {0} / {1}"), FText::AsNumber(FMath::RoundToInt(CurrentHealth)), FText::AsNumber(FMath::RoundToInt(MaxHealth))));
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f);
	}

	if (WaveText)
	{
		WaveText->SetText(FText::Format(NSLOCTEXT("ShooterHUD", "WaveFormat", "Wave: {0}"), FText::AsNumber(CurrentWave)));
	}

	if (EnemiesRemainingText)
	{
		EnemiesRemainingText->SetText(FText::Format(NSLOCTEXT("ShooterHUD", "EnemiesRemainingFormat", "Enemies Remaining: {0}"), FText::AsNumber(EnemiesRemaining)));
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::Format(NSLOCTEXT("ShooterHUD", "ScoreFormat", "Score: {0}"), FText::AsNumber(Score)));
	}
}
