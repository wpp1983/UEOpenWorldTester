// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTestEditorLibrary.h"



#include "ITargetDeviceProxy.h"
#include "ITargetDeviceServicesModule.h"
#include "LevelTestEditorLauncher.h"


void ULevelTestEditorLibrary::EditorRequestPlaySession(const FEditorRequestPlayParams& Params)
{
	GEditor->GetEditorSubsystem<ULevelTestEditorLauncher>()->EditorRequestPlaySession(Params);
}

void ULevelTestEditorLibrary::PackageAndLaunchTest(const FEditorRequestPackageAndLaunchParams& Params)
{
	GEditor->GetEditorSubsystem<ULevelTestEditorLauncher>()->PackageAndLaunchTest(Params);
}

TArray<FString> ULevelTestEditorLibrary::GetDeviceIDs()
{
	TArray<FString> DeviceIDs;
	ITargetDeviceServicesModule* TargetDeviceServicesModule = static_cast<ITargetDeviceServicesModule*>(FModuleManager::Get().LoadModule(TEXT("TargetDeviceServices")));
	for (const auto& Pair : FDataDrivenPlatformInfoRegistry::GetAllPlatformInfos())
	{
		if (Pair.Value.bIsFakePlatform)
		{
			continue;
		}

		const FName PlatformName = Pair.Key;
		const FDataDrivenPlatformInfo& Info = Pair.Value;
		if (FDataDrivenPlatformInfoRegistry::IsPlatformHiddenFromUI(PlatformName))
		{
			continue;
		}

		TArray<TSharedPtr<ITargetDeviceProxy>> DeviceProxies;
		TargetDeviceServicesModule->GetDeviceProxyManager()->GetAllProxies(PlatformName, DeviceProxies);

		if (!DeviceProxies.IsEmpty())
		{
			for (const auto TargetDeviceProxy : DeviceProxies)
			{
				FString DeviceId = TargetDeviceProxy->GetTargetDeviceId(NAME_None);
				DeviceIDs.Add(DeviceId);
			}
		}
	}

	return DeviceIDs;
}

TSoftObjectPtr<ULevelTestRuntimeSettings> ULevelTestEditorLibrary::GetLevelTestDevSetting()
{
	return GetDefault<ULevelTestRuntimeSettings>();
}
