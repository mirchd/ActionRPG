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

#pragma once

#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Containers/Set.h"
#include "Containers/SparseArray.h"
#include "Delegates/Delegate.h"
#include "HAL/PlatformCrt.h"
#include "Input/Reply.h"
#include "Misc/Optional.h"
#include "Styling/SlateTypes.h"
#include "Templates/SharedPointer.h"
#include "Templates/TypeHash.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STreeView.h"



class FHoudiniNodeInfo;
class FHoudiniNetworkInfo;
class ITableRow;
class SWidget;

typedef TSharedPtr<FHoudiniNodeInfo> FHoudiniNodeInfoPtr;


// Largely inspired by SFbxSceneTreeView

//Node use to store the scene hierarchy transform will be relative to the parent
class FHoudiniNodeInfo : public TSharedFromThis<FHoudiniNodeInfo>
{
public:

	FHoudiniNodeInfo()
		: NodeName(TEXT(""))
		, NodeId(-1)
		, NodeHierarchyPath(TEXT(""))
		, bIsRootNode(false)
		, bImportNode(true)
		, Parent(NULL)
	{}

	static void RecursiveSetImport(FHoudiniNodeInfoPtr NodeInfoPtr, bool ImportStatus);

	static bool RecursiveGetImport(FHoudiniNodeInfoPtr NodeInfoPtr);

	FString NodeName;
	int32 NodeId;
	FString NodeHierarchyPath;
	bool bIsRootNode;
	FString NodeType;

	bool bImportNode;

	TArray<FHoudiniNodeInfoPtr> Childrens;
	FHoudiniNodeInfoPtr Parent;
};



class FHoudiniNetworkInfo : public TSharedFromThis<FHoudiniNetworkInfo>
{
public:
	
	FHoudiniNetworkInfo()
	{}

	TArray<FHoudiniNodeInfoPtr> RootNodesInfos;
};



// The item used for visualizing the class in the tree.
class SHoudiniNodeTreeViewItem : public STableRow<FHoudiniNodeInfoPtr>
{
public:

	SLATE_BEGIN_ARGS(SHoudiniNodeTreeViewItem)
		: _HoudiniNodeInfo(nullptr)
		, _Expanded(false)
		, _SingleSelection(false)
	{}

	// Content of this item
	SLATE_ARGUMENT(FHoudiniNodeInfoPtr, HoudiniNodeInfo)
	SLATE_ARGUMENT(bool, Expanded)
	SLATE_ARGUMENT(bool, SingleSelection)
	SLATE_ARGUMENT(TArray<FHoudiniNodeInfoPtr>, HoudiniRootNodeArray)
	SLATE_END_ARGS()

	// Construct the widget
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

private:

	void OnItemCheckChanged(ECheckBoxState CheckType);

	ECheckBoxState IsItemChecked() const;

	// The node info to build the tree view row from.
	FHoudiniNodeInfoPtr HoudiniNodeInfo;

	// Are we in single selection mode ?
	bool bSingleSelectionOnly;

	TArray<FHoudiniNodeInfoPtr> HoudiniRootNodesInfo;
};



class SHoudiniNodeTreeView : public STreeView<FHoudiniNodeInfoPtr>
{
public:
	~SHoudiniNodeTreeView();

	SLATE_BEGIN_ARGS(SHoudiniNodeTreeView)
		: _HoudiniNetworkInfo(NULL)
		, _SingleSelection(false)
	{}
	SLATE_ARGUMENT(TSharedPtr<FHoudiniNetworkInfo>, HoudiniNetworkInfo)
	SLATE_ARGUMENT(bool, SingleSelection)
	SLATE_END_ARGS()

	/** Construct this widget */
	void Construct(const FArguments& InArgs);
	
	TSharedRef< ITableRow > OnGenerateRowHoudiniNodeTreeView(FHoudiniNodeInfoPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildrenHoudiniNodeTreeView(FHoudiniNodeInfoPtr InParent, TArray<FHoudiniNodeInfoPtr>& OutChildren);

	void OnToggleSelectAll(ECheckBoxState CheckType);
	FReply OnExpandAll();
	FReply OnCollapseAll();

protected:

	// Info for all the nodes in the current session
	//TSharedPtr<FHoudiniNetworkInfo> HoudiniNetworkInfo;
	//FHoudiniNetworkInfo HoudiniRootNodesInfo;

	// The elements shown in the tree view 
	TArray<FHoudiniNodeInfoPtr> HoudiniRootNodeArray;

	// Are we in single selection mode ?
	bool bSingleSelectionOnly;

	/** Open a context menu for the current selection */
	TSharedPtr<SWidget> OnOpenContextMenu();
	void AddSelectionToImport();
	void RemoveSelectionFromImport();
	void SetSelectionImportState(bool MarkForImport);
	void OnSelectionChanged(FHoudiniNodeInfoPtr Item, ESelectInfo::Type SelectionType);
	void OnSetExpandRecursive(FHoudiniNodeInfoPtr NodeInfoPtr, bool ExpandState);

	// Used to automatically expand previously selected items' parents upon construction
	void RecursiveSetDefaultExpand(FHoudiniNodeInfoPtr NodeInfo);
};