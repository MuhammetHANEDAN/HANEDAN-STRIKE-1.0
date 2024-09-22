// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyMenuWidget.h"

#include "Components/Button.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"

bool UBuyMenuWidget::Initialize()
{
	HPlayerController = Cast<AHANEDANPlayerController>(GetOwningPlayer());
	
	if(MACButton == nullptr) Super::Initialize();
	if(!MACButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::MACButtonPressed))
	{
		MACButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::MACButtonPressed);
	}

	if(!M3Button->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::M3ButtonPressed))
	{
		M3Button->OnClicked.AddDynamic(this,&UBuyMenuWidget::M3ButtonPressed);
	}

	if(!Ak47Button->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::Ak47ButtonPressed))
	{
		Ak47Button->OnClicked.AddDynamic(this,&UBuyMenuWidget::Ak47ButtonPressed);
	}
	
	if(!M4A1Button->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::M4A1ButtonPressed))
	{
		M4A1Button->OnClicked.AddDynamic(this,&UBuyMenuWidget::M4A1ButtonPressed);
	}

	if(!AWPButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::AWPButtonPressed))
	{
		AWPButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::AWPButtonPressed);
	}

	if(!RocketLauncherButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::RocketLauncherButtonPressed))
	{
		RocketLauncherButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::RocketLauncherButtonPressed);
	}

	if(!KevlarHelmetButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::KevlarHelmetButtonPressed))
	{
		KevlarHelmetButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::KevlarHelmetButtonPressed);
	}

	if(!HGrenadeButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::HGrenadeButtonPressed))
	{
		HGrenadeButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::HGrenadeButtonPressed);
	}
	
	if(!SmokeGrenadeButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::SmokeGrenadeButtonPressed))
	{
		SmokeGrenadeButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::SmokeGrenadeButtonPressed);
	}

	if(!FlashbangButton->OnClicked.IsAlreadyBound(this,&UBuyMenuWidget::FlashbangButtonPressed))
	{
		FlashbangButton->OnClicked.AddDynamic(this,&UBuyMenuWidget::FlashbangButtonPressed);
	}

	return Super::Initialize();
}

void UBuyMenuWidget::MACButtonPressed()
{
	HPlayerController->RequestBuyWeapon(1400,1,1);
}

void UBuyMenuWidget::M3ButtonPressed()
{
	HPlayerController->RequestBuyWeapon(1700,1,2);
}

void UBuyMenuWidget::Ak47ButtonPressed()
{
	HPlayerController->RequestBuyWeapon(2500,1,3);
}

void UBuyMenuWidget::M4A1ButtonPressed()
{
	HPlayerController->RequestBuyWeapon(3100,1,4);
}

void UBuyMenuWidget::AWPButtonPressed()
{
	HPlayerController->RequestBuyWeapon(4750,1,5);
}

void UBuyMenuWidget::RocketLauncherButtonPressed()
{
	HPlayerController->RequestBuyWeapon(6000,1,6);
}

void UBuyMenuWidget::KevlarHelmetButtonPressed()
{
	HPlayerController->RequestBuyKevlarHelmet();
}

void UBuyMenuWidget::HGrenadeButtonPressed()
{
	
}

void UBuyMenuWidget::SmokeGrenadeButtonPressed()
{
	
}

void UBuyMenuWidget::FlashbangButtonPressed()
{
	
}

