#include "Mortar/MortarPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Inventory/AmmoData.h"
#include "Inventory/InventoryComponent.h"
#include "Character/HavelsanMortarSimCharacter.h"
#include "Mortar/MortarProjectile.h"

AMortarPawn::AMortarPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 30.f;

	MortarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MortarMesh"));
	MortarMesh->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = MortarMesh;

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(MortarMesh);
	MuzzleLocation->SetRelativeLocation(FVector(80.f, 0.f, 120.f));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(MortarMesh);
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void AMortarPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMortarPawn::SetupForLocalPlayer);
}

void AMortarPawn::UnPossessed()
{
	TeardownForLocalPlayer();
	Super::UnPossessed();
}

void AMortarPawn::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (GetController())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMortarPawn::SetupForLocalPlayer);
	}
	else
	{
		TeardownForLocalPlayer();
	}
}

void AMortarPawn::SetupForLocalPlayer()
{
	if (!IsLocallyControlled()) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (MortarMappingContext)
		{
			Subsystem->AddMappingContext(MortarMappingContext, 0);
		}
	}

	if (HUDWidgetClass && !ActiveHUDWidget)
	{
		ActiveHUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
		if (ActiveHUDWidget)
		{
			ActiveHUDWidget->AddToViewport();
		}
	}

	PC->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
}

void AMortarPawn::TeardownForLocalPlayer()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (MortarMappingContext)
			{
				Subsystem->RemoveMappingContext(MortarMappingContext);
			}
		}

		PC->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->RemoveFromParent();
		ActiveHUDWidget = nullptr;
	}
}

void AMortarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (RotateAction)
		{
			EIC->BindAction(RotateAction, ETriggerEvent::Triggered, this, &AMortarPawn::Rotate);
		}
		if (TogglePossessAction)
		{
			EIC->BindAction(TogglePossessAction, ETriggerEvent::Started, this, &AMortarPawn::TogglePossess);
		}
	}
}

void AMortarPawn::Rotate(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue)) return;
	Server_Rotate(AxisValue);
}

void AMortarPawn::Server_Rotate_Implementation(float AxisValue)
{
	const float DeltaYaw = AxisValue * YawSpeed * GetWorld()->GetDeltaSeconds();
	AddActorLocalRotation(FRotator(0.f, DeltaYaw, 0.f));
}

void AMortarPawn::TogglePossess(const FInputActionValue& Value)
{
	Server_TogglePossessBack();
}

void AMortarPawn::Server_TogglePossessBack_Implementation()
{
	if (!CachedCharacter) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->Possess(CachedCharacter);
	}
}

void AMortarPawn::SetCharacterReference(ACharacter* InCharacter)
{
	CachedCharacter = InCharacter;
}

void AMortarPawn::SetSelectedAmmo(UAmmoData* NewAmmo)
{
	SelectedAmmo = NewAmmo;
}

void AMortarPawn::SetFireSpeed(float NewSpeed)
{
	FireSpeed = FMath::Max(NewSpeed, 0.f);
}

void AMortarPawn::SetFirePitch(float NewPitch)
{
	FirePitch = FMath::Clamp(NewPitch, 45.f, 90.f);
}

UInventoryComponent* AMortarPawn::GetCharacterInventory() const
{
	if (!CachedCharacter) return nullptr;
	AHavelsanMortarSimCharacter* Char = Cast<AHavelsanMortarSimCharacter>(CachedCharacter);
	return Char ? Char->GetInventory() : nullptr;
}

TArray<FString> AMortarPawn::GetAvailableAmmoNames() const
{
	TArray<FString> Names;
	if (UInventoryComponent* Inv = GetCharacterInventory())
	{
		for (const FInventoryEntry& Entry : Inv->GetItems())
		{
			if (Entry.AmmoData)
			{
				Names.Add(Entry.AmmoData->DisplayName.ToString());
			}
		}
	}
	return Names;
}

void AMortarPawn::SelectAmmoByDisplayName(const FString& Name)
{
	if (UInventoryComponent* Inv = GetCharacterInventory())
	{
		for (const FInventoryEntry& Entry : Inv->GetItems())
		{
			if (Entry.AmmoData && Entry.AmmoData->DisplayName.ToString() == Name)
			{
				SelectedAmmo = Entry.AmmoData;
				return;
			}
		}
	}
}

int32 AMortarPawn::GetSelectedAmmoCount() const
{
	if (!SelectedAmmo) return 0;
	if (UInventoryComponent* Inv = GetCharacterInventory())
	{
		return Inv->GetCountFor(SelectedAmmo);
	}
	return 0;
}

void AMortarPawn::TryFire()
{
	if (!SelectedAmmo) return;
	Server_Fire(SelectedAmmo, FireSpeed, FirePitch);
}

void AMortarPawn::Server_Fire_Implementation(UAmmoData* Ammo, float Speed, float Pitch)
{
	if (!Ammo) return;
	if (!Ammo->ProjectileClass) return;

	UInventoryComponent* Inv = GetCharacterInventory();
	if (!Inv) return;
	if (Inv->GetCountFor(Ammo) <= 0) return;

	const FVector SpawnLoc = MuzzleLocation->GetComponentLocation();
	const FRotator SpawnRot(Pitch, MuzzleLocation->GetComponentRotation().Yaw, 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;
	Params.Instigator = this;

	AMortarProjectile* Proj = GetWorld()->SpawnActor<AMortarProjectile>(Ammo->ProjectileClass, SpawnLoc, SpawnRot, Params);
	if (Proj)
	{
		if (UPrimitiveComponent* ProjRoot = Cast<UPrimitiveComponent>(Proj->GetRootComponent()))
		{
			ProjRoot->IgnoreActorWhenMoving(this, true);
		}
		Proj->SetInitialSpeed(Speed);
		Inv->ConsumeAmmo(Ammo);
		Multicast_PlayFireEffects(SpawnLoc);
	}
}

void AMortarPawn::Multicast_PlayFireEffects_Implementation(FVector Location)
{
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Location);
	}
	OnFireEffects(Location);
}

void AMortarPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMortarPawn, CachedCharacter);
}
