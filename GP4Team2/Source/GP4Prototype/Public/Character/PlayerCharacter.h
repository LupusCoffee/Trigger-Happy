#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/CombatSystem/Components/HealthComponent.h"
#include "Systems/UpgradeSystem/UpgradeManagerComponent.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class GP4PROTOTYPE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Attack Tokens")
	float TotalMeleeAttackTokens = 10.0f;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CPP-Components")
	// UAttributeComponent* AttributeComponent;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CPP-Components")
	// UUpgradeManagerComponent* UpgradeManager;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CPP-Components")
	// UHealthComponent* HealthComp;
};
