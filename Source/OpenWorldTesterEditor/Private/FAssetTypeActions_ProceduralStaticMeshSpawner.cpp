#include "FAssetTypeActions_ProceduralStaticMeshSpawner.h"

#include "ProceduralStaticMeshSpawner.h"

UClass* FAssetTypeActions_ProceduralStaticMeshSpawner::GetSupportedClass() const
{
	return UProceduralStaticMeshSpawner::StaticClass();
}

bool FAssetTypeActions_ProceduralStaticMeshSpawner::CanFilter()
{
	return true;
}