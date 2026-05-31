#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ACharacter;
class AActor;

UCLASS(Blueprintable)
class CUSTOM_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

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

public:
	/** Current wave number exposed to gameplay UI. */
	UFUNCTION(BlueprintPure, Category = "Spawner|Waves")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** Total enemies configured for the active wave. */
	UFUNCTION(BlueprintPure, Category = "Spawner|Waves")
	int32 GetEnemiesRequiredThisWave() const { return EnemiesRequiredThisWave; }

	/** Number of enemies spawned so far during the active wave. */
	UFUNCTION(BlueprintPure, Category = "Spawner|Waves")
	int32 GetEnemiesSpawnedThisWave() const { return EnemiesSpawnedThisWave; }

	/** Number of valid enemies currently alive. */
	UFUNCTION(BlueprintPure, Category = "Spawner|Waves")
	int32 GetAliveEnemyCount() const;

	/** Enemies still alive or waiting to spawn before the active wave is complete. */
	UFUNCTION(BlueprintPure, Category = "Spawner|Waves")
	int32 GetEnemiesRemainingInWave() const;

private:
	FTimerHandle SpawnTimerHandle;
	FTimerHandle NextWaveTimerHandle;
	TArray<TWeakObjectPtr<ACharacter>> AliveEnemies;
	int32 EnemiesSpawnedThisWave = 0;
	int32 EnemiesRequiredThisWave = 0;

	void StartWave();
	void CheckWaveComplete();
	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	void TrySpawnWave();
	void CleanupDeadEnemies();
	bool TryGetSpawnLocation(FVector& OutSpawnLocation) const;
};
