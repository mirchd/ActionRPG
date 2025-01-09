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

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWindow.h"
#include "HoudiniEngineRuntimePrivatePCH.h"
#include "HoudiniRampEditorWindowSubsystem.generated.h"

#define LOCTEXT_NAMESPACE HOUDINI_LOCTEXT_NAMESPACE

class IHoudiniRampCurveEditor;

/** The purpose of this subsystem is to manage opening and closing of ramp editor windows. */
UCLASS()
class UHoudiniRampEditorWindowSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:

	DECLARE_DELEGATE(FOnValueCommitted);

	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;

	/** Destroys all open curve editor windows. */
	void DestroyAllEditorWindows();

	/** Refreshes all open curve editor windows with latest curve info. */
	void RefreshAllEditors();

	/**
	 * Opens a ramp view in a resizable floating window.
	 *
	 * @tparam EditorWidgetType Widget to create in the window. Must have RampView argument which
	 *                          accepts RampViewType. Must implement IHoudiniRampCurveEditor.
	 * @tparam RampViewType Type used by EditorWidgetType to interface with the ramp state.
	 *
	 * @param RampView The ramp which this window will edit.
	 * @param ParentWidget The parent for the new float ramp curve editor window.
	 * @param OnValueCommittedDelegate Delegate to execute when ramp curve editor changes.
	 *
	 * @returns true if the window is created successfully, false otherwise.
	 */
	template<typename EditorWidgetType, typename RampViewType>
	bool
	OpenEditor(
		TSharedPtr<RampViewType> RampView,
		TSharedPtr<const SWidget> ParentWidget,
		FOnValueCommitted OnValueCommittedDelegate)
	{
#if WITH_EDITOR
	
		// A default window size for the ramp which looks nice
		static const FVector2D DefaultWindowSize(800, 400);
	
		// Determine the position of the window so that it will spawn near the mouse, but not go off
		// the screen.
		FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
	
		FSlateRect Anchor(CursorPos.X, CursorPos.Y, CursorPos.X, CursorPos.Y);
	
		constexpr bool bAutoAdjustForDPIScale = true;
		FVector2D AdjustedSummonLocation = FSlateApplication::Get().CalculatePopupWindowPosition(
			Anchor,
			DefaultWindowSize,
			bAutoAdjustForDPIScale,
			FVector2D::ZeroVector,
			Orient_Horizontal);
	
		TSharedRef<SBorder> WindowContent = SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
			.Padding(FMargin(8.0f, 8.0f));
	
		TSharedRef<SWindow> Window = SNew(SWindow)
			.AutoCenter(EAutoCenter::None)
			.ScreenPosition(AdjustedSummonLocation)
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.ClientSize(DefaultWindowSize)
			.SizingRule(ESizingRule::UserSized)
			.Title(LOCTEXT("WindowHeader", "Ramp Editor"))
			[
				WindowContent
			];
	
		TSharedRef<EditorWidgetType> CreatedCurveEditor = SNew(EditorWidgetType)
			.RampView(RampView)
			.OnCurveChanged_Lambda([=]() { OnValueCommittedDelegate.ExecuteIfBound(); });
	
		WindowContent->SetContent(CreatedCurveEditor);
	
		if (ParentWidget.IsValid())
		{
			// Find the window of the parent widget
			FWidgetPath WidgetPath;
			FSlateApplication::Get().GeneratePathToWidgetChecked(ParentWidget.ToSharedRef(), WidgetPath);
			Window = FSlateApplication::Get().AddWindowAsNativeChild(Window, WidgetPath.GetWindow());
		}
		else
		{
			Window = FSlateApplication::Get().AddWindow(Window);
		}
	
		// Hold on to the window created for external use...
		Windows.Add(Window);
		CurveEditors.Add(CreatedCurveEditor);
	
		return true;
#endif // WITH_EDITOR
	
		return false;
	}

private:

	TArray<TWeakPtr<SWindow>> Windows;
	TArray<TWeakPtr<IHoudiniRampCurveEditor>> CurveEditors;

	FDelegateHandle ActorSelectionChangedDelegateHandle;

	// Used to check if actor selection has changed!
	// See note in definition of OnActorSelectionChanged
	TArray<UObject*> PreviousActorSelection;

	void OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bForceRefresh);
};

#undef LOCTEXT_NAMESPACE
