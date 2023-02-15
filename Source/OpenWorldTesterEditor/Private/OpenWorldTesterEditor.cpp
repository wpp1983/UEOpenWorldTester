#include "OpenWorldTesterEditor.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"

#include "ActorFactoryProceduralStaticMesh.h"
#include "FAssetTypeActions_ProceduralStaticMeshSpawner.h"
#include "FProceduralStaticMeshComponentDetails.h"

void FOpenWorldTesterEditorModule::StartupModule()
{

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.RegisterCustomClassLayout("ProceduralStaticMeshComponent", FOnGetDetailCustomizationInstance::CreateStatic(&FProceduralStaticMeshComponentDetails::MakeInstance));

	
	// Actor Factories
	auto ProceduralStaticMeshVolumeFactory = NewObject<UActorFactoryProceduralStaticMesh>();
	GEditor->ActorFactories.Add(ProceduralStaticMeshVolumeFactory);
	

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeActions_ProceduralStaticMeshSpawner>());
	
}

void FOpenWorldTesterEditorModule::ShutdownModule()
{
}

    
IMPLEMENT_MODULE(FOpenWorldTesterEditorModule, OpenWorldTesterEditor)