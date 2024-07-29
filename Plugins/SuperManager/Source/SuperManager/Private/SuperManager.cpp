// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "SlateWidgets/AdvancedDeleteWidget.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
	
	RegisterAdvancedDeleteTab();
}

#pragma region ContentBrowserMenuExtension

void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);

	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/
	
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender (new FExtender());

	if(SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));

		FolderPathsSelected = SelectedPaths;
	}
	
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanced Delete")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvancedDeleteButtonClick)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked()
{
	if(FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok ,TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if(AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok ,TEXT("No assets found"));
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,
		TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" found. \nWould you like to procceed?"));

	if(ConfirmResult == EAppReturnType::No) return;

	UpdateRedirectors();
	
	TArray<FAssetData> UnusedAssetsDataArray;

	for(const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if(UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{	
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok ,TEXT("No unused assets found"));
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	UpdateRedirectors();
	
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for(const FString& FolderPath : FolderPathsArray)
	{
		if(FolderPath.Contains(TEXT("Developers")) || FolderPath.Contains(TEXT("Collections")))
		{
			continue;
		}
		
		if(!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if(EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in: \n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if(ConfirmResult == EAppReturnType::Cancel) return;

	for(const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if(UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)) ++Counter;
			else DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfuly deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}
}

void FSuperManagerModule::OnAdvancedDeleteButtonClick()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDelete"));
}

void FSuperManagerModule::UpdateRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace(FTopLevelAssetPath(TEXT("/Script/CoreUObject.ObjectRedirector")));

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirectors);

	for(const FAssetData& RedirectorData : OutRedirectors)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvancedDeleteTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDelete"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvancedDeleteTab))
		.SetDisplayName(FText::FromString(TEXT("Advanced Delete")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvancedDeleteTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
	SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		SNew(SAdvancedDeleteTab)
		.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
	];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for(const FString& AssetPathName : AssetsPathNames)
	{
		if(AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))
		{
			continue;
		}
		
		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvailableAssetData.Add(MakeShared<FAssetData>(Data));
	}
	
	return AvailableAssetData;
}

#pragma endregion

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)