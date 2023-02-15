#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_ProceduralStaticMeshSpawner: public FAssetTypeActions_Base
{
public:

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ProceduralStaticMeshSpawner", "Procedural StaticMesh Spawner"); }
	virtual FColor GetTypeColor() const override { return FColor(7, 103, 7); }
	virtual UClass* GetSupportedClass() const override;
	virtual bool CanFilter() override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
};
