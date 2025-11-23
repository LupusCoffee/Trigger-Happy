#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"
class UAttributeComponent;
class UCombatEventsSubsystem;
enum class EGameDamageType : uint8;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResurrect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealed, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamaged, EGameDamageType, DamageType, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, EGameDamageType, LastDamageTaken);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GP4PROTOTYPE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// constructor, core overrides
	UHealthComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// methods, setup
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitHealth();
	
	// methods, actual uses
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Value);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeHealthDamage(EGameDamageType DamageType, float DamageValue, float DamageMultiplier = 1);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Die(EGameDamageType LastDamageTaken);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void PlayerDeath(EGameDamageType LastDamageTaken);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void EnemyDeath(EGameDamageType LastDamageTaken);
	
	// methods, setters
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UnlockShield(float InMaxShieldValue, float InTimeBeforeRecharge, float InRechargeRate);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void UnlockResurrect(float InPercentageOfMaxHealthToReplenish);

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnDeath OnDeathAsEnemy;

	// needs to be moved down and only visible later
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentHealth = 100.0f;
	
protected:
	// core overrides
	virtual void BeginPlay() override;
	
	
	// variables, components
	UPROPERTY(VisibleAnywhere)
	UCombatEventsSubsystem* CombatEventsSubsystem = nullptr;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;


	// variables, static values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health --> fallback settings")
	float FallbackMaxHealth = 100.0f;


	// variables, dynamic values
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPlayer = false;

	UPROPERTY()
	bool bIsDead = false;


	// variables, shield --> static and dynamic
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShieldUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxShieldValue = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentShieldValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeBeforeShieldRecharge = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentTimeBeforeShieldRecharge = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ShieldRechargeRate = 2.0f;


	// variables, ressurect --> static and dynamic
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bResurrectUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bResurrectHasBeenUsed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PercentageOfMaxHealthToReplenish = 0.5f;


	// methods
	UFUNCTION()
	bool OwnerIsPlayerControlled() const;

	UFUNCTION()
	void TryTakeShieldDamage(EGameDamageType DamageType, float& DamageValue, float DamageMultiplier = 1);

	UFUNCTION()
	void TickShieldRecharge(float DeltaTime);

	
	// delegates
	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnHealed OnHeal;
	
	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnDamaged OnHealthDamage;

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnDamaged OnShieldDamage;

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnResurrect OnResurrect;

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnDeath OnDeathAsPlayer;


};
