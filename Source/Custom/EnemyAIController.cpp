// Fill out your copyright notice in the Description page of Project Settings.

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyAIController.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	SightConfiguration = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	SetPerceptionComponent(
		*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"))
	);

	SightConfiguration->SightRadius = SightRadius;
	SightConfiguration->PeripheralVisionAngleDegrees = FieldOfView;
	SightConfiguration->SetMaxAge(SightAge);

	SightConfiguration->DetectionByAffiliation.bDetectEnemies = true;
	SightConfiguration->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfiguration->DetectionByAffiliation.bDetectNeutrals = true;

	GetPerceptionComponent()->SetDominantSense(
		*SightConfiguration->GetSenseImplementation()
	);

	GetPerceptionComponent()->ConfigureSense(*SightConfiguration);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	NavigationSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	
	UseBlackboard(AIBlackboard, BlackboardComponent);
	RunBehaviorTree(BehaviourTree);
	
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), false);
		BlackboardComponent->SetValueAsBool(TEXT("HoldsShield"), false);
		BlackboardComponent->SetValueAsBool(TEXT("AttackPlayer"), false);
	}
	
	if (GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(
			this,
			&AEnemyAIController::OnSensesUpdated
		);
	}
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (TargetPlayer && BlackboardComponent)
	{
		BlackboardComponent->SetValueAsVector(
			TEXT("PlayerPosition"), 
			TargetPlayer->GetActorLocation()
		);
		
	}
}


void AEnemyAIController::OnSensesUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!BlackboardComponent)
	{
		return;
	}

	TargetPlayer = nullptr;
	BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), false);

	for (AActor* Actor : UpdatedActors)
	{
		if (APawn* SensedPawn = Cast<APawn>(Actor))
		{
			if (SensedPawn->IsPlayerControlled())
			{
				TargetPlayer = SensedPawn;

				BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), true);
				BlackboardComponent->SetValueAsVector(
					TEXT("PlayerPosition"),
					TargetPlayer->GetActorLocation()
				);
			}
		}
	}
	
}

void AEnemyAIController::AttackPlayer()
{
	if (!TargetPlayer || !GetPawn())
	{
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
	if (DistanceToPlayer > AttackRange)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	StopMovement();
	UGameplayStatics::ApplyDamage(TargetPlayer, AttackDamage, this, GetPawn(), UDamageType::StaticClass());
	LastAttackTime = CurrentTime;
}
