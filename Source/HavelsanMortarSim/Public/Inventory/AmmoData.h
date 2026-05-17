#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoData.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Type1 UMETA(DisplayName = "Type 1 (5kg, Stackable)"),
	Type2 UMETA(DisplayName = "Type 2 (10kg, Non-Stack)"),
	Type3 UMETA(DisplayName = "Type 3 (15kg, Non-Stack)")
};

class AMortarProjectile;
class UStaticMesh;

UCLASS(BlueprintType)
class HAVELSANMORTARSIM_API UAmmoData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	EAmmoType Type = EAmmoType::Type1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.1"))
	float Weight = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	bool bStackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	UStaticMesh* PickupMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FVector PickupMeshScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	TSubclassOf<AMortarProjectile> ProjectileClass;
};