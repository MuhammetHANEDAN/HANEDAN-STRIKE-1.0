// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANGameStateBase.h"

#include "HANEDANSTRIKE/HUD/CharacterOverlay.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "HANEDANSTRIKE/HUD/HANEDANHUD.h"
#include "Net/UnrealNetwork.h"

AHANEDANGameStateBase::AHANEDANGameStateBase()
{
	
}

void AHANEDANGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

