// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

struct FProjectSection
{
public:
	FProjectSection()
	{
		
	}

	FString CaptureName;
	FString EnginePath;
	FString ProjectPath;
	FString MapPath;
	FVector ViewPos;
	FRotator ViewRot;
};

class FPerfectBookmarkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();


	FString VectorToString(FVector InPos);
	FVector StringToVector(FString InStr);
	FString RotationToString(FRotator InRot);
	FRotator StringToRotation(FString InStr);


	void OnMapLoad(const FString& Filename, bool bAsTemplate);
	bool Tick(float DeltaTime);
	FString GetViewPath();
	TSharedPtr<FProjectSection> GetProject(FString InSection);
	bool DelProject(TSharedPtr<FProjectSection> InProject);

	bool IsDefaultMapLoaded = false;
	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickerHandle;
	TSharedPtr<FProjectSection> TempSection;
	
private:
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
