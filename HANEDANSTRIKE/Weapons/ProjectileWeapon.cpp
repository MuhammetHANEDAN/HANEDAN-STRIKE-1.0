// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"

void AProjectileWeapon::WeaponFireAmmo(const FHitResult& OutHitResult)
{
	Super::WeaponFireAmmo(OutHitResult);

	if(Owner->HasAuthority())
	{
		FActorSpawnParameters SParams;
		SParams.Owner = GetOwner();
		SParams.Instigator = Cast<APawn>(GetOwner());
		FVector OutTarget = OutHitResult.ImpactPoint;
		FVector DirectionVector = (OutTarget - GetMuzzleLocation());
		
		AProjectile* Proj = GetWorld()->SpawnActor<AProjectile>(ProjectileClass,GetMuzzleLocation(),
			DirectionVector.Rotation(),SParams);
		
		if(Proj)
		{
			Proj->SetOwner(Owner);
			Proj->HPlayerController = HPlayerController;
		}
	}
}

void AProjectileWeapon::SpendAmmo()
{
	Super::SpendAmmo();
}
