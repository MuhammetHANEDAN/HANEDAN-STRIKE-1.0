// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;


class AHANEDANPlayerController;
class UHanedanAnimInstance;
class AHANEDANPlayerState;
class ABaseWeapon;
class AHANEDANCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HANEDANSTRIKE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	
	virtual void BeginPlay() override;

public:	

	/**
	 * FREE SPACE
	 */

	UPROPERTY()
	AHANEDANCharacter* OwnerHCharacter;

	UPROPERTY()
	AHANEDANPlayerController* HPlayerController;

	UPROPERTY()
	AHANEDANPlayerState* HPlayerState;

	UPROPERTY()
	USkeletalMeshComponent* OwnerHCSkeletalMeshComponent;

	UPROPERTY()
	UHanedanAnimInstance* HanedanAnimInstance;

	void ReloadButtonPressed();
	
	/**
	 * WEAPON 
	 */

	UPROPERTY(ReplicatedUsing=OnRep_bWeaponOnHand)
	bool bWeaponOnHand = false;
	UFUNCTION()
	void OnRep_bWeaponOnHand(bool LastValue);

	UPROPERTY(ReplicatedUsing=OnRep_Aiming, BlueprintReadOnly)
	bool bAiming = false;
	UFUNCTION()
	void OnRep_Aiming();
	//void SetAiming(bool Aiming);
	UFUNCTION(Server, Reliable)
	void Server_ServerSetAiming(bool Aiming);
	UFUNCTION(Server,Reliable)
	void Server_SetbAimingVariable(bool Aiming); // Server sadece bAiming değişkenini setler

	UPROPERTY(Replicated)
	TArray<ABaseWeapon*> AllWeapons;
	
	void BuyWeapon(uint16 WeaponPrice, uint8 Slot, uint8 WeaponNumber);
	UFUNCTION(Server,Reliable)
	void Server_SpawnWeapon(uint8 WeaponSlotNumber, uint8 WeaponNumber);

	void BuyKevlarHelmet();
	UFUNCTION(Server,Reliable)
	void Server_BuyKevlarHelmet();
	
	ABaseWeapon* GetWeaponOnSlot(uint8 Slot); // gelen Slot numarasına göre o numaradaki değeri döndürür o slotta olan silah varmı?
	TSubclassOf<ABaseWeapon> GetWeaponClass(uint8 WeaponNumber); // gelen weapon numbera göre silah classını döndürür.

	UPROPERTY(ReplicatedUsing=OnRep_CurrentWeaponOnHand,BlueprintReadOnly)
	ABaseWeapon* CurrentWeaponOnHand;
	UFUNCTION()
	void OnRep_CurrentWeaponOnHand(); // HUD works

	UPROPERTY(VisibleAnywhere,Replicated)
	ABaseWeapon* WeaponSlot1;
	
	UPROPERTY(VisibleAnywhere,Replicated)
	ABaseWeapon* WeaponSlot2;

	UPROPERTY(VisibleAnywhere,Replicated)
	ABaseWeapon* WeaponSlot3;

	// TODO Grenade slot

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName RiflePKRightHandSocket = FName("Weapon_R"); // rifle pistol ve knife bu sockette

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName C4HandSocket = FName("C4HandSocket"); 

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName RifleBackSocketName = FName("BackRifleSocket");

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName PistolBackSocketName = FName("BackPistolSocket");

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName KnifeBackSocketName = FName("BackKnifeSocket");

	FName GetBackSocketName (uint8 SlotNumber) const ; // Verilen silah slotu bilgisine göre silahın meshte nereye attachlenecei

	/** Server weaponu swaplayınca currentweapononhand değişecek bizde onu kullanarak currenweapon u clientlara attach
	 * edebilirz
	 */
	UFUNCTION(Server,Reliable)
	void Server_SwapWeapon(uint8 SlotNumber);

	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_SwapWeapon(uint8 SlotNumber);
	
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand(); // ABP de animnotify ile kullanacağız
	void AttachCurrentWeaponToBack(ABaseWeapon* WeaponToAttach,const FName& SocketName); // Weaponun character meshteki slot namesi //TODO
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_AttachWeaponToBack(ABaseWeapon* WeaponToAttach,const FName& SocketName);

	/**
	 * WEAPON CLASSES
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> Mac10WeaponClass; // WeaponNumber = 1 

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> M3WeaponClass; // 2

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> Ak47WeaponClass; // 3 

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> M4A1WeaponClass; // 4

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> AWPWeaponClass; // 5

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> RocketLauncherWeaponClass; // 6

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<ABaseWeapon> PistolWeaponClass; // 7


	/**
	 * FIRING
	 */

	bool bFireButtonPressed = false;
	void FirstFiredButtonPressed();
	void FireButtonPressed(bool bPressed);

	void TraceUnderCrosshair(FHitResult& OutHitResult);

	/** STATS */

	UPROPERTY(ReplicatedUsing=OnRep_Health,VisibleAnywhere,BlueprintReadOnly)
	float Health = 100;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float MaxHealth = 100;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(ReplicatedUsing=OnRep_Shield,VisibleAnywhere,BlueprintReadOnly)
	float Shield = 0;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float MaxShield = 100;
	UFUNCTION()
	void OnRep_Shield();

	/**
	 * DROP AND TAKE MECHANIC
	 */

	/*UPROPERTY()
	ABaseWeapon* OverlappingWeapon;*/

	void DropCurrentWeaponOnHand(); // Sadece ilk slottaki weapon droplanabilir
	UFUNCTION(Server,Reliable)
	void Server_DropCurrentWeaponOnHand();
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_DropCurrentWeaponOnHand();

	void TakeWeaponFromGround(ABaseWeapon* WeaponToTake); // Sadece ilk slottaki weapon yoksa ilk slota weapon alınabilir.
	UFUNCTION(Server,Reliable)
	void Server_TakeWeaponFromGround(ABaseWeapon* WeaponToTake);
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_TakeWeaponFromGround(ABaseWeapon* WeaponToTake);

	void DropFirstWeapon();
	UFUNCTION(Server,Reliable)
	void Server_DropFirstWeapon();
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_DropFirstWeapon();

	void DropThirdWeapon();
	UFUNCTION(Server,Reliable)
	void Server_DropThirdWeapon();
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_DropThirdWeapon();

	/**
	 * C4 WORKS
	 */

	/** Bir alan olsun o alana girince true alandan çıkınca false yapalım ve sadece serverda o alanın overlapını tetikleyelim*/
	UPROPERTY(Replicated,BlueprintReadWrite)
	bool bCanPlantBomb = false;

	void DefuseTrace(const FHitResult& OutHitResult1, const FVector& WorldLocation, const FVector& WorldDirection, AHANEDANPlayerController* L_PController);
	UFUNCTION(Server,Reliable)
	void Server_DefuseTrace(const FHitResult& OutHitResult1, const FVector& WorldLocation, const FVector& WorldDirection, AHANEDANPlayerController* L_PController);
	void GetWorldLocationAndRotationForDefuseTrace(FVector& OutWorldLocation, FVector& OutWorldRotation);

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 DefuseVectorLengthMultiplyValue = 200;

	UPROPERTY(BlueprintReadWrite,Replicated)
	FTransform PlantedBombTransfrom;

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_SetPlantedBombTransform(const FTransform& Transform);

	
};
