/*
* Copyright (c) <2023> Side Effects Software Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. The name of Side Effects Software may not be used to endorse or
*    promote products derived from this software without specific prior
*    written permission.
*
* THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
* NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SHoudiniNodeTreeView.h"

#include "HoudiniEngineEditorPrivatePCH.h"
#include "HoudiniEnginePrivatePCH.h"

#include "HoudiniApi.h"
#include "HoudiniEngine.h"
#include "HoudiniEngineUtils.h"

// dlg?
#include "Internationalization/Internationalization.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Views/ITypedTableView.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMath.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Layout/Children.h"
#include "Layout/Visibility.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Attribute.h"
#include "SlotBase.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateIconFinder.h"
#include "Textures/SlateIcon.h"
#include "UObject/NameTypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Views/STableRow.h"

class FUICommandList;
class ITableRow;
class SWidget;
class UClass;
struct FSlateBrush;

#define LOCTEXT_NAMESPACE "HoudiniNodeTreeview"



void
FHoudiniNodeInfo::RecursiveSetImport(FHoudiniNodeInfoPtr NodeInfoPtr, bool bImport)
{
	NodeInfoPtr->bImportNode = bImport;
	for (auto Child : NodeInfoPtr->Childrens)
	{
		RecursiveSetImport(Child, bImport);
	}

	if (!bImport)
	{
		// If we're not imported anymore we need to disable our parents' import  
		// as they are no longer fully selected...
		FHoudiniNodeInfoPtr ParentPtr = NodeInfoPtr->Parent;
		while (ParentPtr != NULL)
		{
			ParentPtr->bImportNode = false;
			ParentPtr = ParentPtr->Parent;
		}
	}
}

bool
FHoudiniNodeInfo::RecursiveGetImport(FHoudiniNodeInfoPtr NodeInfoPtr)
{
	if(NodeInfoPtr->bImportNode)
		return true;

	for (auto Child : NodeInfoPtr->Childrens)
	{
		if (RecursiveGetImport(Child))
			return true;
	}

	return false;
}

//
// SHoudiniNodeTreeViewItem
//

// Constructs the widget for one row of the TreeView
void 
SHoudiniNodeTreeViewItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	HoudiniNodeInfo = InArgs._HoudiniNodeInfo;
	bool bExpanded = InArgs._Expanded;
	bSingleSelectionOnly = InArgs._SingleSelection;
	HoudiniRootNodesInfo = InArgs._HoudiniRootNodeArray;

	//This is suppose to always be valid
	check(HoudiniNodeInfo.IsValid());
	
	const FSlateBrush* ClassIcon = nullptr;
	if (HoudiniNodeInfo->NodeType.Equals("OBJ"))
		ClassIcon = bExpanded ? FAppStyle::GetBrush("Icons.FolderOpen") : FAppStyle::GetBrush("Icons.FolderClosed");
	else
		ClassIcon = FSlateIconFinder::FindIconBrushForClass(AActor::StaticClass());

	//Prepare the tooltip
	FString Tooltip = HoudiniNodeInfo->NodeName;
	if (!HoudiniNodeInfo->NodeType.IsEmpty())
	{
		Tooltip += TEXT(" [") + HoudiniNodeInfo->NodeType + TEXT("]");
	}

	if (!HoudiniNodeInfo->NodeHierarchyPath.IsEmpty())
	{
		Tooltip += TEXT("\n") + HoudiniNodeInfo->NodeHierarchyPath;
	}

	this->ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f, 2.0f, 0.0f)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged(this, &SHoudiniNodeTreeViewItem::OnItemCheckChanged)
			.IsChecked(this, &SHoudiniNodeTreeViewItem::IsItemChecked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SExpanderArrow, SharedThis(this))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(SImage)
			.Image(ClassIcon)
			.Visibility(ClassIcon != FAppStyle::GetDefaultBrush() ? EVisibility::Visible : EVisibility::Collapsed)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 3.0f, 6.0f, 3.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(HoudiniNodeInfo->NodeName))
			.ToolTipText(FText::FromString(Tooltip))
		]
	];

	STableRow<FHoudiniNodeInfoPtr>::ConstructInternal(
		STableRow::FArguments()
		.ShowSelection(true),
		InOwnerTableView
	);
}

void 
SHoudiniNodeTreeViewItem::OnItemCheckChanged(ECheckBoxState CheckType)
{
	if (!HoudiniNodeInfo.IsValid())
		return;

	bool bImport = CheckType == ECheckBoxState::Checked;
	HoudiniNodeInfo->bImportNode = bImport;

	// If in single selection mode - disable all other nodes but us
	if (bSingleSelectionOnly)// && bImport)
	{
		for (int32 Idx = 0; Idx < HoudiniRootNodesInfo.Num(); Idx++)
		{
			//FHoudiniNodeInfoPtr NodeInfo = (*NodeInfoIt);
			FHoudiniNodeInfoPtr NodeInfo = HoudiniRootNodesInfo[Idx];
			if (!NodeInfo.IsValid())
				continue;

			if (NodeInfo->bIsRootNode)
			{
				FHoudiniNodeInfo::RecursiveSetImport(NodeInfo, false);
			}
		}
	}

	// Recursively set our children's import state
	FHoudiniNodeInfo::RecursiveSetImport(HoudiniNodeInfo, bImport);
}

ECheckBoxState
SHoudiniNodeTreeViewItem::IsItemChecked() const
{
	return HoudiniNodeInfo->bImportNode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}


//
// SHoudiniNodeTreeView
//

SHoudiniNodeTreeView::~SHoudiniNodeTreeView()
{
	HoudiniRootNodeArray.Empty();
	bSingleSelectionOnly = false;
}

void
SHoudiniNodeTreeView::Construct(const SHoudiniNodeTreeView::FArguments& InArgs)
{
	TSharedPtr<FHoudiniNetworkInfo> HoudiniNetworkInfo = InArgs._HoudiniNetworkInfo;
	//Build the FHoudiniNodeInfoPtr tree data
	check(HoudiniNetworkInfo.IsValid());
	for (auto NodeInfoIt = HoudiniNetworkInfo->RootNodesInfos.CreateIterator(); NodeInfoIt; ++NodeInfoIt)
	{
		FHoudiniNodeInfoPtr NodeInfo = (*NodeInfoIt);
		if (NodeInfo->bIsRootNode)
		{
			HoudiniRootNodeArray.Add(NodeInfo);
		}
	}

	bSingleSelectionOnly = InArgs._SingleSelection;

	STreeView::Construct
	(
		STreeView::FArguments()
		.TreeItemsSource(&HoudiniRootNodeArray)
		.SelectionMode(bSingleSelectionOnly ? ESelectionMode::SingleToggle : ESelectionMode::Multi)
		.OnGenerateRow(this, &SHoudiniNodeTreeView::OnGenerateRowHoudiniNodeTreeView)
		.OnGetChildren(this, &SHoudiniNodeTreeView::OnGetChildrenHoudiniNodeTreeView)
		.OnContextMenuOpening(this, &SHoudiniNodeTreeView::OnOpenContextMenu)
		.OnSelectionChanged(this, &SHoudiniNodeTreeView::OnSelectionChanged)
		.OnSetExpansionRecursive(this, &SHoudiniNodeTreeView::OnSetExpandRecursive)
	);

	// Expand the previous selection
	for (auto NodeInfo : HoudiniRootNodeArray)
	{
		RecursiveSetDefaultExpand(NodeInfo);
	}
}


TSharedRef<ITableRow>
SHoudiniNodeTreeView::OnGenerateRowHoudiniNodeTreeView(FHoudiniNodeInfoPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	bool bExpanded = IsItemExpanded(Item) || FHoudiniNodeInfo::RecursiveGetImport(Item);
	TSharedRef<SHoudiniNodeTreeViewItem> ReturnRow = SNew(SHoudiniNodeTreeViewItem, OwnerTable)
		.HoudiniNodeInfo(Item)
		.Expanded(bExpanded)
		.SingleSelection(bSingleSelectionOnly)
		.HoudiniRootNodeArray(HoudiniRootNodeArray);
	return ReturnRow;
}


void 
SHoudiniNodeTreeView::OnGetChildrenHoudiniNodeTreeView(FHoudiniNodeInfoPtr InParent, TArray<FHoudiniNodeInfoPtr>& OutChildren)
{
	for (auto Child : InParent->Childrens)
	{
		OutChildren.Add(Child);
	}
}


void 
SHoudiniNodeTreeView::OnToggleSelectAll(ECheckBoxState CheckType)
{
	//check all actor for import
	for(int32 Idx = 0; Idx < HoudiniRootNodeArray.Num(); Idx++)
	{
		FHoudiniNodeInfoPtr NodeInfo = HoudiniRootNodeArray[Idx];
		if (!NodeInfo.IsValid())
			continue;

		if(NodeInfo->bIsRootNode)
		{
			FHoudiniNodeInfo::RecursiveSetImport(NodeInfo, CheckType == ECheckBoxState::Checked);
		}
	}
}

void 
RecursiveSetExpand(SHoudiniNodeTreeView* TreeView, FHoudiniNodeInfoPtr NodeInfoPtr, bool ExpandState)
{
	TreeView->SetItemExpansion(NodeInfoPtr, ExpandState);
	for (auto Child : NodeInfoPtr->Childrens)
	{
		RecursiveSetExpand(TreeView, Child, ExpandState);
	}
}

void 
SHoudiniNodeTreeView::OnSetExpandRecursive(FHoudiniNodeInfoPtr NodeInfoPtr, bool ExpandState)
{
	RecursiveSetExpand(this, NodeInfoPtr, ExpandState);
}

void
SHoudiniNodeTreeView::RecursiveSetDefaultExpand(FHoudiniNodeInfoPtr NodeInfo)
{
	bool bExpanded = IsItemExpanded(NodeInfo) || (FHoudiniNodeInfo::RecursiveGetImport(NodeInfo) && !NodeInfo->bImportNode);
	SetItemExpansion(NodeInfo, bExpanded);

	for (auto Child : NodeInfo->Childrens)
	{
		RecursiveSetDefaultExpand(Child);
	}
}

FReply 
SHoudiniNodeTreeView::OnExpandAll()
{
	for (int32 Idx = 0; Idx < HoudiniRootNodeArray.Num(); Idx++)
	{
		FHoudiniNodeInfoPtr NodeInfo = HoudiniRootNodeArray[Idx];
		if (!NodeInfo.IsValid())
			continue;

		if (NodeInfo->bIsRootNode)
		{
			RecursiveSetExpand(this, NodeInfo, true);
		}
	}
	return FReply::Handled();
}

FReply 
SHoudiniNodeTreeView::OnCollapseAll()
{
	for (int32 Idx = 0; Idx < HoudiniRootNodeArray.Num(); Idx++)
	{
		FHoudiniNodeInfoPtr NodeInfo = HoudiniRootNodeArray[Idx];
		if (!NodeInfo.IsValid())
			continue;

		if (NodeInfo->bIsRootNode)
		{
			RecursiveSetExpand(this, NodeInfo, false);
		}
	}
	return FReply::Handled();
}

TSharedPtr<SWidget> 
SHoudiniNodeTreeView::OnOpenContextMenu()
{
	// Build up the menu for a selection
	const bool bCloseAfterSelection = true;
	FMenuBuilder MenuBuilder(bCloseAfterSelection, TSharedPtr<FUICommandList>());

	//Get the different type of the multi selection
	TArray<FHoudiniNodeInfoPtr> SelectedHoudiniNodeInfos;
	const auto NumSelectedItems = GetSelectedItems(SelectedHoudiniNodeInfos);

	// We always create a section here, even if there is no parent so that clients can still extend the menu
	MenuBuilder.BeginSection("HoudiniSceneTreeViewContextMenuImportSection");
	{
		const FSlateIcon PlusIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus");
		MenuBuilder.AddMenuEntry(LOCTEXT("CheckForImport", "Add Selection To Import"), FText(), PlusIcon, FUIAction(FExecuteAction::CreateSP(this, &SHoudiniNodeTreeView::AddSelectionToImport)));
		const FSlateIcon MinusIcon(FAppStyle::GetAppStyleSetName(), "Icons.Minus");
		MenuBuilder.AddMenuEntry(LOCTEXT("UncheckForImport", "Remove Selection From Import"), FText(), MinusIcon, FUIAction(FExecuteAction::CreateSP(this, &SHoudiniNodeTreeView::RemoveSelectionFromImport)));
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void 
SHoudiniNodeTreeView::AddSelectionToImport()
{
	SetSelectionImportState(true);
}

void
SHoudiniNodeTreeView::RemoveSelectionFromImport()
{
	SetSelectionImportState(false);
}

void 
SHoudiniNodeTreeView::SetSelectionImportState(bool MarkForImport)
{
	TArray<FHoudiniNodeInfoPtr> SelectedHoudiniNodeInfos;
	GetSelectedItems(SelectedHoudiniNodeInfos);
	for (auto Item : SelectedHoudiniNodeInfos)
	{
		FHoudiniNodeInfoPtr ItemPtr = Item;
		FHoudiniNodeInfo::RecursiveSetImport(ItemPtr, MarkForImport);
	}
}

void
SHoudiniNodeTreeView::OnSelectionChanged(FHoudiniNodeInfoPtr Item, ESelectInfo::Type SelectionType)
{
}


#undef LOCTEXT_NAMESPACE

