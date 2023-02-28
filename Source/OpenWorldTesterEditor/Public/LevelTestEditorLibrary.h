// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LevelTestEditorLibrary.generated.h"


USTRUCT(BlueprintType)
struct FEditorRequestPlayParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor,  EditAnywhere)
	int WindowWidth = 1080;

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor, EditAnywhere)
	int WindowHeight = 720;

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor, EditAnywhere)
	FString LaunchParameters;
};

USTRUCT(BlueprintType)
struct FEditorRequestPackageAndLaunchParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor, EditAnywhere)
	FString InLauncherDeviceId;

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor, EditAnywhere)
	FString LaunchParameters;

	UPROPERTY(BlueprintReadWrite, Category = LevelTestEditor, EditAnywhere)
	bool IsDebug = false;

};


class ULevelTestRuntimeSettingsDataAsset;

/**
 * 
 */
UCLASS()
class OPENWORLDTESTEREDITOR_API ULevelTestEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, Category = LevelTestEditor)
	static void EditorRequestPlaySession(const FEditorRequestPlayParams& Params);
	
	UFUNCTION(BlueprintCallable, Category = LevelTestEditor)
	static void PackageAndLaunchTest(const FEditorRequestPackageAndLaunchParams& Params);
	
	UFUNCTION(BlueprintCallable, Category = LevelTestEditor)
	static TArray<FString> GetDeviceIDs();

	UFUNCTION(BlueprintCallable, Category = LevelTestEditor)
	static TSoftObjectPtr<ULevelTestRuntimeSettings> GetLevelTestDevSetting();
	
};
