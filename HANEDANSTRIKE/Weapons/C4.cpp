// Fill out your copyright notice in the Description page of Project Settings.


#include "C4.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "HANEDANSTRIKE/HUD/C4Progress.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AC4::AC4()
{

	bReplicates = true;
	
	C4BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("C4Box Component"));
	C4BoxComponent->SetupAttachment(GetRootComponent());
	C4BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C4BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	C4BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);

	C4StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("C4 Static Mesh Component"));
	C4StaticMeshComponent->SetupAttachment(WeaponSkeletalMeshComponent);
	C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Ignore);
	C4StaticMeshComponent->SetIsReplicated(true);
	C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);
	

	WeaponOverlapCapsuleComponent->SetupAttachment(C4StaticMeshComponent);
	WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);
	
}

void AC4::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AC4,bBombPlanted);
}

void AC4::BeginPlay()
{
	Super::BeginPlay();

	WeaponSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel5,ECR_Overlap);
	C4BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	C4BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel5,ECR_Block);
	C4BoxComponent->OnComponentHit.AddDynamic(this,&AC4::C4BoxComponentOnHit);
	
}

void AC4::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(bBombPlanted)
	{
		if(bC4DefuseTracing)
		{
			C4DefuseTracingTimeTrack += DeltaSeconds;
			if(C4ProgressWidget)
			{
				C4ProgressWidget->C4ProgressBar->SetPercent(C4DefuseTracingTimeTrack/C4DefuseTime);
			}
			if(C4DefuseTracingTimeTrack >= C4DefuseTime)
			{
				C4Defused();
				C4DefuseTracingTimeTrack = 0;
			}
		}
	}
	else
	{
		if(bPlanting)
		{
			PlantingTimeTrack += DeltaSeconds;
			if(C4ProgressWidget)
			{
				C4ProgressWidget->C4ProgressBar->SetPercent(PlantingTimeTrack/BombPlantTime);
			}
			if(PlantingTimeTrack >= BombPlantTime)
			{
				UE_LOG(LogTemp,Error,TEXT("tickin içinden firelandı localbombplanted"));
				LocalBombPlanted();
				bBombPlanted = true;
				PlantingTimeTrack = 0;
			}
		}
	}
}

void AC4::C4BoxComponentOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor->ActorHasTag(FName("CT")))
	{
		if(bC4DefuseTracing == false)
		{
			bC4DefuseTracing = true;
		}
	}
}

void AC4::PlayerOverlappingWeapon(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor->ActorHasTag(FName("T")))
	{
		if(AHANEDANCharacter* OverlappedCharacter = Cast<AHANEDANCharacter>(OtherActor))
		{
			if(OverlappedCharacter->bIsAlive == false) return;
			OverlappedCharacter->CombatComponent->TakeWeaponFromGround(this);
		}
	}
	
}

void AC4::StartDefuse(APlayerController* L_PlayerController)
{
	if(bBombPlanted == false) return;
	bC4DefuseTracing = true;
	if(L_PlayerController->IsLocalController())
	{
		if(C4ProgressWidget)
		{
			C4ProgressWidget->AddToViewport();
		}
		else
		{
			if(C4ProgressWidgetClass == nullptr) return;
			C4ProgressWidget = CreateWidget<UC4Progress>(L_PlayerController,C4ProgressWidgetClass);
			C4ProgressWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("L_PlayerController->IsLocalController değil"));
	}
	if(AHANEDANCharacter* HCharacter = Cast<AHANEDANCharacter>(L_PlayerController->GetPawn()))
	{
		HCharacter->C4ProgressWidgetReference = C4ProgressWidget;
	}
}

void AC4::CancelDefuse(AHANEDANPlayerController* L_hpcontroller)
{
	bC4DefuseTracing = false;
	C4DefuseTracingTimeTrack = 0;
	if(L_hpcontroller->IsLocalController())
	{
		if(C4ProgressWidget)
		{
			C4ProgressWidget->RemoveFromParent();
			C4ProgressWidget= nullptr;
		}
	}
}

void AC4::C4Defused()
{
	UE_LOG(LogTemp,Error,TEXT("c4 defused çalıştı"));
	if(C4ProgressWidget)
	{
		C4ProgressWidget->RemoveFromParent();
		C4ProgressWidget= nullptr;
	}
	bBombPlanted=false;
	HPlayerController->Server_BombDefused(C4DefuseTime);
}

void AC4::StartPlantBomb(AHANEDANPlayerController* L_hpcontroller)
{
	if(bBombPlanted) return;
	bPlanting = true;
	if(C4ProgressWidgetClass == nullptr) return;
	C4ProgressWidget = CreateWidget<UC4Progress>(L_hpcontroller,C4ProgressWidgetClass);
	C4ProgressWidget->AddToViewport();
}

void AC4::CancelPlantBomb(AHANEDANPlayerController* L_hpcontroller)
{
	bPlanting = false;
	PlantingTimeTrack = 0;
	if(C4ProgressWidget)
	{
		C4ProgressWidget->RemoveFromParent();
		C4ProgressWidget= nullptr;
	}
}

void AC4::LocalBombPlanted()
{
	bBombPlanted = true;
	
	if(C4ProgressWidget)
	{
		C4ProgressWidget->RemoveFromParent();
		C4ProgressWidget= nullptr;
	}
	
	if(HasAuthority())
	{
		OwnerHCharacter->CombatComponent->bWeaponOnHand = false;
		OwnerHCharacter->CombatComponent->CurrentWeaponOnHand = nullptr;
		OwnerHCharacter->CombatComponent->WeaponSlot3 = nullptr;
		OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
		OwnerHCharacter->SetCharacterForbWeaponOnHand(false);
	
		/*FDetachmentTransformRules DetachmentTransformRules = FDetachmentTransformRules::KeepRelativeTransform;
		DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DetachmentTransformRules);

		FDetachmentTransformRules TransformRules1 = FDetachmentTransformRules::KeepWorldTransform;
		TransformRules1.ScaleRule=EDetachmentRule::KeepWorld;
		
		C4StaticMeshComponent->DetachFromComponent(TransformRules1);
		C4StaticMeshComponent->SetWorldTransform(OwnerHCharacter->CombatComponent->PlantedBombTransfrom);
		
		SetActorTransform(OwnerHCharacter->CombatComponent->PlantedBombTransfrom);
		C4StaticMeshComponent->SetWorldScale3D(FVector(0.2,0.2,0.4));*/

		MultiCast_DetachBomb();

		EnableAllComponentsOverlapToAmmoTrace();
		
		UE_LOG(LogTemp,Error,TEXT("otoritenin içine girildi"));
		MessageGameMode_BombPlanted();
	}
	else
	{
		Server_BombPlanted(BombPlantTime);
	}
}

void AC4::Server_BombPlanted_Implementation(uint8 L_BombPlantTime)
{

	WeaponOverlapCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	OwnerHCharacter->CombatComponent->bWeaponOnHand = false;
	OwnerHCharacter->CombatComponent->CurrentWeaponOnHand = nullptr;
	OwnerHCharacter->CombatComponent->WeaponSlot3 = nullptr;
	OwnerHCharacter->CurrentWeaponType = EWeaponType::EWT_Unarmed;
	OwnerHCharacter->Multicast_SetCharacterForbWeaponOnHand(false);

	EnableAllComponentsOverlapToAmmoTrace();
	
	MultiCast_DetachBomb();

	bBombPlanted = true;

	if(C4ProgressWidget)
	{
		C4ProgressWidget->RemoveFromParent();
		C4ProgressWidget= nullptr;
	}
	MessageGameMode_BombPlanted();
}

void AC4::MultiCast_DetachBomb_Implementation()
{
	FDetachmentTransformRules DetachmentTransformRules = FDetachmentTransformRules::KeepRelativeTransform;
	DetachmentTransformRules.ScaleRule = EDetachmentRule::KeepRelative;
	DetachAllSceneComponents(OwnerHCharacter->GetMesh(),DetachmentTransformRules);
	
	FDetachmentTransformRules TransformRules1 = FDetachmentTransformRules::KeepWorldTransform;
	TransformRules1.ScaleRule=EDetachmentRule::KeepWorld;
		
	C4StaticMeshComponent->DetachFromComponent(TransformRules1);
	C4StaticMeshComponent->SetWorldTransform(OwnerHCharacter->CombatComponent->PlantedBombTransfrom);
	C4StaticMeshComponent->SetWorldScale3D(FVector(0.2,0.2,0.4));
	SetActorTransform(OwnerHCharacter->CombatComponent->PlantedBombTransfrom);
	
}

bool AC4::Server_BombPlanted_Validate(uint8 L_BombPlantTime)
{
	if(L_BombPlantTime < BombPlantTime)
	{
		return false;
	}
	return true;
}

void AC4::MessageGameMode_BombPlanted()
{
	GetWorld()->GetAuthGameMode<AHANEDANSTRIKEGameModeBase>()->BombHasPlanted();
	UE_LOG(LogTemp,Error,TEXT("gane modeye bomb planted yollandı"));
}

void AC4::EnableAllComponentsOverlapToAmmoTrace()
{
	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);
	WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);
	C4BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);
	C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel4,ECR_Overlap);

	WeaponSkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Overlap);
	WeaponOverlapCapsuleComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Overlap);
	C4BoxComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Overlap);
	C4StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility,ECR_Overlap);
	
}

void AC4::Multicast_ExplodeBomb_Implementation()
{
	if(ExplodeParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this,ExplodeParticle,GetActorLocation());
	}
	if(ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ExplodeSound,GetActorLocation());
	}
	if(HPlayerController && HPlayerController->IsLocalController())
	{
		CancelDefuse(HPlayerController);
	}
	bBombPlanted = false;
}
