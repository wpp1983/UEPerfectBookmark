// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerfectBookmarkCommands.h"

#define LOCTEXT_NAMESPACE "FPerfectBookmarkModule"

void FPerfectBookmarkCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "PerfectBookmark", "Execute PerfectBookmark action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
