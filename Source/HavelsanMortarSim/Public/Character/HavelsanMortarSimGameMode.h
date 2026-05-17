// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HavelsanMortarSimGameMode.generated.h"

class UAmmoData;
class APickupAmmo;

UCLASS(minimalapi)
class AHavelsanMortarSimGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHavelsanMortarSimGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawning")
	TArray<TObjectPtr<UAmmoData>> AmmoTypes;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawning")
	TSubclassOf<APickupAmmo> PickupClass;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawning", meta = (ClampMin = "0"))
	int32 PickupsPerType = 3;

	UPROPERTY(EditAnywhere, Category = "Pickup Spawning")
	FVector SpawnAreaCenter = FVector(0.f, 0.f, 200.f);

	UPROPERTY(EditAnywhere, Category = "Pickup Spawning")
	FVector SpawnAreaExtent = FVector(2000.f, 2000.f, 0.f);
};



