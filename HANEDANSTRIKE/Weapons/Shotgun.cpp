// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Kismet/GameplayStatics.h"


void AShotgun::SpendAmmo()
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo - 1 ,0,WeaponAmmo);
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>
	(OwnerHCharacter->Controller) : HPlayerController;
	HPlayerController->SetCharacterOverlayWeaponAmmoText(WeaponAmmo);
	
}

void AShotgun::WeaponFireAmmo(const FHitResult& OutHitResult)
{
	ABaseWeapon::WeaponFireAmmo(OutHitResult);
	/*FVector Start = GetMuzzleLocation();
	FHitResult HitResult;
	FVector End = OutHitResult.ImpactPoint + (OutHitResult.ImpactPoint-Start)*1.25;
	FPrimitiveDrawInterface::dr
	FSphere Sphere = DrawSphere()

	for (auto Element : ShotgunPellet)
	{
		
	}
	
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
	}*/
	
	
}
