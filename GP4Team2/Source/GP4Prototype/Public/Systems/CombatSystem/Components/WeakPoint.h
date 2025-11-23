// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "WeakPoint.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UWeakPoint : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	UWeakPoint();

	// methods, general
	UFUNCTION()
	void Init(FName InBoneAttachedTo);
	
	// methods, getters
	UFUNCTION(BlueprintPure, Category="Components|WeakPoint")
	FName GetBoneAttachedTo();
	
	UFUNCTION(BlueprintPure, Category="Components|WeakPoint")
	float GetDamageMultiplier();

	UFUNCTION(BlueprintPure, Category="Components|WeakPoint")
	bool UsesSpecialHitCapsule();

	UFUNCTION(BlueprintPure, Category="Components|WeakPoint")
	float GetHitCapsuleRadius();

	UFUNCTION(BlueprintPure, Category="Components|WeakPoint")
	float GetHitCapsuleHalfHeight();

protected:	
	// variables, editable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If this is true, the component will attach a capsule with the set radius and half height at the bone, for collision."))
	bool UseSpecialHitCapsule = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitCapsuleRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitCapsuleHalfHeight = 10.0f;

	// variables, hidden
	UPROPERTY()
	FName BoneAttachedTo;
};
