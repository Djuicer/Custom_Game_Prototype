#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ACharacter;

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
	TArray<TWeakObjectPtr<ACharacter>> AliveEnemies;

	void TrySpawnWave();
	void CleanupDeadEnemies();
	bool TryGetSpawnLocation(FVector& OutSpawnLocation) const;
};
