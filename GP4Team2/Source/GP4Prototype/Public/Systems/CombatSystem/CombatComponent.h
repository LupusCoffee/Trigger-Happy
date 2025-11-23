#pragma once

#include "CoreMinimal.h"
#include "CombatFiniteStateMachine.h"
#include "Bullets/Bullet.h"
#include "Components/ActorComponent.h"
#include "Systems/CombatSystem/CombatEventsSubsystem.h"
#include "Core/Data/Structs/CombatContext.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "CombatComponent.generated.h"


class UAttributeComponent;
class UObjectPoolSubsystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatInteraction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeHit, USkeletalMeshComponent*, SkelMesh);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRecharge, float, CurrentFireRechargeValue, float, MaxFireRechargeValue, bool, RechargingFromFullDepletion,
	bool, UnderThreshold);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// core overrides and constructors
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Debug options
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugOnScreen = false;

	UPROPERTY(EditAnywhere, Category="Debug", meta=(ClampMin="0.0"))
	float DebugDuration = 5.0f;

	// methods --> setup stuff
	UFUNCTION()
	USkeletalMeshComponent* GetSkelMeshByTag(FName Tag);

	
	// methods --> context, input
	UFUNCTION(BlueprintCallable)
	void SetFireContext(bool WantsToFire);

	UFUNCTION(BlueprintCallable)
	void SetMeleeContext(bool WantsToMelee);

	UFUNCTION(BlueprintCallable)
	void SetAbilityContext(bool WantsToAbility);

	
	// methods --> context, value
	UFUNCTION(BlueprintCallable)
	void OnFireExecuted();

	UFUNCTION(BlueprintCallable)
	void OnMeleeStarted();

	UFUNCTION(BlueprintCallable)
	void OnMeleeFinished();

	UFUNCTION(BlueprintCallable)
	void OnAbilityStarted();
	
	
	// delegates
    UPROPERTY(BlueprintAssignable)
    FOnCombatInteraction OnFire;
	
	UPROPERTY(BlueprintAssignable)
	FOnRecharge FireRechargeContext;

	UPROPERTY(BlueprintAssignable)
	FOnCombatInteraction OnMeleeStart;

	UPROPERTY(BlueprintAssignable)
	FOnMeleeHit OnMeleeHit;

	UPROPERTY(BlueprintAssignable)
	FOnCombatInteraction OnMeleeFinish;

	UPROPERTY(BlueprintAssignable)
	FOnCombatInteraction OnAbilityStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnCombatInteraction OnAbilityFinish;
	


	// methods --> verbs (?)
	UFUNCTION(BlueprintCallable)
	void Fire();


	// methods --> setts
	UFUNCTION(BlueprintCallable)
	void SetBullet(TSubclassOf<ABullet> Bullet);

	UFUNCTION(BlueprintCallable)
	void SetOneShotBullet(TSubclassOf<ABullet> Bullet);

	UFUNCTION(BlueprintCallable)
	void ResetOneShotBullet();
	

	// methods --> getters
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetGunMeshComp();
	
	UFUNCTION(BlueprintCallable)
	FName GetFirePointSocketName();
	
	UFUNCTION(BlueprintCallable)
	FTransform GetFirePoint();

	
protected:
	// core overrides
	virtual void BeginPlay() override;


	// variables --> edit, general
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	FName CharacterArmsTag = TEXT("CharArms");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	TEnumAsByte<ECollisionChannel> CombatTraceChannel = ECC_Camera;
	
	
	// variables --> edit, fire state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire --> fallback settings")
	float FallbackFireDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	float BulletSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire --> fallback settings")
	float FallbackFireDelay = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	float FireTraceRange = 800;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire") //todo: change to spread (don't wanna sphere cast)
	float FireTraceRadius = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	TSubclassOf<ABullet> StandardBullet = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	UAnimMontage* GunFireMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	USkeletalMesh* GunMeshAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	TSubclassOf<UAnimInstance> GunAnimBlueprint = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	FName CharArmsGunSocketName = TEXT("ik_hand_gun");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	FName GunMeshFirePointSocket = TEXT("Socket_FirePoint");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire|Pooling", meta=(ClampMin="0"))
	int32 BulletPoolInitialSize = 100;
	
	
	// variables --> edit, recharge / reload state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge")
	int MaxFireChargeCapacity = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge")
	float FireChargeConsumption = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge")
	float TimeBeforeFireRechargeBeginAfterFire = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge --> fallback settings")
	float FallbackRegularRechargeRate = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge")
	float FullyDepletedRechargeRate = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Reload / Fire Recharge", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float RechargeThresholdAfterFullDepletion = 0.2f;
	
	
	// variables --> edit, melee state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee --> fallback settings")
	float FallbackMeleeDamage = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee --> fallback settings")
	float FallbackForwardMeleeKnockbackStrengthForSurvivingEnemies = 250;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float UpwardMeleeKnockbackStrengthForSurvivingEnemies = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee --> fallback settings")
	float FallbackForwardMeleeKnockbackStrengthForDyingEnemies = 2000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float TimeDialationOnKill = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float TimeDialationDuration = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float MeleeForwardStartOffset = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float MeleeRadius = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float MeleeReach = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float MeleeWidth = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee --> fallback settings")
	int FallbackMaxMeleeHitCapacity = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	float MeleeDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee --> fallback settings")
	float FallbackMeleeCooldown = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	UAnimMontage* BatMeleeMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	UStaticMesh* BatMeshAsset = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	FName CharArmsBatSocketName = TEXT("batSocket_l");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	FName BatBottomSocket = TEXT("Socket_BatBottom");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	FName BatTipSocket = TEXT("Socket_BatTip");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	FName MeleeStartNotify = TEXT("MeleeStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Melee")
	FName MeleeEndNotify = TEXT("MeleeEnd");
	

	// variables --> hidden, dynamic
	UPROPERTY()
	float CurrentFireCooldownTime = 0;

	UPROPERTY()
	TSubclassOf<ABullet> OneShotBullet = nullptr;

	UPROPERTY()
	bool UseOneShotBullet = false;

	UPROPERTY()
	float TimeSinceLastFire = 0;

	UPROPERTY(BlueprintReadOnly)
	float CurrentFireChargeCapacity = 0;

	UPROPERTY()
	float CurrentFireRechargeRate = 0;

	UPROPERTY()
	bool bHasBeenFullyDepleted = false;

	UPROPERTY()
	bool UnderThreshold = false;

	UPROPERTY()
	float CurrentMeleeCooldownTime = 0;
	
	
	// variables --> hidden, components and such
	UPROPERTY()
	APlayerController* PlayerController = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCombatFiniteStateMachine> CombatFSM = nullptr;
	
	UPROPERTY()
	FCombatContext InputContext = FCombatContext();

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* CharacterArms = nullptr;

	UPROPERTY(VisibleAnywhere)
	UAnimInstance* CharacterArmAnimInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee")
	USkeletalMeshComponent* GunMeshComp = nullptr;

	UPROPERTY()
	UStaticMeshComponent* BatMeshComp = nullptr;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	UCombatEventsSubsystem* CombatEventsSubsystem = nullptr;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem = nullptr;

	UPROPERTY()
	UObjectPoolSubsystem* PoolSubsystem = nullptr;
	
	// methods
	UFUNCTION()
	void TickFireCooldown(float DeltaTime);

	UFUNCTION()
	void TickFireRecharge(float DeltaTime);
	
	UFUNCTION()
	void TickMeleeCooldown(float DeltaTime);
};
