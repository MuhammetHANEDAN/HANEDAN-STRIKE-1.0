// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HANEDANGameState.generated.h"

class AHANEDANPlayerController;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANGameState : public AGameState
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<AHANEDANPlayerController*> CtPlayerControllers;
	

	UPROPERTY()
	TArray<AHANEDANPlayerController*> TPlayerControllers;
	
};
