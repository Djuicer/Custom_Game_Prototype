// Fill out your copyright notice in the Description page of Project Settings.
#include "EnemyAIController.h"

#include "Enemy.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

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


void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitializeBehavior();
	AssignPlayerTarget();
}

void AEnemyAIController::InitializeBehavior()
{
	if (!AIBlackboard || !BehaviourTree)
	{
		return;
	}

	UseBlackboard(AIBlackboard, BlackboardComponent);
	RunBehaviorTree(BehaviourTree);

	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), true);
		UpdateShieldState();
	}
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	NavigationSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());

	InitializeBehavior();

	if (GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(
			this,
			&AEnemyAIController::OnSensesUpdated
		);
	}

	AssignPlayerTarget();
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!TargetPlayer)
	{
		AssignPlayerTarget();
	}

	if (TargetPlayer && BlackboardComponent)
	{
		BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), true);
		BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), TargetPlayer);
		UpdatePlayerChasePosition();
		UpdateShieldState();
	}
	else
	{
		UpdateShieldState();
	}
}


void AEnemyAIController::OnSensesUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!BlackboardComponent)
	{
		return;
	}

	AssignPlayerTarget();

	if (TargetPlayer)
	{
		BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), true);
		BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), TargetPlayer);
		UpdatePlayerChasePosition();
		UpdateShieldState();
	}

}

void AEnemyAIController::AttackPlayer()
{
	if (!TargetPlayer || !GetPawn())
	{
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
	const bool bInAttackRange = DistanceToPlayer <= AttackRange;
	UpdateShieldState();
	if (!bInAttackRange)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	if (AEnemy* EnemyPawn = Cast<AEnemy>(GetPawn()))
	{
		EnemyPawn->SetShieldRaised(false);
	}
	SetShieldBlackboardState(false, true);

	StopMovement();
	FDamageEvent DamageEvent;
	TargetPlayer->TakeDamage(AttackDamage, DamageEvent, this, GetPawn());
	LastAttackTime = CurrentTime;
}

void AEnemyAIController::AssignPlayerTarget()
{
	if (!GetWorld())
	{
		return;
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		TargetPlayer = PlayerPawn;

		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsBool(TEXT("ChasePlayer"), true);
			BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), TargetPlayer);
			UpdatePlayerChasePosition();
			UpdateShieldState();
		}
	}
}

void AEnemyAIController::UpdateShieldState()
{
	AEnemy* EnemyPawn = Cast<AEnemy>(GetPawn());
	if (!EnemyPawn)
	{
		SetShieldBlackboardState(false, false);
		return;
	}

	const bool bInAttackRange = TargetPlayer && FVector::Dist(EnemyPawn->GetActorLocation(), TargetPlayer->GetActorLocation()) <= AttackRange;
	EnemyPawn->SetShieldRaised(!bInAttackRange);
	SetShieldBlackboardState(EnemyPawn->IsShieldProtecting(), bInAttackRange);
}

void AEnemyAIController::SetShieldBlackboardState(bool bIsHoldingShield, bool bIsAttacking) const
{
	if (!BlackboardComponent)
	{
		return;
	}

	BlackboardComponent->SetValueAsBool(TEXT("HoldsShield"), bIsHoldingShield);
	BlackboardComponent->SetValueAsBool(TEXT("AttackPlayer"), bIsAttacking);
}

void AEnemyAIController::UpdatePlayerChasePosition()
{
	if (!TargetPlayer || !BlackboardComponent)
	{
		return;
	}

	const FVector PlayerLocation = TargetPlayer->GetActorLocation();

	FNavLocation ProjectedLocation;
	const bool bProjected = NavigationSystem && NavigationSystem->ProjectPointToNavigation(
		PlayerLocation,
		ProjectedLocation,
		FVector(500.0f, 500.0f, 2000.0f)
	);

	if (bProjected)
	{
		LastValidPlayerGroundPosition = ProjectedLocation.Location;
		bHasLastValidPlayerGroundPosition = true;
	}

	if (bHasLastValidPlayerGroundPosition)
	{
		BlackboardComponent->SetValueAsVector(TEXT("PlayerPosition"), LastValidPlayerGroundPosition);
	}
	else
	{
		BlackboardComponent->SetValueAsVector(TEXT("PlayerPosition"), PlayerLocation);
	}
}
