#include "FProceduralStaticMeshComponentDetails.h"

#include "ProceduralStaticMeshComponent.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Fonts/SlateFontInfo.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Attribute.h"
#include "ScopedTransaction.h"
#include "SlotBase.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtr.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

class IPropertyHandle;

#define LOCTEXT_NAMESPACE "ProceduralStaticMeshHelperComponentDetails"

TSharedRef<IDetailCustomization> FProceduralStaticMeshComponentDetails::MakeInstance()
{
	return MakeShareable(new FProceduralStaticMeshComponentDetails());

}

void FProceduralStaticMeshComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const FName ProceduralStaticMeshHelperCategoryName("ProceduralStaticMeshHelper");
	IDetailCategoryBuilder& ProceduralStaticMeshHelperCategory = DetailBuilder.EditCategory(ProceduralStaticMeshHelperCategoryName);

	const FText ResimulateText = LOCTEXT("ResimulateButtonText", "Resimulate");
	const FText EmptyText = LOCTEXT("EmptyButtonText", "Empty");

	TArray< TWeakObjectPtr<UObject> > ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (TWeakObjectPtr<UObject>& Object : ObjectsBeingCustomized)
	{
		UProceduralStaticMeshComponent* Component = Cast<UProceduralStaticMeshComponent>(Object.Get());
		if (ensure(Component))
		{
			SelectedComponents.Add(Component);
		}
	}

	TArray<TSharedRef<IPropertyHandle>> AllProperties;
	bool bSimpleProperties = true;
	bool bAdvancedProperties = false;
	// Add all properties in the category in order
	ProceduralStaticMeshHelperCategory.GetDefaultProperties(AllProperties, bSimpleProperties, bAdvancedProperties);
	for (auto& Property : AllProperties)
	{
		ProceduralStaticMeshHelperCategory.AddProperty(Property);
	}

	FDetailWidgetRow& NewRow = ProceduralStaticMeshHelperCategory.AddCustomRow(FText::GetEmpty());

	NewRow.ValueContent()
		.MaxDesiredWidth(120.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()			
			.Padding(4.0f)
			[
				SNew(SButton)
				.OnClicked(this, &FProceduralStaticMeshComponentDetails::OnResimulateClicked)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(ResimulateText)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.OnClicked(this, &FProceduralStaticMeshComponentDetails::OnEmptyClicked)
				.ToolTipText(LOCTEXT("EmptyText", "Empty."))
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(EmptyText)
				]
			]
		];
}

FReply FProceduralStaticMeshComponentDetails::OnResimulateClicked()
{
	for (TWeakObjectPtr<UProceduralStaticMeshComponent>& Component : SelectedComponents)
	{
		if (Component.IsValid())
		{
			Component->Resimulate();
		}
	}

	return FReply::Handled();
}

FReply FProceduralStaticMeshComponentDetails::OnEmptyClicked()
{
	for (TWeakObjectPtr<UProceduralStaticMeshComponent>& Component : SelectedComponents)
	{
		if (Component.IsValid())
		{
			Component->Empty();
		}
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
