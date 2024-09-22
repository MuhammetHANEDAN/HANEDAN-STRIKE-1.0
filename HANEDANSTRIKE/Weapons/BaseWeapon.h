// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HANEDANSTRIKE/Enums/Enums.h"
#include "BaseWeapon.generated.h"

class UCapsuleComponent;
class AHANEDANCharacter;
class AHANEDANPlayerController;
class USoundCue;

UCLASS()
class HANEDANSTRIKE_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ABaseWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	virtual void BeginPlay() override;

public:
	
	/**
	 * COMPONENTS
	 */
	
	UPROPERTY(VisibleAnywhere,Category="Components",Replicated)
	USkeletalMeshComponent* WeaponSkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere,Category="Components")
	UCapsuleComponent* WeaponOverlapCapsuleComponent;

	

	/**
	 * FREE SPACE
	 */

	UPROPERTY(Replicated)
	AHANEDANCharacter* LastOwnerHCharacter;

	UPROPERTY(Replicated)
	AHANEDANCharacter* OwnerHCharacter;
	
	UPROPERTY(Replicated)
	AHANEDANPlayerController* HPlayerController;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	uint8 WeaponSlotNumber = 1; // Weaponun hangi slota ait olduğu

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FName MuzzleName; // Silahta merminin namludan çıktığı yer
	FTransform GetMuzzleTransform() const;
	FVector GetMuzzleLocation() const;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UParticleSystem* MuzzleFireEffect;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float HeadShotDamage = 50.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UAnimationAsset* WeaponFireAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	USoundBase* FireSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	USoundBase* EquippingSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UAnimMontage* EquippingAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UAnimMontage* DisEquippingAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UAnimMontage* ReloadMontage;

	/**
	 * FIRING
	 */

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	bool bAutomatic = false;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float OtoFireRate = 0.15f;
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float OneShotFireRate = 0.25f;
	
	bool bFiring = false;

	
	virtual void FirstFiredPressed(FHitResult& OutHitResult); // input fire started çağıracak
	virtual void FireButtonPressed(bool bPressed, FHitResult& OutHitResult);
	
	virtual void WeaponFire(FHitResult& OutHitResult);
	virtual void LocalFireEffects();
	
	UFUNCTION(Server,Reliable)
	virtual void ServerWeaponFire(const FHitResult& OutHitResult);
	UFUNCTION(NetMulticast,Reliable)
	virtual void Multicast_WeaponFire();

	virtual void WeaponFireAmmo(const FHitResult& OutHitResult); // Weaponun mermiyi ateşlediği fonksiyon
	
	bool bFiringTriggered = false;
	float FireTrigeredTimeTrack = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	bool FirstFire = true; // İlk kurşun sıkıldımı startedla çağırılıcak riflelarda tek sıkım için
	float FirstFireTimeTrack = 0.f;
	
	bool bFireButtonReleased = true; // tekli için kullanabilirz

	bool bOneShotted = false;
	float bOneShotTimeTrack = 0; // one shot fire rateden daha fazla olunca one shot silah fire alabilsin.

	/**
	 * AMMO
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 MaxWeaponAmmo = 30;
	UPROPERTY(ReplicatedUsing=OnRep_WeaponAmmo,EditDefaultsOnly,Category="Properties")
	uint16 WeaponAmmo = 30; // Silahda bulunan ammo
	UFUNCTION()
	void OnRep_WeaponAmmo(); // HUD works

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 MaxAmmo = 120;
	UPROPERTY(ReplicatedUsing=OnRep_Ammo,EditDefaultsOnly,Category="Properties")
	uint16 Ammo = 120; // Taşınan ammo
	UFUNCTION()
	void OnRep_Ammo(); // HUD works

	virtual void SpendAmmo();

	bool IsWeaponFull() const;
	bool IsWeaponEmpty() const;

	/**
	 * RELOAD
	 */

	UFUNCTION(Server,Reliable)
	virtual void Server_PlayReloadMontage(); // her yerde reload animini başlatacak ama reload işlemini serverda yapacak.
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_PlayReloadMontage();
	UFUNCTION(BlueprintCallable)
	virtual void Reload(); // Anim notifydan çağıralacak
	
	/**
	 * WEAPON STATE
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	EWeaponType WeaponType;
	
	void HandleWeaponDropped(); // Weapon droplanınca ne olacak
	bool bWeaponOnGround = false;
	
	void HandleWeaponTakenFromGround(); // Yerde droplanmış şekilde duran silahın üzerine oyuncu gelince ne olacak
	
	UFUNCTION()
	virtual void PlayerOverlappingWeapon(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/**
	 * AWP SCOPE
	 */

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float ScopeX1FOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float ScopeX2FOV = 30.f;

	UPROPERTY()
	uint8 ScopeX = 0;

	//void Scope(); // ScopeX i 1 artıracak 1. ve 2. olacak ScopeX 3 olduğunda close scope gelecek
	UFUNCTION()
	void Server_Scope();
	
	UFUNCTION(Client,Reliable)
	void Client_Scope(uint8 LScopeX);
	
	void CloseScope();

	/**
	 * RECOIL
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	bool bRecoilWeapon = false;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float RecoilAmount = 0.1;

	void Recoil();










	
};
