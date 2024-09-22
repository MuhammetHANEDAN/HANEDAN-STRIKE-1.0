// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"

#include "HANEDANSTRIKE/Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"

class IHitInterface;

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
	if(OtherActor)
	{
		UGameplayStatics::ApplyRadialDamageWithFalloff(this,Damage,MinimumDamage,
				HitResult.ImpactPoint,DamageInnerRadius,DamageOuterRadius,DamageFallOff,
				UDamageType::StaticClass(),TArray<AActor*>(),this,
				UGameplayStatics::GetPlayerController(this,0));
		Destroy();
	}
}

