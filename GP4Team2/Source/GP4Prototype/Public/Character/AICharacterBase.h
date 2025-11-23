// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "AI/Tools/PatrolRoute.h"
#include "GameFramework/Character.h"
#include "Systems/CombatSystem/Components/HealthComponent.h"
#include "AICharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLastManStanding);

class UAttributeComponent;
class AAISpawnBrain;

UCLASS()
class GP4PROTOTYPE_API AAICharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAttributeComponent* AttrComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol Logic")
	bool bStationaryPatrol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol Logic")
	bool bStationaryAttack;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI Patrol Logic")
	TObjectPtr<APatrolRoute> PatrolRoute;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI Brain")
	AAISpawnBrain* SpawnBrain;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Brain")
	bool bHasBeenInjured;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Brain")
	bool bIsAggroed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Instance")
	bool bIsAIUnitAggroed;

	UPROPERTY(BlueprintAssignable, Category="Spawn|Events")
	FOnLastManStanding LastManStanding;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	FDelegateHandle OnDeathDelegateHandle;
	bool bHandledDeath = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category="AI Init")
	void InitializeAICharacterBase();

	UFUNCTION(BlueprintCallable, Category="AI Init")
	void OnAIDeath(EGameDamageType LastDamageTaken);
};
