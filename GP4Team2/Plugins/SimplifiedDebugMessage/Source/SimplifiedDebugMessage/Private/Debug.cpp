// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug.h"


void Debug::Log(FString Message, bool AddOnScreenMessage, float Duration)
{
	UE_LOG(LogTemp, Display, TEXT("%s"), *Message);

	if (AddOnScreenMessage)
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Cyan, *Message);
}

void Debug::LogWarning(FString Message, bool AddOnScreenMessage, float Duration)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (AddOnScreenMessage)
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Yellow, *Message);

}

void Debug::LogError(FString Message, bool AddOnScreenMessage, float Duration)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *Message);

	if (AddOnScreenMessage)
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Red, *Message);
}