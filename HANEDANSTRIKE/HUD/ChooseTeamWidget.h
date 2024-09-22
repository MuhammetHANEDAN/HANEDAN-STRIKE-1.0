// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChooseTeamWidget.generated.h"

class AHANEDANPlayerController;
class UButton;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UChooseTeamWidget : public UUserWidget // Widget create ettikten sonra setupı çağır
{
	GENERATED_BODY()

protected:
	
	public:

	virtual bool Initialize() override;

	void Setup(); /** Widgeti create ettikten sonra çağırın*/
	void TearDown();

	UPROPERTY()
	AHANEDANPlayerController* HPlayerController;

	UPROPERTY(meta=(BindWidget))
	UButton* CTButton;
	UFUNCTION()
	void CTButtonClicked();
	
	UPROPERTY(meta=(BindWidget))
	UButton* TButton;
	UFUNCTION()
	void TButtonClicked();

	/**
	 * Spawn 
	 */

	/** RPC yi bunun içinden göndereceğiz ki çalışsın çünkü çoğaltılmış bir nesne gerek RPC için bildiğimiz kadarıyla*/
	void HanedanPlayerControllerRequestSpawnCharacter();

};
