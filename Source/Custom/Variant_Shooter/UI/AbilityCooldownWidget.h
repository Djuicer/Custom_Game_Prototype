// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityCooldownWidget.generated.h"

class AStickyCylinderExplosive;
class UImage;
class UTextBlock;

/**
 * Blueprint-friendly base widget for a square ability icon bound to StickyCylinderExplosive cooldown state.
 *
 * Create a Widget Blueprint derived from this class, design the layout in UMG,
 * then optionally name an Image widget "AbilityIcon" and a TextBlock widget
 * "CooldownText" to bind them automatically. Gameplay cooldowns are owned by
 * the ability; this widget only represents ability state.
 */
UCLASS()
class CUSTOM_API UAbilityCooldownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAbilityCooldownWidget(const FObjectInitializer& ObjectInitializer);

	/** Binds this widget to an ability instance and immediately refreshes visuals from its state. */
	UFUNCTION(BlueprintCallable, Category="Ability Cooldown")
	void BindToAbility(AStickyCylinderExplosive* Ability);

	/** Clears the current ability binding and returns visuals to ready. */
	UFUNCTION(BlueprintCallable, Category="Ability Cooldown")
	void UnbindFromAbility();

	/** Returns the ability instance driving this widget. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	AStickyCylinderExplosive* GetBoundAbility() const { return BoundAbility; }

	/** Returns true while the bound ability is cooling down. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	bool IsCooldownActive() const;

	/** Remaining cooldown seconds from the bound ability, or 0 when ready. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	float GetCooldownRemaining() const;

	/** Configured cooldown duration from the bound ability, or 0 if unbound. */
	UFUNCTION(BlueprintPure, Category="Ability Cooldown")
	float GetCooldownDuration() const;

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

	/** Called when the bound ability starts cooldown so Blueprint can customize animations or styling. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownStarted(float Duration);

	/** Called every bound ability cooldown update so Blueprint can drive progress materials, animations, etc. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownTick(float Remaining, float Duration, float Percent);

	/** Called when the bound ability cooldown reaches zero. */
	UFUNCTION(BlueprintImplementableEvent, Category="Ability Cooldown")
	void OnCooldownFinished();

private:
	UFUNCTION()
	void HandleAbilityCooldownStarted(float Duration);

	UFUNCTION()
	void HandleAbilityCooldownUpdated(float Remaining, float Duration, float Percent);

	UFUNCTION()
	void HandleAbilityCooldownFinished();

	void ApplyReadyVisuals();
	void ApplyCooldownVisuals(float Remaining, float Duration, float Percent);
	void RefreshFromAbility();
	void UpdateCooldownText(float Remaining) const;
	void ClearCooldownText() const;

	UPROPERTY(Transient)
	TObjectPtr<AStickyCylinderExplosive> BoundAbility;
};
