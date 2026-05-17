#include "Inventory/InventoryComponent.h"
#include "Inventory/AmmoData.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::TryAddAmmo(UAmmoData* AmmoData)
{
	if (!AmmoData) return false;
	if (GetCurrentWeight() + AmmoData->Weight > MaxWeight) return false;

	if (AmmoData->bStackable)
	{
		for (FInventoryEntry& Entry : Items)
		{
			if (Entry.AmmoData == AmmoData)
			{
				++Entry.Count;
				OnInventoryChanged.Broadcast();
				return true;
			}
		}
	}

	Items.Add({ AmmoData, 1 });
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::ConsumeAmmo(UAmmoData* AmmoData)
{
	if (!AmmoData) return false;

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].AmmoData != AmmoData) continue;

		if (--Items[i].Count <= 0)
		{
			Items.RemoveAt(i);
		}
		OnInventoryChanged.Broadcast();
		return true;
	}
	return false;
}

int32 UInventoryComponent::GetCountFor(UAmmoData* AmmoData) const
{
	if (!AmmoData) return 0;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.AmmoData == AmmoData) return Entry.Count;
	}
	return 0;
}

FString UInventoryComponent::GetDisplayText() const
{
	FString Result;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.AmmoData)
		{
			Result += FString::Printf(TEXT("%s : %d\n"),
				*Entry.AmmoData->DisplayName.ToString(),
				Entry.Count);
		}
	}
	if (Result.IsEmpty())
	{
		Result = TEXT("(empty)");
	}
	return Result;
}

FString UInventoryComponent::GetWeightDisplay() const
{
	return FString::Printf(TEXT("%.0f / %.0f kg"), GetCurrentWeight(), MaxWeight);
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.f;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.AmmoData)
		{
			Total += Entry.AmmoData->Weight * Entry.Count;
		}
	}
	return Total;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryChanged.Broadcast();
}
