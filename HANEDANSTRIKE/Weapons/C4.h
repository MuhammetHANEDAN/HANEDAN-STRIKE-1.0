// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "C4.generated.h"

class UC4Progress;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AC4 : public ABaseWeapon
{
	GENERATED_BODY()

public:

	AC4();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* C4BoxComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* C4StaticMeshComponent;

	UFUNCTION()
	void C4BoxComponentOnHit( UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void PlayerOverlappingWeapon(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(Replicated,BlueprintReadWrite)
	bool bBombPlanted = false;
	
	bool bC4DefuseTracing = false;
	
	float C4DefuseTracingTimeTrack = 0.f;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float C4DefuseTime = 5.f;
	
	void StartDefuse(APlayerController* L_PlayerController); 
	void CancelDefuse(AHANEDANPlayerController* L_hpcontroller);
	void C4Defused(); 
	
	bool bPlanting = false;
	float PlantingTimeTrack = 0.f;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	float BombPlantTime = 4.f;
	void StartPlantBomb(AHANEDANPlayerController* L_hpcontroller); 
	void CancelPlantBomb(AHANEDANPlayerController* L_hpcontroller);
	UFUNCTION(NetMulticast,Reliable)
	void MultiCast_DetachBomb();
	
	void LocalBombPlanted();
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_BombPlanted(uint8 L_BombPlantTime); // Sadece serverda çağırılmalı
	void MessageGameMode_BombPlanted();

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	UC4Progress* C4ProgressWidget;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<UC4Progress> C4ProgressWidgetClass;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	UParticleSystem* ExplodeParticle;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	USoundBase* ExplodeSound;

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_ExplodeBomb();

	void EnableAllComponentsOverlapToAmmoTrace();
	
};
