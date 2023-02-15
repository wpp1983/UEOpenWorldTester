// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorFactoryProceduralStaticMesh.h"

#include "ProceduralStaticMeshComponent.h"
#include "ProceduralStaticMeshSpawner.h"
#include "ProceduralStaticMeshVolume.h"


#define LOCTEXT_NAMESPACE "ActorFactoryProceduralStaticMesh"

/*-----------------------------------------------------------------------------
UActorFactoryProceduralFoliage
-----------------------------------------------------------------------------*/
UActorFactoryProceduralStaticMesh::UActorFactoryProceduralStaticMesh(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("ProceduralStaticMeshDisplayName", "Procedural Foliage Volume");
	NewActorClass = AProceduralStaticMeshVolume::StaticClass();
	bUseSurfaceOrientation = true;
}

bool UActorFactoryProceduralStaticMesh::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	return true;
}

bool UActorFactoryProceduralStaticMesh::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.IsInstanceOf(UProceduralStaticMeshSpawner::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoUProceduralStaticMeshSpawner", "A valid UProceduralStaticMeshSpawner must be specified.");
		return false;
	}

	return true;
}

void UActorFactoryProceduralStaticMesh::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
	UProceduralStaticMeshSpawner* StaticMeshSpawner = CastChecked<UProceduralStaticMeshSpawner>(Asset);

	UE_LOG(LogActorFactory, Log, TEXT("Actor Factory created %s"), *StaticMeshSpawner->GetName());

	// Change properties
	AProceduralStaticMeshVolume* PFV = CastChecked<AProceduralStaticMeshVolume>(NewActor);
	UProceduralStaticMeshComponent* ProceduralComponent = PFV->ProceduralComponent;
	check(ProceduralComponent);

	ProceduralComponent->UnregisterComponent();

	ProceduralComponent->StaticMeshSpawner = StaticMeshSpawner;

	// Init Component
	ProceduralComponent->RegisterComponent();
}

UObject* UActorFactoryProceduralStaticMesh::GetAssetFromActorInstance(AActor* Instance)
{
	check(Instance->IsA(NewActorClass));

	AProceduralStaticMeshVolume* PFV = CastChecked<AProceduralStaticMeshVolume>(Instance);
	UProceduralStaticMeshComponent* ProceduralComponent = PFV->ProceduralComponent;
	check(ProceduralComponent);
	
	return ProceduralComponent->StaticMeshSpawner;
}

void UActorFactoryProceduralStaticMesh::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != nullptr && CDO != nullptr)
	{
		UProceduralStaticMeshSpawner* StaticMeshSpawner = CastChecked<UProceduralStaticMeshSpawner>(Asset);
		AProceduralStaticMeshVolume* PFV = CastChecked<AProceduralStaticMeshVolume>(CDO);
		UProceduralStaticMeshComponent* ProceduralComponent = PFV->ProceduralComponent;
		ProceduralComponent->StaticMeshSpawner = StaticMeshSpawner;
	}
}
#undef LOCTEXT_NAMESPACE
