// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HANEDANGameStateBase.generated.h"

class AHANEDANPlayerController;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
	public:

	AHANEDANGameStateBase();

	UPROPERTY()
	AHANEDANPlayerController* HPlayerController;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	uint8 CTPoint = 0;
	uint8 TPoint= 0;
	
};
