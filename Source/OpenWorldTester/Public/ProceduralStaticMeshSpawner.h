// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProceduralStaticMeshSpawner.generated.h"

struct FProceduralStaticMeshInstance;
class UProceduralStaticMeshTile;

USTRUCT(BlueprintType)
struct FProceduralStaticMeshType
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProceduralStaticMeshType", meta=(AllowedClasses = "/Script/Engine.StaticMesh"))
	FSoftObjectPath StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProceduralStaticMeshType", meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float Density = 1;

	/** The seed that determines placement of initial seeds. */
	UPROPERTY( EditAnywhere, Category="ProceduralStaticMeshType")
	float MaxInitialSeedOffset = 0;

	/** If selected, foliage instances will have a random yaw rotation around their vertical axis applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProceduralStaticMeshType")
	uint32 RandomYaw:1;

	/** A random pitch adjustment can be applied to each instance, up to the specified angle in degrees, from the original vertical */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ProceduralStaticMeshType", meta=(UIMin = 0, ClampMin = 0, UIMax = 359, ClampMax = 359, ReapplyCondition="ReapplyRandomPitchAngle"))
	float RandomPitchAngle;
	
};
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class OPENWORLDTESTER_API UProceduralStaticMeshSpawner : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(Category = ProceduralStaticMeshSimulation, EditAnywhere, BlueprintReadWrite)
	TArray<FProceduralStaticMeshType> Types;

	/** The seed used for generating the randomness of the simulation. */
	UPROPERTY(Category = ProceduralStaticMeshSimulation, EditAnywhere, BlueprintReadOnly)
	int32 RandomSeed;

	/** Length of the tile (in cm) along one axis. The total area of the tile will be TileSize*TileSize. */
	UPROPERTY(Category = ProceduralStaticMeshSimulation, EditAnywhere, BlueprintReadOnly)
	float TileSize;

	/** The number of unique tiles to generate. The final simulation is a procedurally determined combination of the various unique tiles. */
	UPROPERTY(Category = ProceduralStaticMeshSimulation, EditAnywhere, BlueprintReadOnly)
	int32 NumUniqueTiles;

	void Simulate();
	
	int32 GetRandomNumber() const;
	const UProceduralStaticMeshTile* GetRandomTile(int32 X, int32 Y);

private:

	TArray<TWeakObjectPtr<UProceduralStaticMeshTile>> PrecomputedTiles;
	
	FRandomStream RandomStream;
};
