// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HANEDANSTRIKE/Enums/Enums.h"
#include "HanedanAnimInstance.generated.h"

class UCharacterMovementComponent;
class AHANEDANCharacter;
class UCombatComponent;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UHanedanAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


	
	UPROPERTY(BlueprintReadWrite)
	AHANEDANCharacter* HanedanCharacter;

	UPROPERTY(BlueprintReadWrite)
	UCombatComponent* HCombatComponent;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	UCharacterMovementComponent* CharacterMovementComponent;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	EPlayerState CurrentPlayerState = EPlayerState::EPS_Unoccupied;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	EWeaponType CurrentWeaponType = EWeaponType::EWT_Unarmed;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	float GroundSpeed; 

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	bool bIsFalling;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	float Direction;

	UPROPERTY(BlueprintReadWrite,Category=Movement)
	bool bWeaponOnHand = false;
	
	UPROPERTY(BlueprintReadWrite,Category=Movement)
	bool bAiming;
	
};
