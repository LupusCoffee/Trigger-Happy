// Fill out your copyright notice in the Description page of Project Settings.

#include "GP4Prototype.h"
#include "Modules/ModuleManager.h"
#include "Systems/SubtitleSystem/SubtitleSubsystem.h"
#include "Subsystems/SubsystemCollection.h"

class FGP4PrototypeModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		UE_LOG(LogTemp, Warning, TEXT("GP4Prototype module starting up"));
		
		// Ensure subsystem class is registered
		UE_LOG(LogTemp, Warning, TEXT("Registering SubtitleSubsystem class: %s"), *USubtitleSubsystem::StaticClass()->GetName());
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogTemp, Warning, TEXT("GP4Prototype module shutting down"));
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FGP4PrototypeModule, GP4Prototype, "GP4Prototype");
