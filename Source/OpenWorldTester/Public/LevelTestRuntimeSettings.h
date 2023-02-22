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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	FSoftObjectPath StartMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	FSoftClassPath FlyGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LevelTest)
	FSoftClassPath ThirdPersonGameMode;
};


/**
 * 
 */
UCLASS(config=Engine, defaultconfig, meta=(DisplayName="LevelTest"))
class OPENWORLDTESTER_API ULevelTestRuntimeSettings final : public UDeveloperSettings 
{
	GENERATED_BODY()
public:

	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = LevelTest)
	FSoftObjectPath SettingData;
};




