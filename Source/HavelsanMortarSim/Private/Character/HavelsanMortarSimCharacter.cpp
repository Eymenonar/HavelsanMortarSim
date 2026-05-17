// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/HavelsanMortarSimCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SphereComponent.h"
#include "Interfaces/Interactable.h"
#include "Inventory/InventoryComponent.h"
#include "Mortar/MortarPawn.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);


AHavelsanMortarSimCharacter::AHavelsanMortarSimCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(200.f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractionSphere->OnComponentBeginOverlap.
	                   AddDynamic(this, &AHavelsanMortarSimCharacter::OnInteractionBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AHavelsanMortarSimCharacter::OnInteractionEndOverlap);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AHavelsanMortarSimCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AHavelsanMortarSimCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &AHavelsanMortarSimCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &AHavelsanMortarSimCharacter::Look);

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,
			                                   &AHavelsanMortarSimCharacter::Interact);
		}

		if (TogglePossessAction)
		{
			EnhancedInputComponent->BindAction(TogglePossessAction, ETriggerEvent::Started, this,
			                                   &AHavelsanMortarSimCharacter::TogglePossess);
		}

		if (ToggleInventoryAction)
		{
			EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this,
			                                   &AHavelsanMortarSimCharacter::ToggleInventory);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void AHavelsanMortarSimCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHavelsanMortarSimCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHavelsanMortarSimCharacter::Interact(const FInputActionValue& Value)
{
	Server_Interact();
}

void AHavelsanMortarSimCharacter::Server_Interact_Implementation()
{
	if (!InteractionSphere) return;

	TArray<AActor*> Overlapping;
	InteractionSphere->GetOverlappingActors(Overlapping);

	AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	const FVector MyLoc = GetActorLocation();
	for (AActor* A : Overlapping)
	{
		if (!IsValid(A)) continue;
		if (!A->Implements<UInteractable>()) continue;
		const float DistSq = FVector::DistSquared(MyLoc, A->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = A;
		}
	}

	if (Best)
	{
		IInteractable::Execute_Interact(Best, this);
	}
}

void AHavelsanMortarSimCharacter::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                            AActor* OtherActor,
                                                            UPrimitiveComponent* OtherComp,
                                                            int32 OtherBodyIndex,
                                                            bool bFromSweep,
                                                            const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!OtherActor)
	{
		return;
	}
	if (OtherActor->Implements<UInteractable>())
	{
		CurrentInteractable = OtherActor;
	}
	if (AMortarPawn* Mortar = Cast<AMortarPawn>(OtherActor))
	{
		NearbyMortar = Mortar;
	}
}

void AHavelsanMortarSimCharacter::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                          AActor* OtherActor,
                                                          UPrimitiveComponent* OtherComp,
                                                          int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor == CurrentInteractable)
	{
		CurrentInteractable = nullptr;
	}
	if (OtherActor == NearbyMortar)
	{
		NearbyMortar = nullptr;
	}
}

void AHavelsanMortarSimCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void AHavelsanMortarSimCharacter::UnPossessed()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->RemoveMappingContext(DefaultMappingContext);
			}
		}
	}

	if (ActiveInventoryWidget)
	{
		ActiveInventoryWidget->RemoveFromParent();
		ActiveInventoryWidget = nullptr;
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	Super::UnPossessed();
}

void AHavelsanMortarSimCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
	}
	else
	{
		if (ActiveInventoryWidget)
		{
			ActiveInventoryWidget->RemoveFromParent();
			ActiveInventoryWidget = nullptr;
		}
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void AHavelsanMortarSimCharacter::TogglePossess(const FInputActionValue& Value)
{
	Server_TogglePossess();
}

void AHavelsanMortarSimCharacter::Server_TogglePossess_Implementation()
{
	if (!NearbyMortar) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	NearbyMortar->SetCharacterReference(this);
	PC->Possess(NearbyMortar);
}

void AHavelsanMortarSimCharacter::ToggleInventory(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	if (!InventoryWidgetClass) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (ActiveInventoryWidget)
	{
		ActiveInventoryWidget->RemoveFromParent();
		ActiveInventoryWidget = nullptr;
		return;
	}

	ActiveInventoryWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
	if (ActiveInventoryWidget)
	{
		ActiveInventoryWidget->AddToViewport();
	}
}
