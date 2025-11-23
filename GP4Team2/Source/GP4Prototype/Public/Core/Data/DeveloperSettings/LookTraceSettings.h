#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LookTraceSettings.generated.h"


UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Look Trace"))
class GP4PROTOTYPE_API ULookTraceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	//Methods
	UFUNCTION(BlueprintPure, Category="LookTrace")
	float GetTraceLength();

	UFUNCTION(BlueprintPure, Category="LookTrace")
	float GetTraceRadius();

	UFUNCTION(BlueprintPure, Category="LookTrace")
	bool DoesDrawDebug();

protected:
	//Variables --> Edit
	UPROPERTY(EditAnywhere, Config, Category="Interaction")
	float PlayerTraceLength = 500.0f;

	UPROPERTY(EditAnywhere, Config, Category="Interaction")
	float PlayerTraceRadius = 12.0f;

	UPROPERTY(EditAnywhere, Config, Category="Interaction")
	bool bDrawTraceDebug = false;
};
