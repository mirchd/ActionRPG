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

#include "HoudiniEngine.h"
#include "HoudiniEngineDetails.h"
#include "HoudiniEngineEditor.h"
#include "HoudiniEngineRuntimeCommon.h"
#include "HoudiniEngineUtils.h"

#include "HoudiniParameterRamp.h"

#include "HoudiniRampView.h"
#include "HoudiniRampEditorWindowSubsystem.h"

#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "PropertyCustomizationHelpers.h"
#include "HoudiniEngineEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE HOUDINI_LOCTEXT_NAMESPACE

class IHoudiniRampCurveEditor
{
public:

	/** Called when a ramp editor widget or window is being refreshed. */
	virtual void RefreshCurveKeys() = 0;
};

template<typename Base, typename InRampViewType>
class SHoudiniRampCurveEditorBase : public Base, public IHoudiniRampCurveEditor
{
public:

	using RampViewType = InRampViewType;
	using ValueType = typename RampViewType::ValueType;
	using ParameterType = typename RampViewType::ParameterType;
	using ParameterWeakPtr = typename RampViewType::ParameterWeakPtr;
	using PointType = typename RampViewType::PointType;

	DECLARE_DELEGATE(FOnCurveChanged);

protected:

	TSharedPtr<RampViewType> RampView;

	/** Delegate to call when the curve is changed */
	FOnCurveChanged OnCurveChangedDelegate;

	/** 
	 * Get data about the curve used by the editor.
	 *
	 * @return Unset optional is only returned if and only if the curve is invalid or if the index
	 *         is out of bounds.
	 */
	virtual TOptional<int32> GetNumCurveKeys() const = 0;
	virtual TOptional<float> GetCurveKeyPosition(const int32 Index) const = 0;
	virtual TOptional<ValueType> GetCurveKeyValue(const int32 Index) const = 0;
	virtual TOptional<ERichCurveInterpMode> GetCurveKeyInterpolationType(
		const int32 Index) const = 0;

	void OnCurveChanged()
	{
		if (!RampView.IsValid())
		{
			return;
		}

		const int32 NumPoints = RampView->GetPointCount();
		const TOptional<int32> NumCurveKeys = GetNumCurveKeys();

		if (!NumCurveKeys.IsSet())
		{
			return;
		}

		// Note! The EPropertyChangeType is (unintuitively) always EPropertyChangeType::ValueSet
		const bool bIsAddingPoints = NumCurveKeys.GetValue() > NumPoints;
		const bool bIsDeletingPoints = NumCurveKeys.GetValue() < NumPoints;

		TArray<int32> ModifiedIndices;
		TArray<TOptional<float>> NewPositions;
		TArray<TOptional<ValueType>> NewValues;
		TArray<TOptional<EHoudiniRampInterpolationType>> NewInterpolationTypes;

		// Find what points were changed...
		{
			int32 PointIndex = 0;
			int32 CurveIndex = 0;

			while (PointIndex < NumPoints && CurveIndex < NumCurveKeys.GetValue())
			{
				const float PointPosition = RampView->GetRampPointPosition(PointIndex).GetValue();
				const ValueType PointValue = RampView->GetRampPointValue(PointIndex).GetValue();
				const EHoudiniRampInterpolationType PointInterpolationType =
					RampView->GetRampPointInterpolationType(PointIndex).GetValue();

				const float CurvePosition = GetCurveKeyPosition(CurveIndex).GetValue();
				const ValueType CurveValue = GetCurveKeyValue(CurveIndex).GetValue();
				const ERichCurveInterpMode CurveInterpolationType =
					GetCurveKeyInterpolationType(CurveIndex).GetValue();

				const bool bIsInterpolationEquivalent =
					IsInterpolationEquivalent(CurveInterpolationType, PointInterpolationType);

				if (CurvePosition == PointPosition
					&& CurveValue == PointValue
					&& bIsInterpolationEquivalent)
				{
					++PointIndex;
					++CurveIndex;
				}
				else // We found a difference!
				{
					if (bIsAddingPoints)
					{
						ModifiedIndices.Add(PointIndex);
						NewPositions.Add(CurvePosition);
						NewValues.Add(CurveValue);
						NewInterpolationTypes.Add(TranslateInterpolation(CurveInterpolationType));
						++CurveIndex;
					}
					else if (bIsDeletingPoints)
					{
						ModifiedIndices.Add(PointIndex);
						++PointIndex;
					}
					else
					{
						ModifiedIndices.Add(PointIndex);
						NewPositions.Add(CurvePosition);
						NewValues.Add(CurveValue);

						// Interpolation is a special case - since Unreal and Houdini interpolation
						// types are different, we do our best to convert the Unreal type to the
						// Houdini type. But, in the case that we consider the two equivalent, to
						// prevent loss of current setting, we re-use the old interpolation type.
						if (bIsInterpolationEquivalent)
						{
							NewInterpolationTypes.Add(PointInterpolationType);
						}
						else
						{
							NewInterpolationTypes.Add(
								TranslateInterpolation(CurveInterpolationType));
						}

						++CurveIndex;
						++PointIndex;
					}
				}
			}

			// We have more points than curve keys (as a result of deletion)
			while (PointIndex < NumPoints)
			{
				ModifiedIndices.Add(PointIndex);

				++PointIndex;
			}

			// We have more curve keys than points (as a result of insertion)
			while (CurveIndex < NumCurveKeys.GetValue())
			{
				ModifiedIndices.Add(CurveIndex);
				NewPositions.Add(GetCurveKeyPosition(CurveIndex).GetValue());
				NewValues.Add(GetCurveKeyValue(CurveIndex).GetValue());
				NewInterpolationTypes.Add(EHoudiniRampInterpolationType::LINEAR);

				++CurveIndex;
			}
		}

		if (bIsAddingPoints)
		{
			if (!ModifiedIndices.IsEmpty()
				&& !NewPositions.IsEmpty()
				&& !NewValues.IsEmpty()
				&& !NewInterpolationTypes.IsEmpty())
			{
				if (NewPositions[0].IsSet()
					&& NewValues[0].IsSet()
					&& NewInterpolationTypes[0].IsSet())
				{
					RampView->InsertRampPoint(
						ModifiedIndices[0],
						NewPositions[0].GetValue(),
						NewValues[0].GetValue(),
						NewInterpolationTypes[0].GetValue());
				}
			}
		}
		else if (bIsDeletingPoints)
		{
			RampView->DeleteRampPoints(ModifiedIndices);
		}
		else
		{
			RampView->SetRampPoints(
				ModifiedIndices, NewPositions, NewValues, NewInterpolationTypes);
		}

		OnCurveChangedDelegate.ExecuteIfBound();
	}

private:

	/** 
	 * Since there are fewer Unreal interpolation types than Houdini interpolation types, we map
	 * multiple Houdini interpolation types to Unreal's cubic interpolation type. Use this to check
	 * if we consider the two equivalent.
	 */
	static bool 
	IsInterpolationEquivalent(
		const ERichCurveInterpMode UnrealInterpolation,
		const EHoudiniRampInterpolationType HoudiniInterpolation)
	{
		switch (UnrealInterpolation)
		{
		case RCIM_Linear:
			return HoudiniInterpolation == EHoudiniRampInterpolationType::LINEAR;
		case RCIM_Constant:
			return HoudiniInterpolation == EHoudiniRampInterpolationType::CONSTANT;
		case RCIM_Cubic:
			switch (HoudiniInterpolation)
			{
			case EHoudiniRampInterpolationType::BEZIER:
			case EHoudiniRampInterpolationType::BSPLINE:
			case EHoudiniRampInterpolationType::CATMULL_ROM:
			case EHoudiniRampInterpolationType::HERMITE:
			case EHoudiniRampInterpolationType::MONOTONE_CUBIC:
				return true;
			default:
				return false;
			}
		case RCIM_None:
		default:
			return HoudiniInterpolation == EHoudiniRampInterpolationType::InValid;
		}
	}

	/** Converts Unreal interpolation type to the most appropriate Houdini interpolation type. */
	static EHoudiniRampInterpolationType
	TranslateInterpolation(const ERichCurveInterpMode InterpMode)
	{
		switch (InterpMode)
		{
		case RCIM_Linear:
			return EHoudiniRampInterpolationType::LINEAR;
		case RCIM_Constant:
			return EHoudiniRampInterpolationType::CONSTANT;
		case RCIM_Cubic:
			return EHoudiniRampInterpolationType::CATMULL_ROM;
		case RCIM_None:
		default:
			return EHoudiniRampInterpolationType::InValid;
		}
	}
};

/** Common widget elements for all types of ramps. */
template<typename InRampViewType, typename InCurveEditorWidgetType>
class SHoudiniRampBase : public SCompoundWidget
{
public:

	using RampViewType = InRampViewType;
	using ValueType = typename RampViewType::ValueType;
	using CurveEditorWidgetType = InCurveEditorWidgetType;

	DECLARE_DELEGATE(FOnValueCommitted);

	SLATE_BEGIN_ARGS(SHoudiniRampBase)
		{}

		SLATE_ARGUMENT(TArrayView<const typename RampViewType::ParameterWeakPtr>, RampParameters)

		/** Called when the ramp is modified using the UI from outside the ramp editor. */
		SLATE_EVENT(FOnValueCommitted, OnValueCommitted)

	SLATE_END_ARGS()

	virtual void Construct(const FArguments& InArgs)
	{
		OnValueCommitted = InArgs._OnValueCommitted;

		RampView = MakeShared<RampViewType>(InArgs._RampParameters);

		ChildSlot
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(2, 2, 5, 2)
				[
					SNew(SBorder)
						.VAlign(VAlign_Fill)
						[
							SAssignNew(CurveEditor, CurveEditorWidgetType)
								.RampView(RampView)
								.OnCurveChanged_Lambda(
									[this]() { OnValueCommitted.ExecuteIfBound(); })
						]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Top)
				[
					ConstructOpenInNewWindowButton()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				ConstructRampPoints()
			]
		];
	}

protected:

	/** Delegate to call when the value is committed */
	FOnValueCommitted OnValueCommitted;

	TSharedPtr<RampViewType> RampView;

	TSharedPtr<CurveEditorWidgetType> CurveEditor;

	/** Called when a point's position is modified using the UI from outside the curve editor. */
	virtual bool OnPointPositionCommit(const int32 Index, const float NewPosition)
	{
		if (!RampView.IsValid())
		{
			return false;
		}

		return RampView->SetRampPointPosition(Index, NewPosition);
	}

	/** Called when a point's value is modified using the UI from outside the curve editor. */
	virtual bool OnPointValueCommit(const int32 Index, const ValueType NewValue)
	{
		if (!RampView.IsValid())
		{
			return false;
		}

		return RampView->SetRampPointValue(Index, NewValue);
	}

	/** 
	 * Called when a point's interpolation type is modified using the UI from outside the curve 
	 * editor. 
	 */
	virtual bool OnPointInterpolationTypeCommit(
		const int32 Index, const EHoudiniRampInterpolationType NewInterpolationType)
	{
		if (!RampView.IsValid())
		{
			return false;
		}

		return RampView->SetRampPointInterpolationType(Index, NewInterpolationType);
	}

	/** Gets the column header label for value type of the ramp. */
	virtual FString GetValueString() const = 0;

	virtual TSharedRef<SWidget> ConstructRampPointValueWidget(const int32 Index) = 0;

	/** Button to call @ref OpenNewWindow. */
	TSharedRef<SWidget> ConstructOpenInNewWindowButton()
	{
		return SNew(SButton)
			.ButtonStyle(_GetEditorStyle(), "SimpleButton")
			.ToolTipText(LOCTEXT("OpenInNewWindow", "Open In New Window"))
			.OnClicked(FOnClicked::CreateLambda(
				[this]() -> FReply
				{
					OpenNewWindow();
					return FReply::Handled();
				}))
			[
				SNew(SImage)
					.Image(_GetEditorStyle().GetBrush("Icons.OpenInExternalEditor"))
			];
	}

	/** Opens the curve editor in a pop-out window. */
	virtual bool OpenNewWindow()
	{
		if (GEditor)
		{
			if (auto RampEditorWindowSubsystem =
				GEditor->GetEditorSubsystem<UHoudiniRampEditorWindowSubsystem>())
			{
				// We want a copy of the OnValueCommitted delegate inside the lambda as the created
				// window (and the lambda it will own) can outlive this widget's lifetime.
				return RampEditorWindowSubsystem->OpenEditor<CurveEditorWidgetType>(
					RampView,
					AsShared(),
					UHoudiniRampEditorWindowSubsystem::FOnValueCommitted::CreateLambda(
						[OnValueCommitted = OnValueCommitted]()
						{
							OnValueCommitted.ExecuteIfBound();
						}));
			}
		}
		return false;
	}

	/** Create the UI for the ramp's stop points. */
	TSharedRef<SWidget> ConstructRampPoints()
	{
		const bool bCookingEnabled = FHoudiniEngine::Get().IsCookingEnabled();

		TSharedRef<SUniformGridPanel> GridPanel = SNew(SUniformGridPanel);

		int32 RowIndex = 0;

		GridPanel->SetSlotPadding(FMargin(2.f, 2.f, 5.f, 3.f));
		GridPanel->AddSlot(0, RowIndex)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Position")))
			.Font(_GetEditorStyle().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
		];

		GridPanel->AddSlot(1, RowIndex)
		[
			SNew(STextBlock)
			.Text(FText::FromString(GetValueString()))
			.Font(_GetEditorStyle().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
		];

		GridPanel->AddSlot(2, RowIndex)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Interp.")))
			.Font(_GetEditorStyle().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
		];

		GridPanel->AddSlot(3, RowIndex)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(3.f, 0.f)
			.MaxWidth(35.f)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeAddButton(
					FSimpleDelegate::CreateLambda(
						[this]()
						{
							if (!RampView.IsValid())
							{
								return;
							}
							if (RampView->InsertRampPoint(RampView->GetPointCount()))
							{
								OnValueCommitted.ExecuteIfBound();
							}
						}),
					LOCTEXT("AddRampPoint", "Add a ramp point to the end"), true)
			]

			+ SHorizontalBox::Slot()
			.Padding(3.f, 0.f)
			.MaxWidth(35.f)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeRemoveButton(
					FSimpleDelegate::CreateLambda(
						[this]()
						{
							if (!RampView.IsValid())
							{
								return;
							}
							if (RampView->DeleteRampPoint(-1))
							{
								OnValueCommitted.ExecuteIfBound();
							}
						}),
					LOCTEXT("DeleteRampPoint", "Delete the last ramp point"), true)
			]
		];

		int32 PointCount = RampView.IsValid() ? RampView->GetPointCount() : 0;
		for (int32 Index = 0; Index < PointCount; ++Index)
		{
			++RowIndex;

			float CurPos = RampView.IsValid() ? RampView->GetRampPointPosition(Index).Get(0.f) : 0.f;

			GridPanel->AddSlot(0, RowIndex)
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.Font(_GetEditorStyle().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
				.Value(CurPos)
				.OnValueChanged_Lambda([](float Val) {})
				.OnValueCommitted_Lambda(
					[this, Index](float Val, ETextCommit::Type TextCommitType)
					{
						// For some reason I can't figure out, UE sends a second commit event with
						// type Default, which has the old value, causing the first commit to be
						// reset. So ignore it.
						if (TextCommitType == ETextCommit::Type::Default)
							return;

						if (OnPointPositionCommit(Index, Val))
						{
							OnValueCommitted.ExecuteIfBound();
						}
					})
				.OnBeginSliderMovement_Lambda([](){})
				.OnEndSliderMovement_Lambda(
					[this, Index](const float Val)
					{
						if (OnPointPositionCommit(Index, Val))
						{
							OnValueCommitted.ExecuteIfBound();
						}
					})
				.SliderExponent(1.0f)
			];

			GridPanel->AddSlot(1, RowIndex)
			[
				ConstructRampPointValueWidget(Index)
			];

			int32 CurChoice = RampView.IsValid() 
				? static_cast<int32>(RampView->GetRampPointInterpolationType(Index).Get(
					EHoudiniRampInterpolationType::InValid))
				: 0;

			TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBoxCurveMethod;
			GridPanel->AddSlot(2, RowIndex)
			[
				SAssignNew(ComboBoxCurveMethod, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(
					FHoudiniEngineEditor::Get().GetHoudiniParameterRampInterpolationMethodLabels())
				.InitiallySelectedItem(
					(*FHoudiniEngineEditor::Get().GetHoudiniParameterRampInterpolationMethodLabels())[CurChoice])
				.OnGenerateWidget_Lambda(
					[](TSharedPtr<FString> ChoiceEntry)
					{
						FText ChoiceEntryText = FText::FromString(*ChoiceEntry);
						return SNew(STextBlock)
							.Text(ChoiceEntryText)
							.ToolTipText(ChoiceEntryText)
							.Font(_GetEditorStyle().GetFontStyle(
								TEXT("PropertyWindow.NormalFont")));
					})
				.OnSelectionChanged_Lambda(
					[this, Index](TSharedPtr<FString> NewChoice, ESelectInfo::Type SelectType) 
					{
						EHoudiniRampInterpolationType NewInterpType =
							UHoudiniParameter::GetHoudiniInterpMethodFromString(*NewChoice.Get());

						if (OnPointInterpolationTypeCommit(Index, NewInterpType))
						{
							OnValueCommitted.ExecuteIfBound();
						}
					})
				[
					SNew(STextBlock)
					.Text_Lambda(
						[this, Index]()
						{
							const auto CurInterpType = RampView.IsValid()
								? RampView->GetRampPointInterpolationType(Index).Get(
									EHoudiniRampInterpolationType::InValid)
								: EHoudiniRampInterpolationType::InValid;

							return FText::FromString(
								UHoudiniParameter::GetStringFromHoudiniInterpMethod(CurInterpType));
						})
					.Font(_GetEditorStyle().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
				]
			];

			GridPanel->AddSlot(3, RowIndex)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(3.f, 0.f)
				.MaxWidth(35.f)
				.AutoWidth()
				[
					PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateLambda(
						[this, Index]()
						{
							if (!RampView.IsValid())
							{
								return;
							}
							if (RampView->InsertRampPoint(Index))
							{
								OnValueCommitted.ExecuteIfBound();
							}
						}),
						LOCTEXT("AddRampPoint", "Add a ramp point before this point"), true)
				]
			
				+ SHorizontalBox::Slot()
				.Padding(3.f, 0.f)
				.MaxWidth(35.f)
				.AutoWidth()
				[
					PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateLambda(
						[this, Index]()
						{
							if (!RampView.IsValid())
							{
								return;
							}
							if (RampView->DeleteRampPoint(Index))
							{
								OnValueCommitted.ExecuteIfBound();
							}
						}),
						LOCTEXT("DeleteRampPoint", "Delete this ramp point"), true)
				]
			];
		}
		return GridPanel;
	}
};

#undef LOCTEXT_NAMESPACE
