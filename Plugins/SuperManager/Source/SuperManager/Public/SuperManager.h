// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtension

	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;
	
	TSharedRef<FExtender> CustomCBMenuExtender( const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);

	void OnDeleteUnusedAssetsButtonClicked();
	
	void OnDeleteEmptyFoldersButtonClicked();

	void OnAdvancedDeleteButtonClick();
	
	void UpdateRedirectors();
	
#pragma endregion

#pragma region CustomEditorTab

	void RegisterAdvancedDeleteTab();

	TSharedRef<SDockTab> OnSpawnAdvancedDeleteTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
	
#pragma endregion

public:

#pragma region ProccessDataForAdvancedDeleteTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetsList(const TArray<FAssetData>& AssetsToDelete);
	
#pragma endregion 
};
