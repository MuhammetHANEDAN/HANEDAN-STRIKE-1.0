// Fill out your copyright notice in the Description page of Project Settings.


#include "HANEDANHUD.h"

#include "AnnoucementWidget.h"
#include "BuyMenuWidget.h"
#include "CharacterOverlay.h"
#include "ChooseTeamWidget.h"
#include "TimeWidget.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

void AHANEDANHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHANEDANHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2d ViewPortSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
		const FVector2d ViewportCenter(ViewPortSize.X/2.f, ViewPortSize.Y/2.f);

		float SpreadScaled = CrosshairSpreadMax * CrosshairSpread;

		if(CrosshairsCenter)
		{
			FVector2d Spread(0,0);
			DrawCrosshair(CrosshairsCenter,ViewportCenter, Spread,CrosshairsColor);
		}
		if(CrosshairsLeft)
		{
			FVector2d Spread(-SpreadScaled,0);
			DrawCrosshair(CrosshairsLeft,ViewportCenter, Spread, CrosshairsColor);
		}
		if(CrosshairsRight)
		{
			FVector2d Spread(SpreadScaled,0);
			DrawCrosshair(CrosshairsRight,ViewportCenter, Spread, CrosshairsColor);
		}
		if(CrosshairsBottom)
		{
			FVector2d Spread(0,SpreadScaled);
			DrawCrosshair(CrosshairsBottom,ViewportCenter, Spread, CrosshairsColor);
		}
		if(CrosshairsTop)
		{
			// UV kordinat olduğu için Y de yukarısı - oluyor
			FVector2d Spread(0,-SpreadScaled);
			DrawCrosshair(CrosshairsTop,ViewportCenter, Spread, CrosshairsColor);
		}
	}
}

void AHANEDANHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor Color)
{
	const float TextureHeight = Texture->GetSizeX();
	const float TextureWidth = Texture->GetSizeY();

	const FVector2d TextureDrawPoint(ViewportCenter.X-(TextureWidth/2.f)+Spread.X,ViewportCenter.Y-(TextureHeight/2.f)+Spread.Y);

	DrawTexture(Texture,TextureDrawPoint.X,TextureDrawPoint.Y,TextureWidth,TextureHeight,0,0,1,1,
		Color);
}

void AHANEDANHUD::AddCharacterOverlay()
{
	if(CharacterOverlayWidgetClass == nullptr) return;
	CharacterOverlayWidget = CreateWidget<UCharacterOverlay>(PlayerOwner.Get(),CharacterOverlayWidgetClass);
	if(CharacterOverlayWidget)
	{
		CharacterOverlayWidget->AddToViewport();
	}
}

void AHANEDANHUD::HideCharacterOverlay(bool bHide)
{
	if(CharacterOverlayWidget)
	{
		if(bHide)
		{
			if(!CharacterOverlayWidget->IsVisible())
			{
				CharacterOverlayWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		else
		{
			if(CharacterOverlayWidget->IsVisible())
			{
				CharacterOverlayWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void AHANEDANHUD::AddChooseTeamWidget()
{
	if(ChooseTeamWidgetClass == nullptr) return;
	UChooseTeamWidget* CWidget = CreateWidget<UChooseTeamWidget>(GetOwningPlayerController(),ChooseTeamWidgetClass);
	if(CWidget)
	{
		CWidget->AddToViewport();
		CWidget->Setup();
	}
}

void AHANEDANHUD::AddTimeWidget()
{
	if(TimeWidgetClass == nullptr) return;
	TimeWidget = CreateWidget<UTimeWidget>(PlayerOwner.Get(),TimeWidgetClass);
	if(TimeWidget)
	{
		TimeWidget->AddToViewport();
	}
}

void AHANEDANHUD::HandleBuyMenuWidget()
{
	if(BuyMenuWidget == nullptr) AddBuyMenuWidget();
	else
	{
		if(BuyMenuWidget->IsVisible())
		{
			BuyMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			FInputModeGameOnly InputModeGameOnly;
			GetOwningPlayerController()->SetInputMode(InputModeGameOnly);
			GetOwningPlayerController()->SetShowMouseCursor(false);
		}
		else
		{
			BuyMenuWidget->SetVisibility(ESlateVisibility::Visible);
			FInputModeGameAndUI InputModeGameAndUI;
			InputModeGameAndUI.SetWidgetToFocus(BuyMenuWidget->TakeWidget());
			GetOwningPlayerController()->SetInputMode(InputModeGameAndUI);
			GetOwningPlayerController()->SetShowMouseCursor(true);
		}
		
	}
}

void AHANEDANHUD::AddBuyMenuWidget()
{
	if(BuyMenuWidgetClass == nullptr) return;
	BuyMenuWidget = CreateWidget<UBuyMenuWidget>(GetOwningPlayerController(),BuyMenuWidgetClass);
	if(BuyMenuWidget == nullptr) return;
	BuyMenuWidget->AddToViewport();
	BuyMenuWidget->SetVisibility(ESlateVisibility::Visible);
	FInputModeGameAndUI InputModeGameAndUI;
	InputModeGameAndUI.SetWidgetToFocus(BuyMenuWidget->TakeWidget());
	GetOwningPlayerController()->SetInputMode(InputModeGameAndUI);
	GetOwningPlayerController()->SetShowMouseCursor(true);
}

void AHANEDANHUD::HideBuyMenuWidget()
{
	if(BuyMenuWidget)
	{
		BuyMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		FInputModeGameOnly InputModeGameOnly;
		GetOwningPlayerController()->SetInputMode(InputModeGameOnly);
		GetOwningPlayerController()->SetShowMouseCursor(false);
	}
}

void AHANEDANHUD::AddAnnouncementWidget()
{
	if(AnnouncementWidgetClass == nullptr) return;
	AnnouncementWidget = CreateWidget<UAnnoucementWidget>(GetOwningPlayerController(),AnnouncementWidgetClass);
	if(AnnouncementWidget == nullptr) return;
	AnnouncementWidget->AddToViewport();
	AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AHANEDANHUD::AddAWPScopeWidget()
{
	if(AWPScopeWidgetClass == nullptr) return;
	AWPScopeWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(),AWPScopeWidgetClass);
	if(AWPScopeWidget == nullptr) return;
	AWPScopeWidget->AddToViewport();
	AWPScopeWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AHANEDANHUD::SetAWPScopeVisibility(bool Visible)
{
	if(AWPScopeWidget)
	{
		if(Visible)
		{
			AWPScopeWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AWPScopeWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}



