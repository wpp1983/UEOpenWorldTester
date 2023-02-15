// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralStaticMeshSpawnerFactory.h"

#include "ProceduralStaticMeshSpawner.h"


UProceduralStaticMeshSpawnerFactory::UProceduralStaticMeshSpawnerFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UProceduralStaticMeshSpawner::StaticClass();
}

UObject* UProceduralStaticMeshSpawnerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewProceduralFoliage = NewObject<UProceduralStaticMeshSpawner>(InParent, Class, Name, Flags | RF_Transactional);

	return NewProceduralFoliage;
}

bool UProceduralStaticMeshSpawnerFactory::ShouldShowInNewMenu() const
{
	return true;	
}
