// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeleteWidget.h"

#include "DebugHeader.h"
#include "SuperManager.h"

void SAdvancedDeleteTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	
	CheckboxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	StoredAssetsData = InArgs._AssetsDataToStore;
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30.f;
	
	ChildSlot
	[
		SNew(SVerticalBox)

		//First Slot
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advanced Delete")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		//Second Slot
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]

		//Third Slot
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)

			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]
		
		//Fourth Slot
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			//DeleteAll Button
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
			]

			//SelectAll Button
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]

			//DeselectAll Button
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeselectAllButton()
			]
		]
	];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeleteTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)
	.ItemHeight(24.f)
	.ListItemsSource(&StoredAssetsData)
	.OnGenerateRow(this, &SAdvancedDeleteTab::OnGenerateRowForList);

	return ConstructedAssetListView.ToSharedRef();
}

void SAdvancedDeleteTab::RefreshAssetListView()
{
	CheckboxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	if(ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region RowWidgetForAssetListView

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
	
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.f))
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
		.FillWidth(.55f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]
		
		//Third Slot
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayName, AssetNameFont)
		]

		//Fourth Slot
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)
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

	CheckboxesArray.Add(ConstructedCheckbox);
	return ConstructedCheckbox;
}

void SAdvancedDeleteTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState) {
	case ECheckBoxState::Unchecked:

		if(AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		
		break;
		
	case ECheckBoxState::Checked:

		AssetsDataToDeleteArray.AddUnique(AssetData);
		
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

TSharedRef<SButton> SAdvancedDeleteTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData> AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvancedDeleteTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvancedDeleteTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>("SuperManager");
	
	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if(bAssetDeleted)
	{
		if(StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}

		
	}
	
	return FReply::Handled();
}



#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvancedDeleteTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvancedDeleteTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;
}

FReply SAdvancedDeleteTab::OnDeleteAllButtonClicked()
{
	if(AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets curently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;

	for(const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>("SuperManager");

	const bool bAssetDataDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetsList(AssetDataToDelete);

	if(bAssetDataDeleted)
	{
		for(const TSharedPtr<FAssetData>& DeletedData : AssetsDataToDeleteArray)
		{
			if(StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}
		}

		RefreshAssetListView();
	}
	
	return FReply::Handled();
}

TSharedRef<SButton> SAdvancedDeleteTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvancedDeleteTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

FReply SAdvancedDeleteTab::OnSelectAllButtonClicked()
{
	if(CheckboxesArray.Num() == 0) return FReply::Handled();

	for(const TSharedRef<SCheckBox> CheckBox : CheckboxesArray)
	{
		if(!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<SButton> SAdvancedDeleteTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvancedDeleteTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvancedDeleteTab::OnDeselectAllButtonClicked()
{
	if(CheckboxesArray.Num() == 0) return FReply::Handled();

	for(const TSharedRef<SCheckBox> CheckBox : CheckboxesArray)
	{
		if(CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvancedDeleteTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15.f;
	
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion 