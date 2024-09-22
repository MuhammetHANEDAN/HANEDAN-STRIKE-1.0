// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4Progress.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UC4Progress : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta=(BindWidget))
	UProgressBar* C4ProgressBar;
	
};
