// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "HANEDANSTRIKE/Enums/Enums.h"
#include "HANEDANSTRIKE/Interfaces/HitInterface.h"
#include "HANEDANCharacter.generated.h"

class UC4Progress;
class AC4;
class AHANEDANPlayerState;
class ABaseWeapon;
class UCombatComponent;
class AHANEDANPlayerController;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class HANEDANSTRIKE_API AHANEDANCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AHANEDANCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void GetHit(FName BoneName, float Damage, float HeadShotDamage, AActor* DamageCauserActor, AController* DamageCauserController) override;
	// Take damageye gelen damage sadece radyal damagelerden gelecektir.
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void CharacterElimned(AController* DeadController, AController* KillerController); // Damage işlemleri Serverde olduğu için Serverda çağırılacağı kesindir.

	/**
	 * REFRESH WORKS
	 */
	void RefreshCharacter();
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_RefreshHandle(); // Refresh atarken bazı fonksiyonlar replicate olmadığından multicast ile hallettim

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/**
	 * UnCategorized
	 */

	UPROPERTY(Replicated)
	AHANEDANPlayerController* HPlayerController;
	
	AHANEDANPlayerController* GetHanedanPlayerController() const ;

	void SetCharacterForbWeaponOnHand(bool OnHand);
	UFUNCTION(Server,Reliable)
	void Server_SetCharacterForbWeaponOnHand(bool OnHand);
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_SetCharacterForbWeaponOnHand(bool OnHand);

	UPROPERTY(ReplicatedUsing=OnRep_IsAlive)
	bool bIsAlive = true;
	UFUNCTION()
	void OnRep_IsAlive(bool LastValue);

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	UAnimMontage* DeathMontage;

	/**
	 * FOR ANIM INSTANCE
	 */
	
	UPROPERTY(Replicated)
	EPlayerState CurrentPlayerState = EPlayerState::EPS_Unoccupied;

	UPROPERTY(BlueprintReadOnly)
	EWeaponType CurrentWeaponType = EWeaponType::EWT_Unarmed;
	

	/**
	 * Components
	 */

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UCombatComponent* CombatComponent;
	
	/**
	 * Input
	 */

	void TrySetupInput();
	FTimerHandle TrySetupInputTimerHandle;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Jumping Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** BuyMenu Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* OpenBuyMenuAction;
	void I_OpenBuMenu();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;
	void I_Reload();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchToWeaponSlot1Action;
	void I_SwitchToWeaponSlot1();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchToWeaponSlot2Action;
	void I_SwitchToWeaponSlot2();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchToWeaponSlot3Action;
	void I_SwitchToWeaponSlot3();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchToWeaponSlot4Action;
	void I_SwitchToWeaponSlot4();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AimingAction;
	void I_AimingButtonPressed();
	void I_AimingButtonReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireButtonAction;
	void I_FireButtonPressed();
	void I_FireButtonReleased();
	void I_FirstFired();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DropWeaponAction;
	void I_DropWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FButtonAction;
	void I_FButtonStarted();
	void I_FButtonReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* QButtonAction;
	void I_QButtonStarted();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EButtonAction;
	void I_EButtonStarted();
	

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	UPROPERTY(Replicated)
	bool DisableMoveAction = false;

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	/** 
	 * Anim Montages
	 */

	void PlayMontage(UAnimMontage* Montage);

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UAnimMontage* RifleFireIronsightMontage;

	AHANEDANPlayerState* GetHanedanPlayerState() const;

	/**
	 * WEAPON PROPERTIES
	 */

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float DefaultFOV = 0;

	void SetCameraFOV(float NewFov);
	void SetCameraFOVtoDefault();

	/**
	 * C4 BOMB
	 */
	
	UPROPERTY(Replicated)
	bool bWasDefusing = false;
	UPROPERTY(Replicated)
	AC4* DefusingBomb = nullptr;

	UPROPERTY(Replicated)
	bool bWasPlanting = false;
	UPROPERTY(Replicated)
	AC4* PlantingBomb = nullptr;
	
	UPROPERTY()
	UC4Progress* C4ProgressWidgetReference;

	

	

};
