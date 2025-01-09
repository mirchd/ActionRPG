/*
* Copyright (c) <2024> Side Effects Software Inc.
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

#include "HoudiniRampEditorWindowSubsystem.h"

#include "SHoudiniRampBase.h"

#define LOCTEXT_NAMESPACE HOUDINI_LOCTEXT_NAMESPACE

void UHoudiniRampEditorWindowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	// Get the level editor module
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::Get().GetModuleChecked<FLevelEditorModule>("LevelEditor");

	ActorSelectionChangedDelegateHandle =
		LevelEditorModule.OnActorSelectionChanged().AddUObject(
			this, &UHoudiniRampEditorWindowSubsystem::OnActorSelectionChanged);
#endif // WITH_EDITOR
}

void
UHoudiniRampEditorWindowSubsystem::Deinitialize()
{
	DestroyAllEditorWindows();
}

void
UHoudiniRampEditorWindowSubsystem::DestroyAllEditorWindows()
{
	for (TWeakPtr<SWindow>& Window : Windows)
	{
		if (!Window.IsValid())
		{
			continue;
		}

		Window.Pin()->RequestDestroyWindow();
	}

	Windows.Empty();
	CurveEditors.Empty();
}

void
UHoudiniRampEditorWindowSubsystem::RefreshAllEditors()
{
	for (TWeakPtr<IHoudiniRampCurveEditor>& CurveEditor : CurveEditors)
	{
		if (!CurveEditor.IsValid())
		{
			continue;
		}

		CurveEditor.Pin()->RefreshCurveKeys();
	}
}

void
UHoudiniRampEditorWindowSubsystem::OnActorSelectionChanged(
	const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
	// This is silly but we need a way to detect when the actor selection actually changes. This
	// delegate is broadcasted during a force refresh of the details panel, even when the actor
	// selection doesn't actually change! So we check ourselves if the selection has changed.
	if (NewSelection == PreviousActorSelection)
	{
		// Since this is the same selection, don't destroy existing ramp curve editor windows, 
		// instead refresh them.
		RefreshAllEditors();

		return;
	}

	DestroyAllEditorWindows();
	PreviousActorSelection = NewSelection;
}

#undef LOCTEXT_NAMESPACE
