// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RocketProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API ARocketProjectile : public AProjectile
{
	GENERATED_BODY()

public:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float MinimumDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float DamageInnerRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float DamageOuterRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float DamageFallOff = 2.f;


	
};
