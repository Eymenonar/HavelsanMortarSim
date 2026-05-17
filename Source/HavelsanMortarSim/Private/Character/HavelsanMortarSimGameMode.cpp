// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/HavelsanMortarSimGameMode.h"
#include "Character/HavelsanMortarSimCharacter.h"
#include "Pickup/PickupAmmo.h"
#include "Inventory/AmmoData.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

AHavelsanMortarSimGameMode::AHavelsanMortarSimGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AHavelsanMortarSimGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;
	if (!PickupClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (UAmmoData* AmmoType : AmmoTypes)
	{
		if (!AmmoType) continue;

		for (int32 i = 0; i < PickupsPerType; ++i)
		{
			const float X = SpawnAreaCenter.X + FMath::FRandRange(-SpawnAreaExtent.X, SpawnAreaExtent.X);
			const float Y = SpawnAreaCenter.Y + FMath::FRandRange(-SpawnAreaExtent.Y, SpawnAreaExtent.Y);

			const FVector TraceStart(X, Y, 10000.f);
			const FVector TraceEnd(X, Y, -10000.f);

			FHitResult Hit;
			FCollisionQueryParams QueryParams;
			QueryParams.bTraceComplex = true;

			FVector SpawnLoc;
			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
			{
				SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 10.f);
			}
			else
			{
				continue;
			}

			APickupAmmo* Pickup = World->SpawnActorDeferred<APickupAmmo>(
				PickupClass, FTransform(SpawnLoc), nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

			if (Pickup)
			{
				Pickup->SetAmmoData(AmmoType);
				Pickup->FinishSpawning(FTransform(SpawnLoc));
			}
		}
	}
}
