// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANGameMode.h"

#include "GameFramework/CheatManager.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Kismet/GameplayStatics.h"


AHANEDANGameMode::AHANEDANGameMode()
{
	bDelayedStart = false;
}

void AHANEDANGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetRealTimeSeconds();

}

void AHANEDANGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	ServerMapTime = GetWorld()->GetRealTimeSeconds()-LevelStartingTime;

	/*if(MatchState == MatchState::StartingState)
	{
		if(bStartingSectionFinished) SetMatchState(MatchState::PurchasingState);
		StartingStateTimeTrack += DeltaSeconds;
		if(StartingStateTimeTrack >= StartingStateTimeAmount)
		{
			bStartingSectionFinished = true;
			StartingStateTimeTrack = 0.f;
			SetMatchState(MatchState::PurchasingState);
		}
	}
	if(MatchState == MatchState::PurchasingState)
	{
		PurchasingStateTimeTrack += DeltaSeconds;
		if(PurchasingStateTimeTrack >= PurchasingStateTimeAmount)
		{
			PurchasingStateTimeTrack = 0;
			SetMatchState(MatchState::RoundState);
		}
	}

	if(MatchState == MatchState::RoundState)
	{
		RoundStateTimeTrack += DeltaSeconds;
		if(RoundStateTimeTrack >= RoundStateTimeAmount)
		{
			RoundStateTimeTrack = 0;
			SetMatchState(MatchState::PurchasingState);
			// TODO Raund bitikten sonra ne olacak veya karakter raund bitmeden ölürse ne olacak
		}
		
	}*/
	
}

void AHANEDANGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	/*if (MatchState == MatchState::StartingState)
	{
		
	}
	else if (MatchState == MatchState::PurchasingState)
	{
		
	}
	else if (MatchState == MatchState::RoundState)
	{
		
	}*/
	
}

void AHANEDANGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	GetWorldTimerManager().SetTimer(SetMatchStateTimerHandle,this,&AHANEDANGameMode::SetMatchStateToStartingState,2.f);
	//SetMatchState(MatchState::StartingState);
}

void AHANEDANGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

void AHANEDANGameMode::SetMatchStateToStartingState()
{
	//SetMatchState(MatchState::StartingState);
}


