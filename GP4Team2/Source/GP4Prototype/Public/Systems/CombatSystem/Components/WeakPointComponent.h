#pragma once

#include "CoreMinimal.h"
#include "WeakPoint.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "WeakPointComponent.generated.h"


USTRUCT(BlueprintType)
struct FWeakPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool UseSpecialHitCapsule = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HitCapsuleRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HitCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator Rotation;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UWeakPointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeakPointComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;


	// variables --> editable
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	FName TagOfDamageableSkelMesh = "DamageableSkelMesh";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	FName CollisionPresetName = "EnemyCharacterDamageableHitMesh";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	TMap<FName, FWeakPointData> WeakPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	bool DrawDebug = true;

	
	// variables --> visible
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visible")
	TArray<UWeakPoint*> CreatedWeakPoints;
	
	
	// variables --> hidden, components
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	

	// methods
	UFUNCTION()
	UWeakPoint* CreateWeakPoint(USkeletalMeshComponent* TargetMesh, FName BoneName, float Radius, float HalfHeight, FRotator Rotation, bool ShouldGenerateOverlapEvents, FName CapsuleCollisionPresetName);

	UFUNCTION()
	void DrawHitCapsuleDebug(UCapsuleComponent* Capsule, float Duration);
};
