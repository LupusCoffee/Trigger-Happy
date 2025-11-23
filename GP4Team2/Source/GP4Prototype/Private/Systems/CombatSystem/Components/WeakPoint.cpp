#include "Systems/CombatSystem/Components/WeakPoint.h"


UWeakPoint::UWeakPoint()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeakPoint::Init(FName InBoneAttachedTo)
{
	BoneAttachedTo = InBoneAttachedTo;
}

FName UWeakPoint::GetBoneAttachedTo()
{
	return BoneAttachedTo;
}

float UWeakPoint::GetDamageMultiplier()
{
	return DamageMultiplier;
}

bool UWeakPoint::UsesSpecialHitCapsule()
{
	return UseSpecialHitCapsule;
}

float UWeakPoint::GetHitCapsuleRadius()
{
	return HitCapsuleRadius;
}

float UWeakPoint::GetHitCapsuleHalfHeight()
{
	return HitCapsuleHalfHeight;
}