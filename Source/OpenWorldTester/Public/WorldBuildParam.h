// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldBuildParam.generated.h"


USTRUCT(BlueprintType)
struct FWorldBuildAreaParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size = FVector2D(10000, 10000);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh")
	TArray<FSoftObjectPath> BigMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh", meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float BigMeshesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh")
	TArray<FSoftObjectPath> MediumMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh", meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float MediumMeshesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh")
	TArray<FSoftObjectPath> SmallMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh", meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float SmallMeshesDensity;


	
	
};

/**
 * 
 */
UCLASS(BlueprintType)
class OPENWORLDTESTER_API UWorldBuildParam final : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area")
	TArray<FWorldBuildAreaParam> Areas;
	


};
