#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UAmmoData;

USTRUCT(BlueprintType)
struct FInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UAmmoData* AmmoData = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HAVELSANMORTARSIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0.1"))
	float MaxWeight = 40.f;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	bool TryAddAmmo(UAmmoData* AmmoData);
	bool ConsumeAmmo(UAmmoData* AmmoData);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetCountFor(UAmmoData* AmmoData) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventoryEntry>& GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FString GetDisplayText() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FString GetWeightDisplay() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<FInventoryEntry> Items;

	UFUNCTION()
	void OnRep_Items();
};