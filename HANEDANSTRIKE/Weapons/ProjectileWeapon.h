// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AProjectileWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere,Category="Properties")
	TSubclassOf<AProjectile> ProjectileClass;

	virtual void WeaponFireAmmo(const FHitResult& OutHitResult) override;
	virtual void SpendAmmo() override;
	
};
