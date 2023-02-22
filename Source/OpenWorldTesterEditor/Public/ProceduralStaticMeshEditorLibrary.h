// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralStaticMeshEditorLibrary.generated.h"


class UProceduralStaticMeshComponent;

/**
 * 
 */
UCLASS()
class OPENWORLDTESTEREDITOR_API UProceduralStaticMeshEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="ProceduralStaticMesh")
	static void ResimulateProceduralStaticMeshComponents(const TArray<UProceduralStaticMeshComponent*>& ProceduralFoliageComponents);
};
