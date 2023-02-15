// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ProceduralStaticMeshInstance.generated.h"

struct  FProceduralStaticMeshType;
/**
 * 
 */
USTRUCT(BlueprintType)
struct OPENWORLDTESTER_API FProceduralStaticMeshInstance 
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	FQuat Rotation;

	UPROPERTY(Category = ProceduralFoliageInstance, EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(Category = ProceduralFoliageInstance, EditAnywhere, BlueprintReadWrite)
	float Age;

	UPROPERTY(Category = ProceduralFoliageInstance, EditAnywhere, BlueprintReadWrite)
	FVector Normal;

	UPROPERTY()
	float Scale;

	const FProceduralStaticMeshType* Type;

	UActorComponent* BaseComponent;
};
