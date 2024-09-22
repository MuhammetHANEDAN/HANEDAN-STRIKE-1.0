// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HanedanAnimInstance.h"
#include "HANEDANSTRIKE/Components/CombatComponent.h"
#include "HANEDANSTRIKE/HUD/C4Progress.h"
#include "HANEDANSTRIKE/HUD/HANEDANHUD.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "HANEDANSTRIKE/PlayerStates/HANEDANPlayerState.h"
#include "HANEDANSTRIKE/Weapons/BaseWeapon.h"
#include "HANEDANSTRIKE/Weapons/C4.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AHANEDANCharacter::AHANEDANCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	/** For fps shooter games */
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	bUseControllerRotationYaw=false;
	GetCharacterMovement()->bOrientRotationToMovement=true;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch=true;
	GetCharacterMovement()->RotationRate=FRotator(0,800.f,0);

	CameraBoom=CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength=600.f;
	CameraBoom->bUsePawnControlRotation=true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false;

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	CombatComponent->SetIsReplicated(true);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	
}

void AHANEDANCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = FollowCamera->FieldOfView;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		HPlayerController = Cast<AHANEDANPlayerController>(PlayerController);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	else
	{
		GetWorldTimerManager().SetTimer(TrySetupInputTimerHandle,this,&AHANEDANCharacter::TrySetupInput,0.15f);
	}

	if(HasAuthority())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel2,ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2,ECR_Overlap);
	}
}

void AHANEDANCharacter::TrySetupInput()
{
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		HPlayerController = Cast<AHANEDANPlayerController>(PlayerController);
		CombatComponent->HPlayerState = Cast<AHANEDANPlayerState>(GetPlayerState());
		CombatComponent->HPlayerController = HPlayerController;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	/*else
	{
		UE_LOG(LogTemp,Error,TEXT("player controller valid değil AHANEDANCharacter::TrySetupInput() "));
	}*/
	
}

void AHANEDANCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(CombatComponent)
	{
		CombatComponent->OwnerHCharacter=this;
		CombatComponent->OwnerHCSkeletalMeshComponent = GetMesh();
		CombatComponent->HPlayerState = Cast<AHANEDANPlayerState>(GetPlayerState());
		CombatComponent->HanedanAnimInstance = Cast<UHanedanAnimInstance>(GetMesh()->GetAnimInstance());
		CombatComponent->HPlayerController = HPlayerController;
	}
}

void AHANEDANCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHANEDANCharacter,DisableMoveAction);
	DOREPLIFETIME(AHANEDANCharacter,CurrentPlayerState);
	DOREPLIFETIME(AHANEDANCharacter,HPlayerController);
	DOREPLIFETIME(AHANEDANCharacter,bIsAlive);
	DOREPLIFETIME_CONDITION(AHANEDANCharacter,bWasDefusing,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHANEDANCharacter,DefusingBomb,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHANEDANCharacter,bWasPlanting,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHANEDANCharacter,PlantingBomb,COND_OwnerOnly);
}

void AHANEDANCharacter::GetHit(FName BoneName, float Damage, float HeadShotDamage,
                               AActor* DamageCauserActor, AController* DamageCauserController)
{
	if(!bIsAlive) return; // hayatta değilse hasar almaz

	bool bSameCT = ActorHasTag(FName("CT")) && DamageCauserActor->ActorHasTag(FName("CT"));
	bool bSameT = ActorHasTag(FName("T")) && DamageCauserActor->ActorHasTag(FName("T"));

	if(bSameCT || bSameT) return;

	if(HasAuthority())
	{
		if(BoneName == FName("head"))
		{
			float DamagedAfterArmor = CombatComponent->Shield-HeadShotDamage;
			if(DamagedAfterArmor > 0)
			{
				// Shield Damageden daha büyüktü.
				CombatComponent->Shield = DamagedAfterArmor;
				HPlayerController->SetCharacterOverlayShieldText(DamagedAfterArmor);
			}
			else if (DamagedAfterArmor <= 0)
			{
				CombatComponent->Shield = 0;
				HPlayerController->SetCharacterOverlayShieldText(0);
				float DamageAmount = FMath::Abs(DamagedAfterArmor); // - değeri pozitif yaptık
				CombatComponent->Health = FMath::Clamp(CombatComponent->Health-DamageAmount,0,CombatComponent->MaxHealth);
				HPlayerController->SetCharacterOverlayHealthText(CombatComponent->Health);
			}
		}
		else
		{
			float DamagedAfterArmor = CombatComponent->Shield-Damage;
			if(DamagedAfterArmor > 0)
			{
				// Shield Damageden daha büyüktü.
				CombatComponent->Shield = DamagedAfterArmor;
				HPlayerController->SetCharacterOverlayShieldText(DamagedAfterArmor);
			}
			else if (DamagedAfterArmor <= 0)
			{
				CombatComponent->Shield = 0;
				HPlayerController->SetCharacterOverlayShieldText(0);
				float DamageAmount = FMath::Abs(DamagedAfterArmor); // - değeri pozitif yaptık
				CombatComponent->Health = FMath::Clamp(CombatComponent->Health-DamageAmount,0,CombatComponent->MaxHealth);
				HPlayerController->SetCharacterOverlayHealthText(CombatComponent->Health);
			}
		}
		if(CombatComponent->Health == 0)
		{
			CharacterElimned(Controller,DamageCauserController);
		}
	}
}

float AHANEDANCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(!bIsAlive) return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); // hayatta değilse hasar almaz
	if(HasAuthority())
	{
		float DamagedAfterArmor = CombatComponent->Shield-DamageAmount;
		if(DamagedAfterArmor > 0)
		{
			// Shield Damageden daha büyüktü.
			CombatComponent->Shield = DamagedAfterArmor;
			HPlayerController->SetCharacterOverlayShieldText(DamagedAfterArmor);
		}
		else if (DamagedAfterArmor <= 0)
		{
			CombatComponent->Shield = 0;
			HPlayerController->SetCharacterOverlayShieldText(0);
			float ldamageamount = FMath::Abs(DamagedAfterArmor); // - değeri pozitif yaptık
			CombatComponent->Health = FMath::Clamp(CombatComponent->Health-ldamageamount,0,CombatComponent->MaxHealth);
			HPlayerController->SetCharacterOverlayHealthText(CombatComponent->Health);
		}

		if(CombatComponent->Health == 0)
		{
			CharacterElimned(Controller,EventInstigator);
		}
		
		return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


void AHANEDANCharacter::CharacterElimned(AController* DeadController, AController* KillerController)
{
	bIsAlive = false; // onrep ile çoğaltıldı
	GetCharacterMovement()->bOrientRotationToMovement=false;
	CombatComponent->GetWeaponOnSlot(2)->WeaponSkeletalMeshComponent->SetVisibility(false);

	GetHanedanPlayerController()->HideCharacterOverlay(true); // overlayı saklıyoruz
	
	if(DeathMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage); // Serverda oynatılıyor on rep ile geri kalanlar oynayacak
	}
	// TODO karakterlerin collisionlarını hallet ve karakter slot1 de weapon varsa düşürsün
	AHANEDANSTRIKEGameModeBase* GameModeBase = Cast<AHANEDANSTRIKEGameModeBase>(GetWorld()->GetAuthGameMode());
	if(GameModeBase)
	{
		GameModeBase->PlayerElimned(DeadController,KillerController);
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("AHANEDANCharacter::CharacterElimned  game mode null dır"));
	}

	if(HPlayerController->IsLocalController())
	{
		if(HPlayerController->HanedanHUD)
		{
			HPlayerController->SetAWPScopeWidgetVisibility(false);
		}
	}

	CombatComponent->DropFirstWeapon();
	CombatComponent->DropThirdWeapon();

	if(bWasPlanting)
	{
		PlantingBomb->CancelPlantBomb(HPlayerController);
	}
	if(bWasDefusing)
	{
		DefusingBomb->CancelDefuse(HPlayerController);
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHANEDANCharacter::OnRep_IsAlive(bool LastValue)
{
	if(LastValue)
	{
		if(DeathMontage)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
		}
		if(CombatComponent->CurrentWeaponOnHand && CombatComponent->CurrentWeaponOnHand->ActorHasTag(FName("AWP")))
		{
			CombatComponent->CurrentWeaponOnHand->CloseScope();
		}
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		// Yaşıyor
	}
}

void AHANEDANCharacter::RefreshCharacter()
{
	bIsAlive = true;
	GetCharacterMovement()->bOrientRotationToMovement=true;
	CombatComponent->GetWeaponOnSlot(2)->WeaponSkeletalMeshComponent->SetVisibility(true);

	CombatComponent->bAiming = false;
	CurrentPlayerState = EPlayerState::EPS_Unoccupied;
	
	CombatComponent->Health = CombatComponent->MaxHealth; // canı refreshledik
	if(CombatComponent->GetWeaponOnSlot(1))
	{
		if(CombatComponent->CurrentWeaponOnHand == CombatComponent->GetWeaponOnSlot(1))
		{
			GetCharacterMovement()->bOrientRotationToMovement=false;
		}
		CombatComponent->GetWeaponOnSlot(1)->WeaponAmmo = CombatComponent->GetWeaponOnSlot(1)->MaxWeaponAmmo;
		CombatComponent->GetWeaponOnSlot(1)->Ammo = CombatComponent->GetWeaponOnSlot(1)->MaxAmmo;
	}
	if(CombatComponent->CurrentWeaponOnHand == CombatComponent->GetWeaponOnSlot(2))
	{
		GetCharacterMovement()->bOrientRotationToMovement=false;
	}
	CombatComponent->GetWeaponOnSlot(2)->WeaponAmmo = CombatComponent->GetWeaponOnSlot(2)->MaxWeaponAmmo;
	CombatComponent->GetWeaponOnSlot(2)->Ammo =  CombatComponent->GetWeaponOnSlot(2)->MaxAmmo; // tabanca elden atılamaz

	if(CombatComponent->CurrentWeaponOnHand)
	{
		CurrentWeaponType = CombatComponent->CurrentWeaponOnHand->WeaponType;
		CombatComponent->bWeaponOnHand=true;
	}
	else
	{
		CurrentWeaponType = EWeaponType::EWT_Unarmed;
		CombatComponent->bWeaponOnHand=false;
	}

	bWasDefusing = false;
	DefusingBomb = nullptr;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	Multicast_RefreshHandle();
	
}

void AHANEDANCharacter::Multicast_RefreshHandle_Implementation()
{
	//UE_LOG(LogTemp,Error,TEXT("multicast stop montage girildi"));
	if(GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0.1);
	}
	if(CombatComponent->CurrentWeaponOnHand)
	{
		CurrentWeaponType = CombatComponent->CurrentWeaponOnHand->WeaponType;
	}
	else
	{
		CurrentWeaponType = EWeaponType::EWT_Unarmed;
	}
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if(C4ProgressWidgetReference)
	{
		C4ProgressWidgetReference->RemoveFromParent();
		C4ProgressWidgetReference = nullptr;
	}
}

void AHANEDANCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AHANEDANPlayerController* AHANEDANCharacter::GetHanedanPlayerController() const
{
	return Cast<AHANEDANPlayerController>(Controller);
}

void AHANEDANCharacter::SetCharacterForbWeaponOnHand(bool OnHand)
{
	if(OnHand)
	{
		bUseControllerRotationYaw= true;
		GetCharacterMovement()->bOrientRotationToMovement= false;
	}
	else
	{
		bUseControllerRotationYaw=false;
		GetCharacterMovement()->bOrientRotationToMovement=true;
	}
	
}

void AHANEDANCharacter::Server_SetCharacterForbWeaponOnHand_Implementation(bool OnHand)
{
	if(OnHand)
	{
		bUseControllerRotationYaw= true;
		GetCharacterMovement()->bOrientRotationToMovement= false;
	}
	else
	{
		bUseControllerRotationYaw=false;
		GetCharacterMovement()->bOrientRotationToMovement=true;
	}
}

void AHANEDANCharacter::Multicast_SetCharacterForbWeaponOnHand_Implementation(bool OnHand)
{
	if(OnHand)
	{
		bUseControllerRotationYaw= true;
		GetCharacterMovement()->bOrientRotationToMovement= false;
	}
	else
	{
		bUseControllerRotationYaw=false;
		GetCharacterMovement()->bOrientRotationToMovement=true;
	}
}

void AHANEDANCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHANEDANCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHANEDANCharacter::Look);

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHANEDANCharacter::Jump);

		EnhancedInputComponent->BindAction(OpenBuyMenuAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_OpenBuMenu);

		// Swap Weapons
		EnhancedInputComponent->BindAction(SwitchToWeaponSlot1Action,ETriggerEvent::Started,this,&AHANEDANCharacter::I_SwitchToWeaponSlot1);
		EnhancedInputComponent->BindAction(SwitchToWeaponSlot2Action,ETriggerEvent::Started,this,&AHANEDANCharacter::I_SwitchToWeaponSlot2);
		EnhancedInputComponent->BindAction(SwitchToWeaponSlot3Action,ETriggerEvent::Started,this,&AHANEDANCharacter::I_SwitchToWeaponSlot3);
		EnhancedInputComponent->BindAction(SwitchToWeaponSlot4Action,ETriggerEvent::Started,this,&AHANEDANCharacter::I_SwitchToWeaponSlot4);

		//Aiming
		EnhancedInputComponent->BindAction(AimingAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_AimingButtonPressed);
		EnhancedInputComponent->BindAction(AimingAction,ETriggerEvent::Completed,this,&AHANEDANCharacter::I_AimingButtonReleased);

		//Firing
		EnhancedInputComponent->BindAction(FireButtonAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_FirstFired);
		EnhancedInputComponent->BindAction(FireButtonAction,ETriggerEvent::Triggered,this,&AHANEDANCharacter::I_FireButtonPressed);
		EnhancedInputComponent->BindAction(FireButtonAction,ETriggerEvent::Completed,this,&AHANEDANCharacter::I_FireButtonReleased);

		//Reload
		EnhancedInputComponent->BindAction(ReloadAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_Reload);

		//Drop Weapon
		EnhancedInputComponent->BindAction(DropWeaponAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_DropWeapon);

		//F Button
		EnhancedInputComponent->BindAction(FButtonAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_FButtonStarted);
		EnhancedInputComponent->BindAction(FButtonAction,ETriggerEvent::Completed,this,&AHANEDANCharacter::I_FButtonReleased);

		// Q and E button for camera positioning
		EnhancedInputComponent->BindAction(QButtonAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_QButtonStarted);
		EnhancedInputComponent->BindAction(EButtonAction,ETriggerEvent::Started,this,&AHANEDANCharacter::I_EButtonStarted);
	}
}

void AHANEDANCharacter::I_DropWeapon()
{
	if(!bIsAlive) return;
	if(CombatComponent && CurrentPlayerState == EPlayerState::EPS_Unoccupied)
	{
		CombatComponent->DropCurrentWeaponOnHand();
	}
}

void AHANEDANCharacter::I_FButtonStarted() // sadece defuse için kullanıcaz
{
	if(!bIsAlive) return;
	if(!HPlayerController->bCtOrT) return; // t defuse yapamaz
	FHitResult HitResult;
	CombatComponent->TraceUnderCrosshair(HitResult);
	FVector WLocation;
	FVector WRotation;
	CombatComponent->GetWorldLocationAndRotationForDefuseTrace(WLocation,WRotation);
	CombatComponent->DefuseTrace(HitResult,WLocation,WRotation,Cast<AHANEDANPlayerController>(Controller));
}

void AHANEDANCharacter::I_FButtonReleased()
{
	if(!bIsAlive) return;
	if(bWasDefusing)
	{
		DefusingBomb->CancelDefuse(HPlayerController);
		bWasDefusing = false;
		DefusingBomb = nullptr;
	}
	DisableMoveAction = false;
}

void AHANEDANCharacter::I_QButtonStarted()
{
	CameraBoom->SocketOffset=FVector(0,-110,65);
}

void AHANEDANCharacter::I_EButtonStarted()
{
	CameraBoom->SocketOffset=FVector(0,100,65);
}

void AHANEDANCharacter::Move(const FInputActionValue& Value)
{
	FVector2d MoveVector = Value.Get<FVector2d>();
	float FrontScale = MoveVector.Y;
	float RightScale = MoveVector.X;

	if(GetController()!=nullptr && !DisableMoveAction && bIsAlive)
	{
		FRotator ControllerRotation = GetControlRotation();
		FRotator YawRotation = FRotator(0,ControllerRotation.Yaw,0);

		FVector FrontDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(FrontDirection,FrontScale);
		AddMovementInput(RightDirection,RightScale);
	}
}

void AHANEDANCharacter::Look(const FInputActionValue& Value)
{
	FVector2d MoveVector = Value.Get<FVector2d>();
	float PitchScale = -MoveVector.Y;
	float YawScale = MoveVector.X;

	if(GetController()!=nullptr && bIsAlive)
	{
		AddControllerPitchInput(PitchScale);
		AddControllerYawInput(YawScale);
	}
}

void AHANEDANCharacter::Jump()
{
	if(bIsAlive)
	{
		Super::Jump();
	}
}

void AHANEDANCharacter::I_OpenBuMenu()
{
	if(bIsAlive == false) return;
	HPlayerController = HPlayerController == nullptr ?  Cast<AHANEDANPlayerController>(Controller) : HPlayerController;
	if(HPlayerController && HPlayerController->MatchState == MatchState::PurchasingState && HPlayerController->HanedanHUD)
	{
		HPlayerController->HanedanHUD->HandleBuyMenuWidget();
	}
}

void AHANEDANCharacter::I_Reload()
{
	if(bIsAlive == false) return;
	CombatComponent->ReloadButtonPressed();
}

void AHANEDANCharacter::I_SwitchToWeaponSlot1()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_SwapWeapon(1);
}

void AHANEDANCharacter::I_SwitchToWeaponSlot2()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_SwapWeapon(2);
}

void AHANEDANCharacter::I_SwitchToWeaponSlot3()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_SwapWeapon(3);
}

void AHANEDANCharacter::I_SwitchToWeaponSlot4()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_SwapWeapon(4);
}

void AHANEDANCharacter::I_AimingButtonPressed()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_ServerSetAiming(true);
}

void AHANEDANCharacter::I_AimingButtonReleased()
{
	if(bIsAlive == false) return;
	CombatComponent->Server_ServerSetAiming(false);
}

void AHANEDANCharacter::I_FireButtonPressed()
{
	if(bIsAlive == false) return;
	if(CombatComponent->CurrentWeaponOnHand == nullptr) return;
	
	HPlayerController = HPlayerController == nullptr ?  Cast<AHANEDANPlayerController>(Controller) : HPlayerController;
	if(HPlayerController && HPlayerController->MatchState == MatchState::PurchasingState)
	{
		return;
	}
	
	CombatComponent->FireButtonPressed(true);
	if(CombatComponent->CurrentWeaponOnHand->WeaponType == EWeaponType::EWT_Rifle && CombatComponent->bAiming)
	{
		PlayMontage(RifleFireIronsightMontage);
	}
}

void AHANEDANCharacter::I_FireButtonReleased()
{
	if(bIsAlive == false) return;
	if(CombatComponent->CurrentWeaponOnHand == nullptr) return;
	CombatComponent->FireButtonPressed(false);
	UHanedanAnimInstance* AnimInstance = Cast<UHanedanAnimInstance>(GetMesh()->GetAnimInstance());
	if(AnimInstance)
	{
		if(AnimInstance->IsAnyMontagePlaying())
		{
			AnimInstance->StopAllMontages(1);
		}
	}
}

void AHANEDANCharacter::I_FirstFired()
{
	if(bIsAlive == false) return;
	if(CombatComponent->CurrentWeaponOnHand == nullptr) return;

	HPlayerController = HPlayerController == nullptr ?  Cast<AHANEDANPlayerController>(Controller) : HPlayerController;
	if(HPlayerController && HPlayerController->MatchState == MatchState::PurchasingState)
	{
		return;
	}
	
	CombatComponent->FirstFiredButtonPressed();
}

void AHANEDANCharacter::PlayMontage(UAnimMontage* Montage)
{
	UHanedanAnimInstance* AnimInstance = Cast<UHanedanAnimInstance>(GetMesh()->GetAnimInstance());
	if(AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

AHANEDANPlayerState* AHANEDANCharacter::GetHanedanPlayerState() const
{
	AHANEDANPlayerState* Ps = Cast<AHANEDANPlayerState>(GetPlayerState());
	return Ps;
}

void AHANEDANCharacter::SetCameraFOV(float NewFov)
{
	FollowCamera->SetFieldOfView(NewFov);
}

void AHANEDANCharacter::SetCameraFOVtoDefault()
{
	FollowCamera->SetFieldOfView(DefaultFOV);
}




