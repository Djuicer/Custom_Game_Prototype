// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "Variant_Shooter/UI/ShooterHUDWidget.h"
#include "EnemySpawner.h"
#include "Custom.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "EngineUtils.h"

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

		CreateShooterHUD();
		BindShooterHUDToSpawner();

		if (bSpawnLegacyBulletCounterUI)
		{
			// create the legacy bullet counter widget and add it to the screen only when explicitly enabled
			BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

			if (BulletCounterUI)
			{
				BulletCounterUI->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogCustom, Error, TEXT("Could not spawn legacy bullet counter widget."));

			}
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

		ShooterCharacter->OnHealthChanged.RemoveDynamic(
			this,
			&AShooterPlayerController::OnPawnHealthChanged
		);

		ShooterCharacter->OnHealthChanged.AddDynamic(
			this,
			&AShooterPlayerController::OnPawnHealthChanged
		);

		ShooterCharacter->OnDestroyedEnemyCountChanged.RemoveDynamic(
			this,
			&AShooterPlayerController::OnDestroyedEnemyCountChanged
		);

		ShooterCharacter->OnDestroyedEnemyCountChanged.AddDynamic(
			this,
			&AShooterPlayerController::OnDestroyedEnemyCountChanged
		);

		InitializeShooterHUDFromPawn(ShooterCharacter);
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


void AShooterPlayerController::OnPawnHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (IsValid(ShooterHUDWidget))
	{
		ShooterHUDWidget->SetPlayerHealth(CurrentHealth, MaxHealth);
	}
}

void AShooterPlayerController::OnDestroyedEnemyCountChanged(int32 DestroyedEnemyCount)
{
	if (IsValid(ShooterHUDWidget))
	{
		ShooterHUDWidget->SetScore(DestroyedEnemyCount);
	}
}

void AShooterPlayerController::OnWaveChanged(int32 CurrentWave)
{
	if (IsValid(ShooterHUDWidget))
	{
		ShooterHUDWidget->SetCurrentWave(CurrentWave);
	}
}

void AShooterPlayerController::OnEnemiesRemainingChanged(int32 EnemiesRemaining)
{
	if (IsValid(ShooterHUDWidget))
	{
		ShooterHUDWidget->SetEnemiesRemaining(EnemiesRemaining);
	}
}

void AShooterPlayerController::CreateShooterHUD()
{
	if (!IsLocalPlayerController() || IsValid(ShooterHUDWidget))
	{
		return;
	}

	TSubclassOf<UShooterHUDWidget> WidgetClass = ShooterHUDWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UShooterHUDWidget::StaticClass();
	}

	ShooterHUDWidget = CreateWidget<UShooterHUDWidget>(this, WidgetClass);
	if (ShooterHUDWidget)
	{
		ShooterHUDWidget->AddToPlayerScreen(0);
	}
	else
	{
		UE_LOG(LogCustom, Error, TEXT("Could not spawn shooter HUD widget."));
	}
}

void AShooterPlayerController::BindShooterHUDToSpawner()
{
	if (!GetWorld() || !IsValid(ShooterHUDWidget))
	{
		return;
	}

	for (TActorIterator<AEnemySpawner> It(GetWorld()); It; ++It)
	{
		BoundEnemySpawner = *It;
		break;
	}

	if (!BoundEnemySpawner)
	{
		UE_LOG(LogCustom, Warning, TEXT("No EnemySpawner found for shooter HUD wave data."));
		return;
	}

	BoundEnemySpawner->OnWaveChanged.RemoveDynamic(this, &AShooterPlayerController::OnWaveChanged);
	BoundEnemySpawner->OnWaveChanged.AddDynamic(this, &AShooterPlayerController::OnWaveChanged);
	BoundEnemySpawner->OnEnemiesRemainingChanged.RemoveDynamic(this, &AShooterPlayerController::OnEnemiesRemainingChanged);
	BoundEnemySpawner->OnEnemiesRemainingChanged.AddDynamic(this, &AShooterPlayerController::OnEnemiesRemainingChanged);

	ShooterHUDWidget->SetCurrentWave(BoundEnemySpawner->GetCurrentWave());
	ShooterHUDWidget->SetEnemiesRemaining(BoundEnemySpawner->GetEnemiesRemainingInWave());
}

void AShooterPlayerController::InitializeShooterHUDFromPawn(AShooterCharacter* ShooterCharacter)
{
	if (!IsValid(ShooterHUDWidget) || !ShooterCharacter)
	{
		return;
	}

	ShooterHUDWidget->SetPlayerHealth(ShooterCharacter->GetCurrentHealth(), ShooterCharacter->GetMaxHealth());
	ShooterHUDWidget->SetScore(ShooterCharacter->GetDestroyedEnemyCount());
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
