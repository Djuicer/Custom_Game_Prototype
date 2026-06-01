#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class AShooterCharacter;
class UStaticMeshComponent;

UCLASS()
class CUSTOM_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	UFUNCTION(BlueprintCallable)
	void Ragdoll();

	UFUNCTION(BlueprintCallable)
	void StopRagdoll();

	UFUNCTION(BlueprintCallable)
	void DealDamage(float Damage);

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable, Category = "Enemy|Shield")
	void SetShieldRaised(bool bShouldRaiseShield);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Shield")
	void BreakShield();

	UFUNCTION(BlueprintPure, Category = "Enemy|Shield")
	bool IsShieldProtecting() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|Shield")
	bool IsShieldBroken() const { return bShieldBroken; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Shield")
	bool HasUsableShield() const { return !bShieldBroken && ShieldHealth > 0.0f; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Ragdoll")
	float RagdollTime = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Ragdoll")
	bool bIsRagdolling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Shield")
	UStaticMeshComponent* ShieldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield")
	float MaxShieldHealth = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield")
	float ShieldHealth = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Shield")
	bool bShieldActive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Shield")
	bool bShieldBroken = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Death")
	bool bHasReportedDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield|Attachment")
	FName ShieldAttachSocket = TEXT("hand_lSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield|Attachment")
	FVector ShieldRelativeLocation = FVector(0.0f, -20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield|Attachment")
	FRotator ShieldRelativeRotation = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Shield|Attachment")
	FVector ShieldRelativeScale = FVector(0.12f, 0.04f, 0.8f);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Shield")
	void OnShieldBroken();

	FTimerHandle RagdollTimerHandle;

private:
	void AttachShieldMesh();
	void ApplyShieldVisibility();
	void ReportDestroyedIfNeeded();
	AShooterCharacter* FindShooterCharacterForDeathCredit() const;
};
