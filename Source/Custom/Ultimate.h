#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Ultimate.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AShooterCharacter;
class AEnemy;
class UPrimitiveComponent;

UCLASS()
class CUSTOM_API AUltimate : public ACharacter
{
	GENERATED_BODY()

public:
	AUltimate();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetReturnShooterCharacter(AShooterCharacter* ShooterCharacter);

protected:
	virtual void BeginPlay() override;

	void MoveForwardAutomatically();

	void DoLook(const FInputActionValue& Value);
	void DoJumpStart(const FInputActionValue& Value);
	void DoJumpEnd(const FInputActionValue& Value);

	void Detonate();

	void SpawnExplosionEffect();

	void ExplosionCheck(const FVector& ExplosionCenter);

	void ProcessHit(
		AActor* HitActor,
		UPrimitiveComponent* HitComp,
		const FVector& HitLocation,
		const FVector& HitDirection,
		float LaunchStrength
	);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* UltimateMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* UltimateLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* UltimateJumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* UltimateDetonateAction;

	UPROPERTY()
	AShooterCharacter* ReturnShooterCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Movement", meta = (AllowPrivateAccess = "true"))
	float AutoMoveScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Movement", meta = (AllowPrivateAccess = "true"))
	float MouseTurnSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Movement", meta = (AllowPrivateAccess = "true"))
	float MouseLookSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	float ExplosionRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	float HitDamage = 100.0f;

	/** Optional Blueprint actor spawned when the Ultimate detonates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> ExplosionEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	float PlayerPushForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	float EnemyPullForce = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	float PhysicsForce = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Explosion", meta = (AllowPrivateAccess = "true"))
	bool bDamageOwner = false;

	bool bHasDetonated = false;

	UPROPERTY()
	APlayerController* CachedPlayerController = nullptr;

	FTimerHandle DestroyUltimateTimerHandle;
	void RemoveUltimateMappingContext();

	void DestroyUltimateAfterReturn();
};
