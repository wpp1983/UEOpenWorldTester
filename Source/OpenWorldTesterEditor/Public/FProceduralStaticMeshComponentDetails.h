#pragma once

#include "Containers/Array.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtrTemplates.h"

class IDetailLayoutBuilder;

class FProceduralStaticMeshComponentDetails : public IDetailCustomization
{
public:
	virtual ~FProceduralStaticMeshComponentDetails(){};

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder );
private:
	FReply OnResimulateClicked();
	FReply OnEmptyClicked();

private:
	TArray< TWeakObjectPtr<class UProceduralStaticMeshComponent> > SelectedComponents;

};
