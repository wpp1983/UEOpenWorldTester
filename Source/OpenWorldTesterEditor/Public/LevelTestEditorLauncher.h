// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

// #if PW7_SCP
// #include "QI_Settings.h"
// #endif


#include "LevelTestEditorLauncher.generated.h"



DECLARE_LOG_CATEGORY_EXTERN(LevelTestEditorLauncher, Log, All);

struct FEditorRequestPlayParams;
struct FEditorRequestPackageAndLaunchParams;
class ULevelTestRuntimeSettingsDataAsset;

/**
 * 
 */
UCLASS()
class OPENWORLDTESTEREDITOR_API ULevelTestEditorLauncher final : public UEditorSubsystem 
{
	GENERATED_BODY()

public:
	//~UEditorSubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of UEditorSubsystem interface

	void EditorRequestPlaySession(const FEditorRequestPlayParams& Params) const;

	// copy and change from UEditorEngine::StartPlayUsingLauncherSession
	void PackageAndLaunchTest(const FEditorRequestPackageAndLaunchParams& Params);

	void CancelPlayUsingLauncher();

	// launch on callbacks
	void HandleStageStarted(const FString& InStage, TWeakPtr<SNotificationItem> NotificationItemPtr);
	void HandleStageCompleted(const FString& InStage, double StageTime, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr);
	void HandleLaunchCanceled(double TotalTime, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr);
	void HandleLaunchCompleted(bool Succeeded, double TotalTime, int32 ErrorCode, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr);

private:
	void ClearProjectPackageSetting();
	void RestoreProjectPackageSetting();

	
	/** This flag is used to skip UAT\UBT compilation on every launch if it was successfully compiled once. */
	bool bUATSuccessfullyCompiledOnce;

	/** 
* The last platform we ran on (as selected by the drop down or via automation)
* Stored outside of the Session Info as the UI needs access to this at all times.
*/
	FString LastPlayUsingLauncherDeviceId;

	
	TArray<FFilePath> Old_MapsToCook;
	TArray<FDirectoryPath> Old_DirectoriesToAlwaysCook;
	FString Old_DefaultGameMap;
	FSoftClassPath Old_GameInstanceClass;
	TArray<FString> Old_PluginsDisableInCook;
// #if PW7_SCP
// 	TMap<FName, FAssetTableConfig> Old_QIConfig;
// #endif
};
