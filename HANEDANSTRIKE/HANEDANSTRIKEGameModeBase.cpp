// Copyright Epic Games, Inc. All Rights Reserved.


#include "HANEDANSTRIKEGameModeBase.h"

#include "EngineUtils.h"
#include "Character/HANEDANCharacter.h"
#include "Components/CombatComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameStates/HANEDANGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerControllers/HANEDANPlayerController.h"
#include "PlayerStates/HANEDANPlayerState.h"
#include "Weapons/BaseWeapon.h"
#include "Weapons/C4.h"

namespace MatchState
{
	const FName StartingState = FName("StartingState");
	const FName PurchasingState = FName("PurchasingState");
	const FName RoundState = FName("RoundState");
	const FName RoundCooldownState = FName("RoundCooldownState");
	const FName BombState = FName("BombState");

}

AHANEDANSTRIKEGameModeBase::AHANEDANSTRIKEGameModeBase()
{
	bUseSeamlessTravel = true;
}

void AHANEDANSTRIKEGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	HanedanGameStateBase = GetGameState<AHANEDANGameStateBase>();

	FirstHPlayerController = Cast<AHANEDANPlayerController>(UGameplayStatics::GetPlayerCharacter(this,0));
	
	SetMatchState(MatchState::StartingState);
	LevelStartingTime = GetWorld()->GetRealTimeSeconds();

	GameModeTimePackage.StartingStateTimeAmount = StartingStateTimeAmount;
	GameModeTimePackage.PurchasingStateTimeAmount = PurchasingStateTimeAmount;
	GameModeTimePackage.RoundStateTimeAmount = RoundStateTimeAmount;
	GameModeTimePackage.RoundCooldownStateTimeAmount = RoundCooldownStateTimeAmount;
	GameModeTimePackage.BombExplosionTimeAmount = BombExplosionTimeAmount;
}

void AHANEDANSTRIKEGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	ServerMapTime = GetWorld()->GetRealTimeSeconds()-LevelStartingTime;

	if(MatchState == MatchState::StartingState)
	{
		if(bStartingSectionFinished) SetMatchState(MatchState::PurchasingState);
		StartingStateTimeTrack += DeltaSeconds;
		TimeRemainSeconds = StartingStateTimeAmount - StartingStateTimeTrack;
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
		TimeRemainSeconds = PurchasingStateTimeAmount - PurchasingStateTimeTrack;
		if(PurchasingStateTimeTrack >= PurchasingStateTimeAmount)
		{
			PurchasingStateTimeTrack = 0;
			SetMatchState(MatchState::RoundState); 
		}
	}

	if(MatchState == MatchState::RoundState)
	{
		RoundStateTimeTrack += DeltaSeconds;
		TimeRemainSeconds = RoundStateTimeAmount - RoundStateTimeTrack;
		if(RoundStateTimeTrack >= RoundStateTimeAmount)
		{
			RoundStateTimeTrack = 0;
			if(CtNumber >= 1)
			{
				CTWin();
			}
			else
			{
				SetMatchState(MatchState::RoundCooldownState);
			}
		}
	}
	if(MatchState==MatchState::RoundCooldownState)
	{
		RoundCooldownStateTimeTrack += DeltaSeconds;
		TimeRemainSeconds = RoundCooldownStateTimeAmount - RoundCooldownStateTimeTrack;
		if(RoundCooldownStateTimeTrack >= RoundCooldownStateTimeAmount)
		{
			RoundCooldownStateTimeTrack = 0;
			SetMatchState(MatchState::PurchasingState);
		}
	}
	if(MatchState == MatchState::BombState)
	{
		BombExplosionTimeTrack += DeltaSeconds;
		TimeRemainSeconds = BombExplosionTimeAmount - BombExplosionTimeTrack;
		if(BombExplosionTimeTrack >= BombExplosionTimeAmount)
		{
			BombExplosionTimeTrack = 0;
			if(C4)
			{
				C4->CancelDefuse(C4->HPlayerController);
				ExplodeBomb();
			}
		}
	}
}

void AHANEDANSTRIKEGameModeBase::PlayerElimned(AController* DeadController, AController* KillerController)
{
	// skor verme işlemlerini yapabilirz

	AHANEDANPlayerController* DeadHPlayerController = Cast<AHANEDANPlayerController>(DeadController);
	AHANEDANPlayerController* KillerHPlayerController = Cast<AHANEDANPlayerController>(KillerController);

	if(DeadHPlayerController && KillerHPlayerController)
	{
		DeadHPlayerController->Server_ReduceScore();
		KillerHPlayerController->Server_AddScore();
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("olmadı"));
	}
	
	if(MatchState == MatchState::StartingState)
	{
		ElimnedPlayerList.AddHead(DeadController);
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle,this,&AHANEDANSTRIKEGameModeBase::RestartOnePlayer,RestartOnePlayerTime);
	}

	else if(MatchState == MatchState::RoundState)
	{
		if(DeadHPlayerController->bCtOrT)
		{
			CtNumber--;
			if(CtNumber == 0)
			{
				TWin();
			}
		}
		else
		{
			TNumber--;
			if(TNumber == 0)
			{
				CTWin();
			}
		}
	}
	else if(MatchState == MatchState::BombState)
	{
		if(DeadHPlayerController->bCtOrT)
		{
			CtNumber--;
			if(CtNumber == 0)
			{
				if(C4)
				{
					C4->Destroy();
				}
				TWin();
			}
		}
		else
		{
			TNumber--;
			if(TNumber == 0)
			{
				// Tüm t ler ölse bile bomba defuse edilmedikçe t lerin kazanma şansı var
			}
		}
	}
	else if(MatchState == MatchState::RoundCooldownState)
	{
		// hiç bir şey yapma
	}
}

void AHANEDANSTRIKEGameModeBase::RestartOnePlayer()
{
	if(MatchState != MatchState::StartingState) return;
	UE_LOG(LogTemp,Error,TEXT("gelindi"));
	if(ElimnedPlayerList.GetTail()->GetValue())
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(ElimnedPlayerList.GetTail()->GetValue());
		AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(HanedanPlayerController->GetPawn());
		
		if(HanedanPlayerController == nullptr || HCharacter == nullptr)
		{
			ElimnedPlayerList.RemoveNode(ElimnedPlayerList.GetTail());
			UE_LOG(LogTemp,Error,TEXT("hplayer controller veya hcharacter null gelmiştir"));
			return;
		}

		if(HanedanPlayerController->bCtOrT && HCharacter)
		{
			AActor* PlayerStart = GetPlayerStartsRandomForTeams(true);
			if(PlayerStart == nullptr) return;
			HCharacter->SetActorLocationAndRotation(PlayerStart->GetActorLocation(),PlayerStart->GetActorRotation());
		}
		else if(!HanedanPlayerController->bCtOrT && HCharacter)
		{
			AActor* PlayerStart = GetPlayerStartsRandomForTeams(false);
			if(PlayerStart == nullptr) return;
			HCharacter->SetActorLocationAndRotation(PlayerStart->GetActorLocation(),PlayerStart->GetActorRotation());
		}

		HCharacter->RefreshCharacter();
		HanedanPlayerController->InitializeCharacterOverlayWidget();
		HanedanPlayerController->HideCharacterOverlay(false); // overlayı geri açıyoruz
		
		ElimnedPlayerList.RemoveNode(ElimnedPlayerList.GetTail()->GetValue());
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("ElimnedPlayerList.GetTail()->GetValue() null dur"));
	}
}

void AHANEDANSTRIKEGameModeBase::SetMatchState(FName State)
{
	if(MatchState == State) return;
	MatchState = State;
	OnMatchStateSet();
}

void AHANEDANSTRIKEGameModeBase::OnMatchStateSet()
{
	if (MatchState == MatchState::StartingState)
	{
		HandleStartingState();
	}
	else if (MatchState == MatchState::PurchasingState)
	{
		HandlePurchasingState();
	}
	else if (MatchState == MatchState::RoundState)
	{
		HandleRoundState();
	}
	else if (MatchState == MatchState::RoundCooldownState)
	{
		HandleRoundCooldownState();
	}

	else if(MatchState == MatchState::BombState)
	{
		HandleBombState();
	}
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			HanedanPlayerController->OnMatchStateSet(MatchState);
			//HanedanPlayerController->SetTimeRemainSeconds(TimeRemainSeconds); Çok hızlı gerçekleştiğinden timeremainseconds yanlış gidiyor
		}
	}
}

void AHANEDANSTRIKEGameModeBase::HandleStartingState()
{
	
}

void AHANEDANSTRIKEGameModeBase::HandlePurchasingState()
{
	if(RoundNumber == MaxRoundNumber) // eğer son roundcooldown state oynanmış ve süresi bitmişse oyunu yeniden başlatıyoruz
	{
		RestartGame();
		return;
	}

	ClearAllWeaponsOnTheGround();

	if(WaitingToRestartControllers.Num()>0)
	{
		for (auto Controller : WaitingToRestartControllers)
		{
			RestartPlayer(Controller);
		}
		WaitingToRestartControllers.Empty();
	}

	SetVisibilityAllPlayersAnnouncementWidgets(false);

	// Refresh All Characters
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(HanedanPlayerController->GetPawn());
			if(HanedanPlayerController && HCharacter)
			{
				HCharacter->RefreshCharacter();
				HanedanPlayerController->AddCharacterOverlayToHud();
				HanedanPlayerController->InitializeCharacterOverlayWidget();
				HanedanPlayerController->HideCharacterOverlay(false);
			}
		}
	}

	if(RoundNumber == 0) // TODO
	{
		SetAllPlayersMoney(StartingMoney);
		InitializeAllPlayersCharacterOverlayWidgets();
	}

	SetAllCharactersLocationsToBase();
	DisableAllCharactersMovements();
	
}

void AHANEDANSTRIKEGameModeBase::HandleRoundState()
{
	SetVisibilityAllPlayersAnnouncementWidgets(false);
	EnableAllCharactersMovements();
	RoundNumber++;
	SetAllCtAndTPlayersNumber(); // oyuna başlamış ve pawnı verilmiş tüm ct ve t sayılarını aldık
	if(C4)
	{
		C4->Destroy();
	}
	SpawnC4BombForOneTPlayer();
}

void AHANEDANSTRIKEGameModeBase::HandleRoundCooldownState()
{
	if(RoundNumber != 1)
	{
		AddMoneyToAllPlayers(AddMoneyPerRound);
	}
	InitializeAllPlayersCharacterOverlayWidgets();
}

void AHANEDANSTRIKEGameModeBase::HandleBombState()
{
	
}

void AHANEDANSTRIKEGameModeBase::HandleNewPlayerStarting(AController* NewPlayer)
{
	if(NewPlayer == nullptr) return;
	
	AHANEDANPlayerController* H_PlayerController = Cast<AHANEDANPlayerController>(NewPlayer);
	
	if(H_PlayerController)
	{
		H_PlayerController->OnMatchStateSet(MatchState);
		H_PlayerController->SetTimeRemainSeconds(TimeRemainSeconds);
		H_PlayerController->Client_AddTimeWidget();
		H_PlayerController->SetTimePackage(GameModeTimePackage);
	}
	
	if (MatchState == MatchState::StartingState)
	{
		RestartPlayer(NewPlayer); // Eğer oyuncu takımı seçtikten sonra starting statedeyse
		H_PlayerController->AddCharacterOverlayToHud();
		H_PlayerController->InitializeCharacterOverlayWidget();
		H_PlayerController->AddAnnouncementWidgetToHUD();
		H_PlayerController->SetAnnouncementWidgetVisibility(true);
		FString String = FString("Match Starting");
		H_PlayerController->SetAnnouncementText(String);
	}
	else if (MatchState == MatchState::PurchasingState) // Eğer oyuna purchasing statede girmişse
	{
		WaitingToRestartControllers.AddUnique(NewPlayer);
	}
	else if (MatchState == MatchState::RoundState) 
	{
		WaitingToRestartControllers.AddUnique(NewPlayer);
	}
	else if (MatchState == MatchState::RoundCooldownState)
	{
		WaitingToRestartControllers.AddUnique(NewPlayer);
	}
	else if (MatchState == MatchState::BombState)
	{
		WaitingToRestartControllers.AddUnique(NewPlayer);
	}
}

void AHANEDANSTRIKEGameModeBase::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if(NewPlayer->GetPlayerState<AHANEDANPlayerState>())
	{
		bool bC =NewPlayer->GetPlayerState<AHANEDANPlayerState>()->bCTorT;
		AActor* StartSpot = GetPlayerStartsRandomForTeams(bC);
		RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
	}
	else
	{
		AActor* StartSpot = nullptr;

		RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
	}
}

void AHANEDANSTRIKEGameModeBase::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if (MustSpectate(Cast<APlayerController>(NewPlayer)))
	{
		UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart: Tried to restart a spectator-only player!"));
		return;
	}

	if(NewPlayer->GetPlayerState<AHANEDANPlayerState>())
	{
		bool bC =NewPlayer->GetPlayerState<AHANEDANPlayerState>()->bCTorT;
		// Try to create a pawn to use of the default class for this player
		APawn* NewPawn = SpawnCtorTCharacter(bC,StartSpot->GetActorTransform());
		if (IsValid(NewPawn))
		{
			NewPlayer->SetPawn(NewPawn);
			if(bC) // Controllera Ct tagı veriyoruzki damage işlemlerinde daha optimize olalım
			{
				CtPlayerControllers.AddUnique(NewPlayer);
				NewPlayer->Tags.AddUnique(FName("CT"));
			}
			else // Controllera t tagı veriyoruzki damage işlemlerinde daha optimize olalım
			{
				TPlayerControllers.AddUnique(NewPlayer);
				NewPlayer->Tags.AddUnique(FName("T"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("pawn oluşturulamadı gmode cpp 117"));
		}
	
		if (!IsValid(NewPlayer->GetPawn()))
		{
			FailedToRestartPlayer(NewPlayer);
		}
		else
		{
			// Tell the start spot it was used
			InitStartSpot(StartSpot, NewPlayer);
			FRotator SpawnRotation = StartSpot->GetActorRotation();

			FinishRestartPlayer(NewPlayer, SpawnRotation);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState valid olmadığı için spawn yapılamadı Gmode cpp 135"));
	}
	
}

void AHANEDANSTRIKEGameModeBase::RestartGame()
{
	GetWorld()->ServerTravel(MatchMapURL);
}

void AHANEDANSTRIKEGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	/*Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	// If players should start as spectators, leave them in the spectator state
	if (!bStartPlayersAsSpectators && !MustSpectate(NewPlayer) && PlayerCanRestart(NewPlayer))
	{
		// Otherwise spawn their pawn immediately
		RestartPlayer(NewPlayer);
	}*/
}

void AHANEDANSTRIKEGameModeBase::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

void AHANEDANSTRIKEGameModeBase::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);
	UE_LOG(LogTemp, Error, TEXT("failedtorestartplayer girildi"));
}

void AHANEDANSTRIKEGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	if(MatchState == MatchState::RoundState)
	{
		if(GetAllHPLayerControllers().Num() == 0 || GetAllHPLayerControllers().Num() == 1 ) return;
		AHANEDANPlayerController* LPlayerController = Cast<AHANEDANPlayerController>(Exiting);
		if(LPlayerController)
		{
			if(LPlayerController->bCtOrT)
			{
				CtNumber--;
				if(CtNumber == 0)
				{
					TWin();
				}
			}
			else
			{
				TNumber--;
				if(TNumber == 0)
				{
					CTWin();
				}
			}
		}
	}
}

void AHANEDANSTRIKEGameModeBase::GameModeRestartPlayer(APlayerController* L_PlayerController) 
{
	HandleNewPlayerStarting(L_PlayerController);
	//RestartPlayer(L_PlayerController);
}

AHANEDANCharacter* AHANEDANSTRIKEGameModeBase::SpawnCtorTCharacter(bool bCt, const FTransform SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	AHANEDANCharacter* ResultCharacter = nullptr;
	if(bCt)
	{
		ResultCharacter = GetWorld()->SpawnActor<AHANEDANCharacter>(CtCharacterClass, SpawnTransform, SpawnInfo);
	}
	else
	{
		ResultCharacter = GetWorld()->SpawnActor<AHANEDANCharacter>(TCharacterClass, SpawnTransform, SpawnInfo);
	}
	
	return ResultCharacter;
}

AActor* AHANEDANSTRIKEGameModeBase::GetPlayerStartsRandomForTeams(bool bCt)
{
	TArray<AActor*> CtPlayerStarts;
	TArray<AActor*> TPlayerStarts;
	
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		
		if (PlayerStart)
		{
			if(PlayerStart->ActorHasTag("CT"))
			{
				CtPlayerStarts.AddUnique(PlayerStart);
			}
			else if(PlayerStart->ActorHasTag("T"))
			{
				TPlayerStarts.AddUnique(PlayerStart);
			}
		}
	}
	if (bCt && CtPlayerStarts.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, CtPlayerStarts.Num() - 1);
		return CtPlayerStarts[Selection];
	}
	else if (!bCt && TPlayerStarts.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, TPlayerStarts.Num() - 1);
		return TPlayerStarts[Selection];
	}
	else
	{
		return nullptr;
	}
}

void AHANEDANSTRIKEGameModeBase::SetAllCharactersLocationsToBase()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(HanedanPlayerController->GetPawn());
			if(HanedanPlayerController->bCtOrT && HCharacter)
			{
				AActor* PlayerStart = GetPlayerStartsRandomForTeams(true);
				if(PlayerStart == nullptr) return;
				HCharacter->SetActorLocationAndRotation(PlayerStart->GetActorLocation(),PlayerStart->GetActorRotation());
			}
			else if(!HanedanPlayerController->bCtOrT && HCharacter)
			{
				AActor* PlayerStart = GetPlayerStartsRandomForTeams(false);
				if(PlayerStart == nullptr) return;
				HCharacter->SetActorLocationAndRotation(PlayerStart->GetActorLocation(),PlayerStart->GetActorRotation());
			}
		}
	}
}

void AHANEDANSTRIKEGameModeBase::DisableAllCharactersMovements()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(HanedanPlayerController->GetPawn());
			if(HCharacter == nullptr) return;
			HCharacter->DisableMoveAction = true;
		}
	}
}

void AHANEDANSTRIKEGameModeBase::EnableAllCharactersMovements()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(HanedanPlayerController->GetPawn());
			if(HCharacter == nullptr) return;
			HCharacter->DisableMoveAction = false;
		}
	}
}

void AHANEDANSTRIKEGameModeBase::SetAllPlayersMoney(uint16 NewMoneyAmount)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			if(AHANEDANPlayerState* HPlayerState = Cast<AHANEDANPlayerState>(HanedanPlayerController->PlayerState))
			{
				HPlayerState->ServerSetMoney(NewMoneyAmount);
			}
		}
	}
}

void AHANEDANSTRIKEGameModeBase::AddMoneyToAllPlayers(uint16 MoneyToAdd)
{
	TArray<AHANEDANPlayerController*> Array =GetAllHPLayerControllersHavePawn();
	for (auto Element : Array)
	{
		if(Element)
		{
			if(AHANEDANPlayerState* HPlayerState = Cast<AHANEDANPlayerState>(Element->PlayerState))
			{
				uint16 NewMoneyAmount = FMath::Clamp(HPlayerState->Money + MoneyToAdd,0,65000);
				HPlayerState->ServerSetMoney(NewMoneyAmount);
			}
		}
	}
}

void AHANEDANSTRIKEGameModeBase::InitializeAllPlayersCharacterOverlayWidgets()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			HanedanPlayerController->InitializeCharacterOverlayWidget();
		}
	}
}

void AHANEDANSTRIKEGameModeBase::AddAnnouncementWidgetToAllPlayers()
{
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(Element)
		{
			Element->AddAnnouncementWidgetToHUD();
		}
	}
}

void AHANEDANSTRIKEGameModeBase::SetVisibilityAllPlayersAnnouncementWidgets(bool bVisible)
{
	for (auto Element : GetAllHPLayerControllers())
	{
		if(Element)
		{
			Element->SetAnnouncementWidgetVisibility(bVisible);
		}
	}
}

void AHANEDANSTRIKEGameModeBase::SetVisilityToHideAllPlayersAfter2Sec()
{
	FTimerHandle SecTimerHandle;
	GetWorldTimerManager().SetTimer(SecTimerHandle,this,&AHANEDANSTRIKEGameModeBase::CallBackToSetVisilityToHideAllPlayersAfter2Sec,2);
	
}

void AHANEDANSTRIKEGameModeBase::CallBackToSetVisilityToHideAllPlayersAfter2Sec()
{
	SetVisibilityAllPlayersAnnouncementWidgets(false);
}

void AHANEDANSTRIKEGameModeBase::SetAllPlayersAnnouncementText(const FString& String)
{
	for (auto Element : GetAllHPLayerControllers())
	{
		if(Element)
		{
			Element->SetAnnouncementText(String);
		}
	}
}

void AHANEDANSTRIKEGameModeBase::PlaySound2dToAllPlayerControllers(USoundBase* Sound)
{
	if(Sound == nullptr) return;
	for (auto Element : GetAllHPLayerControllers())
	{
		if(Element)
		{
			Element->PlaySound2d(Sound);
		}
	}
}

TArray<AHANEDANPlayerController*> AHANEDANSTRIKEGameModeBase::GetAllHPLayerControllers()
{
	TArray<AHANEDANPlayerController*> Array;
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			Array.AddUnique(HanedanPlayerController);
		}
	}
	return Array;
}

TArray<AHANEDANPlayerController*> AHANEDANSTRIKEGameModeBase::GetAllHPLayerControllersHavePawn()
{
	TArray<AHANEDANPlayerController*> Array;
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHANEDANPlayerController* HanedanPlayerController = Cast<AHANEDANPlayerController>(*It);
		if(HanedanPlayerController)
		{
			if(HanedanPlayerController->GetPawn())
			{
				Array.AddUnique(HanedanPlayerController);
			}
		}
	}
	return Array;
}

TArray<AHANEDANCharacter*> AHANEDANSTRIKEGameModeBase::GetAllTCharacters()
{
	if(GetAllHPLayerControllersHavePawn().Num() == 0) return TArray<AHANEDANCharacter*>();
	TArray<AHANEDANCharacter*> Array;
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(!Element->bCtOrT)
		{
			Array.Add(Cast<AHANEDANCharacter>(Element->GetPawn()));
		}
	}
	return Array;
}

void AHANEDANSTRIKEGameModeBase::SetAllCtAndTPlayersNumber()
{
	uint8 lCNumber = 0;
	uint8 lTNumber = 0;
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(Element)
		{
			if(Element->bCtOrT)
			{
				lCNumber++;
			}
			else
			{
				lTNumber++;
			}
		}
	}
	CtNumber = lCNumber;
	TNumber= lTNumber;
}

void AHANEDANSTRIKEGameModeBase::CTWin()
{
	if(GetAllHPLayerControllers().Num() == 0 || GetAllHPLayerControllers().Num() == 1 ) return;
	HanedanGameStateBase->CTPoint++;
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(Element)
		{
			Element->CTPoint++;
		}
	}
	AddAnnouncementWidgetToAllPlayers();
	FString String = FString("CounterTerroristWin");
	SetAllPlayersAnnouncementText(String);
	SetVisibilityAllPlayersAnnouncementWidgets(true);
	PlaySound2dToAllPlayerControllers(CTWinSound);
	SetMatchState(MatchState::RoundCooldownState);
}

void AHANEDANSTRIKEGameModeBase::CTWinWithDefuseBomb()
{
	if(GetAllHPLayerControllers().Num() == 0 || GetAllHPLayerControllers().Num() == 1 ) return;
	HanedanGameStateBase->CTPoint++;
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(Element)
		{
			Element->CTPoint++;
		}
	}
	AddAnnouncementWidgetToAllPlayers();
	FString String = FString("Bomb Has Been Defused");
	SetAllPlayersAnnouncementText(String);
	SetVisibilityAllPlayersAnnouncementWidgets(true);
	PlaySound2dToAllPlayerControllers(CTWinWithBombDefuseSound);
	SetMatchState(MatchState::RoundCooldownState);

	if(C4)
	{
		C4->Destroy();
	}
}

void AHANEDANSTRIKEGameModeBase::TWin()
{
	if(GetAllHPLayerControllers().Num() == 0 || GetAllHPLayerControllers().Num() == 1 ) return;
	HanedanGameStateBase->TPoint++;
	for (auto Element : GetAllHPLayerControllersHavePawn())
	{
		if(Element)
		{
			Element->TPoint++;
		}
	}
	AddAnnouncementWidgetToAllPlayers();
	FString String = FString("TerroristWin");
	SetAllPlayersAnnouncementText(String);
	SetVisibilityAllPlayersAnnouncementWidgets(true);
	PlaySound2dToAllPlayerControllers(TWinSound);
	SetMatchState(MatchState::RoundCooldownState);
}

void AHANEDANSTRIKEGameModeBase::TWinWithBombExplode()
{
	C4->Multicast_ExplodeBomb();
	GetWorldTimerManager().SetTimer(C4DestroyTimerHandle,this,&AHANEDANSTRIKEGameModeBase::DestroyC4,1.f);
	TWin();
}

void AHANEDANSTRIKEGameModeBase::DestroyC4()
{
	if(C4)
	{
		C4->Destroy();
		GetWorldTimerManager().ClearTimer(C4DestroyTimerHandle);
	}
}

void AHANEDANSTRIKEGameModeBase::BombHasPlanted()
{
	if(MatchState == MatchState::BombState || MatchState == MatchState::RoundCooldownState) return;
	if(bBombHasPlanted) return;
	bBombHasPlanted = true;
	PlaySound2dToAllPlayerControllers(BombHasPlantedSound);
	AddAnnouncementWidgetToAllPlayers();
	SetVisibilityAllPlayersAnnouncementWidgets(true);
	SetAllPlayersAnnouncementText(FString("Bomb Has Been Planted"));
	SetVisilityToHideAllPlayersAfter2Sec();
	SetMatchState(MatchState::BombState);
}

void AHANEDANSTRIKEGameModeBase::ExplodeBomb()
{
	bBombHasPlanted = false;
	BombExplosionTimeTrack = 0;
	bBombExploded = true;
	if(C4)
	{
		C4->CancelDefuse(C4->HPlayerController);
	}
	UGameplayStatics::ApplyRadialDamage(this,2000,C4->GetActorLocation(),BombExplosionRadius,
		UDamageType::StaticClass(),TArray<AActor*>());
	TWinWithBombExplode();
}

void AHANEDANSTRIKEGameModeBase::BombDefused()
{
	bBombHasPlanted = false;
	BombExplosionTimeTrack = 0;
	CTWinWithDefuseBomb();
}

void AHANEDANSTRIKEGameModeBase::SpawnC4BombForOneTPlayer()
{
	if(TNumber >= 1)
	{
		uint8 Num =GetAllTCharacters().Num();
		uint8 Selection = FMath::RandRange(0,Num-1);
		FActorSpawnParameters SParams;
		AC4* C4bomb = GetWorld()->SpawnActor<AC4>(C4Class,SParams);

		C4 = C4bomb;
		
		C4bomb->SetOwner(GetAllTCharacters()[Selection]);
		C4bomb->OwnerHCharacter = GetAllTCharacters()[Selection];
		C4bomb->HPlayerController = GetAllTCharacters()[Selection]->GetHanedanPlayerController();
		
		GetAllTCharacters()[Selection]->CombatComponent->WeaponSlot3 = C4bomb;
		GetAllTCharacters()[Selection]->CombatComponent->Multicast_AttachWeaponToBack(GetAllTCharacters()[Selection]->CombatComponent->WeaponSlot3
			,FName("C4Socket"));
	}
}

void AHANEDANSTRIKEGameModeBase::AddWeaponToWeaponsOnGround(ABaseWeapon* WeaponToAdd)
{
	if(!WeaponsOnGround.Contains(WeaponToAdd))
	{
		WeaponsOnGround.Add(WeaponToAdd);
	}
}

void AHANEDANSTRIKEGameModeBase::ClearWeaponReference(ABaseWeapon* WeaponReference)
{
	if(WeaponsOnGround.Contains(WeaponReference))
	{
		WeaponsOnGround.Remove(WeaponReference);
	}
	UE_LOG(LogTemp,Error,TEXT("weapon ref clearlandı"));
}

void AHANEDANSTRIKEGameModeBase::ClearAllWeaponsOnTheGround()
{
	if(WeaponsOnGround.Num() == 0) return;
	for (auto Element : WeaponsOnGround)
	{
		if(Element)
		{
			Element->Destroy();
		}
	}
	WeaponsOnGround.Empty();
}



