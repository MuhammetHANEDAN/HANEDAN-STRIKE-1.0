// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::WeaponFireAmmo(const FHitResult& OutHitResult)
{
	Super::WeaponFireAmmo(OutHitResult);

	FVector Start = GetMuzzleLocation();
	FHitResult HitResult;
	FVector End = OutHitResult.ImpactPoint + (OutHitResult.ImpactPoint-Start)*1.25;
	
	GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECC_GameTraceChannel4);

	if(HitResult.bBlockingHit)
	{
		if(HitResult.GetActor())
		{
			if(IHitInterface* Hit = Cast<IHitInterface>(HitResult.GetActor()))
			{
				Hit->GetHit(HitResult.BoneName,Damage,HeadShotDamage,GetOwner(), UGameplayStatics::GetPlayerController(this,0));
			}
		}
	}
}

