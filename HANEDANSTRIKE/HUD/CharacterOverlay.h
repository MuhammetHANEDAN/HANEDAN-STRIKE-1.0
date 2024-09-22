// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;
	void SetHealthText(uint8 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText;
	void SetShieldText(uint8 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponAmmoText;
	void SetWeaponAmmoText(uint8 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AmmoText;
	void SetAmmoText(uint16 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MoneyText;
	void SetMoneyText(uint16 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CTPointText;
	void SetCTPointText(uint8 Value);

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TPointText;
	void SetTPointText(uint8 Value);






	
	
};
