// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProceduralStaticMeshInstance.h"
#include "ProceduralStaticMeshTile.generated.h"

class UProceduralStaticMeshSpawner;

/**
 * 
 */
UCLASS()
class OPENWORLDTESTER_API UProceduralStaticMeshTile : public UObject
{
	GENERATED_BODY()

public:
	void Simulate(const UProceduralStaticMeshSpawner* InStaticMeshSpawner, const int32 InRandomSeed);

	void ExtractDesiredInstances(TArray<FDesiredProceduralStaticMeshInstance>& OutDesiredInstances, const FTransform& WorldTM,
		const float HalfHeight, const FBodyInstance* VolumeBodyInstance) const;
	
	// UObject interface
	virtual void BeginDestroy() override;

private:
	void StepSimulate();
	void InitSimulate(const UProceduralStaticMeshSpawner* InStaticMeshSpawner, int32 InRandomSeed);
	void AddRandomSeeds(TArray<FProceduralStaticMeshInstance*>& OutInstances) const;
	FProceduralStaticMeshInstance* NewSeed(const FVector& Location, float Scale, const FProceduralStaticMeshType* Type) const;
	void RemoveInstances();
	
private:
	UPROPERTY()
	TObjectPtr<const UProceduralStaticMeshSpawner> StaticMeshSpawner;

	TArray<FProceduralStaticMeshInstance*> InstancesArray;

	int32 RandomSeed;
	FRandomStream RandomStream;

};
