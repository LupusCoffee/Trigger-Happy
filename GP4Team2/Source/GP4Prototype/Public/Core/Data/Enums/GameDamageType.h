#pragma once

UENUM(BlueprintType)
enum class EGameDamageType : uint8
{
	Gun     UMETA(DisplayName = "Gun"),
	Melee  UMETA(DisplayName = "Melee"),
	DashHit UMETA(DisplayName = "DashHit"),
	SlideHit UMETA(DisplayName = "SlideHit"),
	Explosion  UMETA(DisplayName = "Explosion")
};
