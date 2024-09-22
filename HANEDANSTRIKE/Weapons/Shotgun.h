// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:

	virtual void SpendAmmo() override;
	virtual void WeaponFireAmmo(const FHitResult& OutHitResult) override;

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint8 ShotgunPellet = 4;










	
};
