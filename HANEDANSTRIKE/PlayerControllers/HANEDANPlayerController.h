// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HANEDANSTRIKE/HANEDANSTRIKEGameModeBase.h"
#include "HANEDANPlayerController.generated.h"

class AHANEDANPlayerState;
class USoundCue;
class AHANEDANHUD;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

public:
	AHANEDANPlayerController();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * FREE SPACE
	 */

	/** Oyuna yeni giren birisi için aşağıdaki fonklar gamemodeda çağırılıyor gerekli setlenmeler için
	* H_PlayerController->OnMatchStateSet(MatchState);
	* H_PlayerController->SetTimeRemainSeconds(TimeRemainSeconds);
	* Zaman sync için onmatchstate fonku ve setremaineconds çağırılıyor serverda sorun yok ama clientta OnRep_onmatchstate
	* settimeremainsseconds sonra fire yediği için setremainsecondsu overried ediyor ki bu oyuna yeni katılan için kalan state
	* time ın yanlış görünmesine yol açıyor bu değişkenle beraber bu sorunu ortadan kaldırdım
	 */
	bool bNewStartedPlayer = true;
	
	UPROPERTY()
	AHANEDANCharacter* HCharacter;

	UPROPERTY(Replicated,BlueprintReadOnly)
	bool bCtOrT;

	UFUNCTION(Server,Reliable)
	void ServerSetHPC_bCtOrT(bool bCt);

	UPROPERTY()
	AHANEDANGameStateBase* HGameStateBase;

	/**
	 * Buying Equipment Requests To Character Combat Component
	 */

	UPROPERTY()
	AHANEDANPlayerState* HPlayerState;

	/**Buy menu widgetten çağırılıyor weapon slot max 3 weapon number max 6 */
	void RequestBuyWeapon(uint16 WeaponPrice, uint8 WeaponSlot, uint8 WeaponNumber);

	void RequestBuyKevlarHelmet();
	
	// TODO RequestBuyGrenade
	// TODO request buy kevlarhelmet
	
	/**
	 *  Server-Client Time Sync
	 */

	/** Auth Player controller oyunun başında gidip servertime ı alır ve Cliente çoğaltır */
	float ServerMapTime = 0.f;

	UFUNCTION(Client,Reliable)
	void ClientReceivedTime(float L_ServerMapTime);

	UFUNCTION(Server,Reliable)
	void ServerReportTime();
	/** Servere yollanan rpc den sonra serverden cliente dönen rpc ye kadar geçen zamanın bulunması için
	 *  Servere yollanınca true serverden rpc alınınca false olacak.
	 */
	bool bServerRpcSendedTrack = false;
	float RoundTripTime = 0.f;
	

	UPROPERTY(EditAnywhere, Category="Properties")
	float CheckTimeSyncFrequency = 3.f;
	float CheckTimeSyncFrequencyTrack = 0.f;
	bool bTimeSycned = false;

	/**
	 * Match States
	 */

	void OnMatchStateSet(FName State);

	UPROPERTY(ReplicatedUsing=OnRep_MatchState,BlueprintReadOnly)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	void HandleStartingState();
	void HandlePurchasingState();
	void HandleRoundState();
	void HandleRoundCooldownState();
	void HandleBombState();

	/**
	 * Match State Timingler -> Round timelarının gamemode la senkronize edilmesi için player controller oyuna girince gamemode
	 * oyuna girdiği anda hangi state de ise statenin kalan time amountunu player controllerla paylaşıcak
	 */

	/** State değişimlerinde kullanılmak için 1 defa alınacak */
	UPROPERTY(ReplicatedUsing=OnRep_TimePackage)
	FGameModeStateTimePackage TimePackage;
	void SetTimePackage(const FGameModeStateTimePackage& Package);
	UFUNCTION()
	void OnRep_TimePackage();

	/** Oyuncu oyuna sonradan girerse kullanılıyor*/
	void SetTimeRemainSeconds(float TimeRemain);

	UPROPERTY(Replicated)
	float RealTimeRemainSeconds = 0.f;

	/**
	 * HUD Works
	 */

	UPROPERTY()
	AHANEDANHUD* HanedanHUD;

	void SetHUD_TimeText(float Time);

	UFUNCTION(Client,Reliable)
	void Client_AddTimeWidget(); // Spawnlanma işlemi bittikten sonra time widget yerleştirilecek.

	/**
	 * Spawn Character On Gamemode
	 */

	UFUNCTION(Server,Reliable)
	void Server_ClientPCRequestSpawnCharacter(AHANEDANPlayerController* L_HPlayerController);

	/**
	 * SOUND ASSETS
	 */
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* RoundStartSound;
	
	void PlaySound2d(USoundBase* Sound);
	UFUNCTION(Client,Reliable)
	void Client_PlaySound2d(USoundBase* Sound);

	/**
	 * CHARACTER OVERLAY WİDGET WORKS
	 */

	void AddCharacterOverlayToHud(); // Widgetin construct edilerek HUD a ve Viewporta eklenmesi
	UFUNCTION(Client,Reliable)
	void Client_AddCharacterOverlayToHud();

	void InitializeCharacterOverlayWidget(); // Oyuncunun o anlık pawn durumuna göre setlenmesi
	UFUNCTION(Client,Reliable)
	void Client_InitializeCharacterOverlayWidget();
	FTimerHandle InitializeCharacterOverlayWidgetTimerHandle;

	void HideCharacterOverlay(bool bHide); // Saklamak ve açmak için kullanacağız

	/** Oyun içinde dinamik değişimler için kullanacağız */
	void SetCharacterOverlayHealthText(uint8 Value);
	void SetCharacterOverlayShieldText(uint8 Value);
	void SetCharacterOverlayWeaponAmmoText(uint8 Value);
	void SetCharacterOverlayAmmoText(uint16 Value);
	void SetCharacterOverlayMoneyText(uint16 Value);
	void SetCharacterOverlayCTPointText(uint8 Value);
	void SetCharacterOverlayTPointText(uint8 Value);

	/**
	 * ANNOUNCEMENT WIDGET WORKS
	 */

	void AddAnnouncementWidgetToHUD(); // Widgeti construct eder viewporta ekler hidden yapar
	UFUNCTION(Client,Reliable)
	void Client_AddAnnouncementWidgetToHUD(); // Widgeti construct eder viewporta ekler hidden yapar

	void SetAnnouncementWidgetVisibility(bool bVisible);
	UFUNCTION(Client,Reliable)
	void Client_SetAnnouncementWidgetVisibility(bool bVisible);

	void SetAnnouncementText(const FString& String);
	UFUNCTION(Client,Reliable)
	void Client_SetSetAnnouncementText(const FString& String);

	/**
	 * AWP SCOPE WIDGET
	 */

	void AddAWPScopeWidgetToHUD(); // Widgeti construct eder viewporta ekler hidden yapar
	UFUNCTION(Client,Reliable)
	void Client_AddAWPScopeWidgetToHUD();

	void SetAWPScopeWidgetVisibility(bool bVisible);
	UFUNCTION(Client,Reliable)
	void Client_SetAWPScopeWidgetVisibility(bool bVisible);
	void Local_SetAWPScopeVisibility(bool bVisible);


	/**
	 * Player Score Works // TODO şu an çalışmıyor nedense bulamadım
	 */
	UPROPERTY(Replicated,BlueprintReadOnly)
	int32 PlayerScore = 0;
	UFUNCTION(Server,Reliable)
	void Server_AddScore();
	UFUNCTION(Server,Reliable)
	void Server_ReduceScore();

	/**
	 * Game Score Works
	 */
	
	UPROPERTY(ReplicatedUsing=OnRep_CTPoint)
	uint8 CTPoint = 0;
	UFUNCTION()
	void OnRep_CTPoint();

	UPROPERTY(ReplicatedUsing=OnRep_TPoint)
	uint8 TPoint = 0;
	UFUNCTION()
	void OnRep_TPoint();

	/**
	 * C4 MESSAGES
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties") // clientten gelen server rpc de kaç saniyede defuse edildiği kontrol edilmeli
	uint8 BombDefuseTime = 5;

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_BombDefused(uint8 DefuseTime);
	
	










	
};
