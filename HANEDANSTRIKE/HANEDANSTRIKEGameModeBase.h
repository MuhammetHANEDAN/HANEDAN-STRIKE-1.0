// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HANEDANSTRIKEGameModeBase.generated.h"

class AC4;
class ABaseWeapon;
class AHANEDANPlayerController;
class AHANEDANGameStateBase;
class AHANEDANCharacter;

USTRUCT(BlueprintType)
struct FGameModeStateTimePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float StartingStateTimeAmount = 20.f;
	
	UPROPERTY()
	float PurchasingStateTimeAmount = 10.f;
	
	UPROPERTY()
	float RoundStateTimeAmount = 120.f;
	
	UPROPERTY()
	float RoundCooldownStateTimeAmount = 120.f;

	UPROPERTY()
	float BombExplosionTimeAmount = 30.f;
	
};

namespace MatchState
{
	/**
	 * Oyuna ilk başlandığında takım seçildikten sonra takım spawn yerinde doğulacak başlangıç parası yüksek verilecek süre bite
	 * ne kadar free şekilde playerlar oynayabilecek cs sistemi
	 */
	extern HANEDANSTRIKE_API const FName StartingState; 
	extern HANEDANSTRIKE_API const FName PurchasingState;
	extern HANEDANSTRIKE_API const FName RoundState;
	extern HANEDANSTRIKE_API const FName RoundCooldownState;
	extern HANEDANSTRIKE_API const FName BombState; // Bomba Kurulunca geçilecek state
}

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANSTRIKEGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	/** Destroy işlemi yapmadan round bitince karakterleri durumlarına göre initialize edip destroy etmeden locaiton ve rotaion
	 * setleyebiliriz. Destroy işlemi yaparsak player statelerde silinecek çünkü.
	 */

	//-------------------------

protected:

	virtual void BeginPlay() override;

public:

	AHANEDANSTRIKEGameModeBase();

	virtual void Tick(float DeltaSeconds) override;

	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;

	void RestartGame(); // TODO oyunu yeniden başlat
	
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	FString MatchMapURL;
	

	/**
	 * REFERENCES
	 */

	UPROPERTY()
	AHANEDANGameStateBase* HanedanGameStateBase;

	UPROPERTY()
	AHANEDANPlayerController* FirstHPlayerController;

	/**
	 * Oyuncu Bilgileri, Roundlara Göre Durumları  ve Round Sayısı bilgisi , 
	 */

	UPROPERTY()
	TArray<AController*> CtPlayerControllers;
	
	UPROPERTY()
	TArray<AController*> TPlayerControllers;

	UPROPERTY()
	TArray<AController*> WaitingToRestartControllers; // Daha hiç pawnı spawn edilmemiş olanlar için

	UPROPERTY()
	TArray<APlayerController*> ElimnedPlayersLastRound; // todo ilk round oynanmaya başlanmış ve sonraki rounda kadar ölenler

	
	uint8 RoundNumber = 0; // TODO Round stateye geçince +1 ekle max rounda gelince oyun yeniden başlayacak
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint8 MaxRoundNumber = 10;

	void PlayerElimned(AController* DeadController, AController* KillerController);

	/**Starting statede ölenler listeye eklenecek ve belirli bir zaman sonra spawnlanacaklar eğer
	 * starting state devam ediyorsa
	 */
	TDoubleLinkedList<AController*> ElimnedPlayerList;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint8 RestartOnePlayerTime = 2; // Starting statede ölenin eğer starting state devam ediyorsa kaç saniyede spawnlanacağı
	void RestartOnePlayer();

	/**
	 * GAME MODE SET ALL CHARACTERS OR ALL PLAYERCONTROLLERS PROPERTIES
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 StartingMoney = 3000;

	void SetAllCharactersLocationsToBase();
	void DisableAllCharactersMovements();
	void EnableAllCharactersMovements();

	void SetAllPlayersMoney(uint16 NewMoneyAmount);
	void AddMoneyToAllPlayers(uint16 MoneyToAdd);
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 AddMoneyPerRound = 2500;
	void InitializeAllPlayersCharacterOverlayWidgets();

	void AddAnnouncementWidgetToAllPlayers();
	
	void SetVisibilityAllPlayersAnnouncementWidgets(bool bVisible);
	void SetVisilityToHideAllPlayersAfter2Sec();
	void CallBackToSetVisilityToHideAllPlayersAfter2Sec();
	
	void SetAllPlayersAnnouncementText(const FString& String);

	void PlaySound2dToAllPlayerControllers(USoundBase* Sound);

	/**
	 * GAMEMODE GET ALL PROPERTIES
	 */

	TArray<AHANEDANPlayerController*> GetAllHPLayerControllers();
	TArray<AHANEDANPlayerController*> GetAllHPLayerControllersHavePawn(); // pawnı varsa takım seçmiş demektir
	TArray<AHANEDANCharacter*> GetAllTCharacters();
	void SetAllCtAndTPlayersNumber(); // takım seçmiş ve pawnı verilmiş oyuncular için kaç tane CT kaç tane T olduğunu alıp setler
	uint8 CtNumber = 0;
	uint8 TNumber = 0;

	/**
	 * HANDLE WIN
	 */

	void CTWin();
	void CTWinWithDefuseBomb();
	void TWin();
	void TWinWithBombExplode();

	/**
	 * BOMB WORKS
	 */

	bool bBombHasPlanted = false;
	bool bBombExploded = false;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float BombExplosionTimeAmount = 30.f;
	float BombExplosionTimeTrack = 0.f;
	
	void BombHasPlanted();
	void ExplodeBomb(); // Bombayı patlatır ve stateyi roundcooldown stateye geçirtir.
	void BombDefused();
	
	void SpawnC4BombForOneTPlayer();
	
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<AC4> C4Class;

	UPROPERTY()
	AC4* C4;

	FTimerHandle C4DestroyTimerHandle;
	void DestroyC4();

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint16 BombExplosionRadius = 5000;

	/**
	 * Time 
	 */
	
	/** Menüden oyun başlangıç mapinin başlangıcına kadar geçen süre */
	UPROPERTY(BlueprintReadOnly)
	float LevelStartingTime = 0.f;
	/** Mapin başlangıcından itibaren geçen süre */
	float ServerMapTime = 0.f;

	FGameModeStateTimePackage GameModeTimePackage;

	/**
	 * States
	 */

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	FName MatchState;

	void SetMatchState(FName State);

	virtual void OnMatchStateSet();

	/** Her Statede değişimnde yeni statenin bitimi için ne kadar zaman kaldığını  server player controllerlara gönderiyoruz spc lerdeki
	 * timeremainseconds replicated olduğu için clientlerde alıyor ve öncesinde spc ve cpc ler arası servertime sync olduğu
	 * için sorun olmayacak // TODO yanlış yorum
	 */
	float TimeRemainSeconds;

	UPROPERTY(EditAnywhere, Category="Properties")
	float StartingStateTimeAmount = 20.f;
	float StartingStateTimeTrack = 0.f;
	/**  Cs de oyun ilk başlayınca oyuncuların free şekilde oynaması. Bunu startingsection bittiğinde true yapacağızki bir daha
	 *  startingsectionstate ye girmeyelim
	 */
	bool bStartingSectionFinished = false;

	UPROPERTY(EditAnywhere, Category="Properties")
	float PurchasingStateTimeAmount = 10.f;
	float PurchasingStateTimeTrack = 0.f;

	UPROPERTY(EditAnywhere, Category="Properties")
	float RoundStateTimeAmount = 120.f; // Raund zamanı
	float RoundStateTimeTrack = 0.f;

	UPROPERTY(EditAnywhere, Category="Properties")
	float RoundCooldownStateTimeAmount = 120.f; // Raund zamanı
	float RoundCooldownStateTimeTrack = 0.f;

	void HandleStartingState();
	void HandlePurchasingState();
	void HandleRoundState();
	void HandleRoundCooldownState();
	void HandleBombState();
	
	/**
	 * SPAWN İŞLEMLERİ
	 * Player Statedeki bCtorT ye göre spawnlanacak default pawnların belirlenmesi için
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<AHANEDANCharacter> CtCharacterClass;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<AHANEDANCharacter> TCharacterClass;

	AHANEDANCharacter* SpawnCtorTCharacter(bool bCt, const FTransform SpawnTransform);
	AActor* GetPlayerStartsRandomForTeams(bool bCt) ;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override; // Çıkan controller için eğer round statede ise ct veya t sayısını azalt

	/** Oyun başlayınca gamodebase tarafından controller spawnlanıp server ve cliente veriliyor
	 * game state game session player stateler hazır hale getiriliyor
	 * gamemode ve characteri spawnlama fonksiyonunu durdurduk. karakter spawnlama işlemi şu an bu gamemode da bu fonksiyonla
	 * başlıyor, Takım seçme menüsündeki tuşa basınca.
	 */
	void GameModeRestartPlayer(APlayerController* L_PlayerController);
	
	void HandleNewPlayerStarting(AController* NewPlayer); // Yeni bir karakter spawnlandığında hangi statedeyse o stateye ait ek işlemleri yapılmalı
	
	/** Bazı statelerde oyuna girip takım seçen controllerların pawn spawn işlemi belirlenen sonraki statelerde spawnlanacak
	 * o yüzden arraye alınıyolar. Gamemode ONMatchState() purchasing stateye geçtiği an karakterleri spawnlar ve contollera verir
	 * // TODO belki oyunda ölen karakterleride bu arreye atabilirz
	 */
	
	/**
	 * SOUND ASSETS
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* BombHasPlantedSound;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* BombHasDefusedSound;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* CTWinSound;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* CTWinWithBombDefuseSound;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* TWinSound;
	

	/**
	 * WEAPONS ON GROUND
	 */

	UPROPERTY()
	TArray<ABaseWeapon*> WeaponsOnGround; // yere atılan weaponların referansı burda tutulacak eğer silahlar yerden alınmazsa bir sonraki roundda silenecekler

	void AddWeaponToWeaponsOnGround(ABaseWeapon* WeaponToAdd);
	void ClearWeaponReference(ABaseWeapon* WeaponReference); // Silah yerden alınırsa silinmemeli
	void ClearAllWeaponsOnTheGround(); // arraydeki tüm weaponlar silinir
	

	

	

	






	
	
};
