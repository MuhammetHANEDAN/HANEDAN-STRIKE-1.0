// Fill out your copyright notice in the Description page of Project Settings.


#include "ChooseTeamWidget.h"

#include "Components/Button.h"
#include "HANEDANSTRIKE/HANEDANSTRIKEGameModeBase.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "HANEDANSTRIKE/PlayerStates/HANEDANPlayerState.h"

bool UChooseTeamWidget::Initialize()
{
	HPlayerController = Cast<AHANEDANPlayerController>(GetOwningPlayer());
	return Super::Initialize();
}

void UChooseTeamWidget::Setup()
{
	SetIsFocusable(true);
	
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(GetOwningPlayer()) : HPlayerController;
	if(HPlayerController)
	{
		FInputModeUIOnly InputModeData; 
		InputModeData.SetWidgetToFocus(TakeWidget());
		HPlayerController->SetInputMode(InputModeData);
		HPlayerController->SetShowMouseCursor(true);
	}
	
	if(!CTButton->OnClicked.IsAlreadyBound(this,&UChooseTeamWidget::CTButtonClicked))
	{
		CTButton->OnClicked.AddDynamic(this,&UChooseTeamWidget::CTButtonClicked);
	}
	if(!TButton->OnClicked.IsAlreadyBound(this,&UChooseTeamWidget::TButtonClicked))
	{
		TButton->OnClicked.AddDynamic(this,&UChooseTeamWidget::TButtonClicked);
	}
	
}

void UChooseTeamWidget::TearDown()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(GetOwningPlayer()) : HPlayerController;
	if(HPlayerController)
	{
		FInputModeGameOnly InputModeData; 
		HPlayerController->SetInputMode(InputModeData);
		HPlayerController->SetShowMouseCursor(false);
	}
	RemoveFromParent();
}

void UChooseTeamWidget::CTButtonClicked()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(GetOwningPlayer()) : HPlayerController;
	if(HPlayerController == nullptr) return;
	AHANEDANPlayerState* HPlayerState = HPlayerController->GetPlayerState<AHANEDANPlayerState>();
	if(HPlayerState)
	{
		HPlayerState->ServerSetCTorT(true);
		HPlayerController->ServerSetHPC_bCtOrT(true);
	}
	HanedanPlayerControllerRequestSpawnCharacter();
	TearDown();
}

void UChooseTeamWidget::TButtonClicked()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(GetOwningPlayer()) : HPlayerController;
	if(HPlayerController == nullptr) return;
	AHANEDANPlayerState* HPlayerState = HPlayerController->GetPlayerState<AHANEDANPlayerState>();
	if(HPlayerState)
	{
		HPlayerState->ServerSetCTorT(false);
		HPlayerController->ServerSetHPC_bCtOrT(false);
	}
	HanedanPlayerControllerRequestSpawnCharacter();
	TearDown();
}

void UChooseTeamWidget::HanedanPlayerControllerRequestSpawnCharacter()
{
	if(HPlayerController == nullptr) return;
	HPlayerController->Server_ClientPCRequestSpawnCharacter(HPlayerController);
}

