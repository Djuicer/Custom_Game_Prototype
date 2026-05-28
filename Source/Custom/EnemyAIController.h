// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOM_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	AEnemyAIController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION()
	void OnSensesUpdated(const TArray<AActor*>& UpdatedActors);
	
	UFUNCTION()
	void AttackPlayer();

private:
	void AssignPlayerTarget();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Damage")
	float AttackDamage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	float AttackRange = 150.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	float AttackCooldown = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight")
	float SightAge = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight")
	float SightRadius = 150000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Sight")
	float FieldOfView = 360.0f;
	
	UPROPERTY()
	UAISenseConfig_Sight* SightConfiguration;
	UPROPERTY(EditAnywhere)
	UBlackboardData* AIBlackboard;
	UPROPERTY(EditAnywhere)
	UBehaviorTree* BehaviourTree;
	UPROPERTY()
	UBlackboardComponent* BlackboardComponent;
	UPROPERTY()
	UNavigationSystemV1* NavigationSystem;
	UPROPERTY()
	APawn* TargetPlayer;

	float LastAttackTime = -FLT_MAX;
	
	
	
};
