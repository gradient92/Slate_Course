// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeleteWidget.h"

#include "DebugHeader.h"

void SAdvancedDeleteTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	StoredAssetsData = InArgs._AssetsDataToStore;
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30.f;
	
	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advanced Delete")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]

		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)

			+SScrollBox::Slot()
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				.ItemHeight(24.f)
				.ListItemsSource(&StoredAssetsData)
				.OnGenerateRow(this, &SAdvancedDeleteTab::OnGenerateRowForList)
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];
}

TSharedRef<ITableRow> SAdvancedDeleteTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if(!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();
	const FString DisplayName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10.f;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15.f;
	
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
	[
		//First Slot
		SNew(SHorizontalBox)

		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]
		//Second Slot
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.FillWidth(.2f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]
		
		//Third Slot
		+SHorizontalBox::Slot()
		[
			ConstructTextForRowWidget(DisplayName, AssetNameFont)
		]
		
	];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvancedDeleteTab::ConstructCheckBox(const TSharedPtr<FAssetData> AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckbox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvancedDeleteTab::OnCheckBoxStateChanged, AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	return ConstructedCheckbox;
}

void SAdvancedDeleteTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState) {
	case ECheckBoxState::Unchecked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" Unchecked"), FColor::Red);
		
		break;
		
	case ECheckBoxState::Checked:

		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" Checked"), FColor::Green);
		
		break;
		
	case ECheckBoxState::Undetermined:
		break;
	}
}

TSharedRef<STextBlock> SAdvancedDeleteTab::ConstructTextForRowWidget(const FString& TextContent,
	const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}
