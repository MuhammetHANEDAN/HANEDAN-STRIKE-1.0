// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HANEDANPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_bCTorT,BlueprintReadOnly)
	bool bCTorT = true; // Playerın CT mi yoksa T mi olacağı defaultu CT dir

	UFUNCTION(Server,Reliable)
	void ServerSetCTorT(bool bCT);

	UFUNCTION()
	void OnRep_bCTorT();

	UPROPERTY(EditDefaultsOnly,ReplicatedUsing=OnRep_Money,BlueprintReadOnly,Category="Properties")
	int32 Money = 3000;
	UFUNCTION()
	void OnRep_Money();

	UFUNCTION(Server,Reliable)
	void ServerSetMoney(uint16 MoneyAmount); // uint16 max 65k

	UPROPERTY(Replicated,BlueprintReadOnly)
	int32 PlayerScore = 0;
	void AddPointToPlayerScore();
	void SubstractPointToPlayerScore();
	
	
};
