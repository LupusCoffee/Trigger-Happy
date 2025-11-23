#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Systems/CombatSystem/Abilities/GameplayAbilityObject.h"
#include "AbilityComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GP4PROTOTYPE_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// methods
	UFUNCTION()
	void SetCurrentCombatAbility(UGameplayAbilityObject* Ability);

	UFUNCTION()
	void SetCurrentSupportAbility(UGameplayAbilityObject* Ability);

	UFUNCTION()
	void AddPassiveAbility(UGameplayAbilityObject* Ability);
	
	UFUNCTION(BlueprintCallable)
	UGameplayAbilityObject* GetCurrentCombatAbility();
	
	UFUNCTION(BlueprintCallable)
	UGameplayAbilityObject* GetCurrentSupportAbility();

	/*UFUNCTION()
	UGameplayAbilityObject* GetPassiveAbility(Ability Class);*/

	
	//the most temporary shit ever
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool WantsToSupportAbility;
	
	UPROPERTY()
	bool SupportAbilityUsed = false;
	
	UFUNCTION(BlueprintCallable)
	void Ability_BoxTraceMulti(
		const UObject* WorldContextObject,
		FVector Start,
		FVector End,
		FVector HalfSize,
		FRotator Orientation,
		TEnumAsByte<ETraceTypeQuery> TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		EDrawDebugTrace::Type DrawDebugType,
		TArray<FHitResult>& OutHits,
		bool bIgnoreSelf
	);

	UFUNCTION(BlueprintCallable, Category="Ability")
	AActor* SpawnActorForAbility(TSubclassOf<AActor> Class, const FTransform& Xform, ESpawnActorCollisionHandlingMethod Handling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	UFUNCTION(BlueprintCallable, Category="Ability")
	void PlaySound2D(USoundBase* Sound, UAudioComponent*& OutAudioComp);
	

protected:
	virtual void BeginPlay() override;

	// variables --> editable, static values
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	UGameplayAbilityObject* StartingCombatAbility = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	UGameplayAbilityObject* StartingSupportAbility = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="General")
	TArray<UGameplayAbilityObject*> StartingPassiveAbilities;
	
	// variables --> hidden, dynamic values
	UPROPERTY()
	UGameplayAbilityObject* CurrentCombatAbility = nullptr;

	UPROPERTY()
	UGameplayAbilityObject* CurrentSupportAbility = nullptr;

	UPROPERTY()
	TArray<UGameplayAbilityObject*> CurrentPassiveAbilities;

	// methods
	UFUNCTION(BlueprintCallable)
	void StartAbility(UGameplayAbilityObject* Ability);
	
	UFUNCTION()
	void TickAbilityUse(UGameplayAbilityObject* Ability, float DeltaTime);

	UFUNCTION()
	void TickCurrentPassiveAbilitiesUse(float DeltaTime);
	
	UFUNCTION()
	void TickAbilityCooldown(UGameplayAbilityObject* Ability, float DeltaTime);

	UFUNCTION()
	void TickCurrentPassiveAbilitiesCooldown(float DeltaTime);
};
