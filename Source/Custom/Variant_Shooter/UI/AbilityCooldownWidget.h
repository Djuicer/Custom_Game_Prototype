// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityCooldownWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * Blueprint-friendly base widget for a square ability icon with cooldown logic.
 *
 * Create a Widget Blueprint derived from this class, design the layout in UMG,
 * then optionally name an Image widget "AbilityIcon" and a TextBlock widget
 * "CooldownText" to bind them automatically.
 */
UCLASS()
class CUSTOM_API UAbilityCooldownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAbilityCooldownWidget(const FObjectInitializer& ObjectInitializer);

	/** Starts the cooldown unless it is already active. Returns true if a new cooldown began. */
	UFUNCTION(BlueprintCallable, Category="Ability Cooldown")
	bool StartCooldown(float CooldownDuration);

	/** Immediately cancels any active cooldown and returns the widget to the ready state. */
	UFUNCTION(BlueprintCallable, Category="Ability Cooldown")
	void ResetCooldown();

	/** Returns true while the ability is cooling down. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	bool IsCooldownActive() const { return bIsCooldownActive; }

	/** Remaining cooldown seconds, or 0 when ready. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	float GetCooldownRemaining() const { return CooldownRemaining; }

	/** Original cooldown duration for the current cooldown, or 0 when ready. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	float GetCooldownDuration() const { return CooldownDuration; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Optional UMG binding. In the Widget Blueprint, name your square icon Image "AbilityIcon" to bind it. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Ability Cooldown|Bindings")
	TObjectPtr<UImage> AbilityIcon;

	/** Optional UMG binding. In the Widget Blueprint, name your countdown TextBlock "CooldownText" to bind it. */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Ability Cooldown|Bindings")
	TObjectPtr<UTextBlock> CooldownText;

	/** Icon tint when the ability is ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Cooldown|Style")
	FLinearColor ReadyIconTint = FLinearColor::White;

	/** Icon tint while the ability is cooling down. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Cooldown|Style")
	FLinearColor CooldownIconTint = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);

	/** How often the countdown text and Blueprint tick event are updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Cooldown|Timing", meta=(ClampMin="0.01", Units="s"))
	float CooldownUpdateInterval = 0.05f;

	/** Called when a cooldown starts so Blueprint can customize animations or styling. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownStarted(float Duration);

	/** Called every cooldown update so Blueprint can drive progress materials, animations, etc. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownTick(float Remaining, float Duration);

	/** Called when the cooldown reaches zero. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownFinished();

private:
	void HandleCooldownTimerTick();
	void FinishCooldown();
	void ApplyReadyVisuals();
	void ApplyCooldownVisuals();
	void UpdateCooldownText() const;
	void ClearCooldownText() const;

	UPROPERTY(Transient)
	bool bIsCooldownActive = false;

	UPROPERTY(Transient)
	float CooldownDuration = 0.0f;

	UPROPERTY(Transient)
	float CooldownRemaining = 0.0f;

	UPROPERTY(Transient)
	float CooldownEndTime = 0.0f;

	FTimerHandle CooldownTimerHandle;
};
