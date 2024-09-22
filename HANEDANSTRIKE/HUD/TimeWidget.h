// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimeWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UTimeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TimeText;

	void SetTimeText(FText Text);
	
};
