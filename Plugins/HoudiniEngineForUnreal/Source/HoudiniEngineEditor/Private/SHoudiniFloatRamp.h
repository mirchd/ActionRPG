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
#include "Curves/CurveFloat.h"
#include "SCurveEditor.h"
#include "SHoudiniRampBase.h"
#include "HoudiniRampEditorWindowSubsystem.h"

class UHoudiniParameterRampFloat;
class UHoudiniParameterRampFloatPoint;

class FHoudiniFloatRampView : public THoudiniRampViewBase<
	FHoudiniFloatRampView, float, UHoudiniParameterRampFloat, UHoudiniParameterRampFloatPoint>
{
public:

	static constexpr ValueType DefaultInsertValue = 1.f;

	explicit FHoudiniFloatRampView(TArrayView<const ParameterWeakPtr> Parameters)
		: THoudiniRampViewBase(Parameters)
	{
	}
};

class SHoudiniFloatRampCurveEditor
	: public SHoudiniRampCurveEditorBase<SCurveEditor, FHoudiniFloatRampView>
{
public:

	using ValueType = SHoudiniRampCurveEditorBase::ValueType;

	SLATE_BEGIN_ARGS(SHoudiniFloatRampCurveEditor)
		: _ViewMinInput(0.0f)
		, _ViewMaxInput(1.0f)
		, _ViewMinOutput(0.0f)
		, _ViewMaxOutput(1.0f)
		, _InputSnap(0.1f)
		, _OutputSnap(0.05f)
		, _InputSnappingEnabled(false)
		, _OutputSnappingEnabled(false)
		, _ShowTimeInFrames(false)
		, _TimelineLength(1.0f)
		, _DesiredSize(FVector2D::ZeroVector)
		, _DrawCurve(true)
		, _HideUI(true)
		, _AllowZoomOutput(false)
		, _AlwaysDisplayColorCurves(false)
		, _ZoomToFitVertical(false)
		, _ZoomToFitHorizontal(false)
		, _ShowZoomButtons(false)
		, _XAxisName("X")
		, _YAxisName("Y")
		, _ShowInputGridNumbers(false)
		, _ShowOutputGridNumbers(false)
		, _ShowCurveSelector(false)
		, _GridColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.3f))
		{
			_Clipping = EWidgetClipping::ClipToBounds;
		}

		SLATE_ARGUMENT(TSharedPtr<FHoudiniFloatRampView>, RampView)
		SLATE_EVENT(FOnCurveChanged, OnCurveChanged)
		SLATE_ATTRIBUTE(float, ViewMinInput)
		SLATE_ATTRIBUTE(float, ViewMaxInput)
		SLATE_ATTRIBUTE(TOptional<float>, DataMinInput)
		SLATE_ATTRIBUTE(TOptional<float>, DataMaxInput)
		SLATE_ATTRIBUTE(float, ViewMinOutput)
		SLATE_ATTRIBUTE(float, ViewMaxOutput)
		SLATE_ATTRIBUTE(float, InputSnap)
		SLATE_ATTRIBUTE(float, OutputSnap)
		SLATE_ATTRIBUTE(bool, InputSnappingEnabled)
		SLATE_ATTRIBUTE(bool, OutputSnappingEnabled)
		SLATE_ATTRIBUTE(bool, ShowTimeInFrames)
		SLATE_ATTRIBUTE(float, TimelineLength)
		SLATE_ATTRIBUTE(FVector2D, DesiredSize)
		SLATE_ATTRIBUTE(bool, AreCurvesVisible)
		SLATE_ARGUMENT(bool, DrawCurve)
		SLATE_ARGUMENT(bool, HideUI)
		SLATE_ARGUMENT(bool, AllowZoomOutput)
		SLATE_ARGUMENT(bool, AlwaysDisplayColorCurves)
		SLATE_ARGUMENT(bool, ZoomToFitVertical)
		SLATE_ARGUMENT(bool, ZoomToFitHorizontal)
		SLATE_ARGUMENT(bool, ShowZoomButtons)
		SLATE_ARGUMENT(TOptional<FString>, XAxisName)
		SLATE_ARGUMENT(TOptional<FString>, YAxisName)
		SLATE_ARGUMENT(bool, ShowInputGridNumbers)
		SLATE_ARGUMENT(bool, ShowOutputGridNumbers)
		SLATE_ARGUMENT(bool, ShowCurveSelector)
		SLATE_ARGUMENT(FLinearColor, GridColor)
		SLATE_EVENT(FOnSetInputViewRange, OnSetInputViewRange)
		SLATE_EVENT(FOnSetOutputViewRange, OnSetOutputViewRange)
		SLATE_EVENT(FOnSetAreCurvesVisible, OnSetAreCurvesVisible)
		SLATE_EVENT(FSimpleDelegate, OnCreateAsset)

	SLATE_END_ARGS()

public:

	/** Widget construction. **/
	void Construct(const FArguments& InArgs);

	~SHoudiniFloatRampCurveEditor();

	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Refreshes rich curve keys using info from ramp view. */
	void RefreshCurveKeys() override;

protected:

	TOptional<int32> GetNumCurveKeys() const override;
	TOptional<float> GetCurveKeyPosition(const int32 Index) const override;
	TOptional<ValueType> GetCurveKeyValue(const int32 Index) const override;
	TOptional<ERichCurveInterpMode> GetCurveKeyInterpolationType(const int32 Index) const override;

private:

	// We only want to commit curve editor value to the parameter on mouse release. OnUpdateCurve
	// gets called continuously as we drag a point, so we use this value to determine if we should
	// notify the ramp view of a change.
	bool bIsMouseButtonDown = false;

	// Unreal representation of the curve which we display. Typically, this curve object is owned by
	// some object, however we only use it for the purpose of using Unreal's curve editor widget.
	// In order to avoid being garbage collected, we add the curve to root. Thus we need to remove 
	// the curve from root when the widget is destroyed.
	UCurveFloat* Curve;

	FDelegateHandle OnUpdateCurveDelegateHandle;

	void OnUpdateCurve(UCurveBase*, EPropertyChangeType::Type);
};


class SHoudiniFloatRamp
	: public SHoudiniRampBase<FHoudiniFloatRampView, SHoudiniFloatRampCurveEditor>
{
protected:

	/** Gets the column header label for value type of the ramp. */
	FORCEINLINE FString GetValueString() const override
	{
		return TEXT("Value");
	}

	TSharedRef<SWidget> ConstructRampPointValueWidget(const int32 Index) override;
};
