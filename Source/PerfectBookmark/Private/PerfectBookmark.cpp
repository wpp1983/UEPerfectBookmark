// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerfectBookmark.h"
#include "PerfectBookmarkStyle.h"
#include "PerfectBookmarkCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "FileHelpers.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "UnrealEd/Public/IAssetViewport.h"

static const FName PerfectBookmarkTabName("PerfectBookmark");

#define LOCTEXT_NAMESPACE "FPerfectBookmarkModule"

void FPerfectBookmarkModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FPerfectBookmarkStyle::Initialize();
	FPerfectBookmarkStyle::ReloadTextures();

	FPerfectBookmarkCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPerfectBookmarkCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FPerfectBookmarkModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPerfectBookmarkModule::RegisterMenus));

	FEditorDelegates::OnMapOpened.AddRaw(this, &FPerfectBookmarkModule::OnMapLoad);
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FPerfectBookmarkModule::Tick), 0);
}

void FPerfectBookmarkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorDelegates::OnMapOpened.RemoveAll(this);
	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FPerfectBookmarkStyle::Shutdown();

	FPerfectBookmarkCommands::Unregister();
}

void FPerfectBookmarkModule::PluginButtonClicked()
{
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FMyCaptureModule::PluginButtonClicked()")),
							FText::FromString(TEXT("MyCapture.cpp"))
					   );
	FString CaptureName = "Cap1";
	FString EnginePath = FPlatformProcess::ExecutablePath();
	EnginePath = EnginePath.Replace(TEXT("\\"), TEXT("/"));
	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	FString MapPath = GWorld->GetPathName();
	MapPath = MapPath.Left(MapPath.Find("."));
	FVector ViewPos;
	FRotator ViewRot;
	if (GCurrentLevelEditingViewportClient)
	{
		ViewPos = GCurrentLevelEditingViewportClient->GetViewLocation();
		ViewRot = GCurrentLevelEditingViewportClient->GetViewRotation();
	}
	FString CaptureLink = FString::Printf(TEXT("mylauncher://Capture=%s&&Engine=%s&&Project=%s&&Map=%s&&Pos=%s&&Rot=%s"), *CaptureName, *EnginePath, *ProjectPath, *MapPath, *VectorToString(ViewPos), *RotationToString(ViewRot));
	DialogText = FText::FromString(CaptureLink);
	
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FPerfectBookmarkModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FPerfectBookmarkCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FPerfectBookmarkCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}


FString FPerfectBookmarkModule::VectorToString(FVector InPos)
{
	FString Result = FString::Printf(TEXT("(%f,%f,%f)"), InPos.X, InPos.Y, InPos.Z);

	return Result;
}
FVector FPerfectBookmarkModule::StringToVector(FString InStr)
{
	FVector Result;
	int32 LeftPos = InStr.Find("(");
	int32 RightPos = InStr.Find(")");
	if (LeftPos != INDEX_NONE && RightPos != INDEX_NONE)
	{
		InStr = InStr.Mid(1, InStr.Len()-2);
		Result.X = FCString::Atof(*InStr.Left(InStr.Find(",")));
		InStr = InStr.Right(InStr.Len() - InStr.Find(",") - 1);

		Result.Y = FCString::Atof(*InStr.Left(InStr.Find(",")));
		InStr = InStr.Right(InStr.Len() - InStr.Find(",") - 1);

		Result.Z = FCString::Atof(*InStr);
	}

	return Result;
}
FString FPerfectBookmarkModule::RotationToString(FRotator InRot)
{
	FString Result = FString::Printf(TEXT("(%f,%f,%f)"), InRot.Pitch, InRot.Yaw, InRot.Roll);

	return Result;
}

FRotator FPerfectBookmarkModule::StringToRotation(FString InStr)
{
	FRotator Result;
	FVector TempVec = StringToVector(InStr);
	Result.Pitch = TempVec.X;
	Result.Yaw = TempVec.Y;
	Result.Roll = TempVec.Z;

	return Result;
}

void FPerfectBookmarkModule::OnMapLoad(const FString& Filename, bool bAsTemplate)
{
	if (GWorld)
	{
		IsDefaultMapLoaded = true;
	}
}

bool FPerfectBookmarkModule::Tick(float DeltaTime)
{
	TempSection = GetProject("Temp");
	if (TempSection && IsDefaultMapLoaded)
	{
		FString CurMapPath = GWorld->GetPathName();
		CurMapPath = CurMapPath.Left(CurMapPath.Find("."));
		bool IsTargetMapLoaded = true;
		if (TempSection->MapPath != CurMapPath)
		{
			IsTargetMapLoaded = FEditorFileUtils::LoadMap(TempSection->MapPath);
		}
		if (IsTargetMapLoaded && GCurrentLevelEditingViewportClient)
		{
			GCurrentLevelEditingViewportClient->SetViewLocation(TempSection->ViewPos);
			GCurrentLevelEditingViewportClient->SetViewRotation(TempSection->ViewRot);
			//GCurrentLevelEditingViewportClient->RedrawRequested(nullptr);
			FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			TSharedPtr<IAssetViewport> Viewport = LevelEditorModule.GetFirstActiveViewport();
			if (Viewport.IsValid())
			{
				Viewport->GetAssetViewportClient().RedrawRequested(Viewport->GetActiveViewport());
			}
		}

		TempSection = nullptr;
		FString ConfigPath = GetViewPath();
		GConfig->EmptySection(TEXT("Temp"), *ConfigPath);
		GConfig->UnloadFile(ConfigPath);
	}

	return true;
}

FString FPerfectBookmarkModule::GetViewPath()
{
	return FPaths::Combine("D:/", "ViewToOpen.ini");
}

TSharedPtr<FProjectSection> FPerfectBookmarkModule::GetProject(FString InSection)
{
	FString ConfigPath = GetViewPath();
	GConfig->LoadFile(ConfigPath);
	if (!GConfig->DoesSectionExist(*InSection, ConfigPath))
	{
		return nullptr;
	}

	TSharedPtr<FProjectSection> NewItem = MakeShareable(new FProjectSection());
	NewItem->CaptureName = InSection;
	{
		FString EngineValue;
		GConfig->GetString(*InSection, TEXT("Engine"), EngineValue, *ConfigPath);
		NewItem->EnginePath = EngineValue;
	}
	{
		FString ProjectValue;
		GConfig->GetString(*InSection, TEXT("Project"), ProjectValue, *ConfigPath);
		NewItem->ProjectPath = ProjectValue;
	}
	{
		FString MapValue;
		GConfig->GetString(*InSection, TEXT("Map"), MapValue, *ConfigPath);
		NewItem->MapPath = MapValue;
	}
	{
		FString PosValue;
		GConfig->GetString(*InSection, TEXT("Pos"), PosValue, *ConfigPath);
		NewItem->ViewPos.InitFromString(PosValue);
	}
	{
		FString RotValue;
		GConfig->GetString(*InSection, TEXT("Rot"), RotValue, *ConfigPath);
		NewItem->ViewRot.InitFromString(RotValue);
	}
	return NewItem;
}

bool FPerfectBookmarkModule::DelProject(TSharedPtr<FProjectSection> InProject)
{
	FString ConfigPath = GetViewPath();
	GConfig->EmptySection(*InProject->CaptureName, *ConfigPath);
	GConfig->Flush(false, ConfigPath);

	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPerfectBookmarkModule, PerfectBookmark)