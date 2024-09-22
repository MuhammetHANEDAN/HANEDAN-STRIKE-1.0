// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "HANEDANSTRIKE/Character/HanedanAnimInstance.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "HANEDANSTRIKE/PlayerStates/HANEDANPlayerState.h"
#include "HANEDANSTRIKE/Weapons/BaseWeapon.h"
#include "HANEDANSTRIKE/Weapons/C4.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Health = MaxHealth;

	// ...
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwner()->HasAuthority())
	{
		/** Server rpc olmasına rağmen otorite checki yaptım çünkü beginplayde serverdan çağırmadan sıkıntı olabiliyor */
		Server_SpawnWeapon(2,7); // Başlangıçta pistol veriyoruz
	}
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent,AllWeapons);
	DOREPLIFETIME(UCombatComponent,CurrentWeaponOnHand);
	DOREPLIFETIME(UCombatComponent,WeaponSlot1);
	DOREPLIFETIME(UCombatComponent,WeaponSlot2);
	DOREPLIFETIME(UCombatComponent,WeaponSlot3);
	DOREPLIFETIME(UCombatComponent,bWeaponOnHand);
	DOREPLIFETIME(UCombatComponent,bAiming);
	DOREPLIFETIME(UCombatComponent,Health);
	DOREPLIFETIME(UCombatComponent,Shield);
	DOREPLIFETIME_CONDITION(UCombatComponent,bCanPlantBomb,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent,PlantedBombTransfrom);
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::ReloadButtonPressed()
{
	if(CurrentWeaponOnHand && !CurrentWeaponOnHand->IsWeaponFull() && OwnerHCharacter->CurrentPlayerState == EPlayerState::EPS_Unoccupied
		&& CurrentWeaponOnHand->Ammo >0)
	{
		CurrentWeaponOnHand->Server_PlayReloadMontage();
		if(HPlayerController && CurrentWeaponOnHand->ActorHasTag(FName("AWP")) && HPlayerController->IsLocalController()) // elimizdeki silah awp ve localconroller isek
		{
			OwnerHCharacter->SetCameraFOVtoDefault();
			CurrentWeaponOnHand->CloseScope();
		}
	}
}

void UCombatComponent::OnRep_bWeaponOnHand(bool LastValue)
{
	if(LastValue)
	{
		//
	}
	else
	{
		// yaw rot false
	}
}

void UCombatComponent::Server_ServerSetAiming_Implementation(bool Aiming)
{
	if(CurrentWeaponOnHand && CurrentWeaponOnHand->ActorHasTag(FName("AWP"))) // elinde awp varsa
	{
		if(Aiming)
		{
			bAiming = true;
			OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 150.f;
			CurrentWeaponOnHand->Server_Scope();
		}
	}
	else if(CurrentWeaponOnHand)
	{
		bAiming = Aiming;
		if(bAiming)
		{
			OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 150.f;
		}
		else
		{
			OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.f;
		}
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if(bAiming)
	{
		bAiming = true;
		OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 150.f;
	}
	else
	{
		bAiming = false;
		OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void UCombatComponent::Server_SetbAimingVariable_Implementation(bool Aiming)
{
	if(OwnerHCharacter == nullptr) return;
	bAiming = Aiming;
	if(bAiming)
	{
		OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 150.f;
	}
	else
	{
		OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void UCombatComponent::BuyWeapon(uint16 WeaponPrice, uint8 Slot, uint8 WeaponNumber)
{
	HPlayerState = HPlayerState==nullptr ? Cast<AHANEDANPlayerState>(OwnerHCharacter->GetPlayerState()) : HPlayerState;
	if(HPlayerState && HPlayerState->Money >= WeaponPrice && GetWeaponOnSlot(Slot) == nullptr) // istenilen slot boşsa
	{
		uint16 NewMoney = FMath::Clamp(HPlayerState->Money-WeaponPrice,0,65000);
		HPlayerState->ServerSetMoney(NewMoney);
		Server_SpawnWeapon(Slot,WeaponNumber);
	}
}

void UCombatComponent::BuyKevlarHelmet()
{
	if(Shield == MaxShield) return;

	if(OwnerHCharacter->HasAuthority()) // Serverde local controllerdaki karakterdeyiz
	{
		if(Shield>0)
		{
			Shield = MaxShield;
			HPlayerState = HPlayerState == nullptr ? Cast<AHANEDANPlayerState>(OwnerHCharacter->GetPlayerState()) : HPlayerState;
			HPlayerState->Money = FMath::Clamp(HPlayerState->Money-300.f,0,65000.f);
			HPlayerController->SetCharacterOverlayMoneyText(HPlayerState->Money);
			HPlayerController->SetCharacterOverlayShieldText(Shield);
		}
		else
		{
			Shield = MaxShield;
			HPlayerState = HPlayerState == nullptr ? Cast<AHANEDANPlayerState>(OwnerHCharacter->GetPlayerState()) : HPlayerState;
			HPlayerState->Money = (FMath::Clamp(HPlayerState->Money-1000.f,0,65000.f));
			HPlayerController->SetCharacterOverlayMoneyText(HPlayerState->Money);
			HPlayerController->SetCharacterOverlayShieldText(Shield);
		}
	}
	else // Owner Clientteyiz
	{
		Server_BuyKevlarHelmet();
	}
}

void UCombatComponent::Server_BuyKevlarHelmet_Implementation()
{
	if(Shield>0)
	{
		Shield = MaxShield; // hud shielddaki değer onrep_shield ile setlenmekte
		// hud shielddaki money OnRep_money  ile setlenmekte
		HPlayerState = HPlayerState == nullptr ? Cast<AHANEDANPlayerState>(OwnerHCharacter->GetPlayerState()) : HPlayerState;
		HPlayerState->Money = FMath::Clamp(HPlayerState->Money-300.f,0,65000.f);
	}
	else
	{
		Shield = MaxShield;
		HPlayerState = HPlayerState == nullptr ? Cast<AHANEDANPlayerState>(OwnerHCharacter->GetPlayerState()) : HPlayerState;
		HPlayerState->Money = (FMath::Clamp(HPlayerState->Money-1000.f,0,65000.f));
	}
}

void UCombatComponent::Server_SpawnWeapon_Implementation(uint8 WeaponSlotNumber, uint8 WeaponNumber)
{
	if(GetWeaponClass(WeaponNumber) == nullptr) return;
	
	FActorSpawnParameters SParams;
	SParams.Owner = GetOwner();
	SParams.Instigator = Cast<APawn>(GetOwner());
	ABaseWeapon* Weapon = nullptr;
	Weapon = GetWorld()->SpawnActor<ABaseWeapon>(GetWeaponClass(WeaponNumber),SParams);
	Weapon->SetOwner(GetOwner());
	Weapon->OwnerHCharacter = OwnerHCharacter;
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(OwnerHCharacter->GetController()) : HPlayerController;
	if(HPlayerController == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("Spawnlanan weaponun hplayercontrollerı null setlenmiştir"));
	}
	if(HPlayerController)
	{
		UE_LOG(LogTemp,Error,TEXT("Spawnlanan weaponun hplayercontrollerı doğruu setlenmiştir"));
	}
	Weapon->HPlayerController = HPlayerController;
	if(WeaponSlotNumber == 1)
	{
		WeaponSlot1=Weapon;
	}
	else if(WeaponSlotNumber == 2)
	{
		WeaponSlot2=Weapon;
	}
	else if(WeaponSlotNumber == 3)
	{
		WeaponSlot3=Weapon;
	}
	// TODO Weapon slot 4 grenade için yapıalcak

	if (Weapon)
	{
		AllWeapons.AddUnique(Weapon); // spawnlanan weaponu tüm weaponlara ekle 
	}

	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	GetWeaponOnSlot(WeaponSlotNumber)->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,GetBackSocketName(WeaponSlotNumber));
	
}

ABaseWeapon* UCombatComponent::GetWeaponOnSlot(uint8 Slot)
{
	ABaseWeapon* ResultWeaponSlot = nullptr;
	
	switch (Slot)
	{
	default:
		ResultWeaponSlot = WeaponSlot1;
		break;
	case 1:
		ResultWeaponSlot = WeaponSlot1;
		break;
	case 2:
		ResultWeaponSlot = WeaponSlot2;
		break;
	case 3:
		ResultWeaponSlot = WeaponSlot3;
		break;
	}
	return ResultWeaponSlot;
}

TSubclassOf<ABaseWeapon> UCombatComponent::GetWeaponClass(uint8 WeaponNumber)
{
	TSubclassOf<ABaseWeapon> WeaponClass;

	switch (WeaponNumber)
	{
	default:
		WeaponClass = Mac10WeaponClass;
		break;
	case 1:
		WeaponClass = Mac10WeaponClass;
		break;
	case 2:
		WeaponClass = M3WeaponClass;
		break;
	case 3:
		WeaponClass = Ak47WeaponClass;
		break;
	case 4:
		WeaponClass = M4A1WeaponClass;
		break;
	case 5:
		WeaponClass = AWPWeaponClass;
		break;
	case 6:
		WeaponClass = RocketLauncherWeaponClass;
		break;
	case 7:
		WeaponClass = PistolWeaponClass;
		break;
	}
	
	return WeaponClass;
}

void UCombatComponent::OnRep_CurrentWeaponOnHand()
{
	// TODO  kullanmıyoruz şu an silinebilir
}

FName UCombatComponent::GetBackSocketName(uint8 SlotNumber) const
{
	FName ResultName;
	switch (SlotNumber)
	{
	default:
		ResultName = RifleBackSocketName;
		break;
	case 1:
		ResultName = RifleBackSocketName;
		break;
	case 2:
		ResultName = PistolBackSocketName;
		break;
	case 3:
		if(CurrentWeaponOnHand->ActorHasTag(FName("C4")))
		{
			ResultName = FName("C4Socket");
		}
		else
		{
			ResultName = KnifeBackSocketName;
		}
		break;
	}
	return ResultName;
}

void UCombatComponent::Server_SwapWeapon_Implementation(uint8 SlotNumber)
{
	MultiCast_SwapWeapon(SlotNumber);
}


void UCombatComponent::MultiCast_SwapWeapon_Implementation(uint8 SlotNumber)
{
	HanedanAnimInstance = HanedanAnimInstance == nullptr ? Cast<UHanedanAnimInstance>(OwnerHCharacter->GetMesh()->GetAnimInstance()) : HanedanAnimInstance;
	if(OwnerHCharacter->CurrentPlayerState == EPlayerState::EPS_Equiping) return;
	FDetachmentTransformRules DetachmentTransformRules = FDetachmentTransformRules::KeepWorldTransform;

	if(GetWeaponOnSlot(SlotNumber)) // istenilen slotta weapon varmı
	{
		if(CurrentWeaponOnHand) // elimizde weapon varmı
		{
			if(GetWeaponOnSlot(SlotNumber)->WeaponSlotNumber != CurrentWeaponOnHand->WeaponSlotNumber) // istenilen slot elimizdeki slottan farklımı
			{
				CurrentWeaponOnHand->DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DetachmentTransformRules);
				AttachCurrentWeaponToBack(CurrentWeaponOnHand,GetBackSocketName(CurrentWeaponOnHand->WeaponSlotNumber));
				CurrentWeaponOnHand = GetWeaponOnSlot(SlotNumber);
				if(CurrentWeaponOnHand->EquippingAnimMontage == nullptr) return;
				HanedanAnimInstance->Montage_Play(CurrentWeaponOnHand->EquippingAnimMontage);

				if(CurrentWeaponOnHand->ActorHasTag(FName("AWP"))) //awp nin scopeu kapadık
				{
					CurrentWeaponOnHand->CloseScope();
					OwnerHCharacter->SetCameraFOVtoDefault();
				}
				
			}
			else if (GetWeaponOnSlot(SlotNumber)->WeaponSlotNumber == CurrentWeaponOnHand->WeaponSlotNumber) // istenilen slot elimizdeki slotla aynımı
			{
				CurrentWeaponOnHand->DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DetachmentTransformRules);
				AttachCurrentWeaponToBack(CurrentWeaponOnHand,GetBackSocketName(CurrentWeaponOnHand->WeaponSlotNumber));
			}
		}
		else // elimizde weapon yoksa
		{
			CurrentWeaponOnHand = GetWeaponOnSlot(SlotNumber);
			if(CurrentWeaponOnHand->EquippingAnimMontage == nullptr) return;
			HanedanAnimInstance->Montage_Play(CurrentWeaponOnHand->EquippingAnimMontage);
		}
	}
	else
	{
		// o slotta weapon yok
	}
}

void UCombatComponent::AttachWeaponToHand()
{
	if(OwnerHCSkeletalMeshComponent == nullptr || CurrentWeaponOnHand == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("OwnerHCSkeletalMeshComponent == nullptr || L_WeaponSlot == nullptr"));
		return;
	}
	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	if(CurrentWeaponOnHand->ActorHasTag(FName("C4")))
	{
		CurrentWeaponOnHand->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,C4HandSocket);
		FAttachmentTransformRules TransformRules1 = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		TransformRules1.ScaleRule=EAttachmentRule::KeepRelative;
		Cast<AC4>(CurrentWeaponOnHand)->C4StaticMeshComponent->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules1,RiflePKRightHandSocket);
	}
	else
	{
		CurrentWeaponOnHand->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,RiflePKRightHandSocket);
	}
	bWeaponOnHand = true;
	OwnerHCharacter->CurrentWeaponType = CurrentWeaponOnHand->WeaponType;
	OwnerHCharacter->SetCharacterForbWeaponOnHand(true);
	// HUD works
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(OwnerHCharacter->Controller):HPlayerController;
	if(HPlayerController && HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayWeaponAmmoText(CurrentWeaponOnHand->WeaponAmmo);
		HPlayerController->SetCharacterOverlayAmmoText(CurrentWeaponOnHand->Ammo);
	}
}

void UCombatComponent::AttachCurrentWeaponToBack(ABaseWeapon* WeaponToAttach,const FName& SocketName)
{
	if(OwnerHCSkeletalMeshComponent == nullptr || WeaponToAttach == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("OwnerHCSkeletalMeshComponent == nullptr || L_WeaponSlot == nullptr"));
		return;
	}
	
	if(CurrentWeaponOnHand->ActorHasTag(FName("AWP"))) //awp nin scopeu kapadık
	{
		CurrentWeaponOnHand->CloseScope();
		OwnerHCharacter->SetCameraFOVtoDefault();
	}
	
	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	WeaponToAttach->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,SocketName);

	if(WeaponToAttach->ActorHasTag(FName("C4")))
	{
		FAttachmentTransformRules TransformRules1 = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		TransformRules1.ScaleRule=EAttachmentRule::KeepRelative;
		Cast<AC4>(WeaponToAttach)->C4StaticMeshComponent->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules1,FName("C4Socket"));
	}
	
	bWeaponOnHand = false;
	CurrentWeaponOnHand = nullptr;
	OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
	OwnerHCharacter->SetCharacterForbWeaponOnHand(false);
	// HUD works
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(OwnerHCharacter->Controller):HPlayerController;
	if(HPlayerController && HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayWeaponAmmoText(0);
		HPlayerController->SetCharacterOverlayAmmoText(0);
	}
}

void UCombatComponent::Multicast_AttachWeaponToBack_Implementation(ABaseWeapon* WeaponToAttach, const FName& SocketName)
{
	if(OwnerHCSkeletalMeshComponent == nullptr || WeaponToAttach == nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("OwnerHCSkeletalMeshComponent == nullptr || L_WeaponSlot == nullptr"));
		return;
	}
	
	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	WeaponToAttach->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,SocketName);
}

void UCombatComponent::FirstFiredButtonPressed()
{
	bFireButtonPressed = true;
	if(OwnerHCharacter->CurrentPlayerState == EPlayerState::EPS_Unoccupied && CurrentWeaponOnHand &&
		CurrentWeaponOnHand->ActorHasTag(FName("C4")) && bCanPlantBomb)
	{
		if(OwnerHCharacter->bWasPlanting) return;
		AC4* C4 = Cast<AC4>(CurrentWeaponOnHand);
		OwnerHCharacter->bWasPlanting = true;
		OwnerHCharacter->PlantingBomb = C4;
		C4->StartPlantBomb(HPlayerController);
		return;
	}
	if(OwnerHCharacter->CurrentPlayerState == EPlayerState::EPS_Unoccupied && CurrentWeaponOnHand)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		CurrentWeaponOnHand->FirstFiredPressed(HitResult);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bPressed == false)
	{
		if(OwnerHCharacter->bWasPlanting)
		{
			OwnerHCharacter->bWasPlanting = false;
			if(OwnerHCharacter->PlantingBomb)
			{
				OwnerHCharacter->PlantingBomb->CancelPlantBomb(HPlayerController);
				OwnerHCharacter->PlantingBomb = nullptr;
			}
			return;
		}
	}
	if(OwnerHCharacter->CurrentPlayerState == EPlayerState::EPS_Unoccupied && CurrentWeaponOnHand)
	{
		if(bPressed)
		{
			FHitResult OutHitResult;
			TraceUnderCrosshair(OutHitResult);
			CurrentWeaponOnHand->FireButtonPressed(true, OutHitResult);
		}
		else
		{
			FHitResult OutHitResult;
			TraceUnderCrosshair(OutHitResult);
			CurrentWeaponOnHand->FireButtonPressed(false, OutHitResult);
		}
		
	}
}

void UCombatComponent::OnRep_Health()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(OwnerHCharacter->GetController()) : HPlayerController;
	if(HPlayerController == nullptr)
	{
		return;
	}
	if(HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayHealthText(Health);
	}
}

void UCombatComponent::OnRep_Shield()
{
	HPlayerController = HPlayerController == nullptr ? Cast<AHANEDANPlayerController>(OwnerHCharacter->GetController()) : HPlayerController;
	if(HPlayerController == nullptr)
	{
		return;
	}
	if(HPlayerController->IsLocalController())
	{
		HPlayerController->SetCharacterOverlayShieldText(Shield);
	}
}

void UCombatComponent::DropCurrentWeaponOnHand()
{
	if(CurrentWeaponOnHand == nullptr || CurrentWeaponOnHand->WeaponSlotNumber == 2) return; // sadece ilk slot ve 3. slottaki yere atılabilir
	if(OwnerHCharacter->HasAuthority())
	{
		AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
		if(Gmode)
		{
			Gmode->AddWeaponToWeaponsOnGround(GetWeaponOnSlot(CurrentWeaponOnHand->WeaponSlotNumber));
		}
		MultiCast_DropCurrentWeaponOnHand();
	}
	else
	{
		Server_DropCurrentWeaponOnHand();
	}
}

void UCombatComponent::Server_DropCurrentWeaponOnHand_Implementation()
{
	AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
	if(Gmode)
	{
		Gmode->AddWeaponToWeaponsOnGround(GetWeaponOnSlot(CurrentWeaponOnHand->WeaponSlotNumber));
	}
	MultiCast_DropCurrentWeaponOnHand();
}

void UCombatComponent::MultiCast_DropCurrentWeaponOnHand_Implementation()
{
	if(HPlayerController && CurrentWeaponOnHand->ActorHasTag(FName("AWP")) && HPlayerController->IsLocalController()) // elimizdeki silah awp ve localconroller isek
	{
		OwnerHCharacter->SetCameraFOVtoDefault();
		CurrentWeaponOnHand->CloseScope();
	}
	
	bAiming = false;
	bWeaponOnHand = false;
	OwnerHCharacter->GetCharacterMovement()->MaxWalkSpeed = 600;
	//OwnerHCharacter->CurrentPlayerState = EPlayerState::EPS_Unoccupied;
	OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
	OwnerHCharacter->SetCharacterForbWeaponOnHand(false);

	FDetachmentTransformRules DAttachRules = FDetachmentTransformRules::KeepWorldTransform;
	CurrentWeaponOnHand->DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DAttachRules);

	CurrentWeaponOnHand->LastOwnerHCharacter = OwnerHCharacter;
	CurrentWeaponOnHand->HandleWeaponDropped();

	if(CurrentWeaponOnHand->WeaponSlotNumber == 1)
	{
		WeaponSlot1 = nullptr;
	}
	else if(CurrentWeaponOnHand->WeaponSlotNumber == 3)
	{
		WeaponSlot3 = nullptr;
	}
	
	CurrentWeaponOnHand = nullptr;

	if(HPlayerController && HPlayerController->IsLocalController())
	{
		HPlayerController->InitializeCharacterOverlayWidget();
	}
}

void UCombatComponent::DropFirstWeapon()
{
	if(WeaponSlot1 == nullptr) return;
	if(OwnerHCharacter->HasAuthority())
	{
		AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
		if(Gmode)
		{
			Gmode->AddWeaponToWeaponsOnGround(WeaponSlot1);
		}
		MultiCast_DropFirstWeapon();
	}
	else
	{
		Server_DropFirstWeapon();
	}
}

void UCombatComponent::Server_DropFirstWeapon_Implementation()
{
	AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
	if(Gmode)
	{
		Gmode->AddWeaponToWeaponsOnGround(WeaponSlot1);
	}
	MultiCast_DropFirstWeapon();
}

void UCombatComponent::MultiCast_DropFirstWeapon_Implementation()
{
	OwnerHCharacter->SetCharacterForbWeaponOnHand(false);

	FDetachmentTransformRules DAttachRules = FDetachmentTransformRules::KeepWorldTransform;
	WeaponSlot1->DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DAttachRules);

	WeaponSlot1->LastOwnerHCharacter = OwnerHCharacter;
	
	if(CurrentWeaponOnHand == WeaponSlot1)
	{
		CurrentWeaponOnHand = nullptr;
		WeaponSlot1->HandleWeaponDropped();
		WeaponSlot1 = nullptr;
		OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
	}
	else
	{
		WeaponSlot1->HandleWeaponDropped();
		WeaponSlot1 = nullptr;
	}

	if(HPlayerController && HPlayerController->IsLocalController())
	{
		HPlayerController->InitializeCharacterOverlayWidget();
	}
}

void UCombatComponent::DropThirdWeapon()
{
	if(WeaponSlot3 == nullptr) return;
	if(OwnerHCharacter->HasAuthority())
	{
		AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
		if(Gmode)
		{
			Gmode->AddWeaponToWeaponsOnGround(WeaponSlot3);
		}
		MultiCast_DropThirdWeapon();
	}
	else
	{
		Server_DropThirdWeapon();
	}
}

void UCombatComponent::Server_DropThirdWeapon_Implementation()
{
	AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
	if(Gmode)
	{
		Gmode->AddWeaponToWeaponsOnGround(WeaponSlot3);
	}
	MultiCast_DropThirdWeapon();
}

void UCombatComponent::MultiCast_DropThirdWeapon_Implementation()
{
	OwnerHCharacter->SetCharacterForbWeaponOnHand(false);

	FDetachmentTransformRules DAttachRules = FDetachmentTransformRules::KeepWorldTransform;
	WeaponSlot3->DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DAttachRules);

	WeaponSlot3->LastOwnerHCharacter = OwnerHCharacter;
	
	if(CurrentWeaponOnHand == WeaponSlot3)
	{
		CurrentWeaponOnHand = nullptr;
		WeaponSlot3->HandleWeaponDropped();
		WeaponSlot3 = nullptr;
		OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
	}
	else
	{
		WeaponSlot3->HandleWeaponDropped();
		WeaponSlot3 = nullptr;
	}

	if(HPlayerController && HPlayerController->IsLocalController())
	{
		HPlayerController->InitializeCharacterOverlayWidget();
	}
}

void UCombatComponent::TakeWeaponFromGround(ABaseWeapon* WeaponToTake)
{
	if(GetWeaponOnSlot(WeaponToTake->WeaponSlotNumber)) return;
	if(CurrentWeaponOnHand && CurrentWeaponOnHand->WeaponSlotNumber == WeaponToTake->WeaponSlotNumber) return;
	//if(GetWeaponOnSlot(1) != nullptr || GetWeaponOnSlot(3) != nullptr) return;
	if(WeaponToTake->WeaponSlotNumber == 2) return;
	if(OwnerHCharacter->HasAuthority())
	{
		AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
		if(Gmode)
		{
			Gmode->ClearWeaponReference(WeaponToTake);
		}
		if(WeaponToTake->WeaponSlotNumber == 1)
		{
			WeaponSlot1 = WeaponToTake;
			WeaponSlot1->SetOwner(OwnerHCharacter);
			//WeaponSlot1->OwnerHCharacter = OwnerHCharacter;
			//WeaponSlot1->HPlayerController=HPlayerController;
		}
		else if (WeaponToTake->WeaponSlotNumber == 3)
		{
			WeaponSlot3 = WeaponToTake;
			WeaponSlot3->SetOwner(OwnerHCharacter);
			//WeaponSlot3->OwnerHCharacter = OwnerHCharacter;
			//WeaponSlot3->HPlayerController=HPlayerController;
		}
		
		MultiCast_TakeWeaponFromGround(WeaponToTake);
	}
	else
	{
		Server_TakeWeaponFromGround(WeaponToTake);
	}
}

void UCombatComponent::Server_TakeWeaponFromGround_Implementation(ABaseWeapon* WeaponToTake)
{
	AHANEDANSTRIKEGameModeBase* Gmode = GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>();
	if(Gmode)
	{
		Gmode->ClearWeaponReference(WeaponToTake);
	}
	MultiCast_TakeWeaponFromGround(WeaponToTake);
}

void UCombatComponent::MultiCast_TakeWeaponFromGround_Implementation(ABaseWeapon* WeaponToTake)
{
	WeaponToTake->HandleWeaponTakenFromGround();
	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	FAttachmentTransformRules TransformRules1 = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	TransformRules1.ScaleRule=EAttachmentRule::KeepRelative;
	if(WeaponToTake->ActorHasTag(FName("C4")))
	{
		WeaponToTake->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,FName("C4Socket"));
		Cast<AC4>(WeaponToTake)->C4StaticMeshComponent->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules1,FName("C4Socket"));
	}
	else
	{
		WeaponToTake->AttachToComponent(OwnerHCSkeletalMeshComponent,TransformRules,RifleBackSocketName);
	}

	if(WeaponToTake->WeaponSlotNumber == 1)
	{
		WeaponSlot1 = WeaponToTake;
		WeaponSlot1->Owner = OwnerHCharacter;
		WeaponSlot1->OwnerHCharacter = OwnerHCharacter;
		WeaponSlot1->HPlayerController = HPlayerController;
	}
	else if (WeaponToTake->WeaponSlotNumber == 3)
	{
		WeaponSlot3 = WeaponToTake;
		WeaponSlot3->Owner = OwnerHCharacter;
		WeaponSlot3->OwnerHCharacter = OwnerHCharacter;
		WeaponSlot3->HPlayerController = HPlayerController;
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& OutHitResult)
{
	FVector2d ViewportSize;
	
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2d CrosshairLocationOnViewport = FVector2d(ViewportSize.X/2.f,ViewportSize.Y/2.f);
	FVector WorldLocation;
	FVector WorldDirection;
	bool bProjected =UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocationOnViewport,WorldLocation,WorldDirection);

	if(bProjected)
	{
		FVector End = WorldLocation + WorldDirection*TRACE_LENGTH;
		
		GetWorld()->LineTraceSingleByChannel(OutHitResult,WorldLocation,End,ECC_GameTraceChannel4);
		if(!OutHitResult.bBlockingHit)
		{
			OutHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::Server_DefuseTrace_Implementation(const FHitResult& OutHitResult1, const FVector& WorldLocation,
	const FVector& WorldDirection, AHANEDANPlayerController* L_PController)
{
	FVector End = WorldLocation + WorldDirection*DefuseVectorLengthMultiplyValue;

	FHitResult OutHitResult = OutHitResult1;
		
	GetWorld()->LineTraceSingleByChannel(OutHitResult,WorldLocation,End,ECC_GameTraceChannel5);
	DrawDebugLine(GetWorld(),WorldLocation,End,FColor::Red,true);
		
	if(OutHitResult.bBlockingHit)
	{
		if(OutHitResult.GetActor()->ActorHasTag(FName("C4")))
		{
			if(AC4* C4 = Cast<AC4>(OutHitResult.GetActor()))
			{
				UE_LOG(LogTemp,Error,TEXT("c4 e hit trace vurdu"));
				OwnerHCharacter->bWasDefusing = true;
				OwnerHCharacter->DefusingBomb = C4;
				OwnerHCharacter->DisableMoveAction = true;
				
				C4->Owner=L_PController;
				C4->HPlayerController = L_PController;
				C4->StartDefuse(L_PController);
			}
		}
	}
}

void UCombatComponent::DefuseTrace(const FHitResult& OutHitResult1, const FVector& WorldLocation,
	const FVector& WorldDirection, AHANEDANPlayerController* L_PController)
{
	FVector End = WorldLocation + WorldDirection*DefuseVectorLengthMultiplyValue;

	FHitResult OutHitResult = OutHitResult1;
		
	GetWorld()->LineTraceSingleByChannel(OutHitResult,WorldLocation,End,ECC_GameTraceChannel5);
		
	if(OutHitResult.bBlockingHit)
	{
		if(OutHitResult.GetActor()->ActorHasTag(FName("C4")))
		{
			if(AC4* C4 = Cast<AC4>(OutHitResult.GetActor()))
			{
				UE_LOG(LogTemp,Error,TEXT("c4 e trace hit atıldı"));
				OwnerHCharacter->bWasDefusing = true;
				OwnerHCharacter->DefusingBomb = C4;
				OwnerHCharacter->DisableMoveAction = true;
				
				C4->Owner=L_PController;
				C4->HPlayerController = L_PController;
				C4->StartDefuse(L_PController);
			}
		}
	}
}

void UCombatComponent::GetWorldLocationAndRotationForDefuseTrace(FVector& OutWorldLocation, FVector& OutWorldRotation)
{
	FVector2d ViewportSize;
	
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2d CrosshairLocationOnViewport = FVector2d(ViewportSize.X/2.f,ViewportSize.Y/2.f);
	FVector WorldLocation;
	FVector WorldDirection;
	bool bProjected =UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocationOnViewport,WorldLocation,WorldDirection);
	if(bProjected)
	{
		OutWorldLocation=WorldLocation;
		OutWorldRotation=OutWorldRotation;
	}
}

void UCombatComponent::Server_SetPlantedBombTransform_Implementation(const FTransform& Transform)
{
	UE_LOG(LogTemp,Error,TEXT(" server transfrom setted on server rpc"));
	PlantedBombTransfrom = Transform;
}






