#include "EnemySpawner.h"

#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnInterval <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemySpawner '%s' has invalid SpawnInterval. Spawning disabled."), *GetName());
		return;
	}

	if (CurrentWave < 1)
	{
		CurrentWave = 1;
	}

	StartWave();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::TrySpawnWave, SpawnInterval, true, InitialSpawnDelay);
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
}

int32 AEnemySpawner::GetAliveEnemyCount() const
{
	int32 AliveCount = 0;
	for (const TWeakObjectPtr<ACharacter>& Enemy : AliveEnemies)
	{
		if (Enemy.IsValid())
		{
			++AliveCount;
		}
	}

	return AliveCount;
}

int32 AEnemySpawner::GetEnemiesRemainingInWave() const
{
	const int32 NotYetSpawned = FMath::Max(0, EnemiesRequiredThisWave - EnemiesSpawnedThisWave);
	return GetAliveEnemyCount() + NotYetSpawned;
}

void AEnemySpawner::StartWave()
{
	EnemiesSpawnedThisWave = 0;
	EnemiesRequiredThisWave = StartingEnemiesPerWave + ((CurrentWave - 1) * EnemiesAddedPerWave);
	EnemiesRequiredThisWave = FMath::Max(1, EnemiesRequiredThisWave);
}

void AEnemySpawner::CheckWaveComplete()
{
	if (EnemiesSpawnedThisWave < EnemiesRequiredThisWave)
	{
		return;
	}

	CleanupDeadEnemies();
	if (AliveEnemies.Num() > 0 || GetWorldTimerManager().IsTimerActive(NextWaveTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(NextWaveTimerHandle, [this]()
	{
		CurrentWave++;
		StartWave();
	}, TimeBetweenWaves, false);
}

void AEnemySpawner::HandleSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	CleanupDeadEnemies();
	CheckWaveComplete();
}

void AEnemySpawner::CleanupDeadEnemies()
{
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<ACharacter>& Enemy)
	{
		return !Enemy.IsValid();
	});
}

void AEnemySpawner::TrySpawnWave()
{
	if (!EnemyClass)
	{
		return;
	}

	CleanupDeadEnemies();
	CheckWaveComplete();

	if (EnemiesSpawnedThisWave >= EnemiesRequiredThisWave)
	{
		return;
	}

	const int32 RemainingCapacity = MaxAliveEnemies - AliveEnemies.Num();
	if (RemainingCapacity <= 0)
	{
		return;
	}

	const int32 RemainingThisWave = EnemiesRequiredThisWave - EnemiesSpawnedThisWave;
	const int32 SpawnCountThisTick = FMath::Min3(EnemiesPerSpawn, RemainingCapacity, RemainingThisWave);
	for (int32 Index = 0; Index < SpawnCountThisTick; ++Index)
	{
		FVector SpawnLocation;
		if (!TryGetSpawnLocation(SpawnLocation))
		{
			continue;
		}

		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		if (ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(EnemyClass, SpawnTransform, SpawnParams))
		{
			SpawnedEnemy->SpawnDefaultController();
			SpawnedEnemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::HandleSpawnedEnemyDestroyed);
			AliveEnemies.Add(SpawnedEnemy);
			EnemiesSpawnedThisWave++;
		}
	}

	CheckWaveComplete();
}

bool AEnemySpawner::TryGetSpawnLocation(FVector& OutSpawnLocation) const
{
	const int32 EdgeIndex = FMath::RandRange(0, 3);

	FVector EdgeStart;
	FVector EdgeEnd;
	FVector InwardDirection;

	switch (EdgeIndex)
	{
	case 0: // Top
		EdgeStart = TopLeft;
		EdgeEnd = TopRight;
		InwardDirection = FVector(0.0f, -1.0f, 0.0f);
		break;
	case 1: // Right
		EdgeStart = TopRight;
		EdgeEnd = BottomRight;
		InwardDirection = FVector(-1.0f, 0.0f, 0.0f);
		break;
	case 2: // Bottom
		EdgeStart = BottomRight;
		EdgeEnd = BottomLeft;
		InwardDirection = FVector(0.0f, 1.0f, 0.0f);
		break;
	default: // Left
		EdgeStart = BottomLeft;
		EdgeEnd = TopLeft;
		InwardDirection = FVector(1.0f, 0.0f, 0.0f);
		break;
	}

	const float EdgeAlpha = FMath::FRandRange(0.0f, 1.0f);
	const FVector PointOnEdge = FMath::Lerp(EdgeStart, EdgeEnd, EdgeAlpha);
	const float DepthOffset = FMath::FRandRange(0.0f, EdgeSpawnDepth);
	FVector CandidateLocation = PointOnEdge + (InwardDirection * DepthOffset);
	CandidateLocation.Z = 88.0f;

	if (UWorld* World = GetWorld())
	{
		if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			FNavLocation NavLocation;
			const FVector Extent(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent);

			if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, Extent))
			{
				OutSpawnLocation = NavLocation.Location;
				return true;
			}
		}
	}

	OutSpawnLocation = CandidateLocation;
	return true;
}
