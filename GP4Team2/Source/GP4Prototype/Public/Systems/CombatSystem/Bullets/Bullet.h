#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Bullet.generated.h"
class UCombatEventsSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FOnHit, AActor*, HitActor, float, Damage, FName, HitSocketName, UPrimitiveComponent*, OverlappedComp, UPrimitiveComponent*, OtherComp, int32, OtherBodyIndex, bool, bFromSweep, const FHitResult&, SweepResult);

UCLASS()
class GP4PROTOTYPE_API ABullet : public AActor
{
	GENERATED_BODY()

public:
	ABullet();
	virtual void Tick(float DeltaTime) override;

	// methods
	UFUNCTION()
	void Init(UCombatEventsSubsystem* InCombatEventsSubsystem, float Damage, float MoveSpeed);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// variables --> editable values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Values")
	float BulletSpecificDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Values")
	float BulletSpecificSpeedMultiplier = 1.0f;

	
	// variables --> editable components
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UProjectileMovementComponent* MoveComp;



	// variables --> visible components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCombatEventsSubsystem* CombatEventsSubsystem;
	

	// variables --> hidden values
	UPROPERTY()
	float BaseDamage;

	UPROPERTY()
	float BaseMoveSpeed;


	// delegates
	UPROPERTY(BlueprintAssignable)
	FOnHit OnHit;


	// methods
	UFUNCTION()
	void BulletHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
