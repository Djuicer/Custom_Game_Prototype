// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "Custom.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogCustom, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogCustom, Error, TEXT("Could not spawn bullet counter widget."));

		}
		
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Only ShooterCharacter should be allowed to trigger the respawn system.
	// Do NOT bind this to AUltimate.
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		ShooterCharacter->OnDestroyed.RemoveDynamic(
			this,
			&AShooterPlayerController::OnPawnDestroyed
		);

		ShooterCharacter->OnDestroyed.AddDynamic(
			this,
			&AShooterPlayerController::OnPawnDestroyed
		);

		ShooterCharacter->Tags.AddUnique(PlayerPawnTag);
		ShooterCharacter->InitializeGameplayHUD();

		ShooterCharacter->OnBulletCountUpdated.RemoveDynamic(
			this,
			&AShooterPlayerController::OnBulletCountUpdated
		);

		ShooterCharacter->OnBulletCountUpdated.AddDynamic(
			this,
			&AShooterPlayerController::OnBulletCountUpdated
		);

		ShooterCharacter->OnDamaged.RemoveDynamic(
			this,
			&AShooterPlayerController::OnPawnDamaged
		);

		ShooterCharacter->OnDamaged.AddDynamic(
			this,
			&AShooterPlayerController::OnPawnDamaged
		);

		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	AShooterCharacter* DestroyedShooter = Cast<AShooterCharacter>(DestroyedActor);

	// Ignore AUltimate and any other temporary pawn.
	if (!DestroyedShooter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ignoring destroyed non-shooter pawn: %s"),
			*GetNameSafe(DestroyedActor)
		);
		return;
	}

	// Optional but recommended: only respawn if this shooter actually died.
	if (!DestroyedShooter->IsDead())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ignoring destroyed shooter because it was not dead: %s"),
			*GetNameSafe(DestroyedActor)
		);
		return;
	}

	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		APlayerStart::StaticClass(),
		ActorList
	);

	if (ActorList.Num() > 0)
	{
		AActor* RandomPlayerStart =
			ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter* RespawnedCharacter =
			GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
