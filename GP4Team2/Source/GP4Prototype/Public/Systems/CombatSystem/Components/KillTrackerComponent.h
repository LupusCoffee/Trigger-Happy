#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KillTrackerComponent.generated.h"
class UAttributeComponent;
class ABullet;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillInARowThresholdForOneShotBulletAchieved, TSubclassOf<ABullet>, OneShotBullet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillInARowThresholdForLifeStealAchieved, float, HealthToRegain);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UKillTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKillTrackerComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// methods --> setters/adders
	UFUNCTION(BlueprintCallable)
	void AddKill();

	// methods --> getters
	UFUNCTION(BlueprintCallable)
	bool IsKillStreakActive();

	// delegates, event
	UPROPERTY(BlueprintAssignable)
	FOnKillInARowThresholdForOneShotBulletAchieved OnKillInARowThresholdForOneShotBulletAchieved;

	UPROPERTY(BlueprintAssignable)
	FOnKillInARowThresholdForLifeStealAchieved OnKillInARowThresholdForLifeStealAchieved;
	
protected:
	virtual void BeginPlay() override;

	// variables --> editable
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	float MaxTimeBetweenKills = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General --> fallback settings")
	int FallbackKillStreakForOneShotBullet = 6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General --> fallback settings")
	int KillStreakForLifeSteal = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	TSubclassOf<ABullet> OneShotBullet = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	float HealthToRegainUponLifeSteal = 25;

	
	// variables --> visible
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="General")
	int CurrentTotalKills = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="General")
	int CurrentKillStreak = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="General")
	float TimeSinceLastKill = 0;

	
	// variables --> hidden, components
	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;
};
