#pragma once

#include "MovementContext.generated.h"

USTRUCT(BlueprintType)
struct FMovementContext
{
	GENERATED_BODY()


	// input context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	FVector2f MoveInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToDash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToVault;

	
	// movement values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bCanDash;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bCanSlide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bCanVault;

	
	// unlockable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bMeleeDashUnlocked;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bMeleeSlideUnlocked;
	


	// misc
	UPROPERTY()
	FVector VaultStartLocation;
	

	FMovementContext() = default;
};