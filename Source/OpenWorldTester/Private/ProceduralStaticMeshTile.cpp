// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshTile.h"

#include "ProceduralStaticMeshComponent.h"
#include "ProceduralStaticMeshSpawner.h"


void UProceduralStaticMeshTile::Simulate(const UProceduralStaticMeshSpawner* InStaticMeshSpawner, const int32 InRandomSeed)
{
	InitSimulate(InStaticMeshSpawner, InRandomSeed);
	RemoveInstances();	
	StepSimulate();
}

void UProceduralStaticMeshTile::InitSimulate(const UProceduralStaticMeshSpawner* InStaticMeshSpawner, const int32 InRandomSeed)
{
	RandomSeed = InRandomSeed;
	RandomStream.Initialize(RandomSeed);
	StaticMeshSpawner = InStaticMeshSpawner;
}

void UProceduralStaticMeshTile::StepSimulate()
{
	TArray<FProceduralStaticMeshInstance*> NewInstances;
	AddRandomSeeds(NewInstances);
	InstancesArray.Append(NewInstances);
}


void UProceduralStaticMeshTile::ExtractDesiredInstances(TArray<FDesiredProceduralStaticMeshInstance>& OutDesiredInstances,
	const FTransform& WorldTM, const float HalfHeight, const FBodyInstance* VolumeBodyInstance) const
{
	OutDesiredInstances.Reserve(InstancesArray.Num());
	for (const FProceduralStaticMeshInstance* Instance : InstancesArray)
	{
		FVector StartRay = Instance->Location + WorldTM.GetLocation();
		StartRay.Z += HalfHeight;
		FVector EndRay = StartRay;
		EndRay.Z -= (HalfHeight*2.f + 10.f);	//add 10cm to bottom position of raycast. This is needed because volume is usually placed directly on geometry and then you get precision issues

		FDesiredProceduralStaticMeshInstance* DesiredInst = new (OutDesiredInstances)FDesiredProceduralStaticMeshInstance(StartRay, EndRay, Instance->Type);
		DesiredInst->Rotation = Instance->Rotation;
		DesiredInst->ProceduralVolumeBodyInstance = VolumeBodyInstance;
	}
}

void UProceduralStaticMeshTile::BeginDestroy()
{
	UObject::BeginDestroy();
	RemoveInstances();
}
void UProceduralStaticMeshTile::RemoveInstances()
{
	for (FProceduralStaticMeshInstance* Inst : InstancesArray)
	{
		delete Inst;
	}
	InstancesArray.Empty();
}
void UProceduralStaticMeshTile::AddRandomSeeds(TArray<FProceduralStaticMeshInstance*>& OutInstances) const
{
	const float SizeTenM2 = ( StaticMeshSpawner->TileSize * StaticMeshSpawner->TileSize ) / ( 1000.f * 1000.f );

	TMap<const FProceduralStaticMeshType*, int32> SeedsLeftMap;
	TMap<const FProceduralStaticMeshType*, FRandomStream> RandomStreamPerType;

	TArray<const FProceduralStaticMeshType*> TypesToSeed;

	for (int i = 0; i < StaticMeshSpawner->Types.Num(); ++i)
	{
		const FProceduralStaticMeshType* TypeInstance = &(StaticMeshSpawner->Types[i]);
		
		{	//compute the number of initial seeds
			const int32 NumSeeds = FMath::RoundToInt((TypeInstance->Density * TypeInstance->Density) * SizeTenM2);
			SeedsLeftMap.Add(TypeInstance, NumSeeds);
			if (NumSeeds > 0)
			{
				TypesToSeed.Add(TypeInstance);
			}
		}

		{	//save the random stream per type
			RandomStreamPerType.Add(TypeInstance, FRandomStream(RandomSeed));
		}
	}

	int32 TypeIdx = -1;
	const int32 NumTypes = TypesToSeed.Num();
	int32 TypesLeftToSeed = NumTypes;
	while (TypesLeftToSeed > 0)
	{
		TypeIdx = (TypeIdx + 1) % NumTypes;	//keep cycling through the types that we spawn initial seeds for to make sure everyone gets fair chance

		if (const FProceduralStaticMeshType* Type = TypesToSeed[TypeIdx])
		{
			int32& SeedsLeft = SeedsLeftMap.FindChecked(Type);
			if (SeedsLeft == 0)
			{
				continue;
			}

			const float Scale = 1;

			FRandomStream& TypeRandomStream = RandomStreamPerType.FindChecked(Type);

			float InitX = TypeRandomStream.FRandRange(0, StaticMeshSpawner->TileSize);
			float InitY = TypeRandomStream.FRandRange(0, StaticMeshSpawner->TileSize);
			float NeededRadius = 0;

			const float Rad = RandomStream.FRandRange(0, PI*2.f);
			
			
			const FVector GlobalOffset = (RandomStream.FRandRange(0, Type->MaxInitialSeedOffset) + NeededRadius) * FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0.f);

			const float X = InitX + GlobalOffset.X;
			const float Y = InitY + GlobalOffset.Y;

			if (FProceduralStaticMeshInstance* NewInst = NewSeed(FVector(X, Y, 0.f), Scale, Type))
			{
				OutInstances.Add(NewInst);
			}

			--SeedsLeft;
			if (SeedsLeft == 0)
			{
				--TypesLeftToSeed;
			}
		}
	}
}

FProceduralStaticMeshInstance* UProceduralStaticMeshTile::NewSeed(const FVector& Location, float Scale, const FProceduralStaticMeshType* Type) const
{
	FProceduralStaticMeshInstance* NewInst = new FProceduralStaticMeshInstance();
	NewInst->Location = Location;

	// make a new local random stream to avoid changes to instance randomness changing the position of all other procedural instances
	FRandomStream LocalStream = RandomStream;
	RandomStream.GetUnsignedInt(); // advance the parent stream by one

	FRotator Rotation = {0,0,0};
	Rotation.Yaw   = LocalStream.FRandRange(0, Type->RandomYaw ? 360 : 0);
	Rotation.Pitch = LocalStream.FRandRange(0, Type->RandomPitchAngle);
	NewInst->Rotation = FQuat(Rotation);
	NewInst->Type = Type;
	NewInst->Normal = FVector(0, 0, 1);
	NewInst->Scale = Scale;

	return NewInst;
}
