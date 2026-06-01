#include "Ultimate.h"

#include "Enemy.h"
#include "ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

AUltimate::AUltimate()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	GetCharacterMovement()->MaxWalkSpeed = 800.0f;
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 100.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AUltimate::BeginPlay()
{
	Super::BeginPlay();
}

void AUltimate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveUltimateMappingContext();

	Super::EndPlay(EndPlayReason);
}

void AUltimate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveForwardAutomatically();
}

void AUltimate::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

		if (LocalPlayer)
		{
			UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

			if (InputSubsystem && UltimateMappingContext)
			{
				InputSubsystem->AddMappingContext(UltimateMappingContext, 1);
			}
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (UltimateLookAction)
	{
		EnhancedInputComponent->BindAction(
			UltimateLookAction,
			ETriggerEvent::Triggered,
			this,
			&AUltimate::DoLook
		);
	}

	if (UltimateJumpAction)
	{
		EnhancedInputComponent->BindAction(
			UltimateJumpAction,
			ETriggerEvent::Started,
			this,
			&AUltimate::DoJumpStart
		);

		EnhancedInputComponent->BindAction(
			UltimateJumpAction,
			ETriggerEvent::Completed,
			this,
			&AUltimate::DoJumpEnd
		);
	}

	if (UltimateDetonateAction)
	{
		EnhancedInputComponent->BindAction(
			UltimateDetonateAction,
			ETriggerEvent::Started,
			this,
			&AUltimate::Detonate
		);
	}
}

void AUltimate::RemoveUltimateMappingContext()
{
	if (!UltimateMappingContext)
	{
		return;
	}

	APlayerController* PlayerController = CachedPlayerController
		? CachedPlayerController
		: Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UltimateMappingContext);
	}
}

void AUltimate::SetReturnShooterCharacter(AShooterCharacter* ShooterCharacter)
{
	ReturnShooterCharacter = ShooterCharacter;

	if (!IsValid(ReturnShooterCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("Ultimate failed to store return shooter."));
	}
}

void AUltimate::MoveForwardAutomatically()
{
	if (!Controller)
	{
		return;
	}

	AddMovementInput(GetActorForwardVector(), AutoMoveScale);
}

void AUltimate::DoLook(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxis.X * MouseTurnSensitivity);
	AddControllerPitchInput(-LookAxis.Y * MouseLookSensitivity);
}

void AUltimate::DoJumpStart(const FInputActionValue&)
{
	Jump();
}

void AUltimate::DoJumpEnd(const FInputActionValue&)
{
	StopJumping();
}

void AUltimate::Detonate()
{
	if (bHasDetonated)
	{
		return;
	}

	bHasDetonated = true;

	CachedPlayerController = Cast<APlayerController>(GetController());

	if (!CachedPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Ultimate Detonate failed: Ultimate has no PlayerController."));
		bHasDetonated = false;
		return;
	}

	if (!IsValid(ReturnShooterCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("Ultimate Detonate failed: ReturnShooterCharacter is invalid."));
		bHasDetonated = false;
		return;
	}

	const FRotator UltimateViewRotation = CachedPlayerController->GetControlRotation();

	SpawnExplosionEffect();

	ExplosionCheck(GetActorLocation());

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	RemoveUltimateMappingContext();

	CachedPlayerController->Possess(ReturnShooterCharacter);
	CachedPlayerController->SetControlRotation(UltimateViewRotation);

	ReturnShooterCharacter->RestoreAfterUltimateMode();

	CachedPlayerController->ResetIgnoreMoveInput();
	CachedPlayerController->ResetIgnoreLookInput();

	CachedPlayerController->SetIgnoreMoveInput(false);
	CachedPlayerController->SetIgnoreLookInput(false);

	if (CachedPlayerController->GetPawn() != ReturnShooterCharacter)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Possession failed. Controller pawn is now: %s"),
			*GetNameSafe(CachedPlayerController->GetPawn())
		);

		bHasDetonated = false;

		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		return;
	}

	GetWorldTimerManager().SetTimer(
		DestroyUltimateTimerHandle,
		this,
		&AUltimate::DestroyUltimateAfterReturn,
		0.01f,
		false
	);
}

void AUltimate::SpawnExplosionEffect()
{
	if (!ExplosionEffectClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	World->SpawnActor<AActor>(
		ExplosionEffectClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams
	);
}

void AUltimate::ExplosionCheck(const FVector& ExplosionCenter)
{
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (!bDamageOwner)
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	if (IsValid(ReturnShooterCharacter))
	{
		QueryParams.AddIgnoredActor(ReturnShooterCharacter);
	}

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		ExplosionCenter,
		FQuat::Identity,
		ObjectParams,
		OverlapShape,
		QueryParams
	);

	TArray<AActor*> DamagedActors;

	for (const FOverlapResult& CurrentOverlap : Overlaps)
	{
		AActor* OverlappedActor = CurrentOverlap.GetActor();

		if (!OverlappedActor)
		{
			continue;
		}

		if (DamagedActors.Contains(OverlappedActor))
		{
			continue;
		}

		DamagedActors.Add(OverlappedActor);

		ACharacter* HitCharacter = Cast<ACharacter>(OverlappedActor);

		if (!HitCharacter)
		{
			continue;
		}

		const FVector CharacterLocation = HitCharacter->GetActorLocation();

		FVector AffectDirection;
		float LaunchStrength = 0.0f;

		if (HitCharacter->IsPlayerControlled())
		{
			AffectDirection = CharacterLocation - ExplosionCenter;
			LaunchStrength = PlayerPushForce;
		}
		else
		{
			AffectDirection = ExplosionCenter - CharacterLocation;
			LaunchStrength = EnemyPullForce;
		}

		AffectDirection = AffectDirection.GetSafeNormal();

		AffectDirection += FVector(0.0f, 0.0f, 0.15f);
		AffectDirection.Normalize();

		ProcessHit(
			HitCharacter,
			CurrentOverlap.GetComponent(),
			CharacterLocation,
			AffectDirection,
			LaunchStrength
		);
	}
}

void AUltimate::ProcessHit(
	AActor* HitActor,
	UPrimitiveComponent*,
	const FVector&,
	const FVector& HitDirection,
	float LaunchStrength
)
{
	ACharacter* HitCharacter = Cast<ACharacter>(HitActor);

	if (!HitCharacter)
	{
		return;
	}

	if (LaunchStrength <= 0.0f)
	{
		LaunchStrength = PhysicsForce;
	}

	const FVector LaunchVelocity = HitDirection * LaunchStrength;

	HitCharacter->LaunchCharacter(
		LaunchVelocity,
		true,
		true
	);

	if (AEnemy* Enemy = Cast<AEnemy>(HitCharacter))
	{
		Enemy->DealDamage(HitDamage);
	}
}

void AUltimate::DestroyUltimateAfterReturn()
{
	if (!CachedPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot destroy Ultimate: CachedPlayerController is invalid."));
		return;
	}

	if (CachedPlayerController->GetPawn() == this)
	{
		UE_LOG(LogTemp, Error, TEXT("Refusing to destroy Ultimate because it is still possessed."));
		return;
	}

	ReturnShooterCharacter = nullptr;
	CachedPlayerController = nullptr;

	Destroy();
}