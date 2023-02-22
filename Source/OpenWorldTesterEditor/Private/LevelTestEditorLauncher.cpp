// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTestEditorLauncher.h"

#include "AnalyticsEventAttribute.h"
#include "CookerSettings.h"
#include "EditorAnalytics.h"
#include "GameProjectGenerationModule.h"
#include "IDesktopPlatform.h"
#include "ILauncherServicesModule.h"
#include "ILauncherWorker.h"
#include "ITargetDeviceServicesModule.h"
#include "PlatformInfo.h"
#include "Async/Async.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Settings/EditorExperimentalSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "GameFramework/GameModeBase.h"
// #include "ISPC/ispc-1.16.1/examples/cpu/gmres/debug.h"

#include "LevelTestEditorLibrary.h"
#include "LevelTestRuntimeLibrary.h"
#include "LevelTestRuntimeSettings.h"

#define LOCTEXT_NAMESPACE "LevelTestEditorLauncher"

DEFINE_LOG_CATEGORY(LevelTestEditorLauncher);

bool ULevelTestEditorLauncher::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void ULevelTestEditorLauncher::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bUATSuccessfullyCompiledOnce = FApp::IsEngineInstalled() || FApp::GetEngineIsPromotedBuild();
}

void ULevelTestEditorLauncher::Deinitialize()
{
	Super::Deinitialize();
}

void ULevelTestEditorLauncher::EditorRequestPlaySession(const FEditorRequestPlayParams& Params) const
{
	auto Settings = ULevelTestRuntimeLibrary::GetRuntimeSettings();
	if (Settings == nullptr)
		return;
	
	const FSoftObjectPath& Map = Settings->StartMap;
	// Initialize our own copy of the Editor Play settings which we will adjust defaults on.
	ULevelEditorPlaySettings* PlayInEditorSettings = NewObject<ULevelEditorPlaySettings>();
	PlayInEditorSettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlayInEditorSettings->SetPlayNumberOfClients(1);
	PlayInEditorSettings->bLaunchSeparateServer = false;
	PlayInEditorSettings->SetRunUnderOneProcess(true);
	PlayInEditorSettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;
	PlayInEditorSettings->bUseNonRealtimeAudioDevice = true;
	PlayInEditorSettings->AdditionalLaunchParameters = Params.LaunchParameters;
	PlayInEditorSettings->NewWindowWidth = Params.WindowWidth;
	PlayInEditorSettings->NewWindowHeight = Params.WindowHeight;
	
	FRequestPlaySessionParams PlaySessionParams;
	PlaySessionParams.GlobalMapOverride = Map.GetLongPackageName();
	PlaySessionParams.EditorPlaySettings = PlayInEditorSettings;

	
	// if (GameModeClass.IsValid())
	// {
	// 	if (const TSubclassOf<AGameModeBase> OverrideGameModeClass = LoadClass<AGameModeBase>(nullptr, *GameModeClass.ToString()); OverrideGameModeClass != nullptr)
	// 	{
	// 		PlaySessionParams.GameModeOverride = OverrideGameModeClass;
	// 	}
	// }
	
	GEditor->RequestPlaySession(PlaySessionParams);
}

static void HandleHyperlinkNavigate()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("OutputLog"));
}

static void HandleCancelButtonClicked(ILauncherWorkerPtr LauncherWorker)
{
	if (LauncherWorker.IsValid())
	{
		LauncherWorker->Cancel();
	}
}
static void HandleOutputReceived(const FString& InMessage)
{
	if (InMessage.Contains(TEXT("Error:")))
	{
		UE_LOG(LevelTestEditorLauncher, Error, TEXT("UAT: %s"), *InMessage);
	}
	else if (InMessage.Contains(TEXT("Warning:")))
	{
		UE_LOG(LevelTestEditorLauncher, Warning, TEXT("UAT: %s"), *InMessage);
	}
	else
	{
		UE_LOG(LevelTestEditorLauncher, Log, TEXT("UAT: %s"), *InMessage);
	}
}

void ULevelTestEditorLauncher::PackageAndLaunchTest(const FEditorRequestPackageAndLaunchParams& Params)
{
	const ULevelTestRuntimeSettingsDataAsset* SettingsDataAsset = ULevelTestRuntimeLibrary::GetRuntimeSettings();
	
	auto& LauncherServicesModule = FModuleManager::LoadModuleChecked<ILauncherServicesModule>("LauncherServices");
	auto& TargetDeviceServicesModule = FModuleManager::LoadModuleChecked<ITargetDeviceServicesModule>("TargetDeviceServices");

	LastPlayUsingLauncherDeviceId = Params.InLauncherDeviceId;
	FString PlayUsingLauncherDeviceName = LastPlayUsingLauncherDeviceId.Right(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
	
	FString LaunchPlatformName = LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
	FString LaunchPlatformNameFromID = LastPlayUsingLauncherDeviceId.Right(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
	ITargetPlatform* LaunchPlatform = GetTargetPlatformManagerRef().FindTargetPlatform(LaunchPlatformName);
	FString IniPlatformName = LaunchPlatformName;

	// create a temporary device group and launcher profile
	ILauncherDeviceGroupRef DeviceGroup = LauncherServicesModule.CreateDeviceGroup(FGuid::NewGuid(), TEXT("PlayOnDevices"));
	if (LaunchPlatform != nullptr)
	{
		IniPlatformName = LaunchPlatform->IniPlatformName();
		if (LaunchPlatformNameFromID.Equals(LaunchPlatformName))
		{
			// create a temporary list of devices for the target platform
			TArray<ITargetDevicePtr> TargetDevices;
			LaunchPlatform->GetAllDevices(TargetDevices);

			for (const ITargetDevicePtr& PlayDevice : TargetDevices)
			{
				// compose the device id
				FString PlayDeviceId = LaunchPlatformName + TEXT("@") + PlayDevice.Get()->GetId().GetDeviceName();
				if (PlayDevice.IsValid() && !PlayDevice->IsAuthorized())
				{
					CancelPlayUsingLauncher();
				}
				else
				{
					DeviceGroup->AddDevice(PlayDeviceId);
					UE_LOG(LevelTestEditorLauncher, Log, TEXT("Launcher Device ID: %s"), *PlayDeviceId);
				}
			}
		}
		else
		{
			ITargetDevicePtr PlayDevice = LaunchPlatform->GetDefaultDevice();
			if (PlayDevice.IsValid() && !PlayDevice->IsAuthorized())
			{
				CancelPlayUsingLauncher();
			}
			else
			{

				DeviceGroup->AddDevice(LastPlayUsingLauncherDeviceId);
				UE_LOG(LevelTestEditorLauncher, Log, TEXT("Launcher Device ID: %s"), *LastPlayUsingLauncherDeviceId);
			}
		}

		if (DeviceGroup.Get().GetNumDevices() == 0)
		{
			return;
		}
	}	
	
	// set the build/launch configuration 
	EBuildConfiguration BuildConfiguration = EBuildConfiguration::Development;

	// does the project have any code?
	FGameProjectGenerationModule& GameProjectModule = FModuleManager::LoadModuleChecked<FGameProjectGenerationModule>(TEXT("GameProjectGeneration"));
	bool bPlayUsingLauncherHasCode = GameProjectModule.Get().ProjectHasCodeFiles();

	// Figure out if we need to build anything
	ELauncherProfileBuildModes::Type BuildMode = ELauncherProfileBuildModes::Auto;

	// Setup launch profile, keep the setting here to a minimum.
	ILauncherProfileRef LauncherProfile = LauncherServicesModule.CreateProfile(TEXT("Launch On Device"));
	LauncherProfile->SetBuildMode(BuildMode);
	LauncherProfile->SetBuildConfiguration(BuildConfiguration);
	LauncherProfile->SetAdditionalCommandLineParameters(Params.LaunchParameters);
	LauncherProfile->AddCookedPlatform(LaunchPlatformName);
	
	// select the quickest cook mode based on which in editor cook mode is enabled
	const UCookerSettings& CookerSettings = *GetDefault<UCookerSettings>();
	const UEditorExperimentalSettings& ExperimentalSettings = *GetDefault<UEditorExperimentalSettings>();

	bool bInEditorCooking = false;
	bool bCookOnTheFly = false;
	ELauncherProfileCookModes::Type CurrentLauncherCookMode = ELauncherProfileCookModes::ByTheBook;

	bool bIncrementalCooking = (CookerSettings.bIterativeCookingForLaunchOn || ExperimentalSettings.bSharedCookedBuilds) && !bCookOnTheFly;

	TStringBuilder<256> CookOptions;
	CookOptions << LauncherProfile->GetCookOptions();
	ensure(CookOptions.Len() == 0);
	auto SetCookOption = [&CookOptions](FStringView Option, bool bOptionOn)
	{
		ensure(CookOptions.ToView().Find(Option) == INDEX_NONE);
		if (bOptionOn)
		{
			CookOptions << (CookOptions.Len() > 0 ? TEXTVIEW(" ") : TEXTVIEW(""));
			CookOptions << Option;
		}
	};

	// content only projects won't have multiple targets to pick from, and pasing -target=UnrealGame will fail if what C++ thinks
	// is a content only project needs a temporary target.cs file in UBT, 
	// only set the BuildTarget in code-based projects
	if (bPlayUsingLauncherHasCode)
	{
		const FTargetInfo* TargetInfo = GetDefault<UProjectPackagingSettings>()->GetLaunchOnTargetInfo();
		if (TargetInfo != nullptr)
		{
			LauncherProfile->SetBuildTarget(TargetInfo->Name);
		}
	}

	LauncherProfile->SetCookMode(CurrentLauncherCookMode);
	LauncherProfile->SetUnversionedCooking(!bIncrementalCooking); // Unversioned cooking is not allowed with incremental cooking
	LauncherProfile->SetIncrementalCooking(bIncrementalCooking);
	
	SetCookOption(TEXTVIEW("-IgnoreIniSettingsOutOfDate"), bIncrementalCooking && CookerSettings.bIgnoreIniSettingsOutOfDateForIteration);
	SetCookOption(TEXTVIEW("-IgnoreScriptPackagesOutOfDate"), bIncrementalCooking && CookerSettings.bIgnoreScriptPackagesOutOfDateForIteration);
	SetCookOption(TEXTVIEW("-IterateSharedCookedbuild"), bIncrementalCooking && ExperimentalSettings.bSharedCookedBuilds);
	SetCookOption(TEXTVIEW("-NoGameAlwaysCook"), true);
	SetCookOption(TEXTVIEW("-NoGameAlwaysCook"), true);
	
	LauncherProfile->SetDeployedDeviceGroup(DeviceGroup);
	LauncherProfile->SetIncrementalDeploying(bIncrementalCooking);
	LauncherProfile->SetEditorExe(FUnrealEdMisc::Get().GetExecutableForCommandlets());
	// LauncherProfile->SetShouldUpdateDeviceFlash(InRequestParams.LauncherTargetDevice->bUpdateDeviceFlash);
	LauncherProfile->SetCookOptions(*CookOptions);

	if (LauncherProfile->IsBuildingUAT() && !GetDefault<UEditorPerProjectUserSettings>()->bAlwaysBuildUAT && bUATSuccessfullyCompiledOnce)
	{
		// UAT was built on a first launch and there's no need to rebuild it any more
		LauncherProfile->SetBuildUAT(false);
	}

	const FString DummyIOSDeviceName(FString::Printf(TEXT("All_iOS_On_%s"), FPlatformProcess::ComputerName()));
	const FString DummyTVOSDeviceName(FString::Printf(TEXT("All_tvOS_On_%s"), FPlatformProcess::ComputerName()));

	if ((LaunchPlatformName != TEXT("IOS") && LaunchPlatformName != TEXT("TVOS")) ||
		(!PlayUsingLauncherDeviceName.Contains(DummyIOSDeviceName) && !PlayUsingLauncherDeviceName.Contains(DummyTVOSDeviceName)))
	{
		LauncherProfile->SetLaunchMode(ELauncherProfileLaunchModes::DefaultRole);
	}

	// switch(EditorPlaySettings->PackFilesForLaunch)
	// {
	// default:
	// case EPlayOnPakFileMode::NoPak:
	// 	break;
	// case EPlayOnPakFileMode::PakNoCompress:
	// 	LauncherProfile->SetCompressed( false );
	// 	LauncherProfile->SetDeployWithUnrealPak( true );
	// 	break;
	// case EPlayOnPakFileMode::PakCompress:
	// 	LauncherProfile->SetCompressed( true );
	// 	LauncherProfile->SetDeployWithUnrealPak( true );
	// 	break;
	// }

	// TArray<UBlueprint*> ErroredBlueprints;
	// FInternalPlayLevelUtils::ResolveDirtyBlueprints(!EditorPlaySettings->bAutoCompileBlueprintsOnLaunch, ErroredBlueprints, false);

	
	for (auto MapsOfNeedAdd : SettingsDataAsset->Maps)
	{
		LauncherProfile->AddCookedMap(MapsOfNeedAdd.GetLongPackageName());
	}
	LauncherProfile->AddCookedMap(SettingsDataAsset->StartMap.GetLongPackageName());
	LauncherProfile->GetDefaultLaunchRole()->SetInitialMap(SettingsDataAsset->StartMap.GetLongPackageName());


	ILauncherPtr Launcher = LauncherServicesModule.CreateLauncher();
	GEditor->LauncherWorker = Launcher->Launch(TargetDeviceServicesModule.GetDeviceProxyManager(), LauncherProfile);

	
	// create notification item
	FText LaunchingText = LOCTEXT("LauncherTaskInProgressNotificationNoDevice", "Launching...");
	FNotificationInfo Info(LaunchingText);

	Info.Image = FAppStyle::GetBrush(TEXT("MainFrame.CookContent"));
	Info.bFireAndForget = false;
	Info.ExpireDuration = 10.0f;
	Info.Hyperlink = FSimpleDelegate::CreateStatic(HandleHyperlinkNavigate);
	Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
	Info.ButtonDetails.Add(
		FNotificationButtonInfo(
			LOCTEXT("LauncherTaskCancel", "Cancel"),
			LOCTEXT("LauncherTaskCancelToolTip", "Cancels execution of this task."),
			FSimpleDelegate::CreateStatic(HandleCancelButtonClicked, GEditor->LauncherWorker)
		)
	);
	
	// Launch doesn't block PIE/Compile requests as it's an async background process, so we just
	// cancel the request to denote it as having been handled. This has to come after we've used
	// anything we might need from the original request.
	GEditor->CancelRequestPlaySession();
	
	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	if (!NotificationItem.IsValid())
	{
		return;
	}

	// analytics for launch on
	TArray<FAnalyticsEventAttribute> AnalyticsParamArray;
	if (LaunchPlatform != nullptr)
	{
		LaunchPlatform->GetPlatformSpecificProjectAnalytics(AnalyticsParamArray);
	}
	FEditorAnalytics::ReportEvent(TEXT("Editor.LaunchOn.Started"), LaunchPlatformName, bPlayUsingLauncherHasCode, AnalyticsParamArray);


	NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);

	TWeakPtr<SNotificationItem> NotificationItemPtr(NotificationItem);
	if (GEditor->LauncherWorker.IsValid() && GEditor->LauncherWorker->GetStatus() != ELauncherWorkerStatus::Completed)
	{

		GEditor->LauncherWorker->OnOutputReceived().AddStatic(HandleOutputReceived);
		GEditor->LauncherWorker->OnStageStarted().AddUObject(this, &ULevelTestEditorLauncher::HandleStageStarted, NotificationItemPtr);
		GEditor->LauncherWorker->OnStageCompleted().AddUObject(this, &ULevelTestEditorLauncher::HandleStageCompleted, bPlayUsingLauncherHasCode, NotificationItemPtr);
		GEditor->LauncherWorker->OnCompleted().AddUObject(this, &ULevelTestEditorLauncher::HandleLaunchCompleted, bPlayUsingLauncherHasCode, NotificationItemPtr);
		GEditor->LauncherWorker->OnCanceled().AddUObject(this, &ULevelTestEditorLauncher::HandleLaunchCanceled, bPlayUsingLauncherHasCode, NotificationItemPtr);
	}
	else
	{
		GEditor->LauncherWorker.Reset();

		NotificationItem->SetText(LOCTEXT("LauncherTaskFailedNotification", "Failed to launch task!"));
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->ExpireAndFadeout();
		
		// analytics for launch on
		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), 0.0));
		FEditorAnalytics::ReportEvent(TEXT("Editor.LaunchOn.Failed"), LaunchPlatformName, bPlayUsingLauncherHasCode, EAnalyticsErrorCodes::LauncherFailed, ParamArray);
		
	}
}

void ULevelTestEditorLauncher::CancelPlayUsingLauncher()
{
		FText LaunchingText = LOCTEXT("LauncherTaskInProgressNotificationNotAuthorized", "Cannot launch to this device until this computer is authorized from the device");
		FNotificationInfo Info(LaunchingText);
		Info.ExpireDuration = 5.0f;
		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
		if (Notification.IsValid())
		{
			Notification->SetCompletionState(SNotificationItem::CS_Fail);
			Notification->ExpireAndFadeout();
		}
}

/* FMainFrameActionCallbacks callbacks
 *****************************************************************************/

class FLauncherNotificationTask
{
public:

	FLauncherNotificationTask( TWeakPtr<SNotificationItem> InNotificationItemPtr, SNotificationItem::ECompletionState InCompletionState, const FText& InText )
		: CompletionState(InCompletionState)
		, NotificationItemPtr(InNotificationItemPtr)
		, Text(InText)
	{ }

	void DoTask( ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent )
	{
		if (NotificationItemPtr.IsValid())
		{
			const ULevelEditorPlaySettings* EditorPlaySettings = GetDefault<ULevelEditorPlaySettings>();
			if (EditorPlaySettings->EnablePIEEnterAndExitSounds)
			{
				if (CompletionState == SNotificationItem::CS_Fail)
				{
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
				}
				else if (CompletionState == SNotificationItem::CS_Success)
				{
					GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
				}
			}

			TSharedPtr<SNotificationItem> NotificationItem = NotificationItemPtr.Pin();
			NotificationItem->SetText(Text);
			NotificationItem->SetCompletionState(CompletionState);
			if (CompletionState == SNotificationItem::CS_Success || CompletionState == SNotificationItem::CS_Fail)
			{
				NotificationItem->ExpireAndFadeout();
			}
		}
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
	ENamedThreads::Type GetDesiredThread( ) { return ENamedThreads::GameThread; }
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FLauncherNotificationTask, STATGROUP_TaskGraphTasks);
	}

private:

	SNotificationItem::ECompletionState CompletionState;
	TWeakPtr<SNotificationItem> NotificationItemPtr;
	FText Text;
};



void ULevelTestEditorLauncher::HandleStageStarted(const FString& InStage, TWeakPtr<SNotificationItem> NotificationItemPtr)
{
	bool bSetNotification = true;
	FFormatNamedArguments Arguments;
	FText NotificationText;
	if (InStage.Contains(TEXT("Cooking")) || InStage.Contains(TEXT("Cook Task")))
	{
		FString PlatformName = LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
		PlatformName = PlatformInfo::FindPlatformInfo(*PlatformName)->VanillaInfo->Name.ToString();
		Arguments.Add(TEXT("PlatformName"), FText::FromString(PlatformName));
		NotificationText = FText::Format(LOCTEXT("LauncherTaskProcessingNotification", "Processing Assets for {PlatformName}..."), Arguments);
	}
	else if (InStage.Contains(TEXT("Build Task")))
	{
		FString PlatformName = LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
		PlatformName = PlatformInfo::FindPlatformInfo(*PlatformName)->VanillaInfo->Name.ToString();
		Arguments.Add(TEXT("PlatformName"), FText::FromString(PlatformName));

		NotificationText = FText::Format(LOCTEXT("LauncherTaskBuildNotification", "Building Executable for {PlatformName}..."), Arguments);
		
	}
	else if (InStage.Contains(TEXT("Deploy Task")))
	{
		FString PlayUsingLauncherDeviceName = LastPlayUsingLauncherDeviceId.Right(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
		Arguments.Add(TEXT("DeviceName"), FText::FromString(PlayUsingLauncherDeviceName));
		NotificationText = FText::Format(LOCTEXT("LauncherTaskStageNotification", "Deploying Executable and Assets to {DeviceName}..."), Arguments);
		
	}
	else if (InStage.Contains(TEXT("Run Task")))
	{
		FString PlayUsingLauncherDeviceName = LastPlayUsingLauncherDeviceId.Right(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
		Arguments.Add(TEXT("GameName"), FText::FromString(FApp::GetProjectName()));
		Arguments.Add(TEXT("DeviceName"), FText::FromString(PlayUsingLauncherDeviceName));
		NotificationText = FText::Format(LOCTEXT("LauncherTaskRunNotification", "Running {GameName} on {DeviceName}..."), Arguments);
		
	}
	else
	{
		bSetNotification = false;
	}

	if (bSetNotification)
	{
		TGraphTask<FLauncherNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Pending,
			NotificationText
		);
	}
}

void ULevelTestEditorLauncher::HandleStageCompleted(const FString& InStage, double StageTime, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr)
{
	UE_LOG(LevelTestEditorLauncher, Log, TEXT("Completed Launch On Stage: %s, Time: %f"), *InStage, StageTime);

	// analytics for launch on
	TArray<FAnalyticsEventAttribute> ParamArray;
	ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), StageTime));
	ParamArray.Add(FAnalyticsEventAttribute(TEXT("StageName"), InStage));
	FEditorAnalytics::ReportEvent(TEXT( "Editor.LaunchOn.StageComplete" ), LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))), bHasCode, ParamArray);
}

void ULevelTestEditorLauncher::HandleLaunchCanceled(double TotalTime, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr)
{
	TGraphTask<FLauncherNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
		NotificationItemPtr,
		SNotificationItem::CS_Fail,
		LOCTEXT("LaunchtaskFailedNotification", "Launch canceled!")
	);

	// analytics for launch on
	TArray<FAnalyticsEventAttribute> ParamArray;
	ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TotalTime));
	FEditorAnalytics::ReportEvent(TEXT( "Editor.LaunchOn.Canceled" ), LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))), bHasCode, ParamArray);

}

void ULevelTestEditorLauncher::HandleLaunchCompleted(bool Succeeded, double TotalTime, int32 ErrorCode, bool bHasCode, TWeakPtr<SNotificationItem> NotificationItemPtr)
{
	FString PlayUsingLauncherDeviceName = LastPlayUsingLauncherDeviceId.Right(LastPlayUsingLauncherDeviceId.Find(TEXT("@")));
	
	const FString DummyIOSDeviceName(FString::Printf(TEXT("All_iOS_On_%s"), FPlatformProcess::ComputerName()));
	const FString DummyTVOSDeviceName(FString::Printf(TEXT("All_tvOS_On_%s"), FPlatformProcess::ComputerName()));
	if (Succeeded)
	{
		FText CompletionMsg;
		if ((LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))) == TEXT("IOS") && PlayUsingLauncherDeviceName.Contains(DummyIOSDeviceName)) ||
			(LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))) == TEXT("TVOS") && PlayUsingLauncherDeviceName.Contains(DummyTVOSDeviceName)))
		{
			CompletionMsg = LOCTEXT("DeploymentTaskCompleted", "Deployment complete! Open the app on your device to launch.");
		}
		else
		{
			CompletionMsg = LOCTEXT("LauncherTaskCompleted", "Launch complete!!");
		}

		TGraphTask<FLauncherNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Success,
			CompletionMsg
		);

		// analytics for launch on
		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TotalTime));
		FEditorAnalytics::ReportEvent(TEXT( "Editor.LaunchOn.Completed" ), LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))), bHasCode, ParamArray);

		UE_LOG(LevelTestEditorLauncher, Log, TEXT("Launch On Completed. Time: %f"), TotalTime);

		bUATSuccessfullyCompiledOnce = true;
	}
	else
	{
		FText CompletionMsg;
		if ((LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))) == TEXT("IOS") && PlayUsingLauncherDeviceName.Contains(DummyIOSDeviceName)) ||
			(LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))) == TEXT("TVOS") && PlayUsingLauncherDeviceName.Contains(DummyTVOSDeviceName)))
		{
			CompletionMsg = LOCTEXT("DeploymentTaskFailed", "Deployment failed!");
		}
		else
		{
			CompletionMsg = LOCTEXT("LauncherTaskFailed", "Launch failed!");
		}
		
		AsyncTask(ENamedThreads::GameThread, [=]
		{
			FMessageLog MessageLog("PackagingResults");

			MessageLog.Error()
				->AddToken(FTextToken::Create(CompletionMsg))
				->AddToken(FTextToken::Create(FText::FromString(FEditorAnalytics::TranslateErrorCode(ErrorCode))));

			// flush log, because it won't be destroyed until the notification popup closes
			MessageLog.NumMessages(EMessageSeverity::Info);
		});

		TGraphTask<FLauncherNotificationTask>::CreateTask().ConstructAndDispatchWhenReady(
			NotificationItemPtr,
			SNotificationItem::CS_Fail,
			CompletionMsg
		);

		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), TotalTime));
		FEditorAnalytics::ReportEvent(TEXT( "Editor.LaunchOn.Failed" ), LastPlayUsingLauncherDeviceId.Left(LastPlayUsingLauncherDeviceId.Find(TEXT("@"))), bHasCode, ErrorCode, ParamArray);
	}

}
