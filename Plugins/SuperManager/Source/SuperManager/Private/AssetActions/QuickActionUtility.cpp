// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickActionUtility.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

void UQuickActionUtility::DuplicateAsset(int32 NumOfDuplicates)
{
	if(NumOfDuplicates <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for(int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.GetSoftObjectPath().ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("SUCCESS - " + FString::FromInt(Counter) + " files"));
		/*Print(TEXT("SUCCESS - " + FString::FromInt(Counter) + " files"), FColor::Green);*/
	}
}

void UQuickActionUtility::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for(UObject* SelectedObject : SelectedObjects)
	{
		if(!SelectedObject) continue;

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if(!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if(OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		if(SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd("_Inst");
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		++Counter;
	}

	if(Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfuly renamed " + FString::FromInt(Counter) + " assets"));
	}
	
}

void UQuickActionUtility::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	for(const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetSoftObjectPath().ToString());

		if(AssetReferences.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if(UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("No unused assets"), false);
		return;
	}
	
	int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);

	if(NumOfAssetsDeleted == 0) return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + " unused assets"));
}
