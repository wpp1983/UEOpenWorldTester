// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "WorldPartition/WorldPartitionActorLoaderInterface.h"
#include "ProceduralStaticMeshVolume.generated.h"

class UProceduralStaticMeshComponent;
class FLoaderAdapterActor;

UCLASS()
class OPENWORLDTESTER_API AProceduralStaticMeshVolume final : public AVolume, public IWorldPartitionActorLoaderInterface
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Category = ProceduralStaticMesh, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProceduralStaticMeshComponent> ProceduralComponent;

#if WITH_EDITOR
	//~ Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	//~ End AActor Interface

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	virtual void PostEditImport() override;
	//~ End UObject Interface

	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;

	//~ Begin IWorldPartitionActorLoaderInterface interface
	virtual ILoaderAdapter* GetLoaderAdapter() override;
	//~ End IWorldPartitionActorLoaderInterface interface

	private:
	FLoaderAdapterActor* WorldPartitionActorLoader;
#endif
};
