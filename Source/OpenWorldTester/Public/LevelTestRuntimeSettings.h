// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "LevelTestRuntimeSettings.generated.h"


UCLASS(BlueprintType)
class OPENWORLDTESTER_API ULevelTestRuntimeSettingsDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	TArray<FSoftObjectPath> Maps;


	
};



USTRUCT(BlueprintType)
struct OPENWORLDTESTER_API FLevelTestPackagingSetting 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	FSoftObjectPath StartMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest, meta=(AllowedClasses="GameModeBase", DisplayName="GameMode", LongPackageName))
	TArray<FSoftClassPath> GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest, meta=(DisplayName="Additional Asset Directories to Cook", LongPackageName))
	TArray<FDirectoryPath> DirectoriesToAlwaysCook;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	TArray<FString> PluginsToDisableInCook;
	
};


/**
 * 
 */
UCLASS(config=Engine, defaultconfig, meta=(DisplayName="LevelTest"))
class OPENWORLDTESTER_API ULevelTestRuntimeSettings final : public UDeveloperSettings 
{
	GENERATED_BODY()
public:

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = LevelTest, AdvancedDisplay)
	FSoftObjectPath SettingData;

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = PackagingSetting, AdvancedDisplay)
	FLevelTestPackagingSetting PackagingSetting;
};




