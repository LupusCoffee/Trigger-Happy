#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// Centralized native declarations for all Attribute.* gameplay tags.
// Using native tags ensures cooked builds work without relying on runtime registration.
// Access via the variables or use FindTagByString to resolve from string names.
// Access via the variables or use FindTagByString to resolve from string names.

namespace AttributeTags
{
	// Health/Defense
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MaxDefense);

	// Dash
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_CooldownPerCharge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_KnockbackForce);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_KnockbackForceDead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_Strength);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_MaxCharges);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Dash_CollisionDamage);

	// Killstreak
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Killstreak_ExplosiveRounds);

	// Movement
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Movement_SprintSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Movement_CrouchSpeed);

	// Melee
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_KnockbackForce);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_KnockbackForceDead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_HitDetection);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_HitDetectionRadius);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Melee_SwingAmount);

	// Slide
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Slide_Strength);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Slide_Duration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Slide_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Slide_CollisionDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Slide_ScopeTimeDilation);

	// SlowMo
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_SlowMo_TimeDilation);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_SlowMo_MaxDuration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_SlowMo_Cooldown);

	// Throw
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Throw_Force);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Throw_Damage);

	// Weapon
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Weapon_FireDelay);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Weapon_ReloadSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Weapon_Damage);


	// Helper: map string name (e.g., "Attribute.MaxHealth") to the native FGameplayTag, or return Invalid if unknown.
	GP4PROTOTYPE_API FGameplayTag FindTagByString(const FString& TagName);
}
