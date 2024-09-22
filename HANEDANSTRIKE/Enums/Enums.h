#pragma once

/**
 * CHARACTER
 */

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	EPS_Unoccupied UMETA(DisplayName= "Unoccupied"),
	EPS_Reloading UMETA(DisplayName= "Reloading"),
	EPS_Equiping UMETA(DisplayName= "Equiping"),

	ECS_MAX UMETA(DisplayName = "Default MAX")
};

/**
 * WEAPON
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Unarmed UMETA(DisplayName= "Unarmed"),
	EWT_Rifle UMETA(DisplayName= "Rifle"),
	EWT_Pistol UMETA(DisplayName= "Pistol"),
	EWT_Knife UMETA(DisplayName= "Knife"),
	EWT_Grenade UMETA(DisplayName= "Grenade"),
	
	ECS_MAX UMETA(DisplayName = "Default MAX")
};

