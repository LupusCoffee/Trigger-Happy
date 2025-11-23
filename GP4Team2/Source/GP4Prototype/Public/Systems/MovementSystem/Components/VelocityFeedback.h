#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VelocityFeedback.generated.h"
class UCameraComponent;
class UNiagaraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GP4PROTOTYPE_API UVelocityFeedback : public UActorComponent
{
	GENERATED_BODY()

public:
	UVelocityFeedback();

protected:
	virtual void BeginPlay() override;

	//Non-editable Components
	UPROPERTY()
	UCameraComponent* CameraComp;
	UPROPERTY()
	APlayerController* PlayerController;

	//Editable Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake")
	UCameraShakeBase* CameraShake;
	
	//Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "General")
	float MaxVelocitySize = 30000.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FOV")
	float MinFOV = 90.0f; 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FOV")
	float MaxFOV = 150.0f; 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FOV")
	float FovMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	float ShakeVelocityThreshold = 5000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	float MinShake = 90.0f; 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	float MaxShake = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	float ShakeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed Lines")
	class UNiagaraComponent* SpeedLineEffect;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector MoveDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector ThisFrameOwnerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LastFrameOwnerPosition;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VelocityFeedback")
	float SpeedOfEasingIntoMaxFOV = 2.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VelocityFeedback")
	float FovToEaseAt = 130.0f;*/

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
