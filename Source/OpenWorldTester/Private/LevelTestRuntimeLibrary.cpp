// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTestRuntimeLibrary.h"

#include "LevelTestRuntimeSettings.h"

DEFINE_LOG_CATEGORY(LevelTestRuntime);

void ULevelTestRuntimeLibrary::RequestTestMapOnGameRuntime(const FSoftObjectPath& Map, UGameInstance* GameInstance,
                                                           FSoftClassPath GameMode)
{
	if (Map.IsValid())
	{
		FString LoadMapError;
		FURL URL;
		URL.Map = Map.GetLongPackageName();
		URL.AddOption(*FString::Printf(TEXT("Game=%s"), *GameMode.ToString()));
		GEngine->LoadMap(*GameInstance->GetWorldContext(), URL, nullptr, LoadMapError);
	}
}

void ULevelTestRuntimeLibrary::SetRuntimeSettings(const FSoftObjectPath& DataAsset)
{
	ULevelTestRuntimeSettings* Settings = Cast<ULevelTestRuntimeSettings>(ULevelTestRuntimeSettings::StaticClass()->GetDefaultObject());
	Settings->SettingData = DataAsset;
	Settings->TryUpdateDefaultConfigFile();
}

ULevelTestRuntimeSettingsDataAsset* ULevelTestRuntimeLibrary::GetRuntimeSettings()
{
	ULevelTestRuntimeSettings* Settings = Cast<ULevelTestRuntimeSettings>(
		ULevelTestRuntimeSettings::StaticClass()->GetDefaultObject());
	Settings->LoadConfig();

	const FSoftObjectPath DataAssetPath = Settings->SettingData;
	if (!DataAssetPath.IsValid())
	{
		UE_LOG(LevelTestRuntime, Error,
		       TEXT(
			       "UPerfectEngineTestSubsystem::Initialize  ---- UPerfectEngineTestDevSettings::GetDevDataAssetDefault() Is Null"
		       ));
	}
	else
	{
		const auto TestSettingsDataAsset = LoadObject<ULevelTestRuntimeSettingsDataAsset>(
			nullptr, *DataAssetPath.GetAssetPathString(), nullptr, LOAD_None, nullptr);

		if (IsValid(TestSettingsDataAsset))
		{
			UE_LOG(LevelTestRuntime, Log, TEXT("OK--TestSettingsDataAsset: %s"),
			       *DataAssetPath.GetLongPackageName());
			return TestSettingsDataAsset;
		}
		else
		{
			UE_LOG(LevelTestRuntime, Error, TEXT("Failed-TestSettingsDataAsset: %s"),
			       *DataAssetPath.GetLongPackageName());
		}
	}

	return nullptr;
}
