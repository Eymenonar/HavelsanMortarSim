#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "PickupAmmo.generated.h"

class UAmmoData;
class UStaticMeshComponent;

UCLASS()
class HAVELSANMORTARSIM_API APickupAmmo : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickupAmmo();

	virtual void Interact_Implementation(AActor* Interactor) override;

	void SetAmmoData(UAmmoData* InAmmoData) { AmmoData = InAmmoData; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyAmmoVisuals();

	UFUNCTION()
	void OnRep_AmmoData();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", ReplicatedUsing = OnRep_AmmoData)
	TObjectPtr<UAmmoData> AmmoData;
};
