// Fill out your copyright notice in the Description page of Project Settings.


#include "StateToStateNotifyState.h"

#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"

void UStateToStateNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if(AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(MeshComp->GetOwner()))
	{
		if(HCharacter->HasAuthority())
		{
			if(HCharacter->CurrentPlayerState != NewState)
			{
				HCharacter->CurrentPlayerState=NewState;
			}
		}
	}
}

void UStateToStateNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if(AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(MeshComp->GetOwner()))
	{
		if(HCharacter->HasAuthority())
		{
			if(HCharacter->CurrentPlayerState != EndState)
			{
				HCharacter->CurrentPlayerState=EndState;
			}
		}
	}
}


