// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "BrainComponent.h"
#include "EnemyAIController.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(GetMesh());
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultShieldMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultShieldMesh.Succeeded())
	{
		ShieldMesh->SetStaticMesh(DefaultShieldMesh.Object);
	}
}

void AEnemy::Ragdoll()
{
	if (bIsRagdolling)
	{
		return;
	}

	ReportDestroyedIfNeeded();
	OnEnemyDefeated.Broadcast(this);

	// Cast<AEnemyAIController>(GetController())->BrainComponent->PauseLogic("Ragdolling");
	SetShieldRaised(false);
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
	if (bIsRagdolling || Damage <= 0.0f)
	{
		return;
	}

	float DamageToEnemy = Damage;
	if (IsShieldProtecting())
	{
		const float DamageAbsorbedByShield = FMath::Min(ShieldHealth, Damage);
		ShieldHealth = FMath::Clamp(ShieldHealth - DamageAbsorbedByShield, 0.0f, MaxShieldHealth);
		DamageToEnemy -= DamageAbsorbedByShield;

		if (ShieldHealth <= 0.0f)
		{
			BreakShield();
		}
	}

	if (DamageToEnemy <= 0.0f)
	{
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageToEnemy, 0.0f, MaxHealth);
	if (CurrentHealth <= 0)
	{
		Ragdoll();
	}
}

float AEnemy::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DealDamage(Damage);
	return Damage;
}

void AEnemy::Destroyed()
{
	ReportDestroyedIfNeeded();

	Super::Destroyed();
}

void AEnemy::SetShieldRaised(bool bShouldRaiseShield)
{
	bShieldActive = bShouldRaiseShield && !bShieldBroken && ShieldHealth > 0.0f;
	ApplyShieldVisibility();
}

void AEnemy::BreakShield()
{
	if (bShieldBroken)
	{
		return;
	}

	bShieldBroken = true;
	bShieldActive = false;
	ShieldHealth = 0.0f;
	ApplyShieldVisibility();
	OnShieldBroken();
}

bool AEnemy::IsShieldProtecting() const
{
	return bShieldActive && !bShieldBroken && ShieldHealth > 0.0f;
}

void AEnemy::AttachShieldMesh()
{
	if (!ShieldMesh)
	{
		return;
	}

	USceneComponent* AttachParent = GetMesh() ? Cast<USceneComponent>(GetMesh()) : GetRootComponent();
	if (!AttachParent)
	{
		return;
	}

	const bool bUseSocket = GetMesh() && ShieldAttachSocket != NAME_None && GetMesh()->DoesSocketExist(ShieldAttachSocket);
	const FName SocketName = bUseSocket ? ShieldAttachSocket : NAME_None;
	ShieldMesh->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
	ShieldMesh->SetRelativeLocation(ShieldRelativeLocation);
	ShieldMesh->SetRelativeRotation(ShieldRelativeRotation);
	ShieldMesh->SetRelativeScale3D(ShieldRelativeScale);
}

void AEnemy::ReportDestroyedIfNeeded()
{
	if (bHasReportedDestroyed)
	{
		return;
	}

	bHasReportedDestroyed = true;

	if (AShooterCharacter* ShooterCharacter = FindShooterCharacterForDeathCredit())
	{
		ShooterCharacter->RegisterDestroyedEnemy(this);
	}
}

AShooterCharacter* AEnemy::FindShooterCharacterForDeathCredit() const
{
	return Cast<AShooterCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AEnemy::ApplyShieldVisibility()
{
	if (!ShieldMesh)
	{
		return;
	}

	const bool bShowShield = IsShieldProtecting();
	ShieldMesh->SetVisibility(bShowShield, true);
	ShieldMesh->SetHiddenInGame(!bShowShield, true);
	ShieldMesh->SetComponentTickEnabled(bShowShield);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	MaxShieldHealth = FMath::Max(0.0f, MaxShieldHealth);
	ShieldHealth = FMath::Clamp(ShieldHealth, 0.0f, MaxShieldHealth);
	bShieldBroken = ShieldHealth <= 0.0f;
	bShieldActive = !bShieldBroken;
	AttachShieldMesh();
	ApplyShieldVisibility();
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
