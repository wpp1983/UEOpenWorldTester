// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldBuildTools.generated.h"

class UWorldBuildParam;
/**
 * 
 */
UCLASS()
class OPENWORLDTESTER_API UWorldBuildTools final : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	UFUNCTION(BlueprintCallable, Category = OpenWorldTester)
	static void BuildOpenWorldTestLevel(const UWorldBuildParam* BuildParams );
};
