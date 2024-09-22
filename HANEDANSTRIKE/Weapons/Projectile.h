// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class AHANEDANPlayerController;
class UCapsuleComponent;
class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class HANEDANSTRIKE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	virtual void BeginPlay() override;

public:

	/**
	 * COMPONENTS
	 */
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMeshComponent;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* AmmoBoxComponent;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* TraceComponent;

	/**
	 * REFERENCES
	 */

	UPROPERTY()
	AHANEDANPlayerController* HPlayerController;
	
	/**
	 * FREE SPACE
	 */
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float Damage = 20.f; // Bu projectileyi spawnlayacak olan spawnladığı andan sonra projectile ın damagesini setlemeli

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float HeadShotDamage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UParticleSystem* TraceParticle;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	USoundCue* ImpactSound;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& HitResult);

	/**
	 * DESTROY WORKS
	 */

	UPROPERTY(EditDefaultsOnly,Category="Properties")
	uint8 DestroyTime = 10; // Beginplayden sonra belli bir zaman içinde projectile destroy edilecek
	FTimerHandle DestroyTimerHandle;
	UPROPERTY(Replicated)
	bool bTimeDestroy = false;
	void BeginTimeDestroy();
	
	virtual void Destroyed() override;

};
