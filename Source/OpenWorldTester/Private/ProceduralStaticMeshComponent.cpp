// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshComponent.h"

#include "ProceduralFoliageComponent.h"
#include "ProceduralStaticMeshInstance.h"
#include "ProceduralStaticMeshSpawner.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Volume.h"
#include "UObject/ConstructorHelpers.h"


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
	Empty();

	StaticMeshSpawner->Simulate(SpawningVolume->GetBounds());

	for (auto Instance : StaticMeshSpawner->InstancesSet)
	{
		if (Instance->Type == nullptr || !Instance->Type->StaticMesh.IsValid())
			continue;

		float Height = SpawningVolume->GetBounds().BoxExtent.Z;
		FVector StartTrace = Instance->Location;
		StartTrace.Z += Height * 0.5f;
		FVector EndTrace = StartTrace;
		EndTrace.Z -= (Height + 10);
		const FVector Dir = (EndTrace -StartTrace).GetSafeNormal();

		TArray<FHitResult> Hits;
		FCollisionShape SphereShape;
		// SphereShape.SetSphere(DesiredInstance.TraceRadius);
		GetWorld()->SweepMultiByObjectType(Hits, StartTrace, EndTrace, FQuat::Identity,
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
			
			FString StaticMeshPath = Instance->Type->StaticMesh.ToString();
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *StaticMeshPath);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("ProceduralStaticMesh");
			SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
			AStaticMeshActor* NewActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
			NewActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepWorldTransform);
			NewActor->SetActorLocation(OutHit.Location);
			NewActor->SetActorRotation(Instance->Rotation.Rotator());
			NewActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			NewActor->SetMobility(EComponentMobility::Static);


			SpawnedActors.Add(NewActor);
		}
	}

	StaticMeshSpawner->Empty();
}

void UProceduralStaticMeshComponent::Empty()
{
	for (auto SpawnedActor : SpawnedActors)
	{
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
