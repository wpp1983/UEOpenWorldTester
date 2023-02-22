// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshSpawner.h"

#include "Async/Async.h"
#include "ProceduralStaticMeshInstance.h"
#include "ProceduralStaticMeshTile.h"
#include "Misc/FeedbackContext.h"

#define LOCTEXT_NAMESPACE "ProceduralStaticMesh"

UProceduralStaticMeshSpawner::UProceduralStaticMeshSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TileSize = 10000;	//100 m
	NumUniqueTiles = 10;
	RandomSeed = 42;
}
void UProceduralStaticMeshSpawner::Simulate()
{
	RandomStream.Initialize(RandomSeed);

	PrecomputedTiles.Empty();
	TArray<TFuture< UProceduralStaticMeshTile* >> Futures;

	for (int i = 0; i < NumUniqueTiles; ++i)
	{
		UProceduralStaticMeshTile* NewTile = NewObject<UProceduralStaticMeshTile>(this);
		const int32 RandomNumber = GetRandomNumber();

		Futures.Add(Async(EAsyncExecution::ThreadPool, [=]()
		{
			NewTile->Simulate(this, RandomNumber);
			return NewTile;
		}));
	}

	const FText StatusMessage = LOCTEXT("SimulateProceduralStaticMesh", "Simulate ProceduralStaticMesh...");
	GWarn->BeginSlowTask(StatusMessage, true, true);
	
	const int32 TotalTasks = Futures.Num();

	for (int32 FutureIdx = 0; FutureIdx < Futures.Num(); ++FutureIdx)
	{
		// Sleep for 100ms if not ready. Needed so cancel is responsive.
		while (Futures[FutureIdx].WaitFor(FTimespan::FromMilliseconds(100.0)) == false)
		{
			GWarn->StatusUpdate(FutureIdx, TotalTasks, LOCTEXT("SimulateProceduralStaticMesh", "Simulate ProceduraStaticMesh..."));
		}

		// Even if canceled, block until all threads have exited safely. This ensures memory isn't GC'd.
		PrecomputedTiles.Add(Futures[FutureIdx].Get());		
	}

	GWarn->EndSlowTask();
}


const UProceduralStaticMeshTile* UProceduralStaticMeshSpawner::GetRandomTile(int32 X, int32 Y)
{
	if (PrecomputedTiles.Num())
	{
		// Random stream to use as a hash function
		FRandomStream HashStream;	
		
		HashStream.Initialize(X);
		const double XRand = HashStream.FRand();
		
		HashStream.Initialize(Y);
		const double YRand = HashStream.FRand();
		
		const int32 RandomNumber = (RAND_MAX * XRand / (YRand + 0.01));
		const int32 Idx = FMath::Clamp(RandomNumber % PrecomputedTiles.Num(), 0, PrecomputedTiles.Num() - 1);
		return PrecomputedTiles[Idx].Get();
	}

	return nullptr;
}


int32 UProceduralStaticMeshSpawner::GetRandomNumber() const
{
	return RandomStream.FRand() * static_cast<float>(RAND_MAX);
}

#undef LOCTEXT_NAMESPACE