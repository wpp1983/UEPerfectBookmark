// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "PerfectBookmarkStyle.h"

class FPerfectBookmarkCommands : public TCommands<FPerfectBookmarkCommands>
{
public:

	FPerfectBookmarkCommands()
		: TCommands<FPerfectBookmarkCommands>(TEXT("PerfectBookmark"), NSLOCTEXT("Contexts", "PerfectBookmark", "PerfectBookmark Plugin"), NAME_None, FPerfectBookmarkStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
