// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "BrainComponent.h"
#include "EnemyAIController.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	

}

void AEnemy::Ragdoll()
{
	// Cast<AEnemyAIController>(GetController())->BrainComponent->PauseLogic("Ragdolling");
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));

	GetWorld()->GetTimerManager().SetTimer(
		RagdollTimerHandle,
		this,
		&AEnemy::StopRagdoll,
		RagdollTime,
		false
	);

	bIsRagdolling = true;
}

void AEnemy::StopRagdoll()
{
	this->Destroy();
}

void AEnemy::DealDamage(float Damage)
{
	if (bIsRagdolling)
	{
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	if (CurrentHealth <= 0)
	{
		Ragdoll();
		
	}
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

