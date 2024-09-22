// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HANEDANGameMode.generated.h"


/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANGameMode : public AGameMode
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

public:

	AHANEDANGameMode();

	virtual void Tick(float DeltaSeconds) override;
	
	/** Menüden oyun başlangıç mapinin başlangıcına kadar geçen süre */
	UPROPERTY(BlueprintReadOnly)
	float LevelStartingTime = 0.f;
	/** Mapin başlangıcından itibaren geçen süre */
	float ServerMapTime = 0.f;

	/**
	 * States
	 */

	virtual void OnMatchStateSet() override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	
	

	UPROPERTY(EditAnywhere, Category="Properties")
	float StartingStateTimeAmount = 20.f;
	float StartingStateTimeTrack = 0.f;
	/**  Cs de oyun ilk başlayınca oyuncuların free şekilde oynaması. Bunu startingsection bittiğinde true yapacağızki bir daha
	 *  startingsectionstate ye girmeyelim
	 */
	bool bStartingSectionFinished = false;
	void SetMatchStateToStartingState();
	FTimerHandle SetMatchStateTimerHandle;
	//void HandleStartingState(); // Oyuncuların spawnlanması ve engine
	

	UPROPERTY(EditAnywhere, Category="Properties")
	float PurchasingStateTimeAmount = 10.f;
	float PurchasingStateTimeTrack = 0.f;
	//void HandlePurchasingState(); // Oyuncuların spawnlanması ve engine
	

	UPROPERTY(EditAnywhere, Category="Properties")
	float RoundStateTimeAmount = 120.f; // Raund zamanı
	float RoundStateTimeTrack = 0.f;
	//void HandleInProgressState(); // Oyuncuların spawnlanması ve engine

	

	
};


	
	