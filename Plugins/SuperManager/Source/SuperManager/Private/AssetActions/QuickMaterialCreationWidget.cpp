// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"

#pragma region QuickMaterialCreationCore

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if(bCustomMaterialName)
	{
		if(MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter valid name"));
			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;

	if(!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath)) return;

	if(CheckIsNameUsed(SelectedTextureFolderPath, MaterialName)) return;

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName,SelectedTextureFolderPath);

	if(!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("Failed to create material"));
		return;
	}

	for(UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if(!SelectedTexture) continue;

		switch(ChannelPackingType)
		{
		case E_ChannelPackingType::ECPT_NoChannelPacking:

			Default_CreateMaterialNodes(CreatedMaterial,SelectedTexture,PinsConnectedCounter);
			break;

		case E_ChannelPackingType::ECPT_ORM:

			ORM_CreateMaterialNodes(CreatedMaterial,SelectedTexture,PinsConnectedCounter);
			break;

		case E_ChannelPackingType::ECPT_MAX:
			break;

		default:
			break;
		}		
	}

	if(PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully connected ") 
		+ FString::FromInt(PinsConnectedCounter) + (TEXT(" pins")));
	}

	MaterialName = TEXT("M_");
}

#pragma endregion

#pragma region QuickMaterialCreation

bool UQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if(SelectedDataToProcess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}

	bool bMaterialNameSet = false;
	
	for(const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();

		if(!SelectedAsset) continue;

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);

		if(!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n") +
				SelectedAsset->GetName() + TEXT(" is not a texture"));

			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if(OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}

		if(!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));

			bMaterialNameSet = true;
		}
	}

	return true;
}

bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck,false);

	for(const FString& ExistingAssetPath:ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if(ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,MaterialNameToCheck + TEXT(" is already used by asset"));

			return true;
		}
	}

	return false;
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial,PathToPutMaterial,
	UMaterial::StaticClass(),MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture,
	uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode =
	NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	
	if(!TextureSampleNode) return;
	
	if(!CreatedMaterial->HasBaseColorConnected())
	{
		if(TryConnectBaseColor(TextureSampleNode,SelectedTexture,CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	
	if(!CreatedMaterial->HasMetallicConnected())
	{
		if(TryConnectMetallic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	
	if(!CreatedMaterial->HasRoughnessConnected())
	{
		if(TryConnectRoughness(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if(!CreatedMaterial->HasNormalConnected())
	{
		if(TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if(!CreatedMaterial->HasAmbientOcclusionConnected())
	{
		if(TryConnectAO(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	
}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture,
	uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode =
	NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);

	if(!TextureSampleNode) return;

	if(!CreatedMaterial->HasBaseColorConnected())
	{
		if(TryConnectBaseColor(TextureSampleNode,SelectedTexture,CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if(!CreatedMaterial->HasNormalConnected())
	{
		if(TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if(!CreatedMaterial->HasRoughnessConnected())
	{
		if(TryConnectORM(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter+=3;
			return;
		}
	}
}

#pragma endregion

#pragma region CreateMaterialNodesConnectPins

bool UQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample * TextureSampleNode, UTexture2D * SelectedTexture, UMaterial * CreatedMaterial)
{
	for(const FString& BaseColorName:BaseColorArray)
	{
		if(SelectedTexture->GetName().Contains(BaseColorName))
		{
			//Connect pins to base color socket here
			TextureSampleNode->Texture = SelectedTexture;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor)->Connect(0,TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -=600;

			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for(const FString& MetallicName : MetallicArray)
	{
		if(SelectedTexture->GetName().Contains(MetallicName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -=600;
			TextureSampleNode->MaterialExpressionEditorY +=240;

			return true;
		}
	}
	
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(UMaterialExpressionTextureSample * TextureSampleNode, UTexture2D * SelectedTexture, UMaterial * CreatedMaterial)
{
	for(const FString& RoughnessName : RoughnessArray)
	{
		if(SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -=600;
			TextureSampleNode->MaterialExpressionEditorY +=480;

			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(UMaterialExpressionTextureSample * TextureSampleNode, UTexture2D * SelectedTexture, UMaterial * CreatedMaterial)
{
	for(const FString& NormalName : NormalArray)
	{	
		if(SelectedTexture->GetName().Contains(NormalName))
		{
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Normal)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 720;

			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAO(UMaterialExpressionTextureSample * TextureSampleNode, UTexture2D * SelectedTexture, UMaterial * CreatedMaterial)
{
	for(const FString& AOName : AmbientOcclusionArray)
	{	
		if(SelectedTexture->GetName().Contains(AOName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();


			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 960;

			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for(const FString& ORM_Name : ORMArray)
	{
		if(SelectedTexture->GetName().Contains(ORM_Name))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(1, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(2, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(3, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 960;

			return true;
		}
	}
	return false;
}

#pragma endregion
