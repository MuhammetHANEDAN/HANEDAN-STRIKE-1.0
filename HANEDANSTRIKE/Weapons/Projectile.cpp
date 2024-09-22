// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HANEDANSTRIKE/Character/HANEDANCharacter.h"
#include "HANEDANSTRIKE/PlayerControllers/HANEDANPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AProjectile::AProjectile()
{
 	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	AmmoBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	SetRootComponent(AmmoBoxComponent);
	AmmoBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	AmmoBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh Component"));
	ProjectileMeshComponent->SetupAttachment(GetRootComponent());
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMeshComponent->SetRelativeRotation(FRotator(270,0,0));

	TraceComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trace Comp"));
	TraceComponent->SetupAttachment(GetRootComponent());

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Comp"));

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(TraceParticle && ProjectileMeshComponent)
	{
		TraceComponent = UGameplayStatics::SpawnEmitterAttached(TraceParticle,TraceComponent,NAME_None,FVector(ForceInit),
			FRotator::ZeroRotator,FVector(1),EAttachLocation::Type::KeepRelativeOffset,false);
	}
	if(Owner->HasAuthority())
	{
		AmmoBoxComponent->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);
		AmmoBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		GetWorldTimerManager().SetTimer(DestroyTimerHandle,this,&AProjectile::BeginTimeDestroy,DestroyTime);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile,bTimeDestroy);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	if(OtherActor)
	{
		if(IHitInterface* Hit = Cast<IHitInterface>(OtherActor))
		{
			Hit->GetHit(HitResult.BoneName,Damage,HeadShotDamage,GetOwner(),UGameplayStatics::GetPlayerController(this,0));
		}
		Destroy();
	}
}

void AProjectile::BeginTimeDestroy()
{
	bTimeDestroy = true;
	Destroy();
}

void AProjectile::Destroyed()
{
	if(bTimeDestroy)
	{
		Super::Destroyed();
	}
	else
	{
		if(ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this,ImpactParticles,GetActorLocation(),
			GetActorRotation());
		}
		if(ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,GetActorLocation(),GetActorRotation());	
		}
	
		Super::Destroyed();
	}
}

