// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANPlayerController.h"

#include "Components/TextBlock.h"
#include "HANEDANSTRIKE/HANEDANSTRIKEGameModeBase.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "HANEDANSTRIKE/GameStates/HANEDANGameStateBase.h"
#include "HANEDANSTRIKE/HUD/AnnoucementWidget.h"
#include "HANEDANSTRIKE/HUD/CharacterOverlay.h"
#include "HANEDANSTRIKE/HUD/HANEDANHUD.h"
#include "HANEDANSTRIKE/HUD/TimeWidget.h"
#include "HANEDANSTRIKE/PlayerStates/HANEDANPlayerState.h"
#include "HANEDANSTRIKE/Weapons/BaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AHANEDANPlayerController::AHANEDANPlayerController()
{
	
}

void AHANEDANPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if(AHANEDANSTRIKEGameModeBase* HanedanGameMode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>()) // Sadece serverda valid olacak
	{
		ServerMapTime = HanedanGameMode->ServerMapTime;
	}
	if(!HasAuthority() &&  IsLocalPlayerController()) // Owner Client isek
	{
		ServerMapTime = 0.f;
		bServerRpcSendedTrack = true ;
		ServerReportTime();
	}

	HanedanHUD = Cast<AHANEDANHUD>(GetHUD());
	if(HanedanHUD && IsLocalPlayerController())
	{
		HanedanHUD->AddChooseTeamWidget();
	}
	
	AddAWPScopeWidgetToHUD();

	HGameStateBase =GetWorld()->GetGameState<AHANEDANGameStateBase>();

	// HPlayerState = GetPlayerState<AHANEDANPlayerState>(); // null geliyor
	
}

void AHANEDANPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ServerMapTime += DeltaSeconds;

	if(bServerRpcSendedTrack) // Serverdada yapmamız önemsiz gibi o yüzden checklemedim
	{
		RoundTripTime += DeltaSeconds;
	}

	if(!HasAuthority() && IsLocalController() && bTimeSycned) // Owner Client isek
	{
		CheckTimeSyncFrequencyTrack += DeltaSeconds;
		if(CheckTimeSyncFrequencyTrack >= CheckTimeSyncFrequency)
		{
			ServerReportTime();
			bTimeSycned = false;
		}
	}

	SetHUD_TimeText(RealTimeRemainSeconds - ServerMapTime);
	
}

void AHANEDANPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHANEDANPlayerController,MatchState);
	DOREPLIFETIME(AHANEDANPlayerController,RealTimeRemainSeconds);
	DOREPLIFETIME(AHANEDANPlayerController,TimePackage);
	DOREPLIFETIME(AHANEDANPlayerController,bCtOrT);
	DOREPLIFETIME(AHANEDANPlayerController,PlayerScore);
	DOREPLIFETIME_CONDITION(AHANEDANPlayerController,CTPoint,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHANEDANPlayerController,TPoint,COND_OwnerOnly);
}

void AHANEDANPlayerController::ServerSetHPC_bCtOrT_Implementation(bool bCt)
{
	bCtOrT = bCt;
}

// Client
void AHANEDANPlayerController::ClientReceivedTime_Implementation(float L_ServerMapTime)
{
	bServerRpcSendedTrack = false; // rpc servere gitti ve cliente geri göderildi
	ServerMapTime = L_ServerMapTime +(RoundTripTime/2);
	RoundTripTime = 0;
	CheckTimeSyncFrequencyTrack = 0;
	bTimeSycned = true;
}

// Server
void AHANEDANPlayerController::ServerReportTime_Implementation()
{
	ClientReceivedTime(ServerMapTime);
}

void AHANEDANPlayerController::OnMatchStateSet(FName State)
{
	//UE_LOG(LogTemp,Error,TEXT("hpc server onmachstateset edildi"));
	MatchState = State;
	
	if(MatchState == MatchState::StartingState)
	{
		HandleStartingState();
		SetTimeRemainSeconds(TimePackage.StartingStateTimeAmount);
	}
	else if(MatchState == MatchState::PurchasingState)
	{
		HandlePurchasingState();
		SetTimeRemainSeconds(TimePackage.PurchasingStateTimeAmount);
	}
	else if(MatchState == MatchState::RoundState)
	{
		HandleRoundState();
		SetTimeRemainSeconds(TimePackage.RoundStateTimeAmount);
	}
	
	else if(MatchState == MatchState::RoundCooldownState)
	{
		HandleRoundCooldownState();
		SetTimeRemainSeconds(TimePackage.RoundCooldownStateTimeAmount);
	}

	else if(MatchState == MatchState::BombState)
	{
		HandleBombState();
		SetTimeRemainSeconds(TimePackage.BombExplosionTimeAmount);
	}
}

void AHANEDANPlayerController::OnRep_MatchState()
{
	//UE_LOG(LogTemp,Error,TEXT("hpc client onmachstateset edildi"));
	if(MatchState == MatchState::StartingState)
	{
		HandleStartingState();
		if(bNewStartedPlayer)
		{
			bNewStartedPlayer = false;
		}
		else
		{
			SetTimeRemainSeconds(TimePackage.StartingStateTimeAmount);
		}
	}
	else if(MatchState == MatchState::PurchasingState)
	{
		HandlePurchasingState();
		if(bNewStartedPlayer)
		{
			bNewStartedPlayer = false;
		}
		else
		{
			SetTimeRemainSeconds(TimePackage.PurchasingStateTimeAmount);
		}
	}
	else if(MatchState == MatchState::RoundState)
	{
		HandleRoundState();
		if(bNewStartedPlayer)
		{
			bNewStartedPlayer = false;
		}
		else
		{
			SetTimeRemainSeconds(TimePackage.RoundStateTimeAmount);
		}
	}
	
	else if(MatchState == MatchState::RoundCooldownState)
	{
		HandleRoundCooldownState();
		if(bNewStartedPlayer)
		{
			bNewStartedPlayer = false;
		}
		else
		{
			SetTimeRemainSeconds(TimePackage.RoundCooldownStateTimeAmount);
		}
	}

	else if(MatchState == MatchState::BombState)
	{
		HandleBombState();
		if(bNewStartedPlayer)
		{
			bNewStartedPlayer = false;
		}
		else
		{
			SetTimeRemainSeconds(TimePackage.BombExplosionTimeAmount);
		}
		
	}
}

void AHANEDANPlayerController::HandleStartingState()
{
	
}

void AHANEDANPlayerController::HandlePurchasingState()
{
	
}

void AHANEDANPlayerController::HandleRoundState()
{
	UGameplayStatics::PlaySound2D(this,RoundStartSound);
	if(HanedanHUD)
	{
		HanedanHUD->HideBuyMenuWidget();
	}
}

void AHANEDANPlayerController::HandleRoundCooldownState()
{
	
}

void AHANEDANPlayerController::HandleBombState()
{
	// TODO
}

void AHANEDANPlayerController::SetTimePackage(const FGameModeStateTimePackage& Package)
{
	TimePackage.StartingStateTimeAmount = Package.StartingStateTimeAmount;
	TimePackage.PurchasingStateTimeAmount = Package.PurchasingStateTimeAmount;
	TimePackage.RoundStateTimeAmount = Package.RoundStateTimeAmount;
	TimePackage.RoundCooldownStateTimeAmount = Package.RoundCooldownStateTimeAmount;
	TimePackage.BombExplosionTimeAmount = Package.BombExplosionTimeAmount;
}

void AHANEDANPlayerController::OnRep_TimePackage()
{
	
}

void AHANEDANPlayerController::SetTimeRemainSeconds(float TimeRemain)
{
	RealTimeRemainSeconds = TimeRemain + ServerMapTime; // RealTimeRemainSeconds Replicated
}

void AHANEDANPlayerController::SetHUD_TimeText(float Time)
{
	int32 Minute = Time/60;
	int32 Seconds = Time - Minute*60;

	if(HanedanHUD && HanedanHUD->TimeWidget)
	{
		FString TextString = FString::Printf(TEXT("%02d : %02d"),Minute,Seconds);
		HanedanHUD->TimeWidget->SetTimeText(FText::FromString(TextString));
	}
}

void AHANEDANPlayerController::Client_AddTimeWidget_Implementation()
{
	if(HanedanHUD)
	{
		HanedanHUD->AddTimeWidget();
	}
}

void AHANEDANPlayerController::Server_ClientPCRequestSpawnCharacter_Implementation(
	AHANEDANPlayerController* L_HPlayerController)
{
	UWorld* World = GetWorld();
	if(World)
	{
		AHANEDANSTRIKEGameModeBase* GMode = World->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
		if(GMode)
		{
			GMode->GameModeRestartPlayer(L_HPlayerController);
		}
	}
}

void AHANEDANPlayerController::RequestBuyWeapon(uint16 WeaponPrice, uint8 Slot, uint8 WeaponNumber)
{
	HCharacter = HCharacter == nullptr ? Cast<AHANEDANCharacter>(GetPawn()) : HCharacter;
	if(HCharacter)
	{
		HCharacter->CombatComponent->BuyWeapon(WeaponPrice,Slot,WeaponNumber);
	}
}

void AHANEDANPlayerController::RequestBuyKevlarHelmet()
{
	HCharacter = HCharacter == nullptr ? Cast<AHANEDANCharacter>(GetPawn()) : HCharacter;
	if(HCharacter)
	{
		if(HCharacter->CombatComponent == nullptr)
		{
			return;
		}
		HCharacter->CombatComponent->BuyKevlarHelmet();
	}
}

void AHANEDANPlayerController::InitializeCharacterOverlayWidget()
{
	HCharacter = HCharacter==nullptr ? Cast<AHANEDANCharacter>(GetPawn()) : HCharacter;
	if(HCharacter == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("HCharacter nulldur."));
		return;
	}
	if(HasAuthority() && IsLocalController())
	{
		if(HanedanHUD && HanedanHUD->CharacterOverlayWidget && HCharacter)
		{
			SetCharacterOverlayHealthText(HCharacter->CombatComponent->Health);
			SetCharacterOverlayShieldText(HCharacter->CombatComponent->Shield);
			if(HCharacter->GetHanedanPlayerState())
			{
				SetCharacterOverlayMoneyText(HCharacter->GetHanedanPlayerState()->Money);	
			}
			//HGameStateBase = HGameStateBase == nullptr ? GetWorld()->GetGameState<AHANEDANGameStateBase>() : HGameStateBase;
			if(HGameStateBase)
			{
				SetCharacterOverlayCTPointText(HGameStateBase->CTPoint);
				SetCharacterOverlayTPointText(HGameStateBase->TPoint);
			}
			
			if(HCharacter->CombatComponent->CurrentWeaponOnHand == nullptr)
			{
				SetCharacterOverlayWeaponAmmoText(0);
				SetCharacterOverlayAmmoText(0);
			}
			else
			{
				SetCharacterOverlayWeaponAmmoText(HCharacter->CombatComponent->CurrentWeaponOnHand->WeaponAmmo);
				SetCharacterOverlayAmmoText(HCharacter->CombatComponent->CurrentWeaponOnHand->Ammo);
			}
		}
	}
	else
	{
		// Büyük ihtimalle pawn onrep ile gittiği için getpawndan gelen değer null oluyordu hatayı gidermek delay verdim
		GetWorldTimerManager().SetTimer(InitializeCharacterOverlayWidgetTimerHandle,this,
			&AHANEDANPlayerController::Client_InitializeCharacterOverlayWidget,0.1f);
	}
}

void AHANEDANPlayerController::Client_InitializeCharacterOverlayWidget_Implementation()
{
	HCharacter = HCharacter==nullptr ? Cast<AHANEDANCharacter>(GetPawn()) : HCharacter;
	
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget && HCharacter)
	{
		SetCharacterOverlayHealthText(HCharacter->CombatComponent->Health);
		SetCharacterOverlayShieldText(HCharacter->CombatComponent->Shield);
		SetCharacterOverlayMoneyText(HCharacter->GetHanedanPlayerState()->Money);

		/* GameState ile bu sistemi clientler için çalıştıramadık o yüzden ct ve t pointi clientler için
		 * playercontrollera taşıdım playercontrollerdaki ct ve t point değişince widgettakilerde onrep ile değişecek
		 * o yüzden clientler için değerleri değiştirmen yeterli
		HGameStateBase = HGameStateBase == nullptr ? GetWorld()->GetGameState<AHANEDANGameStateBase>() : HGameStateBase;
		if(HGameStateBase)
		{
			SetCharacterOverlayCTPointText(HGameStateBase->CTPoint);
			SetCharacterOverlayTPointText(HGameStateBase->TPoint);
		}*/

		SetCharacterOverlayCTPointText(CTPoint);
		SetCharacterOverlayTPointText(TPoint);
		
		if(HCharacter->CombatComponent->CurrentWeaponOnHand == nullptr)
		{
			SetCharacterOverlayWeaponAmmoText(0);
			SetCharacterOverlayAmmoText(0);
		}
		else
		{
			SetCharacterOverlayWeaponAmmoText(HCharacter->CombatComponent->CurrentWeaponOnHand->WeaponAmmo);
			SetCharacterOverlayAmmoText(HCharacter->CombatComponent->CurrentWeaponOnHand->Ammo);
		}
	}
}

void AHANEDANPlayerController::PlaySound2d(USoundBase* Sound)
{
	if(Sound == nullptr) return;
	if(HasAuthority() && IsLocalController())
	{
		UGameplayStatics::PlaySound2D(this,Sound);
	}
	else
	{
		ClientPlaySound(Sound);
	}
}

void AHANEDANPlayerController::Client_PlaySound2d_Implementation(USoundBase* Sound)
{
	if(IsLocalController() && Sound)
	{
		UE_LOG(LogTemp,Error,TEXT("gelindigdfg"))
		UGameplayStatics::PlaySound2D(this,Sound);
	}
}

void AHANEDANPlayerController::AddCharacterOverlayToHud()
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD && HanedanHUD->CharacterOverlayWidget == nullptr)
		{
			HanedanHUD->AddCharacterOverlay();
		}
	}
	else
	{
		Client_AddCharacterOverlayToHud();
	}
}

void AHANEDANPlayerController::Client_AddCharacterOverlayToHud_Implementation()
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget == nullptr)
	{
		HanedanHUD->AddCharacterOverlay();
	}
}

void AHANEDANPlayerController::HideCharacterOverlay(bool bHide)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD)
	{
		HanedanHUD->HideCharacterOverlay(bHide);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayHealthText(uint8 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetHealthText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayShieldText(uint8 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetShieldText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayWeaponAmmoText(uint8 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetWeaponAmmoText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayAmmoText(uint16 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetAmmoText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayMoneyText(uint16 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetMoneyText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayCTPointText(uint8 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetCTPointText(Value);
	}
}

void AHANEDANPlayerController::SetCharacterOverlayTPointText(uint8 Value)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetTPointText(Value);
	}
}

void AHANEDANPlayerController::AddAWPScopeWidgetToHUD()
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD && HanedanHUD->AWPScopeWidget==nullptr)
		{
			HanedanHUD->AddAWPScopeWidget();
		}
	}
	else
	{
		Client_AddAWPScopeWidgetToHUD();
	}
}

void AHANEDANPlayerController::SetAWPScopeWidgetVisibility(bool bVisible)
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD)
		{
			HanedanHUD->SetAWPScopeVisibility(bVisible);
		}
	}
	else
	{
		Client_SetAWPScopeWidgetVisibility(bVisible);
	}
}

void AHANEDANPlayerController::Local_SetAWPScopeVisibility(bool bVisible)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD)
	{
		HanedanHUD->SetAWPScopeVisibility(bVisible);
	}
}

void AHANEDANPlayerController::Client_SetAWPScopeWidgetVisibility_Implementation(bool bVisible)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD)
	{
		HanedanHUD->SetAWPScopeVisibility(bVisible);
	}
}

void AHANEDANPlayerController::Client_AddAWPScopeWidgetToHUD_Implementation()
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->AWPScopeWidget==nullptr)
	{
		HanedanHUD->AddAWPScopeWidget();
	}
}

void AHANEDANPlayerController::AddAnnouncementWidgetToHUD()
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD && HanedanHUD->AnnouncementWidget==nullptr)
		{
			HanedanHUD->AddAnnouncementWidget();
		}
	}
	else
	{
		Client_AddAnnouncementWidgetToHUD();
	}
}

void AHANEDANPlayerController::Client_AddAnnouncementWidgetToHUD_Implementation()
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->AnnouncementWidget==nullptr)
	{
		HanedanHUD->AddAnnouncementWidget();
	}
}

void AHANEDANPlayerController::SetAnnouncementWidgetVisibility(bool bVisible)
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD && HanedanHUD->AnnouncementWidget)
		{
			if(bVisible)
			{
				HanedanHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				HanedanHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	else
	{
		Client_SetAnnouncementWidgetVisibility(bVisible);
	}
}

void AHANEDANPlayerController::Client_SetAnnouncementWidgetVisibility_Implementation(bool bVisible)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->AnnouncementWidget)
	{
		if(bVisible)
		{
			HanedanHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			HanedanHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


void AHANEDANPlayerController::SetAnnouncementText(const FString& String)
{
	if(HasAuthority() && IsLocalController())
	{
		HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
		if(HanedanHUD && HanedanHUD->AnnouncementWidget)
		{
			HanedanHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(String));
		}
	}
	else
	{
		Client_SetSetAnnouncementText(String);
	}
}

void AHANEDANPlayerController::Client_SetSetAnnouncementText_Implementation(const FString& String)
{
	HanedanHUD = HanedanHUD == nullptr ? Cast<AHANEDANHUD>(GetHUD()) : HanedanHUD;
	if(HanedanHUD && HanedanHUD->AnnouncementWidget)
	{
		HanedanHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(String));
	}
}

void AHANEDANPlayerController::Server_ReduceScore_Implementation()
{
	PlayerScore--;
}

void AHANEDANPlayerController::Server_AddScore_Implementation()
{
	PlayerScore++;
}

void AHANEDANPlayerController::OnRep_CTPoint()
{
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetCTPointText(CTPoint);
	}
}

void AHANEDANPlayerController::OnRep_TPoint()
{
	if(HanedanHUD && HanedanHUD->CharacterOverlayWidget)
	{
		HanedanHUD->CharacterOverlayWidget->SetTPointText(TPoint);
	}
}

void AHANEDANPlayerController::Server_BombDefused_Implementation(uint8 DefuseTime)
{
	GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>()->BombDefused();
}

bool AHANEDANPlayerController::Server_BombDefused_Validate(uint8 DefuseTime)
{
	if(DefuseTime < BombDefuseTime)
	{
		return false;
	}
	return true;
}





