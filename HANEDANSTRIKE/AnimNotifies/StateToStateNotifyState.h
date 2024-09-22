// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HANEDANSTRIKE/Enums/Enums.h"
#include "StateToStateNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API UStateToStateNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category="Properties")
	EPlayerState NewState;
	UPROPERTY(EditAnywhere, Category="Properties")
	EPlayerState EndState;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
