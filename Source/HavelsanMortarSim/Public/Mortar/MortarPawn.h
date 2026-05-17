#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MortarPawn.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UAmmoData;
class UInventoryComponent;
class USoundBase;
struct FInputActionValue;

UCLASS()
class HAVELSANMORTARSIM_API AMortarPawn : public APawn
{
	GENERATED_BODY()

public:
	AMortarPawn();

	void SetCharacterReference(ACharacter* InCharacter);

	FORCEINLINE USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

	UFUNCTION(BlueprintCallable, Category = "Mortar")
	void SetSelectedAmmo(UAmmoData* NewAmmo);

	UFUNCTION(BlueprintCallable, Category = "Mortar")
	void SetFireSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Mortar")
	void SetFirePitch(float NewPitch);

	UFUNCTION(BlueprintCallable, Category = "Mortar")
	void TryFire();

	UFUNCTION(BlueprintPure, Category = "Mortar")
	UAmmoData* GetSelectedAmmo() const { return SelectedAmmo; }

	UFUNCTION(BlueprintPure, Category = "Mortar")
	float GetFireSpeed() const { return FireSpeed; }

	UFUNCTION(BlueprintPure, Category = "Mortar")
	float GetFirePitch() const { return FirePitch; }

	UFUNCTION(BlueprintPure, Category = "Mortar")
	UInventoryComponent* GetCharacterInventory() const;

	UFUNCTION(BlueprintPure, Category = "Mortar")
	TArray<FString> GetAvailableAmmoNames() const;

	UFUNCTION(BlueprintCallable, Category = "Mortar")
	void SelectAmmoByDisplayName(const FString& Name);

	UFUNCTION(BlueprintPure, Category = "Mortar")
	int32 GetSelectedAmmoCount() const;

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MortarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> MuzzleLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MortarMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> TogglePossessAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mortar")
	float YawSpeed = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveHUDWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Mortar")
	TObjectPtr<UAmmoData> SelectedAmmo;

	UPROPERTY(BlueprintReadOnly, Category = "Mortar")
	float FireSpeed = 3000.f;

	UPROPERTY(BlueprintReadOnly, Category = "Mortar")
	float FirePitch = 45.f;

	UPROPERTY(Replicated)
	TObjectPtr<ACharacter> CachedCharacter;

	void Rotate(const FInputActionValue& Value);
	void TogglePossess(const FInputActionValue& Value);

	UFUNCTION(Server, Unreliable)
	void Server_Rotate(float AxisValue);

	UFUNCTION(Server, Reliable)
	void Server_TogglePossessBack();

	UFUNCTION(Server, Reliable)
	void Server_Fire(UAmmoData* Ammo, float Speed, float Pitch);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayFireEffects(FVector Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
	void OnFireEffects(const FVector& Location);

private:
	void SetupForLocalPlayer();
	void TeardownForLocalPlayer();
};
