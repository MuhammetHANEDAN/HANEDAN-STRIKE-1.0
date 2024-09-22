// Fill out your copyright notice in the Description page of Project Settings.


#include "HanedanAnimInstance.h"

#include "HANEDANCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UHanedanAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	HanedanCharacter = Cast<AHANEDANCharacter>(TryGetPawnOwner());
	if(HanedanCharacter)
	{
		HCombatComponent = HanedanCharacter->CombatComponent;
	}
	if(HanedanCharacter && HanedanCharacter->GetCharacterMovement())
	{
		CharacterMovementComponent = HanedanCharacter->GetCharacterMovement();
	}
}

void UHanedanAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(HanedanCharacter == nullptr)
	{
		HanedanCharacter = Cast<AHANEDANCharacter>(TryGetPawnOwner());
		if(HanedanCharacter && HanedanCharacter->GetCharacterMovement())
		{
			CharacterMovementComponent = HanedanCharacter->GetCharacterMovement();
		}
	}
	if(HanedanCharacter == nullptr) return;

	if(CharacterMovementComponent && HCombatComponent)
	{
		CurrentPlayerState = HanedanCharacter->CurrentPlayerState;
		CurrentWeaponType = HanedanCharacter->CurrentWeaponType;
		GroundSpeed=UKismetMathLibrary::VSizeXY(CharacterMovementComponent->Velocity);
		bIsFalling=CharacterMovementComponent->IsFalling();
		bIsAccelerating = CharacterMovementComponent->GetCurrentAcceleration().Size() > 0.f ? true : false ;
		Direction=CalculateDirection(CharacterMovementComponent->Velocity,HanedanCharacter->GetActorRotation());
		bWeaponOnHand = HCombatComponent->bWeaponOnHand;
		bAiming = HCombatComponent->bAiming;
	}
}







