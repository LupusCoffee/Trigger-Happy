#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GP4Prototype/Public/Core/Data/Structs/MovementContext.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "CustomCharacterMovementComponent.generated.h"
class UMovementFiniteStateMachine;
class UBaseMovementFiniteStateMachine;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveInteraction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDash, float, CurrentDashChargeTime, float, MaxDashChargeTime, int, CurrentDashCharges, int, MaxDashCharges);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GP4PROTOTYPE_API UCustomCharacterMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// constructors / core overrides
	UCustomCharacterMovementComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	// methods --> input context
	UFUNCTION(BlueprintCallable)
	void SetMoveContext(FVector2f MoveInput);

	UFUNCTION(BlueprintCallable)
	void SetSprintContext(bool WantsToSprint);

	UFUNCTION(BlueprintCallable)
	void SetCrouchContext(bool WantsToCrouch);

	UFUNCTION(BlueprintCallable)
	void SetDashContext(bool WantsToDash);

	// methods --> unlock abilities
	UFUNCTION(BlueprintCallable)
	void UnlockMeleeDash();

	UFUNCTION(BlueprintCallable)
	void UnlockMeleeSlide();

	
	// methods --> value context
	UFUNCTION()
	void OnDashExecuted();

	UFUNCTION()
	void OnSlideExecuted();

	UFUNCTION()
	void OnSlideVaultExecuted();

	
	// delegates
	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Walk state covers both idle and normal speed walking"))
	FOnMoveInteraction OnWalkStateEnter;

	UPROPERTY(BlueprintAssignable, meta = (ToolTip = "Walk state covers both idle and normal speed walking"))
	FOnMoveInteraction OnWalkStateExit;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnCrouchStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnCrouchFinish;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSprintStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSprintFinish;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSlideStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSlideFinish;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnDashStart;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnDashFinish;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction DashChargeReplenish;

	UPROPERTY(BlueprintAssignable)
	FOnDash DashContextUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnRegularVaultStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnRegularVaultFinish;

	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSlideVaultStart;
	
	UPROPERTY(BlueprintAssignable)
	FOnMoveInteraction OnSlideVaultFinish;
	

protected:
	// core overrides
	virtual void BeginPlay() override;


	// variables --> edit, general
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	float FallSpeedInAir = 9999999999.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	float GravityScale = 9999999999.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	TEnumAsByte<ECollisionChannel> MovementTraceChannel = ECC_Visibility;
	
	// variables --> edit, walk/sprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Walk/Sprint --> fallback settings")
	float FallbackWalkSpeed = 700;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Walk/Sprint --> fallback settings")
	float FallbackSprintSpeed = 1200;

	// variables --> edit, crouch
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Crouch")
	float CrouchDownSpeed = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Crouch")
	float UncrouchSpeed = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Crouch --> fallback settings")
	float FallbackCrouchSpeed = 450;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Crouch")
	float CrouchHeightMultiplier = 0.7f;

	// variables --> edit, dash
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash --> fallback settings")
	float FallbackDashSpeed = 5000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash")
	float DashDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash")
	float AccelerationDuringDash = 9999999999.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash")
	float DashFallSpeedInAir = 9999999999.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash")
	float DashGravityScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash")
	bool bCanDashInAir = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash --> fallback settings")
	int FallbackMaxDashCharges = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash --> fallback settings")
	float FallbackDashCooldownPerChargeTime = 0.2f;

	// variables --> edit, dash hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash Hit")
	int MaxDashHitCapacity = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash Hit --> fallback settings")
	float FallbackDashHitDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash Hit")
	float FallbackForwardHitKnockbackStrengthForSurvivingEnemies = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash Hit")
	float UpwardHitKnockbackStrengthForSurvivingEnemies = 300;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Dash Hit")
	float FallbackForwardHitKnockbackStrengthForDyingEnemies = 2000;

	// variables --> edit, slide
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide --> fallback settings")
	float FallbackSlideStrength = 2500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide")
	float SlideFrictionLevel = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide --> fallback settings")
	float FallbackSlideDuration = 2.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide --> fallback settings")
	float FallbackSlideCooldownTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide")
	float SlideHeightMultiplier = 0.7f;

	// variables --> edit, slide hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	int MaxSlideHitCapacity = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit --> fallback settings")
	float FallbackSlideHitDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	float FallbackForwardSlideHitKnockbackStrengthForSurvivingEnemies = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	float UpwardSlideHitKnockbackStrengthForSurvivingEnemies = 300;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	float FallbackForwardSlideHitKnockbackStrengthForDyingEnemies = 2000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	float SlideHitTraceLength = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Slide hit")
	float SlideHitTraceRadius = 2;

	// variables --> edit, Vault
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	FName RegularVaultableTagName = "RegularVaultable";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	FName SlideVaultableTagName = "SlideVaultable";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float VaultTraceLength = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float VaultTraceRadius = 2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float SlideVaultStrength = 900;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float SlideVaultFrictionLevel = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float SlideVaultCooldownTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float SlideVaultHeightMultiplier = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float SprintToVaultHeightRequirement = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Movement, Vault")
	float DashToVaultHeightRequirement = 500.0f;

	
	// variables --> hidden, dynamic
	UPROPERTY(BlueprintReadOnly, Category="Movement, Dash")
	int CurrentDashCharges = 0;

	UPROPERTY(BlueprintReadOnly, Category="Movement, Dash")
	float CurrentDashCooldownTime = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="Movement, Slide")
	float CurrentSlideCooldownTime = 0;

	UPROPERTY(BlueprintReadOnly, Category="Movement, Vault")
	float CurrentVaultCooldownTime = 0;

	
	// variables --> hidden, static
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMovementFiniteStateMachine> MovementFSM = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FMovementContext InputContext = FMovementContext();
	
	UPROPERTY()
	AActor* Owner = nullptr;
	
	UPROPERTY()
	APawn* Pawn = nullptr;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;


	// methods
	UFUNCTION()
	void TickDashCooldown(float DeltaTime);

	UFUNCTION()
	void TickSlideCooldown(float DeltaTime);

	UFUNCTION()
	void TickVaultCooldown(float DeltaTime);

	// methods, event subscriptions
	UFUNCTION()
	void OnAttributeValueChanged(FGameplayTag AttributeTag, float OldValue, float NewValue);
};
