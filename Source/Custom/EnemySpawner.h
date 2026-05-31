#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaveChangedDelegate, int32, CurrentWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemiesRemainingChangedDelegate, int32, EnemiesRemaining);

class ACharacter;
class AActor;

UCLASS(Blueprintable)
class CUSTOM_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	/** Broadcast when a new wave starts. */
	UPROPERTY(BlueprintAssignable, Category = "Spawner|HUD")
	FWaveChangedDelegate OnWaveChanged;

	/** Broadcast when the number of enemies remaining in the active wave changes. */
	UPROPERTY(BlueprintAssignable, Category = "Spawner|HUD")
	FEnemiesRemainingChangedDelegate OnEnemiesRemainingChanged;

	UFUNCTION(BlueprintPure, Category = "Spawner|HUD")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Spawner|HUD")
	int32 GetEnemiesRemainingInWave() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Enemy")
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Timing", meta = (ClampMin = "0.01"))
	float SpawnInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Timing", meta = (ClampMin = "0.0"))
	float InitialSpawnDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Waves", meta = (ClampMin = "1"))
	int32 StartingEnemiesPerWave = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Waves", meta = (ClampMin = "0"))
	int32 EnemiesAddedPerWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Waves", meta = (ClampMin = "0.0"))
	float TimeBetweenWaves = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Waves", meta = (ClampMin = "1"))
	int32 CurrentWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Limits", meta = (ClampMin = "1"))
	int32 MaxAliveEnemies = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Limits", meta = (ClampMin = "1"))
	int32 EnemiesPerSpawn = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement", meta = (ClampMin = "0.0"))
	float EdgeSpawnDepth = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement")
	FVector TopRight = FVector(3540.0f, 3570.0f, 88.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement")
	FVector BottomRight = FVector(3480.0f, -3460.0f, 88.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement")
	FVector BottomLeft = FVector(-3550.0f, -3460.0f, 88.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement")
	FVector TopLeft = FVector(-3550.0f, 3570.0f, 88.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Placement", meta = (ClampMin = "0.0"))
	float NavProjectionExtent = 500.0f;

private:
	FTimerHandle SpawnTimerHandle;
	FTimerHandle NextWaveTimerHandle;
	TArray<TWeakObjectPtr<ACharacter>> AliveEnemies;
	int32 EnemiesSpawnedThisWave = 0;
	int32 EnemiesRequiredThisWave = 0;
	int32 EnemiesDefeatedThisWave = 0;

	void StartWave();
	void CheckWaveComplete();
	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleSpawnedEnemyDefeated();

	void TrySpawnWave();
	void CleanupDeadEnemies();
	bool TryGetSpawnLocation(FVector& OutSpawnLocation) const;
	void BroadcastWaveHUDState();
};
