// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshSpawner.h"

#include "ProceduralStaticMeshInstance.h"

UProceduralStaticMeshSpawner::UProceduralStaticMeshSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}
void UProceduralStaticMeshSpawner::Simulate(const FBoxSphereBounds& Bounds)
{
	Empty();
	
	RandomStream.Initialize(RandomSeed);

	TArray<FProceduralStaticMeshInstance*> NewInstances;
	
	AddRandomSeeds(NewInstances, Bounds);

	InstancesSet.Append(NewInstances);
	
}

void UProceduralStaticMeshSpawner::AddRandomSeeds(TArray<FProceduralStaticMeshInstance*>& OutInstances, const FBoxSphereBounds& Bounds)
{
	const float SizeTenM2 = ( Bounds.BoxExtent.X * Bounds.BoxExtent.Y) / ( 100.f * 100.f );

	TMap<const FProceduralStaticMeshType*, int32> SeedsLeftMap;
	TMap<const FProceduralStaticMeshType*, FRandomStream> RandomStreamPerType;

	TArray<const FProceduralStaticMeshType*> TypesToSeed;

	for (int i = 0; i < Types.Num(); ++i)
	{
		const FProceduralStaticMeshType* TypeInstance = &Types[i];
		
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

			float InitX = TypeRandomStream.FRandRange(Bounds.Origin.X - Bounds.BoxExtent.X, Bounds.Origin.X + Bounds.BoxExtent.X);
			float InitY = TypeRandomStream.FRandRange(Bounds.Origin.Y - Bounds.BoxExtent.Y, Bounds.Origin.Y + Bounds.BoxExtent.Y);
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

FProceduralStaticMeshInstance* UProceduralStaticMeshSpawner::NewSeed(const FVector& Location, float Scale, const FProceduralStaticMeshType* Type)
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

void UProceduralStaticMeshSpawner::Empty()
{
	for (FProceduralStaticMeshInstance* Inst : InstancesSet)
	{
		delete Inst;
	}

	InstancesSet.Empty();
}

int32 UProceduralStaticMeshSpawner::GetRandomNumber()
{
	return RandomStream.FRand() * float(RAND_MAX);
}