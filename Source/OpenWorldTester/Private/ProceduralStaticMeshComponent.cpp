// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshComponent.h"

#include "ProceduralFoliageComponent.h"
#include "ProceduralStaticMeshInstance.h"
#include "ProceduralStaticMeshSpawner.h"
#include "ProceduralStaticMeshTile.h"
#include "Async/Async.h"
#include "Components/BrushComponent.h"
#include "Engine/LevelBounds.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Volume.h"
#include "Misc/FeedbackContext.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "ProceduralStaticMesh"

// Sets default values for this component's properties
UProceduralStaticMeshComponent::UProceduralStaticMeshComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UProceduralStaticMeshComponent::Resimulate()
{
#if WITH_EDITOR

	TArray<FDesiredProceduralStaticMeshInstance> DesiredFoliageInstances;
	if (GenerateProceduralContent(DesiredFoliageInstances))
	{
		if (DesiredFoliageInstances.Num() > 0)
		{
			// clear old StaticMesh Actors
			Empty();

			// Spawn new StaticMesh Actors
			SpawnMeshes(DesiredFoliageInstances);
		}
	}

#endif
}

bool UProceduralStaticMeshComponent::GenerateProceduralContent(TArray<FDesiredProceduralStaticMeshInstance>& OutInstances) const
{
#if WITH_EDITOR
	// Establish basic info about the tiles
	const float TileSize = StaticMeshSpawner->TileSize;
	const FVector WorldPosition = GetWorldPosition();
	FProceduralStaticMeshTileLayout TileLayout;
	GetTileLayout(TileLayout);
	
	StaticMeshSpawner->Simulate();

	TArray<TFuture<TArray<FDesiredProceduralStaticMeshInstance>*>> Futures;
	for (int32 X = 0; X < TileLayout.NumTilesX; ++X)
	{
		for (int32 Y = 0; Y < TileLayout.NumTilesY; ++Y)
		{
			// We have to get the simulated tiles and create new ones to build on main thread
			const UProceduralStaticMeshTile* Tile = StaticMeshSpawner->GetRandomTile(
				X + TileLayout.BottomLeftX, Y + TileLayout.BottomLeftY);
			if (Tile == nullptr)
			{
				// Simulation was either canceled or failed
				return false;
			}

			Futures.Add(Async(EAsyncExecution::ThreadPool, [=]()
				{
					const FVector OrientedOffset = FVector(X, Y, 0.f) * TileSize;
					const FTransform TileTM(OrientedOffset + WorldPosition);

					TArray<FDesiredProceduralStaticMeshInstance>* DesiredInstances = new TArray<FDesiredProceduralStaticMeshInstance>();
					Tile->ExtractDesiredInstances(*DesiredInstances, TileTM, TileLayout.HalfHeight, nullptr);

					return DesiredInstances;
				})
			);
		}
	}

	const FText StatusMessage = LOCTEXT("PlaceProcedural", "Placing Procedural...");
	GWarn->BeginSlowTask(StatusMessage, true, false);


	int32 FutureIdx = 0;
	uint32 OutInstanceGrowth = 0;
	for (int X = 0; X < TileLayout.NumTilesX; ++X)
	{
		for (int Y = 0; Y < TileLayout.NumTilesY; ++Y)
		{
			bool bFirstTime = true;
			while (Futures[FutureIdx].WaitFor(FTimespan::FromMilliseconds(100.0)) == false || bFirstTime)
			{
				{
					GWarn->StatusUpdate(Y + X * TileLayout.NumTilesY, TileLayout.NumTilesX * TileLayout.NumTilesY, StatusMessage);
				}

				bFirstTime = false;
			}

			TArray<FDesiredProceduralStaticMeshInstance>* DesiredInstances = Futures[FutureIdx++].Get();
			OutInstanceGrowth += DesiredInstances->Num();
		}
	}

	OutInstances.Reserve(OutInstances.Num() + OutInstanceGrowth);
	FutureIdx = 0;
	for (int X = 0; X < TileLayout.NumTilesX; ++X)
	{
		for (int Y = 0; Y < TileLayout.NumTilesY; ++Y)
		{
			TArray<FDesiredProceduralStaticMeshInstance>* DesiredInstances = Futures[FutureIdx++].Get();
			OutInstances.Append(MoveTemp(*DesiredInstances));
			delete DesiredInstances;
		}
	}

	GWarn->EndSlowTask();

#endif
	
	return true;
}

FVector UProceduralStaticMeshComponent::GetWorldPosition() const
{
	FProceduralStaticMeshTileLayout TileLayout;
	GetTileLayout(TileLayout);

	const float TileSize = StaticMeshSpawner->TileSize;
	return FVector(TileLayout.BottomLeftX * TileSize, TileLayout.BottomLeftY * TileSize, GetBounds().GetCenter().Z);
}

void UProceduralStaticMeshComponent::GetTileLayout(FProceduralStaticMeshTileLayout& OutTileLayout) const
{
	if (const FBox Bounds = GetBounds(); Bounds.IsValid)
	{
		// Determine the bottom-left-most tile that contains the min position (when accounting for overlap)
		const FVector MinPosition = Bounds.Min;
		OutTileLayout.BottomLeftX = FMath::FloorToInt(MinPosition.X / StaticMeshSpawner->TileSize);
		OutTileLayout.BottomLeftY = FMath::FloorToInt(MinPosition.Y / StaticMeshSpawner->TileSize);

		// Determine the total number of tiles along each active axis
		const FVector MaxPosition = Bounds.Max;
		const int32 MaxXIdx = FMath::FloorToInt(MaxPosition.X / StaticMeshSpawner->TileSize);
		const int32 MaxYIdx = FMath::FloorToInt(MaxPosition.Y / StaticMeshSpawner->TileSize);

		OutTileLayout.NumTilesX = (MaxXIdx - OutTileLayout.BottomLeftX) + 1;
		OutTileLayout.NumTilesY = (MaxYIdx - OutTileLayout.BottomLeftY) + 1;

		OutTileLayout.HalfHeight = Bounds.GetExtent().Z;
	}
}

FBox UProceduralStaticMeshComponent::GetBounds() const
{
	if (const UBrushComponent* Brush = SpawningVolume ? SpawningVolume->GetBrushComponent() : nullptr)
	{
		return Brush->Bounds.GetBox();
	}
	else
	{
		AActor* LocalOwner = GetOwner();
		ULevel* Level = LocalOwner ? LocalOwner->GetLevel() : nullptr;
		ALevelBounds* LevelBoundsActor = Level ? Level->LevelBoundsActor.Get() : nullptr;
		if (LevelBoundsActor)
		{
			return LevelBoundsActor->GetComponentsBoundingBox(false);
		}
	}

	return FBox(ForceInitToZero);
}

void UProceduralStaticMeshComponent::SpawnMeshes(const TArray<FDesiredProceduralStaticMeshInstance>& Instances)
{
#if WITH_EDITOR
	
	for (auto Instance : Instances)
	{
		if (Instance.ProceduralStaticMeshType == nullptr || !Instance.ProceduralStaticMeshType->StaticMesh.IsValid())
			continue;


		TArray<FHitResult> Hits;
		FCollisionShape SphereShape;
		GetWorld()->SweepMultiByObjectType(Hits, Instance.StartTrace, Instance.EndTrace, FQuat::Identity,
										   FCollisionObjectQueryParams(ECC_WorldStatic), SphereShape);

		FHitResult OutHit;
		bool IsHit = false;
		for (const FHitResult& Hit : Hits)
		{
			const FActorInstanceHandle& HitObjectHandle = Hit.HitObjectHandle;
			if (HitObjectHandle.IsValid() && HitObjectHandle.DoesRepresentClass(AVolume::StaticClass()))
			{
				continue;
			}

			const UPrimitiveComponent* HitComponent = Hit.GetComponent();
			check(HitComponent);

			// In the editor traces can hit "No Collision" type actors, so ugh. (ignore these)
			if (!HitComponent->IsQueryCollisionEnabled() || HitComponent->GetCollisionResponseToChannel(ECC_WorldStatic) != ECR_Block)
			{
				continue;
			}

			// Don't place foliage on invisible walls / triggers / volumes
			if (HitComponent->IsA<UBrushComponent>())
			{
				continue;
			}

			IsHit = true;
			OutHit = Hit;
			break;
		}

		if (IsHit)
		{
			
			FString StaticMeshPath = Instance.ProceduralStaticMeshType->StaticMesh.ToString();
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *StaticMeshPath);
			FActorSpawnParameters SpawnParams;
			// SpawnParams.Name = TEXT("ProceduralStaticMesh");
			// SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
			AStaticMeshActor* NewActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
			// NewActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepWorldTransform);
			NewActor->SetActorLocation(OutHit.Location);
			NewActor->SetActorRotation(Instance.Rotation.Rotator());
			NewActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			NewActor->SetMobility(EComponentMobility::Static);


			SpawnedActors.Add(NewActor);
		}
	}

#endif
}

void UProceduralStaticMeshComponent::Empty()
{
	for (const auto SpawnedActor : SpawnedActors)
	{
		if (SpawnedActor.IsValid())
			SpawnedActor->Destroy();
	}

	SpawnedActors.Empty();
}


// Called when the game starts
void UProceduralStaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UProceduralStaticMeshComponent::BeginDestroy()
{
	Super::BeginDestroy();
	Empty();
}


// Called every frame
void UProceduralStaticMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#undef LOCTEXT_NAMESPACE