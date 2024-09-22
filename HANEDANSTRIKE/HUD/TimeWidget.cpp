// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeWidget.h"

#include "Components/TextBlock.h"

void UTimeWidget::SetTimeText(FText Text)
{
	TimeText->SetText(Text);
}
