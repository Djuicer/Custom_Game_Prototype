#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class APawn;
class UAISenseConfig_Sight;
class UBehaviorTree;
class UBlackboardComponent;
class UBlackboardData;
class UNavigationSystemV1;

UCLASS()
class CUSTOM_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnSensesUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void AttackPlayer();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Damage")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
	float AttackRange = 200.0f;

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

	FVector LastValidPlayerGroundPosition = FVector::ZeroVector;
	bool bHasLastValidPlayerGroundPosition = false;
	float LastAttackTime = -FLT_MAX;

private:
	void InitializeBehavior();
	void AssignPlayerTarget();
	void UpdatePlayerChasePosition();
	void UpdateShieldState();
	void SetShieldBlackboardState(bool bIsHoldingShield, bool bIsAttacking) const;
};
