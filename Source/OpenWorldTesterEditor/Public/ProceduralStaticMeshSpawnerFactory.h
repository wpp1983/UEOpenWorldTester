// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralStaticMeshSpawnerFactory.generated.h"

class FFeedbackContext;
class UClass;
class UObject;

/**
 * 
 */
UCLASS()
class OPENWORLDTESTEREDITOR_API UProceduralStaticMeshSpawnerFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	// End of UFactory interface
};
