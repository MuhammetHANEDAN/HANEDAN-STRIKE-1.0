// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANPlayerState.h"

#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Net/UnrealNetwork.h"

void AHANEDANPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHANEDANPlayerState,bCTorT);
	DOREPLIFETIME_CONDITION(AHANEDANPlayerState,Money,COND_OwnerOnly);
	DOREPLIFETIME(AHANEDANPlayerState,PlayerScore);
}

void AHANEDANPlayerState::ServerSetCTorT_Implementation(bool bCT)
{
	bCTorT = bCT;
}

void AHANEDANPlayerState::OnRep_bCTorT()
{
	if(bCTorT == false)
	{
		if(GetPlayerController() == nullptr) return;
		if(GetPlayerController()->ActorHasTag(FName("CT")))
		{
			GetPlayerController()->Tags.Remove(FName("CT"));
		}
		GetPlayerController()->Tags.AddUnique(FName("T"));
	}
}

void AHANEDANPlayerState::OnRep_Money()
{
	AHANEDANPlayerController* HPlayerController = Cast<AHANEDANPlayerController>(GetPlayerController());
	if(HPlayerController)
	{
		HPlayerController->SetCharacterOverlayMoneyText(Money);
	}
}

void AHANEDANPlayerState::AddPointToPlayerScore()
{
	PlayerScore = PlayerScore + 1;
}

void AHANEDANPlayerState::SubstractPointToPlayerScore()
{
	PlayerScore = PlayerScore - 1;
}

void AHANEDANPlayerState::ServerSetMoney_Implementation(uint16 MoneyAmount)
{
	Money = MoneyAmount;
	AHANEDANPlayerController* HPlayerController = Cast<AHANEDANPlayerController>(GetPlayerController());
	if(HPlayerController)
	{
		HPlayerController->SetCharacterOverlayMoneyText(Money);
    }
}
