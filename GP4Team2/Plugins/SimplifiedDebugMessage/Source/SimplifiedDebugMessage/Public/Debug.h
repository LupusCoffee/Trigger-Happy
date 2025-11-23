// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SIMPLIFIEDDEBUGMESSAGE_API Debug
{
public:
	static void Log(FString Message, bool AddOnScreenMessage = false, float Duration = 15);
	static void LogWarning(FString Message, bool AddOnScreenMessage = false, float Duration = 15);
	static void LogError(FString Message, bool AddOnScreenMessage = false, float Duration = 15);
};
