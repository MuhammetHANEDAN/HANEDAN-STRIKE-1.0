// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HANEDANHUD.generated.h"

class UAnnoucementWidget;
class UBuyMenuWidget;
class UTimeWidget;
class UChooseTeamWidget;
class UCharacterOverlay;
/**
 * 
 */
UCLASS()
class HANEDANSTRIKE_API AHANEDANHUD : public AHUD
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	virtual void DrawHUD() override;

public:

	UPROPERTY()
	UCharacterOverlay* CharacterOverlayWidget;
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	TSubclassOf<UUserWidget> CharacterOverlayWidgetClass;
	void AddCharacterOverlay();
	void HideCharacterOverlay(bool bHide);
	
	/** Choose team widgeti 2 butondan birine basınca kendisi remove olacak */
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	TSubclassOf<UUserWidget> ChooseTeamWidgetClass;
	void AddChooseTeamWidget();

	UPROPERTY()
	UTimeWidget* TimeWidget;
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	TSubclassOf<UUserWidget> TimeWidgetClass;
	void AddTimeWidget();

	UPROPERTY()
	UBuyMenuWidget* BuyMenuWidget; // Removefromparent kullanmayacağız
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	TSubclassOf<UUserWidget> BuyMenuWidgetClass;
	void HandleBuyMenuWidget(); // Dışarıdan bunu çağır menu widgeti doğru şekilde kullanmak için
	void AddBuyMenuWidget();
	void HideBuyMenuWidget(); // purchasing state bitimine kadar oyuncu tarafından kapatılmazsa dışarıdan kapatmak için

	UPROPERTY()
	UAnnoucementWidget* AnnouncementWidget;
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	TSubclassOf<UUserWidget> AnnouncementWidgetClass;
	void AddAnnouncementWidget();

	/**
	 * AWP SCOPE
	 */
	
	UPROPERTY()
	UUserWidget* AWPScopeWidget;
	UPROPERTY(EditDefaultsOnly,Category="Properties")
	TSubclassOf<UUserWidget> AWPScopeWidgetClass;
	void AddAWPScopeWidget();
	void SetAWPScopeVisibility(bool Visible);

	/**
	 *  CROSSHAIR
	 */

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	UTexture2D* CrosshairsBottom;
	
	float CrosshairSpread = 0;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	float CrosshairSpreadMax = 8;

	UPROPERTY(EditDefaultsOnly, Category="Properties")
	FLinearColor CrosshairsColor = FLinearColor::White;
	
	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor Color);













	
};
