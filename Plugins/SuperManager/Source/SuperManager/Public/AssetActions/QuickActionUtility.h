// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "QuickActionUtility.generated.h"

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor)
	static void DuplicateAsset(int32 NumOfDuplicates);
};
