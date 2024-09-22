// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuyMenuWidget.generated.h"

class AHANEDANPlayerController;
class ABaseWeapon;
class UButton;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UBuyMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual bool Initialize() override;

	UPROPERTY()
	AHANEDANPlayerController* HPlayerController;

	UPROPERTY(meta=(BindWidget))
	UButton* MACButton;
	UFUNCTION()
	void MACButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* M3Button;
	UFUNCTION()
	void M3ButtonPressed();
	
	UPROPERTY(meta=(BindWidget))
	UButton* Ak47Button;
	UFUNCTION()
	void Ak47ButtonPressed();
	
	UPROPERTY(meta=(BindWidget))
	UButton* M4A1Button;
	UFUNCTION()
	void M4A1ButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* AWPButton;
	UFUNCTION()
	void AWPButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* RocketLauncherButton;
	UFUNCTION()
	void RocketLauncherButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* KevlarHelmetButton;
	UFUNCTION()
	void KevlarHelmetButtonPressed();
	
	UPROPERTY(meta=(BindWidget))
	UButton* HGrenadeButton;
	UFUNCTION()
	void HGrenadeButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* SmokeGrenadeButton;
	UFUNCTION()
	void SmokeGrenadeButtonPressed();

	UPROPERTY(meta=(BindWidget))
	UButton* FlashbangButton;
	UFUNCTION()
	void FlashbangButtonPressed();

	
	
};
