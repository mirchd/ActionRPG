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

#include "SHoudiniColorRamp.h"

#include "HoudiniParameterRamp.h"
#include "HoudiniParameterFloat.h"
#include "HoudiniParameterColor.h"
#include "HoudiniParameterChoice.h"
#include "Editor/CurveEditor/Public/CurveEditorSettings.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"

#include "HoudiniEngine.h"
#include "HoudiniEngineDetails.h"
#include "HoudiniEngineUtils.h"

#define LOCTEXT_NAMESPACE HOUDINI_LOCTEXT_NAMESPACE

void
SHoudiniColorRampCurveEditor::Construct(const FArguments& InArgs)
{
	Curve = NewObject<UCurveLinearColor>(
		GetTransientPackage(),
		UCurveLinearColor::StaticClass(),
		NAME_None,
		RF_Transactional | RF_Public);

	if (!Curve)
	{
		return;
	}

	// Add the ramp curve to root to avoid garbage collected
	Curve->AddToRoot();

	OnUpdateCurveDelegateHandle = Curve->OnUpdateCurve.AddRaw(
		this, &SHoudiniColorRampCurveEditor::OnUpdateCurve);

	RampView = InArgs._RampView;
	OnCurveChangedDelegate = InArgs._OnCurveChanged;

	SColorGradientEditor::Construct(
		SColorGradientEditor::FArguments()
		.ViewMinInput(InArgs._ViewMinInput)
		.ViewMaxInput(InArgs._ViewMaxInput));

	// Avoid showing tooltips inside of the curve editor
	EnableToolTipForceField(true);

	SetCurveOwner(Curve);

	RefreshCurveKeys();
}

SHoudiniColorRampCurveEditor::~SHoudiniColorRampCurveEditor()
{
	if (Curve)
	{
		SetCurveOwner(nullptr);

		Curve->OnUpdateCurve.Remove(OnUpdateCurveDelegateHandle);

		// Remove the ramp curve to root so it can be garbage collected
		Curve->RemoveFromRoot();
	}
}

FReply
SHoudiniColorRampCurveEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsMouseButtonDown = true;
	return SColorGradientEditor::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply
SHoudiniColorRampCurveEditor::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsMouseButtonDown = false;

	OnCurveChanged();

	return SColorGradientEditor::OnMouseButtonUp(MyGeometry, MouseEvent);
}

void
SHoudiniColorRampCurveEditor::OnUpdateCurve(UCurveBase*, EPropertyChangeType::Type)
{
	if (bIsMouseButtonDown)
	{
		return;
	}

	OnCurveChanged();
}

void
SHoudiniColorRampCurveEditor::RefreshCurveKeys()
{
	if (!RampView.IsValid())
	{
		return;
	}

	if (!Curve)
	{
		return;
	}

	TArrayView<FRichCurve> FloatCurves(Curve->FloatCurves, 4);

	const int32 PointCount = RampView->GetPointCount();
	for (int32 CurveIdx = 0; CurveIdx < 4; ++CurveIdx)
	{
		FRichCurve& RichCurve = FloatCurves[CurveIdx];
		RichCurve.Reset();

		for (int32 i = 0; i < PointCount; ++i)
		{
			ERichCurveInterpMode RichCurveInterpMode =
				UHoudiniParameter::EHoudiniRampInterpolationTypeToERichCurveInterpMode(
					RampView->GetRampPointInterpolationType(i).GetValue());

			const FKeyHandle KeyHandle = RichCurve.AddKey(
				RampView->GetRampPointPosition(i).GetValue(), 
				RampView->GetRampPointValue(i).GetValue().Component(CurveIdx));
			RichCurve.SetKeyInterpMode(KeyHandle, RichCurveInterpMode);
		}
	}
}

TOptional<int32>
SHoudiniColorRampCurveEditor::GetNumCurveKeys() const
{
	if (!Curve)
	{
		return {};
	}

	return Curve->FloatCurves[0].GetNumKeys();
}

TOptional<float>
SHoudiniColorRampCurveEditor::GetCurveKeyPosition(const int32 Index) const
{
	if (!Curve)
	{
		return {};
	}

	if (!Curve->FloatCurves[0].Keys.IsValidIndex(Index))
	{
		return {};
	}

	return Curve->FloatCurves[0].Keys[Index].Time;
}

TOptional<FLinearColor>
SHoudiniColorRampCurveEditor::GetCurveKeyValue(const int32 Index) const
{
	if (!Curve)
	{
		return {};
	}

	if (!Curve->FloatCurves[0].Keys.IsValidIndex(Index))
	{
		return {};
	}

	return FLinearColor(
		Curve->FloatCurves[0].Keys[Index].Value,
		Curve->FloatCurves[1].Keys[Index].Value,
		Curve->FloatCurves[2].Keys[Index].Value);
}

TOptional<ERichCurveInterpMode>
SHoudiniColorRampCurveEditor::GetCurveKeyInterpolationType(const int32 Index) const
{
	if (!Curve)
	{
		return {};
	}

	if (!Curve->FloatCurves[0].Keys.IsValidIndex(Index))
	{
		return {};
	}

	return Curve->FloatCurves[0].Keys[Index].InterpMode.GetValue();
}

TSharedRef<SWidget>
SHoudiniColorRamp::ConstructRampPointValueWidget(const int32 Index)
{
	if (!RampView.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SColorBlock)
		.Color(RampView->GetRampPointValue(Index).Get(FLinearColor::Black))
		.OnMouseButtonDown(FPointerEventHandler::CreateLambda(
			[this, Index](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
			{
				if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
					return FReply::Unhandled();

				FColorPickerArgs PickerArgs;
				PickerArgs.bUseAlpha = true;
				PickerArgs.DisplayGamma = TAttribute<float>::Create(
					TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
				PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateLambda(
					[this, Index](FLinearColor InColor)
					{
						if (OnPointValueCommit(Index, InColor))
						{
							OnValueCommitted.ExecuteIfBound();
						}
					});
				FLinearColor InitColor = RampView->GetRampPointValue(Index).Get(FLinearColor::Black);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
				PickerArgs.InitialColor = InitColor;
#else
				PickerArgs.InitialColorOverride = InitColor;
#endif
				PickerArgs.bOnlyRefreshOnOk = true;
				OpenColorPicker(PickerArgs);
				return FReply::Handled();
			}));
}

#undef LOCTEXT_NAMESPACE
