#include "Systems/AttributeSystem/AttributeTags.h"
#include "GameplayTagsManager.h"

// Define native gameplay tags. These are loaded at startup in cooked builds.
namespace AttributeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Attribute_MaxHealth, "Attribute.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_MaxDefense, "Attribute.MaxDefense");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_Cooldown, "Attribute.Dash.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_CooldownPerCharge, "Attribute.Dash.CooldownPerCharge");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_KnockbackForce, "Attribute.Dash.KnockbackForce");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_KnockbackForceDead, "Attribute.Dash.KnockbackForceDead");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_Strength, "Attribute.Dash.Strength");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_MaxCharges, "Attribute.Dash.MaxCharges");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Dash_CollisionDamage, "Attribute.Dash.CollisionDamage");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Killstreak_ExplosiveRounds, "Attribute.Killstreak.ExplosiveRounds");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Movement_WalkSpeed, "Attribute.Movement.WalkSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Movement_SprintSpeed, "Attribute.Movement.SprintSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Movement_CrouchSpeed, "Attribute.Movement.CrouchSpeed");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_Damage, "Attribute.Melee.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_KnockbackForce, "Attribute.Melee.KnockbackForce");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_KnockbackForceDead, "Attribute.Melee.KnockbackForceDead");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_Cooldown, "Attribute.Melee.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_HitDetection, "Attribute.Melee.HitDetection");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_HitDetectionRadius, "Attribute.Melee.HitDetectionRadius");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Melee_SwingAmount, "Attribute.Melee.SwingAmount");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Slide_Strength, "Attribute.Slide.Strength");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Slide_Duration, "Attribute.Slide.Duration");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Slide_Cooldown, "Attribute.Slide.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Slide_CollisionDamage, "Attribute.Slide.CollisionDamage");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Slide_ScopeTimeDilation, "Attribute.Slide.ScopeTimeDilation");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_SlowMo_TimeDilation, "Attribute.SlowMo.TimeDilation");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_SlowMo_MaxDuration, "Attribute.SlowMo.MaxDuration");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_SlowMo_Cooldown, "Attribute.SlowMo.Cooldown");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Throw_Force, "Attribute.Throw.Force");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Throw_Damage, "Attribute.Throw.Damage");

	UE_DEFINE_GAMEPLAY_TAG(Attribute_Weapon_FireDelay, "Attribute.Weapon.FireDelay");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Weapon_ReloadSpeed, "Attribute.Weapon.ReloadSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Weapon_Damage, "Attribute.Weapon.Damage");

	static const TMap<FString, FGameplayTag> NameToTag = {
		{ TEXT("Attribute.MaxHealth"), Attribute_MaxHealth },
		{ TEXT("Attribute.MaxDefense"), Attribute_MaxDefense },
		{ TEXT("Attribute.Dash.Cooldown"), Attribute_Dash_Cooldown },
		{ TEXT("Attribute.Dash.CooldownPerCharge"), Attribute_Dash_CooldownPerCharge },
		{ TEXT("Attribute.Dash.KnockbackForce"), Attribute_Dash_KnockbackForce },
		{ TEXT("Attribute.Dash.KnockbackForceDead"), Attribute_Dash_KnockbackForceDead },
		{ TEXT("Attribute.Dash.Strength"), Attribute_Dash_Strength },
		{ TEXT("Attribute.Dash.MaxCharges"), Attribute_Dash_MaxCharges },
		{ TEXT("Attribute.Dash.CollisionDamage"), Attribute_Dash_CollisionDamage },
		{ TEXT("Attribute.Killstreak.ExplosiveRounds"), Attribute_Killstreak_ExplosiveRounds },
		{ TEXT("Attribute.Movement.WalkSpeed"), Attribute_Movement_WalkSpeed },
		{ TEXT("Attribute.Movement.SprintSpeed"), Attribute_Movement_SprintSpeed },
		{ TEXT("Attribute.Movement.CrouchSpeed"), Attribute_Movement_CrouchSpeed },
		{ TEXT("Attribute.Melee.Damage"), Attribute_Melee_Damage },
		{ TEXT("Attribute.Melee.KnockbackForce"), Attribute_Melee_KnockbackForce },
		{ TEXT("Attribute.Melee.KnockbackForceDead"), Attribute_Melee_KnockbackForceDead },
		{ TEXT("Attribute.Melee.Cooldown"), Attribute_Melee_Cooldown },
		{ TEXT("Attribute.Melee.HitDetection"), Attribute_Melee_HitDetection },
		{ TEXT("Attribute.Melee.HitDetectionRadius"), Attribute_Melee_HitDetectionRadius },
		{ TEXT("Attribute.Melee.SwingAmount"), Attribute_Melee_SwingAmount },
		{ TEXT("Attribute.Slide.Strength"), Attribute_Slide_Strength },
		{ TEXT("Attribute.Slide.Duration"), Attribute_Slide_Duration },
		{ TEXT("Attribute.Slide.Cooldown"), Attribute_Slide_Cooldown },
		{ TEXT("Attribute.Slide.CollisionDamage"), Attribute_Slide_CollisionDamage },
		{ TEXT("Attribute.Slide.ScopeTimeDilation"), Attribute_Slide_ScopeTimeDilation },
		{ TEXT("Attribute.SlowMo.TimeDilation"), Attribute_SlowMo_TimeDilation },
		{ TEXT("Attribute.SlowMo.MaxDuration"), Attribute_SlowMo_MaxDuration },
		{ TEXT("Attribute.SlowMo.Cooldown"), Attribute_SlowMo_Cooldown },
		{ TEXT("Attribute.Throw.Force"), Attribute_Throw_Force },
		{ TEXT("Attribute.Throw.Damage"), Attribute_Throw_Damage },
		{ TEXT("Attribute.Weapon.FireDelay"), Attribute_Weapon_FireDelay },
		{ TEXT("Attribute.Weapon.ReloadSpeed"), Attribute_Weapon_ReloadSpeed },
		{ TEXT("Attribute.Weapon.Damage"), Attribute_Weapon_Damage },
	};

	FGameplayTag FindTagByString(const FString& TagName)
	{
		if (const FGameplayTag* Found = NameToTag.Find(TagName))
		{
			return *Found;
		}
		return FGameplayTag();
	}
}
