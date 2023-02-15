// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshVolume.h"
#include "ProceduralStaticMeshComponent.h"
#include "Components/BrushComponent.h"
#include "WorldPartition/LoaderAdapter/LoaderAdapterActor.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ProceduralStaticMeshVolume)

AProceduralStaticMeshVolume::AProceduralStaticMeshVolume(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	ProceduralComponent = ObjectInitializer.CreateDefaultSubobject<UProceduralStaticMeshComponent>(this, TEXT("ProceduralStaticMeshHelperComponent"));
	ProceduralComponent->SetSpawningVolume(this);

	if (UBrushComponent* MyBrushComponent = GetBrushComponent())
	{
		MyBrushComponent->SetCollisionObjectType(ECC_WorldStatic);
		MyBrushComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

		// This is important because the volume overlaps with all procedural foliage
		// That means during streaming we'll get a huge hitch for UpdateOverlaps
		MyBrushComponent->SetGenerateOverlapEvents(false);
	}
}

#if WITH_EDITOR
void AProceduralStaticMeshVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (!GetWorld()->IsGameWorld() && GetWorld()->IsPartitionedWorld() && !WorldPartitionActorLoader)
	{
		WorldPartitionActorLoader = new FLoaderAdapterActor(this);
	}
}

void AProceduralStaticMeshVolume::BeginDestroy()
{
	if (WorldPartitionActorLoader)
	{
		delete WorldPartitionActorLoader;
		WorldPartitionActorLoader = nullptr;
	}

	Super::BeginDestroy();
}

IWorldPartitionActorLoaderInterface::ILoaderAdapter* AProceduralStaticMeshVolume::GetLoaderAdapter()
{
	return WorldPartitionActorLoader;
}

void AProceduralStaticMeshVolume::PostEditImport()
{
	// Make sure that this is the component's spawning volume
	ProceduralComponent->SetSpawningVolume(this);
}

bool AProceduralStaticMeshVolume::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);

	if (ProceduralComponent && ProceduralComponent->StaticMeshSpawner)
	{
		Objects.Add(ProceduralComponent->StaticMeshSpawner);
	}
	return true;
}
#endif

