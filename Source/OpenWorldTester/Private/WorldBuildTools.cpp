// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBuildTools.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "Editor/EditorEngine.h"
#include "WorldBuildParam.h"

void InitializeDefaultHeightData(TArray<uint16>& OutData,
                                 int32 NewLandscape_SectionsPerComponent, int32 NewLandscape_QuadsPerSection,
                                 const FIntPoint& NewLandscape_ComponentCount)
{
	const int32 QuadsPerComponent = NewLandscape_SectionsPerComponent * NewLandscape_QuadsPerSection;
	const int32 SizeX = NewLandscape_ComponentCount.X * QuadsPerComponent + 1;
	const int32 SizeY = NewLandscape_ComponentCount.Y * QuadsPerComponent + 1;
	const int32 TotalSize = SizeX * SizeY;
	// Initialize heightmap data
	OutData.Reset();
	OutData.AddUninitialized(TotalSize);
	
	TArray<uint16> StrideData;
	StrideData.AddUninitialized(SizeX);
	// Initialize blank heightmap data
	for (int32 X = 0; X < SizeX; ++X)
	{
		StrideData[X] = LandscapeDataAccess::MidValue;
	}
	for (int32 Y = 0; Y < SizeY; ++Y)
	{
		FMemory::Memcpy(&OutData[Y * SizeX], StrideData.GetData(), sizeof(uint16) * SizeX);
	}
}

ALandscape* CreateNewLandscape(UWorld* InWorld, FVector NewLandscape_Location, FVector NewLandscape_Scale)
{
	const FRotator NewLandscape_Rotation = FRotator(0, 0, 0);
	
	int NewLandscape_SectionsPerComponent = 1;
	int NewLandscape_QuadsPerSection = 63;
	const int32 ComponentCountX = 8;
	const int32 ComponentCountY = 8;
	const int32 QuadsPerComponent = NewLandscape_SectionsPerComponent * NewLandscape_QuadsPerSection;
	const int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
	const int32 SizeY = ComponentCountY * QuadsPerComponent + 1;

	const FString ReimportHeightmapFilePath;

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	
	TArray<uint16> OutHeightData;
	InitializeDefaultHeightData(OutHeightData, NewLandscape_SectionsPerComponent, NewLandscape_QuadsPerSection, FIntPoint(ComponentCountX, ComponentCountY));
	HeightDataPerLayers.Add(FGuid(), OutHeightData);

	TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
	MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(MaterialImportLayers));

	const FVector Offset = FTransform(NewLandscape_Rotation, FVector::ZeroVector, NewLandscape_Scale).TransformVector(FVector(-ComponentCountX * QuadsPerComponent / 2, -ComponentCountY * QuadsPerComponent / 2, 0));
		
	ALandscape* NewLandscape = InWorld->SpawnActor<ALandscape>(NewLandscape_Location + Offset, NewLandscape_Rotation);
	
	NewLandscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1,
		NewLandscape_SectionsPerComponent, NewLandscape_QuadsPerSection,
		HeightDataPerLayers, *ReimportHeightmapFilePath, MaterialLayerDataPerLayers,
		ELandscapeImportAlphamapType::Additive);
		
	return NewLandscape;
	
}

void AddArea(const FWorldBuildAreaParam& AreaParam, UWorld* InWorld, FVector2D StartPos)
{
	const FVector2D MinPos = StartPos;
	FVector2D MaxPos = MinPos + AreaParam.Size;
	
	
	// Generate static Mesh
	
	


	// Generate Foliage
	
	
}


UWorldBuildTools::UWorldBuildTools(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UWorldBuildTools::BuildOpenWorldTestLevel(const UWorldBuildParam* BuildParams)
{
	UWorld* World = nullptr;
#if WITH_EDITOR
	if (UEditorEngine *EEngine = Cast<UEditorEngine>(GEngine); GIsEditor && EEngine != nullptr)
	{
		World = EEngine->GetEditorWorldContext().World();
	}

#endif

	check(World != nullptr);
	
	//Create Landscape
	CreateNewLandscape(World, FVector::Zero(), FVector(100,100,100));


	//add Areas
	for (auto AreaParam : BuildParams->Areas)
	{
		// AddArea(AreaParam, World);
		
	}
	
}



