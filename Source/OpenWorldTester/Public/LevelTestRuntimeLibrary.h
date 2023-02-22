// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LevelTestRuntimeLibrary.generated.h"

class ULevelTestRuntimeSettingsDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LevelTestRuntime, Log, All);

/**
 * 
 */
UCLASS()
class OPENWORLDTESTER_API ULevelTestRuntimeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:	
	UFUNCTION(BlueprintCallable, Category = LevelTestRuntimeLibrary)
	static void RequestTestMapOnGameRuntime(const FSoftObjectPath& Map, UGameInstance* GameInstance, FSoftClassPath GameMode);

	UFUNCTION(BlueprintCallable, Category = LevelTestRuntimeLibrary)
	static void SetRuntimeSettings(const FSoftObjectPath& DataAsset);

	UFUNCTION(BlueprintCallable, Category = LevelTestRuntimeLibrary)
	static ULevelTestRuntimeSettingsDataAsset*  GetRuntimeSettings();
	
};
