// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class CUSTOM_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	
	UFUNCTION(BlueprintCallable)
	void Ragdoll();
	
	UFUNCTION(BlueprintCallable)
	void StopRagdoll();
	
	UFUNCTION(BlueprintCallable)
	void DealDamage(float Damage);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Health")
	float CurrentHealth = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Ragdoll")
	float RagdollTime = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Ragdoll")
	bool bIsRagdolling = false;
	
	
	
	FTimerHandle RagdollTimerHandle;
	FTimerHandle ShootingTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
