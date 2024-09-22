// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"

#include "C4.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Skeletal Mesh Component"));
	SetRootComponent(WeaponSkeletalMeshComponent);
	WeaponSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponSkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponSkeletalMeshComponent->SetIsReplicated(true);

	WeaponOverlapCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponOverlapCapsuleComponent"));
	WeaponOverlapCapsuleComponent->SetupAttachment(GetRootComponent());
	WeaponOverlapCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SetReplicateMovement(true);
	}
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(bFiringTriggered)
	{
		FireTrigeredTimeTrack += DeltaTime;
	}
	if(FirstFire == false)
	{
		FirstFireTimeTrack += DeltaTime;
		if(FirstFireTimeTrack >= OneShotFireRate)
		{
			FirstFire = true;
			FirstFireTimeTrack = 0;
		}
	}
	if(bOneShotted)
	{
		bOneShotTimeTrack +=DeltaTime;
		if(bOneShotTimeTrack >= OneShotFireRate)
		{
			bOneShotted = false;
			bOneShotTimeTrack = 0;
		}
	}
}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseWeapon,OwnerHCharacter);
	DOREPLIFETIME(ABaseWeapon,WeaponAmmo);
	DOREPLIFETIME(ABaseWeapon,Ammo);
	DOREPLIFETIME(ABaseWeapon,HPlayerController);
	DOREPLIFETIME(ABaseWeapon,LastOwnerHCharacter);
}

void ABaseWeapon::FirstFiredPressed(FHitResult& OutHitResult)
{
	if(WeaponAmmo == 0) return; // silahta mermi yoksa direk dön
	if(bAutomatic)
	{
		if(FirstFire)
		{
			WeaponFire(OutHitResult);
			bFiringTriggered = true;
			FireTrigeredTimeTrack = 0;
		}
	}
	else
	{
		if(bOneShotted == false)
		{
			bOneShotted = true;
			WeaponFire(OutHitResult);
		}
		/*if(FirstFire)
		{
			WeaponFire(OutHitResult);
		}*/
	}
	
}

void ABaseWeapon::FireButtonPressed(bool bPressed, FHitResult& OutHitResult) // Triggering -> true completed -> false
{
	if(WeaponAmmo == 0) return; // silahta mermi yoksa direk dön
	if(bPressed) // Fire buttona basılmaya başnadı
	{
		if(bAutomatic)
		{
			if(FireTrigeredTimeTrack >= OtoFireRate && bFiringTriggered  && FirstFire)
			{
				if(bRecoilWeapon)
				{
					Recoil();
				}
				WeaponFire(OutHitResult);
				FireTrigeredTimeTrack = 0;
			}
			else
			{
				
			}
		}
		else // Oto olmayanlar için
		{
			
		}
	}
	else // Fire butona basılı tutulma bırakıldı
	{
		if(bAutomatic)
		{
			FirstFire = false;
			FireTrigeredTimeTrack = 0;
			/*if(FireTrigeredTimeTrack >= OtoFireRate)
			{
				
			}
			else
			{
				
			}*/
		}
		else // oto olmayanlar için
		{
			//FirstFire = false;
		}
	}
	
}

void ABaseWeapon::WeaponFire(FHitResult& OutHitResult)
{
	if(!Owner->HasAuthority())
	{
		LocalFireEffects();
	}
	ServerWeaponFire(OutHitResult);
}

void ABaseWeapon::ServerWeaponFire_Implementation(const FHitResult& OutHitResult)
{
	LocalFireEffects();
	Multicast_WeaponFire(); // For Sim Proxies Local effects
	WeaponFireAmmo(OutHitResult); // Weapon Serverde Ateşlenecek
}

void ABaseWeapon::Multicast_WeaponFire_Implementation()
{
	if(Owner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		LocalFireEffects();
	}
	else
	{
		
	}
}

void ABaseWeapon::LocalFireEffects()
{
	if(WeaponFireAnimation)
	{
		WeaponSkeletalMeshComponent->PlayAnimation(WeaponFireAnimation,false);
	}
	else
	{
		if(MuzzleFireEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this,MuzzleFireEffect,
			WeaponSkeletalMeshComponent->GetSocketLocation(MuzzleName),
			WeaponSkeletalMeshComponent->GetSocketRotation(MuzzleName));
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,FireSound,GetActorLocation());
		}
	}
}

void ABaseWeapon::WeaponFireAmmo(const FHitResult& OutHitResult) // Sadece Serverda Çalışmaktadır
{
	SpendAmmo();
}

void ABaseWeapon::SpendAmmo() // Sadece serverda çalışmaktadır
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo - 1 ,0,WeaponAmmo);
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>
	(OwnerHCharacter->Controller) : HPlayerController;
	HPlayerController->SetCharacterOverlayWeaponAmmoText(WeaponAmmo);
}

bool ABaseWeapon::IsWeaponFull() const
{
	return WeaponAmmo == MaxWeaponAmmo;
}

bool ABaseWeapon::IsWeaponEmpty() const 
{
	return WeaponAmmo == 0;
}

void ABaseWeapon::OnRep_WeaponAmmo() 
{
	
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>
	(OwnerHCharacter->Controller) : HPlayerController;
	
	if(HPlayerController == nullptr) return;
	if(HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayWeaponAmmoText(WeaponAmmo);
	}
}

void ABaseWeapon::OnRep_Ammo()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>
	(OwnerHCharacter->Controller) : HPlayerController;
	if(HPlayerController == nullptr) return;
	
	if(HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayAmmoText(Ammo);
	}
}

void ABaseWeapon::Server_PlayReloadMontage_Implementation()
{
	MultiCast_PlayReloadMontage();
}

void ABaseWeapon::MultiCast_PlayReloadMontage_Implementation()
{
	UAnimInstance* AnimInstance = OwnerHCharacter->GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
	}
}

void ABaseWeapon::Reload()
{
	if(Ammo == 0) return;
	if(Owner->HasAuthority())
	{
		uint16 ReloadAmount = MaxWeaponAmmo - WeaponAmmo;
		if(Ammo >= ReloadAmount)
		{
			Ammo = Ammo - ReloadAmount;
			WeaponAmmo = WeaponAmmo + ReloadAmount;
		}
		else if (Ammo < ReloadAmount)
		{
			WeaponAmmo = WeaponAmmo + Ammo;
			Ammo = 0;
		}
		if(HPlayerController && HPlayerController->HanedanHUD) // Eğer serverdeki owner ise
		{
			HPlayerController->SetCharacterOverlayWeaponAmmoText(WeaponAmmo);
			HPlayerController->SetCharacterOverlayAmmoText(Ammo);
		}
	}
}

void ABaseWeapon::HandleWeaponDropped()
{
	if(ActorHasTag(FName("AWP")))
	{
		CloseScope();
	}
	
	OwnerHCharacter = nullptr;
	HPlayerController = nullptr;
	bWeaponOnGround = true;

	if(HasAuthority())
	{
		if(!WeaponOverlapCapsuleComponent->OnComponentBeginOverlap.IsAlreadyBound(this,&ABaseWeapon::PlayerOverlappingWeapon))
		{
			UE_LOG(LogTemp,Error,TEXT("overlap dynamic eklendi"));
			WeaponOverlapCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this,&ABaseWeapon::PlayerOverlappingWeapon);
		}
		WeaponOverlapCapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel2);
		WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2,ECR_Overlap);
		WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
		WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Overlap);
	}
	
	WeaponOverlapCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	FVector ActorForwardVector = LastOwnerHCharacter->GetActorForwardVector();
	ActorForwardVector.Y = 0;
	ActorForwardVector.Z=0;
	FVector ImpulseVector = ActorForwardVector*1000;

	if(ActorHasTag(FName("C4")))
	{
		AC4* L_C4 = Cast<AC4>(this);

		WeaponSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
		WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
		WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Overlap);
		
		L_C4->C4StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		L_C4->C4StaticMeshComponent->SetCollisionObjectType(ECC_GameTraceChannel2);
		
		L_C4->C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
		L_C4->C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
		L_C4->C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Overlap);
		
		L_C4->C4StaticMeshComponent->SetEnableGravity(true);
		L_C4->C4StaticMeshComponent->SetSimulatePhysics(true);
		L_C4->C4StaticMeshComponent->AddImpulse(ImpulseVector);

		// son eklenen defusede sıkıntı olursa sil
		L_C4->C4BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Overlap);
		
		return;
	}
	WeaponSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Overlap);
	WeaponSkeletalMeshComponent->SetEnableGravity(true);
	WeaponSkeletalMeshComponent->SetSimulatePhysics(true);
	WeaponSkeletalMeshComponent->AddImpulse(ImpulseVector);
}

void ABaseWeapon::HandleWeaponTakenFromGround()
{
	if(ActorHasTag(FName("C4")))
	{
		AC4* L_C4 = Cast<AC4>(this);
		L_C4->C4StaticMeshComponent->SetEnableGravity(false);
		L_C4->C4StaticMeshComponent->SetSimulatePhysics(false);
		L_C4->C4StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bWeaponOnGround = false;
		return;
	}
	WeaponSkeletalMeshComponent->SetEnableGravity(false);
	WeaponSkeletalMeshComponent->SetSimulatePhysics(false);
	WeaponOverlapCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bWeaponOnGround = false;
}

void ABaseWeapon::PlayerOverlappingWeapon(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp,Error,TEXT("base wepaon overlaplanıyor"));
	if(OtherActor->ActorHasTag(FName("CT")) || OtherActor->ActorHasTag(FName("T")))
	{
		if(AHANEDANCharacter* OverlappedCharacter = Cast<AHANEDANCharacter>(OtherActor))
		{
			UE_LOG(LogTemp,Error,TEXT("ct veya t overlaplıyor"));
			OverlappedCharacter->CombatComponent->TakeWeaponFromGround(this);
		}
	}
}

FTransform ABaseWeapon::GetMuzzleTransform() const
{
	if(WeaponSkeletalMeshComponent)
	{
		return WeaponSkeletalMeshComponent->GetSocketTransform(MuzzleName);
	}
	return FTransform();
}

FVector ABaseWeapon::GetMuzzleLocation() const
{
	if(WeaponSkeletalMeshComponent)
	{
		return WeaponSkeletalMeshComponent->GetSocketLocation(MuzzleName);
	}
	return FVector();
}

void ABaseWeapon::Server_Scope()
{
	ScopeX++;
	
	if(OwnerHCharacter->HasAuthority() && HPlayerController->IsLocalController())
	{
		if(ScopeX == 1)
		{
			OwnerHCharacter->SetCameraFOV(ScopeX1FOV);
			HPlayerController->AddAWPScopeWidgetToHUD();
			HPlayerController->SetAWPScopeWidgetVisibility(true);
		}
		else if(ScopeX == 2)
		{
			OwnerHCharacter->SetCameraFOV(ScopeX2FOV);
		}
		else if (ScopeX == 3)
		{
			CloseScope();
		}
	}
	else
	{
		if(ScopeX == 1)
		{
			Client_Scope(1);
		}
		else if(ScopeX == 2)
		{
			Client_Scope(2);
		}
		else if (ScopeX == 3)
		{
			Client_Scope(3);
			ScopeX = 0;
		}
	}
}

void ABaseWeapon::Client_Scope_Implementation(uint8 LScopeX)
{
	if(HPlayerController == nullptr) return;
	if(OwnerHCharacter == nullptr) return;
	if(!HPlayerController->IsLocalController()) return;
	ScopeX = LScopeX;
	if(ScopeX == 1)
	{
		OwnerHCharacter->SetCameraFOV(ScopeX1FOV);
		HPlayerController->AddAWPScopeWidgetToHUD();
		HPlayerController->SetAWPScopeWidgetVisibility(true);
	}
	else if(ScopeX == 2)
	{
		OwnerHCharacter->SetCameraFOV(ScopeX2FOV);
	}
	else if (ScopeX == 3)
	{
		if(HPlayerController == nullptr) return;
		if(OwnerHCharacter == nullptr) return;
		if(OwnerHCharacter->CombatComponent == nullptr) return;
		ScopeX = 0;
		OwnerHCharacter->SetCameraFOVtoDefault();
		OwnerHCharacter->CombatComponent->Server_SetbAimingVariable(false);
		if(HPlayerController && HPlayerController->HanedanHUD)
		{
			HPlayerController->Local_SetAWPScopeVisibility(false);
		}
	}
}

void ABaseWeapon::CloseScope()
{
	if(HPlayerController == nullptr) return;
	if(OwnerHCharacter == nullptr) return;
	if(OwnerHCharacter->CombatComponent == nullptr) return;
	ScopeX = 0;
	OwnerHCharacter->SetCameraFOVtoDefault();
	OwnerHCharacter->CombatComponent->Server_SetbAimingVariable(false);
	if(HPlayerController && HPlayerController->HanedanHUD)
	{
		HPlayerController->Local_SetAWPScopeVisibility(false);
	}
	if(HasAuthority())
	{
		Client_Scope(3);
	}
}

void ABaseWeapon::Recoil()
{
	if(HPlayerController && HPlayerController->IsLocalController())
	{
		uint8 YawOrPitchSelection = FMath::RandRange(0,1);
		if(YawOrPitchSelection == 0)
		{
			HPlayerController->AddYawInput(RecoilAmount);
		}
		else if(YawOrPitchSelection == 1)
		{
			HPlayerController->AddPitchInput(RecoilAmount);
		}
	}
}

