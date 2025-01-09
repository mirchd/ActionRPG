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

// #include "Containers/Array.h"
// #include "Containers/BitArray.h"
// #include "Containers/Set.h"
// #include "Containers/SparseArray.h"
// #include "Delegates/Delegate.h"
// #include "HAL/PlatformCrt.h"
// #include "Input/Reply.h"
// #include "Misc/Optional.h"
// #include "Styling/SlateTypes.h"
#include "Templates/SharedPointer.h"
// #include "Templates/TypeHash.h"
// #include "Types/SlateEnums.h"
// #include "Widgets/DeclarativeSyntaxSupport.h"
// #include "Widgets/Views/STableViewBase.h"
// #include "Widgets/Views/STreeView.h"

#include "SHoudiniNodeTreeView.h"

// Dlg
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"

class FHoudiniNodeInfo;
class FHoudiniNetworkInfo;
class ITableRow;
class SWidget;

typedef TSharedPtr<FHoudiniNodeInfo> FHoudiniNodeInfoPtr;


class SSelectHoudiniPathDialog : public SWindow
{
public:

	SLATE_BEGIN_ARGS(SSelectHoudiniPathDialog) {}
		SLATE_ARGUMENT(FText, InitialPath)
		SLATE_ARGUMENT(FText, TitleText)
		SLATE_ARGUMENT(bool, SingleSelection)
	SLATE_END_ARGS()

	SSelectHoudiniPathDialog();

	void Construct(const FArguments& InArgs);

	EAppReturnType::Type ShowModal();

	const FText& GetFolderPath() const;

	void UpdateNodePathFromTreeView(FHoudiniNodeInfoPtr& InNodeInfo, FString& OutPath);

	void FillHoudiniNetworkInfo();

	void FillHoudiniNodeInfo(FHoudiniNodeInfoPtr InNodeInfo);

private:

	void OnPathChange(const FString& NewPath);
	FReply OnButtonClick(EAppReturnType::Type ButtonID);

	TSharedPtr<SHoudiniNodeTreeView> HoudiniNodeTreeView;

	FHoudiniNetworkInfo NetworkInfo;

	EAppReturnType::Type UserResponse;
	FText FolderPath;
	TArray<FString> SplitFolderPath;
	bool bSingleSelectionOnly;
};
