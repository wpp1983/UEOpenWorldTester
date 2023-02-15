// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralStaticMeshComponent.generated.h"

class UProceduralStaticMeshSpawner;

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
