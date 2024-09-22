// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHitScanWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:

	virtual void WeaponFireAmmo(const FHitResult& OutHitResult) override;
	
	
};
