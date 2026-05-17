#include "Pickup/PickupAmmo.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Inventory/AmmoData.h"
#include "Inventory/InventoryComponent.h"
#include "Character/HavelsanMortarSimCharacter.h"

APickupAmmo::APickupAmmo()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = MeshComponent;
}

void APickupAmmo::BeginPlay()
{
	Super::BeginPlay();
	ApplyAmmoVisuals();
}

void APickupAmmo::ApplyAmmoVisuals()
{
	if (!AmmoData || !AmmoData->PickupMesh || !MeshComponent) return;

	MeshComponent->SetStaticMesh(AmmoData->PickupMesh);
	MeshComponent->SetWorldScale3D(AmmoData->PickupMeshScale);

	if (HasAuthority())
	{
		const FBoxSphereBounds NewBounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
		const float BottomZ = NewBounds.Origin.Z - NewBounds.BoxExtent.Z;
		const float LiftAmount = GetActorLocation().Z - BottomZ;
		AddActorWorldOffset(FVector(0.f, 0.f, LiftAmount));
	}
}

void APickupAmmo::OnRep_AmmoData()
{
	ApplyAmmoVisuals();
}

void APickupAmmo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickupAmmo, AmmoData);
}

void APickupAmmo::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	if (!AmmoData) return;

	AHavelsanMortarSimCharacter* Char = Cast<AHavelsanMortarSimCharacter>(Interactor);
	if (!Char) return;

	UInventoryComponent* Inv = Char->GetInventory();
	if (!Inv) return;

	if (Inv->TryAddAmmo(AmmoData))
	{
		Destroy();
	}
}
