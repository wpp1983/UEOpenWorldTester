// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralStaticMeshInstance.h"
#include "ProceduralStaticMeshComponent.generated.h"

class UProceduralStaticMeshSpawner;
struct FProceduralStaticMeshType;
class AStaticMeshActor;

struct FProceduralStaticMeshTileLayout
{
	FProceduralStaticMeshTileLayout()
		: BottomLeftX(0), BottomLeftY(0), NumTilesX(0), NumTilesY(0), HalfHeight(0.f)
	{
	}

	// The X coordinate (in whole tiles) of the bottom-left-most active tile
	int32 BottomLeftX;

	// The Y coordinate (in whole tiles) of the bottom-left-most active tile
	int32 BottomLeftY;

	// The total number of active tiles along the x-axis
	int32 NumTilesX;

	// The total number of active tiles along the y-axis
	int32 NumTilesY;
	

	float HalfHeight;
};

/** Used to define a vector along which we'd like to spawn an instance. */
struct FDesiredProceduralStaticMeshInstance
{
	FDesiredProceduralStaticMeshInstance()
		: ProceduralStaticMeshType(nullptr)
		, StartTrace(ForceInit)
		, EndTrace(ForceInit)
		, Rotation(ForceInit)
	{

	}

	FDesiredProceduralStaticMeshInstance(const FVector& InStartTrace, const FVector& InEndTrace, const FProceduralStaticMeshType* InProceduralStaticMeshType)
		: ProceduralStaticMeshType(InProceduralStaticMeshType)
		, StartTrace(InStartTrace)
		, EndTrace(InEndTrace)
		, Rotation(ForceInit)
	{
	}

	const FProceduralStaticMeshType* ProceduralStaticMeshType;
	FGuid ProceduralGuid;
	FVector StartTrace;
	FVector EndTrace;
	FQuat Rotation;
	FLinearColor MaterialInstanceColor;
	const struct FBodyInstance* ProceduralVolumeBodyInstance;
};


UCLASS(BlueprintType)
class OPENWORLDTESTER_API UProceduralStaticMeshComponent final : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	/** The procedural StaticMesh spawner used to generate StaticMesh instances within this volume. */
	UPROPERTY(Category = "ProceduralStaticMesh", BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UProceduralStaticMeshSpawner> StaticMeshSpawner;
	

	// Sets default values for this component's properties
	UProceduralStaticMeshComponent();

	void Resimulate();
	void Empty();

private:
	bool GenerateProceduralContent(TArray<FDesiredProceduralStaticMeshInstance>& OutInstances) const;
	FVector GetWorldPosition() const;
	void GetTileLayout(FProceduralStaticMeshTileLayout& OutTileLayout) const;
	FBox GetBounds() const;
	void SpawnMeshes(const TArray<FDesiredProceduralStaticMeshInstance>& Instances);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;


	void SetSpawningVolume(AVolume* InSpawningVolume) { SpawningVolume = InSpawningVolume; }

private:
	UPROPERTY()
	TObjectPtr<AVolume> SpawningVolume;

	UPROPERTY()
	TArray<TSoftObjectPtr<AStaticMeshActor>> SpawnedActors;
};
