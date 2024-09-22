// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"

#include "Components/TextBlock.h"


void UCharacterOverlay::SetHealthText(uint8 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	HealthText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetShieldText(uint8 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	ShieldText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetWeaponAmmoText(uint8 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	WeaponAmmoText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetAmmoText(uint16 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	AmmoText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetMoneyText(uint16 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	MoneyText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetCTPointText(uint8 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	CTPointText->SetText(FText::FromString(TextS));
}

void UCharacterOverlay::SetTPointText(uint8 Value)
{
	FString TextS = FString::Printf(TEXT("%u"),Value);
	TPointText->SetText(FText::FromString(TextS));
}

