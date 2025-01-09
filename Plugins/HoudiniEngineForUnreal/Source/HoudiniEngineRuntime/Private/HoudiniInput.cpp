/*
* Copyright (c) <2021> Side Effects Software Inc.
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

#include "HoudiniInput.h"

#include "HoudiniEngineRuntime.h"
#include "HoudiniEngineRuntimePrivatePCH.h"
#include "HoudiniAssetComponent.h"
#include "HoudiniOutput.h"
#include "HoudiniSplineComponent.h"
#include "HoudiniAsset.h"
#include "HoudiniGeoPartObject.h"
#include "HoudiniAssetComponent.h"
#include "HoudiniAssetBlueprintComponent.h"
#include "UnrealObjectInputRuntimeTypes.h"
#include "UnrealObjectInputManager.h"
#include "UnrealObjectInputRuntimeUtils.h"
#include "LandscapeSplineActor.h"

#include "EngineUtils.h"
#include "Engine/Brush.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "Model.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UObjectGlobals.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	#include "GeometryCollection/GeometryCollectionActor.h"
	#include "GeometryCollection/GeometryCollectionComponent.h"
	#include "GeometryCollection/GeometryCollectionObject.h"
#else
	#include "GeometryCollectionEngine/Public/GeometryCollection/GeometryCollectionActor.h"
	#include "GeometryCollectionEngine/Public/GeometryCollection/GeometryCollectionComponent.h"
	#include "GeometryCollectionEngine/Public/GeometryCollection/GeometryCollectionObject.h"	
#endif


#if WITH_EDITOR
	#include "Kismet2/BlueprintEditorUtils.h"
	#include "Kismet2/KismetEditorUtilities.h"
#endif

//
UHoudiniInput::UHoudiniInput()
	: Type(EHoudiniInputType::Invalid)
	, PreviousType(EHoudiniInputType::Invalid)
	, AssetNodeId(-1)
	, InputNodeId(-1)
	, InputIndex(0)
	, ParmId(-1)
	, bIsObjectPathParameter(false)
	, bHasChanged(false)
	, bPackBeforeMerge(false)
	, bDirectlyConnectHdas(true)
	, bExportOptionsMenuExpanded(false)
	, bGeometryInputsMenuExpanded(true)
	, bLandscapeOptionsMenuExpanded(false)
	, bWorldInputsMenuExpanded(true)
	, bCurveInputsMenuExpanded(true)
	, bCurvePointSelectionMenuExpanded(true)
	, bCurvePointSelectionUseAbsLocation(false)
	, bCurvePointSelectionUseAbsRotation(false)
	, bCookOnCurveChanged(true)
	, bStaticMeshChanged(false)
	, bInputAssetConnectedInHoudini(false)
	, DefaultCurveOffset(0.f)
	, bIsWorldInputBoundSelector(false)
	, bWorldInputBoundSelectorAutoUpdate(false)
	, bCanDeleteHoudiniNodes(true)
{
	Name = TEXT("");
	Label = TEXT("");
	SetFlags(RF_Transactional);

	// Geometry inputs always have one null default object
	GeometryInputObjects.Add(nullptr);

	InputSettings.KeepWorldTransform = GetDefaultXTransformType();

#if WITH_EDITORONLY_DATA
	bLandscapeUIAdvancedIsExpanded = false;
#endif
	
	CachedBounds.Init();
}

void
UHoudiniInput::BeginDestroy()
{
	InvalidateData();

	// DO NOT MANUALLY DESTROY OUR INPUT OBJECTS!
	// This messes up unreal's Garbage collection and would cause crashes on duplication

	// Mark all our input objects for destruction
	ForAllHoudiniInputObjectArrays([](TArray<TObjectPtr<UHoudiniInputObject>>& ObjectArray) {
		ObjectArray.Empty();
	});

	Super::BeginDestroy();
}

#if WITH_EDITOR
void UHoudiniInput::PostEditUndo() 
{
	 Super::PostEditUndo();
	 TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(Type);
	 if (!InputObjectsPtr)
		 return;

	 MarkChanged(true);
	 bool bBlueprintStructureChanged = false;

	 if (HasInputTypeChanged()) 
	 {
		 // If the input type has changed on undo, previousType becomes new type
		 /*   This does not work properly, see the corresponding part in FHoudiniInputDetails::AddInputTypeComboBox(...), after Transaction(...
		 )
		 {
			 EHoudiniInputType NewType = PreviousType;
			 SetInputType(NewType);
		 }
		 */
		 EHoudiniInputType Temp = Type;
		 Type = PreviousType;
		 PreviousType = EHoudiniInputType::Invalid;

		 // If the undo action caused input type changing, treat it as a regular type changing
		 // after set up the new and prev types properly 
		 SetInputType(Temp, bBlueprintStructureChanged);
	 }
	 else
	 {
		 if (Type == EHoudiniInputType::World)
		 {
		 	if (WorldInputObjects.Num() == 0 && InputNodeId >= 0)
		 	{
		 		// If the ref counted input system is being used, we must not delete nodes managed by the system
				TSet<int32> ManagedNodeIds;
				{
					IUnrealObjectInputManager const* const Manager = FUnrealObjectInputManager::Get();
					if (Manager)
					{
						TArray<int32> ManagedNodeIdArray;
						if (Manager->GetAllHAPINodeIds(ManagedNodeIdArray))
							ManagedNodeIds.Append(ManagedNodeIdArray);
					}
				}

			 	for (auto & NextNodeId : CreatedDataNodeIds)
			 	{
			 		if (ManagedNodeIds.Contains(NextNodeId))
			 			continue;
			 		
			 		if (bCanDeleteHoudiniNodes)
			 			FHoudiniEngineRuntime::Get().MarkNodeIdAsPendingDelete(NextNodeId, true);
			 	}

			 	CreatedDataNodeIds.Empty();

			 	if (bCanDeleteHoudiniNodes)
			 		MarkInputNodeAsPendingDelete();
			 	InputNodeId = -1;
			}
		 }

		 if (Type == EHoudiniInputType::Curve)
		 {
			 if (PreviousType != EHoudiniInputType::Curve)
			 {
				 for (auto& NextInput : *GetHoudiniInputObjectArray(Type))
				 {
					 UHoudiniInputHoudiniSplineComponent* SplineInput = Cast< UHoudiniInputHoudiniSplineComponent>(NextInput);
					 if (!IsValid(SplineInput))
						 continue;

					 UHoudiniSplineComponent * HoudiniSplineComponent = SplineInput->GetCurveComponent();
					 if (!IsValid(HoudiniSplineComponent))
						 continue;

					 USceneComponent* OuterComponent = Cast<USceneComponent>(GetOuter());

					 // Attach the new Houdini spline component to it's owner
					 HoudiniSplineComponent->RegisterComponent();
					 HoudiniSplineComponent->AttachToComponent(OuterComponent, FAttachmentTransformRules::KeepRelativeTransform);
					 HoudiniSplineComponent->SetVisibility(true, true);
					 HoudiniSplineComponent->SetHoudiniSplineVisible(true);
					 HoudiniSplineComponent->SetHiddenInGame(false, true);
					 HoudiniSplineComponent->MarkChanged(true);
				 }
				 return;
			 }
			 bool bUndoDelete = false;
			 bool bUndoInsert = false;
			 bool bUndoDeletedObjArrayEmptied = false;

			 TArray< USceneComponent* > childActor;
			 UHoudiniAssetComponent* OuterHAC = Cast<UHoudiniAssetComponent>(GetOuter());
			 if (IsValid(OuterHAC))
				 childActor = OuterHAC->GetAttachChildren();

			 // Undo delete input objects action
			 for (int Index = 0; Index < GetNumberOfInputObjects(); ++Index)
			 {
				 UHoudiniInputObject* InputObject = (*InputObjectsPtr)[Index];
				 if (!IsValid(InputObject))
					 continue;

				 UHoudiniInputHoudiniSplineComponent * HoudiniSplineInputObject = Cast<UHoudiniInputHoudiniSplineComponent>(InputObject);

				 if (!IsValid(HoudiniSplineInputObject))
					 continue;

				 UHoudiniSplineComponent* SplineComponent = HoudiniSplineInputObject->GetCurveComponent();

				 if (!IsValid(SplineComponent))
					 continue;

				 // If the last change deleted this curve input, recreate this Houdini Spline input.
				 if (!SplineComponent->GetAttachParent())
				 {
					 bUndoDelete = true;

					 if (!bUndoDeletedObjArrayEmptied)  
						 LastUndoDeletedInputs.Empty();

					 bUndoDeletedObjArrayEmptied = true;

					 UHoudiniSplineComponent * ReconstructedSpline = NewObject<UHoudiniSplineComponent>(
						 GetOuter(), UHoudiniSplineComponent::StaticClass());

					 if (!IsValid(ReconstructedSpline))
						 continue;

					 ReconstructedSpline->SetFlags(RF_Transactional);
					 ReconstructedSpline->CopyHoudiniData(SplineComponent);

					 UHoudiniInputObject * ReconstructedInputObject = UHoudiniInputHoudiniSplineComponent::Create(
						 ReconstructedSpline, GetOuter(), ReconstructedSpline->GetHoudiniSplineName(), InputSettings);
					 UHoudiniInputHoudiniSplineComponent *ReconstructedHoudiniSplineInput = (UHoudiniInputHoudiniSplineComponent*)ReconstructedInputObject;
					 (*InputObjectsPtr)[Index] = ReconstructedHoudiniSplineInput;

					 ReconstructedSpline->RegisterComponent();
					 ReconstructedSpline->SetFlags(RF_Transactional);

					 CreateHoudiniSplineInput(ReconstructedHoudiniSplineInput, true, true, bBlueprintStructureChanged);

					 // Cast the reconstructed Houdini Spline Input to a generic HoudiniInput object.
					 UHoudiniInputObject * ReconstructedHoudiniInput = Cast<UHoudiniInputObject>(ReconstructedHoudiniSplineInput);

					 LastUndoDeletedInputs.Add(ReconstructedHoudiniInput);
					 // Reset the LastInsertedInputsArray for redoing this undo action.
				 }
			 }

			 if (bUndoDelete)
				 return;

			 // Undo insert input objects action
			 for (int Index = 0; Index < LastInsertedInputs.Num(); ++Index)
			 {
				 bUndoInsert = true;
				 UHoudiniInputHoudiniSplineComponent* SplineInputComponent = LastInsertedInputs[Index];
				 if (!IsValid(SplineInputComponent)) 
					 continue;

				 UHoudiniSplineComponent* HoudiniSplineComponent = SplineInputComponent->GetCurveComponent();
				 if (!IsValid(HoudiniSplineComponent)) 
					 continue;

				 HoudiniSplineComponent->DestroyComponent();
			 }

			 if (bUndoInsert)
				 return;

			 for (int Index = 0; Index < LastUndoDeletedInputs.Num(); ++Index)
			 {
				 UHoudiniInputObject* NextInputObject = LastUndoDeletedInputs[Index];

				 UHoudiniInputHoudiniSplineComponent* SplineInputComponent = Cast<UHoudiniInputHoudiniSplineComponent>(NextInputObject);
				 if (!IsValid(SplineInputComponent))
					 continue;

				 UHoudiniSplineComponent* HoudiniSplineComponent = SplineInputComponent->GetCurveComponent();
				 if (!IsValid(HoudiniSplineComponent))
					 continue;

				 FDetachmentTransformRules DetachTransRules(EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, false);
				 HoudiniSplineComponent->DetachFromComponent(DetachTransRules);

				 HoudiniSplineComponent->DestroyComponent();
			 }
		 }
	 }

	 if (bBlueprintStructureChanged)
	 {
		 UHoudiniAssetComponent* OuterHAC = Cast<UHoudiniAssetComponent>(GetOuter());
		 FHoudiniEngineRuntimeUtils::MarkBlueprintAsStructurallyModified(OuterHAC);
	 }

}
#endif

void
UHoudiniInput::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FHoudiniCustomSerializationVersion::GUID);
	if (Ar.CustomVer(FHoudiniCustomSerializationVersion::GUID) < VER_HOUDINI_PLUGIN_SERIALIZATION_VERSION_INPUT_OBJECT_SETTINGS_STRUCT)
	{
		HOUDINI_LOG_WARNING(TEXT("Loading a deprecated input - its input settings will be lost"));
	}
	
	// On load, change the input selection from old input types to
	// the new input types (to Geometry or World)
	// Move the selected objects to the appropriate array
	if (Ar.IsLoading()) 
	{
		int32 IntType = (int32)Type;
		if ((IntType == 6) || (IntType == 7))
		{
			// Deprecated Skeletal or geometry input - Change to a geometry input
			HOUDINI_LOG_WARNING(TEXT("Loading a deprecated input - its input objects will be lost"));
			Type = EHoudiniInputType::Geometry;
		}
		else if ((IntType == 3) || (IntType == 4))
		{
			// Deprecated Landscape or Asset input - Change to a world input
			HOUDINI_LOG_WARNING(TEXT("Loading a deprecated input - its input objects will be lost"));
			Type = EHoudiniInputType::World;
		}
	}
}

FBox 
UHoudiniInput::GetBounds(UWorld * World) 
{
	FBox BoxBounds(ForceInitToZero);

	// Prevent crash when World is null
	if (!IsValid(World))
		return BoxBounds;

	// Return the cached bounds to prevent crashing when trying to access the input objects during GC/ saving
	// Fixes #126804
	//
	// Also, don't do this when playing, only in Editor, This fixes issue in world partition when actors are not
	// loaded. 

	bool bUseCachedBounds = UE::IsSavingPackage(nullptr) || World->IsGameWorld() || GIsCookerLoadingPackage;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bUseCachedBounds |= IsGarbageCollecting() || IsGarbageCollectingAndLockingUObjectHashTables();
#else
	bUseCachedBounds |= IsGarbageCollecting();
#endif

	if (bUseCachedBounds)
		return CachedBounds;

	switch (Type)
	{
	case EHoudiniInputType::Curve:
	{
		for (int32 Idx = 0; Idx < CurveInputObjects.Num(); ++Idx)
		{
			const UHoudiniInputHoudiniSplineComponent* CurInCurve = Cast<UHoudiniInputHoudiniSplineComponent>(CurveInputObjects[Idx]);
			if (!IsValid(CurInCurve))
				continue;

			UHoudiniSplineComponent* CurCurve = CurInCurve->GetCurveComponent();
			if (!IsValid(CurCurve))
				continue;

			FBox CurCurveBound(ForceInitToZero);
			for (auto & Trans : CurCurve->CurvePoints)
			{
				CurCurveBound += Trans.GetLocation();
			}

			UHoudiniAssetComponent* OuterHAC = Cast<UHoudiniAssetComponent>(GetOuter());

			if (IsValid(OuterHAC))
				BoxBounds += CurCurveBound.MoveTo(OuterHAC->GetComponentLocation());
		}
	}
	break;

	case EHoudiniInputType::World:
	{
		for (int32 Idx = 0; Idx < WorldInputObjects.Num(); ++Idx)
		{
			UHoudiniInputActor* CurInActor = nullptr;
			UHoudiniInputHoudiniAsset* CurInAsset = nullptr;
			UHoudiniInputLandscape* CurInLandscape = nullptr;
			if (IsValid(CurInActor = Cast<UHoudiniInputActor>(WorldInputObjects[Idx])))
			{
				AActor* Actor = CurInActor->GetActor();
				if (!IsValid(Actor))
					continue;

				FVector Origin, Extent;
				Actor->GetActorBounds(false, Origin, Extent);

				BoxBounds += FBox::BuildAABB(Origin, Extent);
			}
			else if (IsValid(CurInAsset = Cast<UHoudiniInputHoudiniAsset>(WorldInputObjects[Idx])))
			{
				// World Input now also support HoudiniAssets
				UHoudiniAssetComponent* CurInHAC = CurInAsset->GetHoudiniAssetComponent();
				if (!IsValid(CurInHAC))
					continue;

				BoxBounds += CurInHAC->GetAssetBounds(nullptr, false);
			}
			else if (IsValid(CurInLandscape = Cast<UHoudiniInputLandscape>(WorldInputObjects[Idx])))
			{
				ALandscapeProxy* CurLandscape = CurInLandscape->GetLandscapeProxy();
				if (!IsValid(CurLandscape))
					continue;

				FVector Origin, Extent;
				CurLandscape->GetActorBounds(false, Origin, Extent);

				BoxBounds += FBox::BuildAABB(Origin, Extent);
			}
		}
	}
	break;

	case EHoudiniInputType::Invalid:
	default:
		break;
	}

	// Cache the new bounds
	CachedBounds = BoxBounds;

	return BoxBounds;
}

bool
UHoudiniInput::IsAssetInput() const
{
	if (Type != EHoudiniInputType::World)
		return false;

	if (!bDirectlyConnectHdas)
		return false;

	for (auto CurObj : *GetHoudiniInputObjectArray(Type))
	{
		UHoudiniInputHoudiniAsset* CurObjAsAsset = Cast<UHoudiniInputHoudiniAsset>(CurObj);
		if (IsValid(CurObjAsAsset))
			return true;
	}
	return false;
}

bool
UHoudiniInput::IsLandscapeInput() const
{
	if (Type != EHoudiniInputType::World)
		return false;

	for (auto CurObj : *GetHoudiniInputObjectArray(Type))
	{
		UHoudiniInputLandscape* CurObjAsLandscape = Cast<UHoudiniInputLandscape>(CurObj);
		if (IsValid(CurObjAsLandscape))
			return true;
	}
	return false;
}

void UHoudiniInput::UpdateLandscapeInputSelection()
{
	LandscapeSelectedComponents.Reset();
	if (!InputSettings.bLandscapeExportSelectionOnly) return;

#if WITH_EDITOR
	if (!IsLandscapeInput())
		return;

	for (UHoudiniInputObject* NextInputObj : *GetHoudiniInputObjectArray(Type))
	{
		UHoudiniInputLandscape* CurrentInputLandscape = Cast<UHoudiniInputLandscape>(NextInputObj);
		if (!CurrentInputLandscape)
			continue;

		ALandscapeProxy* CurrentInputLandscapeProxy = CurrentInputLandscape->GetLandscapeProxy();
		if (!CurrentInputLandscapeProxy)
			continue;

		// Get selected components if bLandscapeExportSelectionOnly or bLandscapeAutoSelectComponent is true
		FBox Bounds(ForceInitToZero);
		if ( InputSettings.bLandscapeAutoSelectComponent )
		{
			// Get our asset's or our connected input asset's bounds
			UHoudiniAssetComponent* AssetComponent = Cast<UHoudiniAssetComponent>(GetOuter());
			if (IsValid(AssetComponent))
			{
				Bounds = AssetComponent->GetAssetBounds(this, true);
			}
		}
	
		if ( InputSettings.bLandscapeExportSelectionOnly )
		{
			const ULandscapeInfo * LandscapeInfo = CurrentInputLandscapeProxy->GetLandscapeInfo();
			if ( IsValid(LandscapeInfo) )
			{
				// Get the currently selected components
				for(ULandscapeComponent* LandscapeComponent : LandscapeInfo->GetSelectedComponents())
					LandscapeSelectedComponents.Add(LandscapeComponent);
			}
	
			if ( InputSettings.bLandscapeAutoSelectComponent && LandscapeSelectedComponents.Num() <= 0 && Bounds.IsValid )
			{
				// We'll try to use the asset bounds to automatically "select" the components
				for ( int32 ComponentIdx = 0; ComponentIdx < CurrentInputLandscapeProxy->LandscapeComponents.Num(); ComponentIdx++ )
				{
					ULandscapeComponent * LandscapeComponent = CurrentInputLandscapeProxy->LandscapeComponents[ ComponentIdx ];
					if ( !IsValid(LandscapeComponent) )
						continue;
	
					FBoxSphereBounds WorldBounds = LandscapeComponent->CalcBounds( LandscapeComponent->GetComponentTransform());
	
					if ( Bounds.IntersectXY( WorldBounds.GetBox() ) )
						LandscapeSelectedComponents.Add( LandscapeComponent );
				}
	
				int32 Num = LandscapeSelectedComponents.Num();
				HOUDINI_LOG_MESSAGE( TEXT("Landscape input: automatically selected %d components within the asset's bounds."), Num );
			}
		}
		else
		{
			// Add all the components of the landscape to the selected set
			ULandscapeInfo* LandscapeInfo = CurrentInputLandscapeProxy->GetLandscapeInfo();
			if (LandscapeInfo)
			{
				LandscapeInfo->ForAllLandscapeComponents([&](ULandscapeComponent* Component)
				{
					LandscapeSelectedComponents.Add(Component);
				});
			}
		}

		CurrentInputLandscape->MarkChanged(true);
	}
#endif
}

void
UHoudiniInput::MarkInputNodeAsPendingDelete()
{
	if (InputNodeId < 0)
		return;

	InputNodesPendingDelete.Add(InputNodeId);
	InputNodeId = -1;
}

FString
UHoudiniInput::InputTypeToString(const EHoudiniInputType& InInputType)
{
	FString InputTypeStr;
	switch (InInputType)
	{
		case EHoudiniInputType::Geometry:
		{
			InputTypeStr = TEXT("Geometry Input");
		}
		break;

		case EHoudiniInputType::Curve:
		{
			InputTypeStr = TEXT("Curve Input");
		}
		break;

		case EHoudiniInputType::World:
		{
			InputTypeStr = TEXT("World Input");
		}
		break;

		case EHoudiniInputType::Invalid:
		{
			InputTypeStr = TEXT("INVALID INPUT");
		}
		break;

		default:
		{
			InputTypeStr = TEXT("(deprecated) INVALID INPUT");
		}
		break;
	}

	return InputTypeStr;
}

EHoudiniInputType
UHoudiniInput::StringToInputType(const FString& InInputTypeString)
{
	if (InInputTypeString.StartsWith(TEXT("Geometry"), ESearchCase::IgnoreCase))
	{
		return EHoudiniInputType::Geometry;
	}
	else if (InInputTypeString.StartsWith(TEXT("Curve"), ESearchCase::IgnoreCase))
	{
		return EHoudiniInputType::Curve;
	}
	else if (InInputTypeString.StartsWith(TEXT("World"), ESearchCase::IgnoreCase))
	{
		return EHoudiniInputType::World;
	}

	return EHoudiniInputType::Invalid;
}

EHoudiniCurveType UHoudiniInput::StringToHoudiniCurveType(const FString& HoudiniCurveTypeString) 
{
	if (HoudiniCurveTypeString.StartsWith(TEXT("Polygon"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveType::Polygon;
	}
	else if (HoudiniCurveTypeString.StartsWith(TEXT("Nurbs"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveType::Nurbs;
	}
	else if (HoudiniCurveTypeString.StartsWith(TEXT("Bezier"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveType::Bezier;
	}
	else if (HoudiniCurveTypeString.StartsWith(TEXT("Points"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveType::Points;
	}
	
	return EHoudiniCurveType::Invalid;
}

EHoudiniCurveMethod UHoudiniInput::StringToHoudiniCurveMethod(const FString& HoudiniCurveMethodString) 
{
	if (HoudiniCurveMethodString.StartsWith(TEXT("CVs"), ESearchCase::IgnoreCase)) 
	{
		return EHoudiniCurveMethod::CVs;
	}
	else if (HoudiniCurveMethodString.StartsWith(TEXT("Breakpoints"), ESearchCase::IgnoreCase)) 
	{
		return EHoudiniCurveMethod::Breakpoints;
	}

	else if (HoudiniCurveMethodString.StartsWith(TEXT("Freehand"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveMethod::Freehand;
	}

	return EHoudiniCurveMethod::Invalid;

}

EHoudiniCurveBreakpointParameterization UHoudiniInput::StringToHoudiniCurveBreakpointParameterization(const FString& CurveParameterizationString)
{
	if (CurveParameterizationString.StartsWith(TEXT("Uniform"), ESearchCase::IgnoreCase)) 
	{
		return EHoudiniCurveBreakpointParameterization::Uniform;
	}
	else if (CurveParameterizationString.StartsWith(TEXT("Chord"), ESearchCase::IgnoreCase)) 
	{
		return EHoudiniCurveBreakpointParameterization::Chord;
	}

	else if (CurveParameterizationString.StartsWith(TEXT("Centripetal"), ESearchCase::IgnoreCase))
	{
		return EHoudiniCurveBreakpointParameterization::Centripetal;
	}

	return EHoudiniCurveBreakpointParameterization::Invalid;
}

// 
void 
UHoudiniInput::SetSOPInput(const int32& InInputIndex)
{
	// Set the input index
	InputIndex = InInputIndex;

	// Invalidate objpath parameter
	ParmId = -1;
	bIsObjectPathParameter = false;
}

void
UHoudiniInput::SetObjectPathParameter(const int32& InParmId)
{
	// Set as objpath parameter
	ParmId = InParmId;
	bIsObjectPathParameter = true;

	// Invalidate the geo input
	InputIndex = -1;
}

EHoudiniXformType 
UHoudiniInput::GetDefaultXTransformType() 
{
	switch (Type)
	{
		case EHoudiniInputType::Curve:
		case EHoudiniInputType::Geometry:
			return EHoudiniXformType::None;

		case EHoudiniInputType::World:
			return EHoudiniXformType::IntoThisObject;
	}

	return EHoudiniXformType::Auto;
}

bool
UHoudiniInput::GetKeepWorldTransform() const
{
	bool bReturn = false;
	switch (InputSettings.KeepWorldTransform)
	{
		case EHoudiniXformType::Auto:
		{
			// Return default values corresponding to the input type:
			if (Type == EHoudiniInputType::Curve || Type == EHoudiniInputType::Geometry)
			{
				// NONE  for Geo, and Curve IN
				bReturn = false;
			}
			else
			{
				// INTO THIS OBJECT for World IN
				bReturn = true;
			}
			break;
		}
		
		case EHoudiniXformType::None:
		{
			bReturn = false;
			break;
		}

		case EHoudiniXformType::IntoThisObject:
		{
			bReturn = true;
			break;
		}
	}

	return bReturn;
}

void
UHoudiniInput::SetKeepWorldTransform(const bool& bInKeepWorldTransform)
{
	if (bInKeepWorldTransform)
	{
		InputSettings.KeepWorldTransform = EHoudiniXformType::IntoThisObject;
	}
	else
	{
		InputSettings.KeepWorldTransform = EHoudiniXformType::None;
	}
}

void 
UHoudiniInput::SetInputType(const EHoudiniInputType& InInputType, bool& bOutBlueprintStructureModified)
{ 
	if (InInputType == Type)
		return;

	SetPreviousInputType(Type);

	// Mark this input as changed
	MarkChanged(true);
	bOutBlueprintStructureModified = true;

	// Check previous input type
	switch (PreviousType) 
	{
		case EHoudiniInputType::Curve:
		{
			// detach the input curves from the asset component
			if (GetNumberOfInputObjects() > 0)
			{
				for (UHoudiniInputObject * CurrentInput : *GetHoudiniInputObjectArray(Type))
				{
					UHoudiniInputHoudiniSplineComponent * CurrentInputHoudiniSpline = Cast<UHoudiniInputHoudiniSplineComponent>(CurrentInput);

					if (!IsValid(CurrentInputHoudiniSpline))
						continue;

					UHoudiniSplineComponent * HoudiniSplineComponent = CurrentInputHoudiniSpline->GetCurveComponent();


					if (!IsValid(HoudiniSplineComponent))
						continue;

					HoudiniSplineComponent->Modify();

					const bool bIsArchetype = HoudiniSplineComponent->HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject);

					if (bIsArchetype)
					{
#if WITH_EDITOR
						check(HoudiniSplineComponent->IsTemplate());
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bVisible", false);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bHiddenInGame", true);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bIsHoudiniSplineVisible", false);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bHasChanged", true);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bNeedsToTriggerUpdate", true);
#endif
					}
					else
					{
						AActor* OwningActor = HoudiniSplineComponent->GetOwner();
						check(OwningActor);

						FDetachmentTransformRules DetachTransRules(EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, false);
						HoudiniSplineComponent->DetachFromComponent(DetachTransRules);						
				 		HoudiniSplineComponent->SetVisibility(false, true);
						HoudiniSplineComponent->SetHoudiniSplineVisible(false);
						HoudiniSplineComponent->SetHiddenInGame(true, true);
						
						// This NodeId shouldn't be invalidated like this. If a spline component
						// or curve input is no longer valid, the input object should be removed from the HoudinInput
						// to get cleaned up properly.
						// HoudiniSplineComponent->SetNodeId(-1);
						HoudiniSplineComponent->MarkChanged(true);
					}

					bOutBlueprintStructureModified = true;
				}
			}
			break;
		}

		case EHoudiniInputType::Geometry:
		case EHoudiniInputType::World:
			break;

		default:
			break;
	}


	Type = InInputType;

	// TODO: NOPE, not needed
	// Set keep world transform to default w.r.t to new input type.
	//KeepWorldTransform = GetDefaultXTransformType();
	
	// Check current input type
	switch (InInputType) 
	{
		case EHoudiniInputType::World:
		{
			UHoudiniAssetComponent* OuterHAC = Cast<UHoudiniAssetComponent>(GetOuter());
			if (OuterHAC && !InputSettings.bImportAsReference) 
			{
				for (auto& CurrentInput : *GetHoudiniInputObjectArray(Type)) 
				{
					UHoudiniInputHoudiniAsset* HoudiniAssetInput = Cast<UHoudiniInputHoudiniAsset>(CurrentInput);
					if (!IsValid(HoudiniAssetInput))
						continue;

					UHoudiniAssetComponent* CurrentHAC = HoudiniAssetInput->GetHoudiniAssetComponent();
					if (!IsValid(CurrentHAC))
						continue;

					CurrentHAC->AddDownstreamHoudiniAsset(OuterHAC);
				}
			}
		}
		break;

		case EHoudiniInputType::Curve:
		{
			if (GetNumberOfInputObjects() == 0)
			{
				CreateNewCurveInputObject(bOutBlueprintStructureModified);
				MarkChanged(true);
			}
			else
			{
				for (auto& CurrentInput : *GetHoudiniInputObjectArray(Type))
				{
					UHoudiniInputHoudiniSplineComponent* SplineInput = Cast< UHoudiniInputHoudiniSplineComponent>(CurrentInput);
					if (!IsValid(SplineInput))
						continue;

					UHoudiniSplineComponent * HoudiniSplineComponent = SplineInput->GetCurveComponent();
					if (!IsValid(HoudiniSplineComponent))
						continue;
				
					HoudiniSplineComponent->Modify();

					const bool bIsArchetype = HoudiniSplineComponent->HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject);

					if (bIsArchetype)
					{
#if WITH_EDITOR
						check(HoudiniSplineComponent->IsTemplate());
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bVisible", true);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bHiddenInGame", false);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bIsHoudiniSplineVisible", true);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bHasChanged", true);
						FHoudiniEngineRuntimeUtils::SetTemplatePropertyValue(HoudiniSplineComponent, "bNeedsToTriggerUpdate", true);
#endif
					}
					else
					{
						// Attach the new Houdini spline component to it's owner
						AActor* OwningActor = HoudiniSplineComponent->GetOwner();
						check(OwningActor);
						USceneComponent* OuterComponent = Cast<USceneComponent>(GetOuter());
						HoudiniSplineComponent->RegisterComponent();
						HoudiniSplineComponent->AttachToComponent(OuterComponent, FAttachmentTransformRules::KeepRelativeTransform);
						HoudiniSplineComponent->SetHoudiniSplineVisible(true);
						HoudiniSplineComponent->SetHiddenInGame(false, true);
						HoudiniSplineComponent->SetVisibility(true, true);
						HoudiniSplineComponent->MarkChanged(true);
					
					}

					bOutBlueprintStructureModified = true;
				}
			}
		}
		break;

		case EHoudiniInputType::Geometry:
		default:
			break;
	}
}

UHoudiniInputObject*
UHoudiniInput::CreateNewCurveInputObject(bool& bOutBlueprintStructureModified)
{
	if (CurveInputObjects.Num() > 0)
		return nullptr;

	UHoudiniInputHoudiniSplineComponent* NewCurveInputObject = CreateHoudiniSplineInput(nullptr, true, false, bOutBlueprintStructureModified);
	if (!IsValid(NewCurveInputObject))
		return nullptr;

	UHoudiniSplineComponent * HoudiniSplineComponent = NewCurveInputObject->GetCurveComponent();
	if (!IsValid(HoudiniSplineComponent))
		return nullptr;

    // Default Houdini spline component input should not be visible at initialization
	HoudiniSplineComponent->SetVisibility(true, true);
	HoudiniSplineComponent->SetHoudiniSplineVisible(true);
	HoudiniSplineComponent->SetHiddenInGame(false, true);

	CurveInputObjects.Add(NewCurveInputObject);
	SetInputObjectsNumber(EHoudiniInputType::Curve, 1);
	CurveInputObjects.SetNum(1);

	return NewCurveInputObject;
}

UHoudiniInputHoudiniSplineComponent* UHoudiniInput::GetOrCreateCurveInputObjectAt(const int32 Index,
	const bool bCreateIndex,
	bool& bOutBlueprintStructureModified)
{

	if (!bCreateIndex && !CurveInputObjects.IsValidIndex(Index))
	{
		// Index is not valid, and we shouldn't create an entry here either
		return nullptr;
	}
	
	// If the given index does not exist, resize the CurveInputs array to accomodate the entry.
	if (!CurveInputObjects.IsValidIndex(Index))
	{
		CurveInputObjects.SetNum(Index+1);
	}

	UHoudiniInputObject* ExistingInputObject = Cast<UHoudiniInputObject>(GetInputObjectAt(EHoudiniInputType::Curve, Index));
	UHoudiniInputHoudiniSplineComponent* CurveInputObject = Cast<UHoudiniInputHoudiniSplineComponent>( ExistingInputObject );

	if (IsValid(CurveInputObject))
	{
		return CurveInputObject;
	}
	
	if (IsValid(ExistingInputObject) && !IsValid(CurveInputObject))
	{
		// There is an existing input object, but it is not a curve. Delete the existing input object and so that
		// we can create a new object in its place
		DeleteInputObjectAt(Index, false);
		MarkChanged(true);
	}

	CurveInputObject = CreateHoudiniSplineInput(nullptr, true, false, bOutBlueprintStructureModified);
	if (!IsValid(CurveInputObject))
	{
		return nullptr;
	}
	
	CurveInputObjects[Index] = CurveInputObject;

	UHoudiniSplineComponent* HoudiniSplineComponent = CurveInputObject->GetCurveComponent();
	if (!IsValid(HoudiniSplineComponent))
	{
		return nullptr;
	}

    // Default Houdini spline component input should not be visible at initialization
	HoudiniSplineComponent->SetVisibility(true, true);
	HoudiniSplineComponent->SetHoudiniSplineVisible(true);
	HoudiniSplineComponent->SetHiddenInGame(false, true);

	return CurveInputObject;
}

void
UHoudiniInput::MarkAllInputObjectsChanged(const bool& bInChanged)
{
	MarkDataUploadNeeded(bInChanged);

	// Mark all the objects from this input has changed so they upload themselves

	TSet<EHoudiniInputType> InputTypes;
	InputTypes.Add(Type);
	InputTypes.Add(EHoudiniInputType::Curve);
	
	TArray<TObjectPtr<UHoudiniInputObject>>* NewInputObjects = GetHoudiniInputObjectArray(Type);
	if (NewInputObjects)
	{
		for (auto CurInputObject : *NewInputObjects)
		{
			if (IsValid(CurInputObject))
				CurInputObject->MarkChanged(bInChanged);
		}
	}
}

UHoudiniInput * UHoudiniInput::DuplicateAndCopyState(UObject * DestOuter, bool bInCanDeleteHoudiniNodes)
{
	UHoudiniInput* NewInput = Cast<UHoudiniInput>(StaticDuplicateObject(this, DestOuter));

	NewInput->CopyStateFrom(this, false, bInCanDeleteHoudiniNodes);

	return NewInput;
}

void UHoudiniInput::CopyStateFrom(UHoudiniInput* InInput, bool bCopyAllProperties, bool bInCanDeleteHoudiniNodes)
{
	// Preserve the current input objects before the copy to ensure we don't lose 
	// access to input objects and have them end up in the garbage.	
	TMap<EHoudiniInputType, TArray<TObjectPtr<UHoudiniInputObject>>*> PrevInputObjectsMap;
	for(EHoudiniInputType InputType : HoudiniInputTypeList)
	{
		PrevInputObjectsMap.Add(InputType, GetHoudiniInputObjectArray(InputType));
	}
	
	// TArray<UHoudiniInputObject*> PrevInputObjects;
	// TArray<UHoudiniInputObject*>* OldToInputObjects = GetHoudiniInputObjectArray(Type);
	// if (OldToInputObjects)
	// PrevInputObjects = *OldToInputObjects;

	// Copy the state of this UHoudiniInput object.
	if (bCopyAllProperties)
	{
		UEngine::FCopyPropertiesForUnrelatedObjectsParams Params;
		Params.bDoDelta = false; // Perform a deep copy
		Params.bClearReferences = false; // References will be replaced afterwards.
		UEngine::CopyPropertiesForUnrelatedObjects(InInput, this, Params);
	}

	AssetNodeId = InInput->AssetNodeId;
	InputNodeId = InInput->GetInputNodeId();
	ParmId = InInput->ParmId;
	bCanDeleteHoudiniNodes = bInCanDeleteHoudiniNodes;

	//if (bInCanDeleteHoudiniNodes)
	//{
	//	// Delete stale data nodes before they get overwritten.
	//	TSet<int32> NewNodeIds(InInput->CreatedDataNodeIds);
	//	for (int32 NodeId : CreatedDataNodeIds)
	//	{
	//		if (!NewNodeIds.Contains(NodeId))
	//		{
	//			FHoudiniEngineRuntime::Get().MarkNodeIdAsPendingDelete(NodeId);
	//		}
	//	}
	//}

	CreatedDataNodeIds = InInput->CreatedDataNodeIds;

	// Important note: At this point the new object may still share objects with InInput.
	// The CopyInputs() will properly duplicate inputs where necessary.

	// Copy states of Input Objects that correspond to the current type.
	for(auto& Entry : PrevInputObjectsMap)
	{
		EHoudiniInputType InputType = Entry.Key;
		TArray<TObjectPtr<UHoudiniInputObject>>* PrevInputObjects = Entry.Value;
		TArray<TObjectPtr<UHoudiniInputObject>>* ToInputObjects = GetHoudiniInputObjectArray(InputType);
		TArray<TObjectPtr<UHoudiniInputObject>>* FromInputObjects = InInput->GetHoudiniInputObjectArray(InputType);

		if (ToInputObjects && FromInputObjects)
		{
			*ToInputObjects = *PrevInputObjects;
			CopyInputs(*ToInputObjects, *FromInputObjects, bInCanDeleteHoudiniNodes);
		}
	}
}

void UHoudiniInput::SetCanDeleteHoudiniNodes(bool bInCanDeleteNodes)
{
	bCanDeleteHoudiniNodes = bInCanDeleteNodes;
	for(UHoudiniInputObject* InputObject : GeometryInputObjects)
	{
		if (!InputObject)
			continue;
		InputObject->SetCanDeleteHoudiniNodes(bInCanDeleteNodes);
	}

	for(UHoudiniInputObject* InputObject : CurveInputObjects)
	{
		if (!InputObject)
			continue;
		InputObject->SetCanDeleteHoudiniNodes(bInCanDeleteNodes);
	}

	for(UHoudiniInputObject* InputObject : WorldInputObjects)
	{
		if (!InputObject)
			continue;
		InputObject->SetCanDeleteHoudiniNodes(bInCanDeleteNodes);
	}
}

void UHoudiniInput::InvalidateData()
{
	for(UHoudiniInputObject* InputObject : GeometryInputObjects)
	{
		if (!InputObject)
			continue;
		InputObject->InvalidateData();
	}

	for(UHoudiniInputObject* InputObject : CurveInputObjects)
	{
		if (!InputObject)
			continue;
		InputObject->InvalidateData();
	}

	for(UHoudiniInputObject* InputObject : WorldInputObjects)
	{
		if (!InputObject)
			continue;

		if (InputObject->IsA<UHoudiniInputHoudiniAsset>())
		{
			// When the input object is a HoudiniAssetComponent, 
			// we need to be sure that this HDA node id is not in CreatedDataNodeIds
			// We dont want to delete the input HDA node!
			const int32 ObjInputNodeId = InputObject->GetInputNodeId();
			if (ObjInputNodeId >= 0)
				CreatedDataNodeIds.Remove(ObjInputNodeId);
		}

		InputObject->InvalidateData();
	}

	if (bCanDeleteHoudiniNodes && !CreatedDataNodeIds.IsEmpty())
	{
		// If the ref counted input system is being used, we must not delete nodes managed by the system
		TSet<int32> ManagedNodeIds;
		{
			IUnrealObjectInputManager const* const Manager = FUnrealObjectInputManager::Get();
			if (Manager)
			{
				TArray<int32> ManagedNodeIdArray;
				if (Manager->GetAllHAPINodeIds(ManagedNodeIdArray))
					ManagedNodeIds.Append(ManagedNodeIdArray);
			}
		}
		
		auto& HoudiniEngineRuntime = FHoudiniEngineRuntime::Get();
		for (int32 NodeId : CreatedDataNodeIds)
		{
			if (ManagedNodeIds.Contains(NodeId))
				continue;
			
			HoudiniEngineRuntime.MarkNodeIdAsPendingDelete(NodeId, true);
		}
	}
	
	CreatedDataNodeIds.Empty();
}

void UHoudiniInput::CopyInputs(TArray<TObjectPtr<UHoudiniInputObject>>& ToInputObjects, TArray<TObjectPtr<UHoudiniInputObject>>& FromInputObjects, bool bInCanDeleteHoudiniNodes)
{
	TSet<UHoudiniInputObject*> StaleObjects(ToInputObjects);

	const int32 NumInputs = FromInputObjects.Num();
	UObject* TargetOuter = GetOuter();
	
	ToInputObjects.SetNum(NumInputs);


	for (int i = 0; i < NumInputs; i++)
	{
		UHoudiniInputObject* FromObject = FromInputObjects[i];
		UHoudiniInputObject* ToObject = ToInputObjects[i];

		if (!FromObject)
		{
			ToInputObjects[i] = nullptr;
			continue;
		}

		if (ToObject)
		{
			bool IsValid = true;
			// Is ToInput and FromInput the same or do we have to create a input object?
			IsValid = IsValid && ToObject->Matches(*FromObject);
			IsValid = IsValid && ToObject->GetOuter() == TargetOuter;

			if (!IsValid)
			{
				ToObject = nullptr;
			}
		}

		if (ToObject)
		{
			// We have an existing (matching) object. Copy the
			// state from the incoming input.
			StaleObjects.Remove(ToObject);
			ToObject->CopyStateFrom(FromObject, true);
		}
		else
		{
			// We need to create a new input here.
			ToObject = FromObject->DuplicateAndCopyState(TargetOuter);
			ToInputObjects[i] = ToObject;
		}

		ToObject->SetCanDeleteHoudiniNodes(bInCanDeleteHoudiniNodes);
	}


	for (UHoudiniInputObject* StaleInputObject : StaleObjects)
	{
		if (!StaleInputObject)
			continue;
		if (StaleInputObject->GetOuter() == this)
		{
			StaleInputObject->SetCanDeleteHoudiniNodes(bInCanDeleteHoudiniNodes);
		}
	}
}


UHoudiniInputHoudiniSplineComponent*
UHoudiniInput::CreateHoudiniSplineInput(UHoudiniInputHoudiniSplineComponent * FromHoudiniSplineInputComponent, const bool & bAttachToparent, const bool & bAppendToInputArray, bool& bOutBlueprintStructureModified)
{
	UHoudiniInputHoudiniSplineComponent* HoudiniSplineInput = nullptr;
	UHoudiniSplineComponent* HoudiniSplineComponent = nullptr;

	UObject* OuterObj = GetOuter();
	USceneComponent* OuterComp = Cast<USceneComponent>(GetOuter());
	bool bOuterIsTemplate = (OuterObj && OuterObj->IsTemplate());

	if (!FromHoudiniSplineInputComponent)
	{
		// NOTE: If we're inside the Blueprint editor, the outer here is going to the be HAC component template.
		check(OuterObj)
		
		// Create a default Houdini spline input if a null pointer is passed in.
		FName HoudiniSplineName = MakeUniqueObjectName(OuterComp, UHoudiniSplineComponent::StaticClass(), TEXT("Houdini Spline"));

		// Create a Houdini Input Object.
		UHoudiniInputObject * NewInputObject = UHoudiniInputHoudiniSplineComponent::Create(
			nullptr, OuterObj, HoudiniSplineName.ToString(), InputSettings);

		if (!IsValid(NewInputObject))
			return nullptr;

		HoudiniSplineInput = Cast<UHoudiniInputHoudiniSplineComponent>(NewInputObject);
		if (!HoudiniSplineInput)
			return nullptr;

		HoudiniSplineComponent = NewObject<UHoudiniSplineComponent>(
			HoudiniSplineInput,	UHoudiniSplineComponent::StaticClass());

		if (!IsValid(HoudiniSplineComponent))
			return nullptr;

		HoudiniSplineInput->Update(HoudiniSplineComponent, InputSettings);

		HoudiniSplineComponent->SetHoudiniSplineName(HoudiniSplineName.ToString());
		HoudiniSplineComponent->SetFlags(RF_Transactional);

		// Set the default position of curve to avoid overlapping.
		HoudiniSplineComponent->SetOffset(DefaultCurveOffset);
		DefaultCurveOffset += 100.f;

		if (!bOuterIsTemplate)
		{ 
			HoudiniSplineComponent->RegisterComponent();

			// Attach the new Houdini spline component to it's owner.
			if (bAttachToparent)
				HoudiniSplineComponent->AttachToComponent(OuterComp, FAttachmentTransformRules::KeepRelativeTransform);
		}

		//push the new input object to the array for new type.
		if (bAppendToInputArray &&  Type == EHoudiniInputType::Curve)
			GetHoudiniInputObjectArray(Type)->Add(NewInputObject);

#if WITH_EDITOR
		if (bOuterIsTemplate)
		{
			UHoudiniAssetBlueprintComponent* HAB = Cast<UHoudiniAssetBlueprintComponent>(OuterObj);
			if (HAB)
			{
				UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(HAB->GetOuter());
				UBlueprint* Blueprint = Cast<UBlueprint>(BPGC->ClassGeneratedBy);
				if (Blueprint)
				{
					TArray<UActorComponent*> Components;
					Components.Add(HoudiniSplineComponent);

					USCS_Node* HABNode = HAB->FindSCSNodeForTemplateComponentInClassHierarchy(HAB);

					// NOTE: FAddComponentsToBlueprintParams was introduced in 4.26 so for the sake of 
					// backwards compatibility, manually determine which SCSNode was added instead of
					// relying on Params.OutNodes.
					FKismetEditorUtilities::FAddComponentsToBlueprintParams Params;
					Params.OptionalNewRootNode = HABNode;
					const TSet<USCS_Node*> PreviousSCSNodes(Blueprint->SimpleConstructionScript->GetAllNodes());

					FKismetEditorUtilities::AddComponentsToBlueprint(Blueprint, Components, Params);
					USCS_Node* NewNode = nullptr;
					const TSet<USCS_Node*> CurrentSCSNodes(Blueprint->SimpleConstructionScript->GetAllNodes());
					const TSet<USCS_Node*> AddedNodes = CurrentSCSNodes.Difference(PreviousSCSNodes);
					
					if (AddedNodes.Num() > 0)
					{
						// Record Input / SCS node mapping 
						USCS_Node* SCSNode = AddedNodes.Array()[0];
						HAB->AddInputObjectMapping(NewInputObject->GetInputGuid(), SCSNode->VariableGuid);
						SCSNode->ComponentTemplate->SetFlags(RF_Public | RF_ArchetypeObject | RF_DefaultSubObject);
					}

					Blueprint->Modify();
					bOutBlueprintStructureModified = true;
				}
			}
		}
#endif
	}
	else 
	{
		// Otherwise, get the Houdini spline, and Houdini spline input from the argument.
		HoudiniSplineInput = FromHoudiniSplineInputComponent;
		HoudiniSplineComponent = FromHoudiniSplineInputComponent->GetCurveComponent();

		if (!IsValid(HoudiniSplineComponent))
			return nullptr;

		// Attach the new Houdini spline component to it's owner.
		HoudiniSplineComponent->AttachToComponent(OuterComp, FAttachmentTransformRules::KeepRelativeTransform);
	}

	// Mark the created UHoudiniSplineComponent as an input, and set its InputObject.
	HoudiniSplineComponent->SetIsInputCurve(true);

	// HoudiniSplineComponent->SetInputObject(HoudiniSplineInput);

	// Set Houdini Spline Component bHasChanged and bNeedsToTrigerUpdate to true.
	HoudiniSplineComponent->MarkChanged(true);

	

	return HoudiniSplineInput;
}

void
UHoudiniInput::RemoveSplineFromInputObject(
	UHoudiniInputHoudiniSplineComponent* InHoudiniSplineInputObject,
	bool& bOutBlueprintStructureModified) const
{
	if (!InHoudiniSplineInputObject)
		return;

	UObject* OuterObj = GetOuter();
	const bool bOuterIsTemplate = OuterObj && OuterObj->IsTemplate();

	if (bOuterIsTemplate)
	{
#if WITH_EDITOR
		// Find the SCS node that corresponds to this input and remove it.
		UHoudiniAssetBlueprintComponent* HAB = Cast<UHoudiniAssetBlueprintComponent>(OuterObj);
		if (HAB)
		{
			const UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(HAB->GetOuter());
			UBlueprint* Blueprint = Cast<UBlueprint>(BPGC->ClassGeneratedBy);
			if (Blueprint)
			{
				USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
				check(SCS);
				FGuid SCSGuid;
				if (HAB->GetInputObjectSCSVariableGuid(InHoudiniSplineInputObject->Guid, SCSGuid))
				{
					// TODO: Move this SCS variable removal code to a reusable utility function. We're
					// going to need to reuse this in a few other places too.
					USCS_Node* SCSNode = SCS->FindSCSNodeByGuid(SCSGuid);
					if (SCSNode)
					{
						SCS->RemoveNodeAndPromoteChildren(SCSNode);
						SCSNode->SetOnNameChanged(FSCSNodeNameChanged());
						bOutBlueprintStructureModified = true;
						HAB->RemoveInputObjectSCSVariableGuid(InHoudiniSplineInputObject->Guid);

						if (SCSNode->ComponentTemplate != nullptr)
						{
							const FName TemplateName = SCSNode->ComponentTemplate->GetFName();
							const FString RemovedName = SCSNode->GetVariableName().ToString() + TEXT("_REMOVED_") + FGuid::NewGuid().ToString();

							SCSNode->ComponentTemplate->Modify();
							SCSNode->ComponentTemplate->Rename(*RemovedName, /*NewOuter =*/nullptr, REN_DontCreateRedirectors);

							TArray<UObject*> ArchetypeInstances;
							auto DestroyArchetypeInstances = [&ArchetypeInstances, &RemovedName](UActorComponent* ComponentTemplate)
							{
								ComponentTemplate->GetArchetypeInstances(ArchetypeInstances);
								for (UObject* ArchetypeInstance : ArchetypeInstances)
								{
									if (!ArchetypeInstance->HasAllFlags(RF_ArchetypeObject | RF_InheritableComponentTemplate))
									{
										CastChecked<UActorComponent>(ArchetypeInstance)->DestroyComponent();
										ArchetypeInstance->Rename(*RemovedName, nullptr, REN_DontCreateRedirectors);
									}
								}
							};

							DestroyArchetypeInstances(SCSNode->ComponentTemplate);
							
							if (Blueprint)
							{
								// Children need to have their inherited component template instance of the component renamed out of the way as well
								TArray<UClass*> ChildrenOfClass;
								GetDerivedClasses(Blueprint->GeneratedClass, ChildrenOfClass);

								for (UClass* ChildClass : ChildrenOfClass)
								{
									UBlueprintGeneratedClass* BPChildClass = CastChecked<UBlueprintGeneratedClass>(ChildClass);

									if (UActorComponent* Component = (UActorComponent*)FindObjectWithOuter(BPChildClass, UActorComponent::StaticClass(), TemplateName))
									{
										Component->Modify();
										Component->Rename(*RemovedName, /*NewOuter =*/nullptr, REN_DontCreateRedirectors);

										DestroyArchetypeInstances(Component);
									}
								}
							}
						}
					}
				} // if (HAB->GetInputObjectSCSVariableGuid(InHoudiniSplineInputObject->Guid, SCSGuid))
			} // if (Blueprint)
		}
#endif
	} // if (bIsOuterTemplate)
	else
	{
		UHoudiniSplineComponent* HoudiniSplineComponent = InHoudiniSplineInputObject->GetCurveComponent();
		if (HoudiniSplineComponent)
		{
			// detach the input curves from the asset component
			//FDetachmentTransformRules DetachTransRules(EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, false);
			//HoudiniSplineComponent->DetachFromComponent(DetachTransRules);
			// Destroy the Houdini Spline Component
			//InputObjectsPtr->RemoveAt(AtIndex);
			HoudiniSplineComponent->DestroyComponent();
		}
	}
	InHoudiniSplineInputObject->Update(nullptr, InputSettings);
}


TArray<TObjectPtr<UHoudiniInputObject>>*
UHoudiniInput::GetHoudiniInputObjectArray(const EHoudiniInputType& InType)
{
	switch (InType)
	{
	case EHoudiniInputType::Geometry:
		return &GeometryInputObjects;

	case EHoudiniInputType::Curve:
		return &CurveInputObjects;

	case EHoudiniInputType::World:
		return &WorldInputObjects;

	default:
	case EHoudiniInputType::Invalid:
		return nullptr;
	}
}

TArray<TObjectPtr<AActor>>*
UHoudiniInput::GetBoundSelectorObjectArray()
{	
	switch (Type)
	{
		case EHoudiniInputType::World:
			return &WorldInputBoundSelectorObjects;

		default:
			return nullptr;
	}
}

const TArray<TObjectPtr<AActor>>*
UHoudiniInput::GetBoundSelectorObjectArray() const
{
	switch (Type)
	{
		case EHoudiniInputType::World:
			return &WorldInputBoundSelectorObjects;

		default:
			return nullptr;
	}
}

const TArray<TObjectPtr<UHoudiniInputObject>>*
UHoudiniInput::GetHoudiniInputObjectArray(const EHoudiniInputType& InType) const
{
	switch (InType)
	{
		case EHoudiniInputType::Geometry:
			return &GeometryInputObjects;

		case EHoudiniInputType::Curve:
			return &CurveInputObjects;

		case EHoudiniInputType::World:
			return &WorldInputObjects;

		case EHoudiniInputType::Invalid:
		default:
			return nullptr;
	}
}

UHoudiniInputObject*
UHoudiniInput::GetHoudiniInputObjectAt(const int32& AtIndex)
{
	return GetHoudiniInputObjectAt(Type, AtIndex);
}

const UHoudiniInputObject*
UHoudiniInput::GetHoudiniInputObjectAt(const int32& AtIndex) const
{
	const TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsArray = GetHoudiniInputObjectArray(Type);
	if (!InputObjectsArray || !InputObjectsArray->IsValidIndex(AtIndex))
		return nullptr;

	return (*InputObjectsArray)[AtIndex];
}

UHoudiniInputObject*
UHoudiniInput::GetHoudiniInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsArray = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsArray || !InputObjectsArray->IsValidIndex(AtIndex))
		return nullptr;

	return (*InputObjectsArray)[AtIndex];
}

UObject*
UHoudiniInput::GetInputObjectAt(const int32& AtIndex)
{
	return GetInputObjectAt(Type, AtIndex);
}

AActor*
UHoudiniInput::GetBoundSelectorObjectAt(const int32& AtIndex)
{
	TArray<TObjectPtr<AActor>>* BoundSelectorObjects = GetBoundSelectorObjectArray();

	if (!BoundSelectorObjects)
		return nullptr;

	if (!BoundSelectorObjects->IsValidIndex(AtIndex))
		return nullptr;

	return (*BoundSelectorObjects)[AtIndex];
}

UObject*
UHoudiniInput::GetInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex)
{
	UHoudiniInputObject* HoudiniInputObject = GetHoudiniInputObjectAt(InType, AtIndex);
	if (!IsValid(HoudiniInputObject))
		return nullptr;

	return HoudiniInputObject->GetObject();
}

void
UHoudiniInput::InsertInputObjectAt(const int32& AtIndex)
{
	InsertInputObjectAt(Type, AtIndex);
}

void
UHoudiniInput::InsertInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return;

	InputObjectsPtr->InsertDefaulted(AtIndex, 1);
	MarkChanged(true);
}

void
UHoudiniInput::DeleteInputObjectAt(const int32& AtIndex, const bool bInRemoveIndexFromArray)
{
	DeleteInputObjectAt(Type, AtIndex, bInRemoveIndexFromArray);
}

void
UHoudiniInput::DeleteInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex, const bool bInRemoveIndexFromArray)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return;

	if (!InputObjectsPtr->IsValidIndex(AtIndex))
		return;

	bool bBlueprintStructureModified = false;

	if (Type == EHoudiniInputType::Curve)
	{
		UHoudiniInputHoudiniSplineComponent* HoudiniSplineInputObject = Cast<UHoudiniInputHoudiniSplineComponent>((*InputObjectsPtr)[AtIndex]);
		if (HoudiniSplineInputObject)
		{
			RemoveSplineFromInputObject(HoudiniSplineInputObject, bBlueprintStructureModified);
		}
	}
	else if (Type == EHoudiniInputType::Geometry) 
	{
		// ... TODO operations for removing geometry input type
	}
	else if (Type == EHoudiniInputType::World) 
	{
		// ... TODO operations for removing world input type
	}
	else 
	{
		// ... invalid input type
	}

	MarkChanged(true);

	UHoudiniInputObject* InputObjectToDelete = (*InputObjectsPtr)[AtIndex];
	if (IsValid(InputObjectToDelete))
	{		
		// Mark the input object's nodes for deletion
		InputObjectToDelete->InvalidateData();

		// If the deleted object wasnt null, trigger a re upload of the input data
		MarkDataUploadNeeded(true);
	}

	if (bInRemoveIndexFromArray)
	{
		InputObjectsPtr->RemoveAt(AtIndex);

		// Delete the merge node when all the input objects are deleted.
		if (InputObjectsPtr->Num() == 0 && InputNodeId >= 0)
		{
			if (bCanDeleteHoudiniNodes)
				MarkInputNodeAsPendingDelete();
			InputNodeId = -1;
		}
	}
	else
	{
		(*InputObjectsPtr)[AtIndex] = nullptr;
	}

#if WITH_EDITOR
	if (bBlueprintStructureModified)
	{
		UActorComponent* Component = Cast<UActorComponent>(GetOuter());
		FHoudiniEngineRuntimeUtils::MarkBlueprintAsStructurallyModified(Component);
	}
#endif
}

void
UHoudiniInput::DuplicateInputObjectAt(const int32& AtIndex)
{
	DuplicateInputObjectAt(Type, AtIndex);
}

void
UHoudiniInput::DuplicateInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return;

	if (!InputObjectsPtr->IsValidIndex(AtIndex))
		return;

	// If the duplicated object is not null, trigger a re upload of the input data
	bool bTriggerUpload = (*InputObjectsPtr)[AtIndex] != nullptr;

	// TODO: Duplicate the UHoudiniInputObject!!
	UHoudiniInputObject* DuplicateInput = (*InputObjectsPtr)[AtIndex];
	InputObjectsPtr->Insert(DuplicateInput, AtIndex);

	MarkChanged(true);

	if (bTriggerUpload)
		MarkDataUploadNeeded(true);
}

int32
UHoudiniInput::GetNumberOfInputObjects()
{
	return GetNumberOfInputObjects(Type);
}

int32
UHoudiniInput::GetNumberOfInputObjects(const EHoudiniInputType& InType)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return 0;

	return InputObjectsPtr->Num();
}

int32
UHoudiniInput::GetNumberOfInputMeshes()
{
	return GetNumberOfInputMeshes(Type);
}

int32
UHoudiniInput::GetNumberOfInputMeshes(const EHoudiniInputType& InType)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return 0;

	// TODO?
	// If geometry input, and we only have one null object, return 0
	int32 Num = InputObjectsPtr->Num();

	// Special case for Actors and BP
	// we need to add extra input objects stored in the components array
	for (auto InputObj : *InputObjectsPtr)
	{
		if (!IsValid(InputObj))
			continue;

		UHoudiniInputActor* InputActor = Cast<UHoudiniInputActor>(InputObj);
		if (IsValid(InputActor))
		{
			if (InputActor->GetActorComponents().Num() > 0)
			{
				Num += (InputActor->GetActorComponents().Num() - 1);
			}
		}

		UHoudiniInputBlueprint* InputBP = Cast<UHoudiniInputBlueprint>(InputObj);
		if (IsValid(InputBP))
		{
			if (InputBP->GetComponents().Num() > 0)
			{
				Num += (InputBP->GetComponents().Num() - 1);
			}
		}
	}

	return Num;
}


int32
UHoudiniInput::GetNumberOfBoundSelectorObjects() const
{
	const TArray<TObjectPtr<AActor>>* BoundSelectorObjects = GetBoundSelectorObjectArray();

	if (!BoundSelectorObjects)
		return 0;

	return BoundSelectorObjects->Num();
}

void
UHoudiniInput::SetInputObjectAt(const int32& AtIndex, UObject* InObject)
{
	return SetInputObjectAt(Type, AtIndex, InObject);
}

void
UHoudiniInput::SetInputObjectAt(const EHoudiniInputType& InType, const int32& AtIndex, UObject* InObject)
{
	// Start by making sure we have the proper number of input objects
	int32 NumIntObject = GetNumberOfInputObjects(InType);
	if (NumIntObject <= AtIndex)
	{
		// We need to resize the array
		SetInputObjectsNumber(InType, AtIndex + 1);
	}

	UObject* CurrentInputObject = GetInputObjectAt(InType, AtIndex);
	if (CurrentInputObject == InObject)
	{
		// Nothing to do
		return;
	}

	UHoudiniInputObject* CurrentInputObjectWrapper = GetHoudiniInputObjectAt(InType, AtIndex);
	if (!InObject)
	{
		// We want to set the input object to null
		TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
		if (!ensure(InputObjectsPtr != nullptr && InputObjectsPtr->IsValidIndex(AtIndex)))
			return;

		if (CurrentInputObjectWrapper)
		{
			// TODO: Check this case
			// Do not destroy the input object manually! this messes up GC
			//CurrentInputObjectWrapper->ConditionalBeginDestroy();
			MarkDataUploadNeeded(true);
		}

		(*InputObjectsPtr)[AtIndex] = nullptr;
		return;
	}

	// Get the type of the previous and new input objects
	EHoudiniInputObjectType NewObjectType = InObject ? UHoudiniInputObject::GetInputObjectTypeFromObject(InObject) : EHoudiniInputObjectType::Invalid;
	EHoudiniInputObjectType CurrentObjectType = CurrentInputObjectWrapper ? CurrentInputObjectWrapper->Type : EHoudiniInputObjectType::Invalid;

	// See if we can reuse the existing InputObject		
	if (CurrentObjectType == NewObjectType && NewObjectType != EHoudiniInputObjectType::Invalid)
	{
		// The InputObjectTypes match, we can just update the existing object
		CurrentInputObjectWrapper->Update(InObject, InputSettings);
		CurrentInputObjectWrapper->MarkChanged(true);
		return;
	}

	// Destroy the existing input object
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!ensure(InputObjectsPtr))
		return;

	UHoudiniInputObject* NewInputObject = UHoudiniInputObject::CreateTypedInputObject(InObject, this, FString::FromInt(AtIndex + 1), InputSettings);
	if (!ensure(NewInputObject))
		return;

	// Mark that input object as changed so we know we need to update it
	NewInputObject->MarkChanged(true);
	if (IsValid(CurrentInputObjectWrapper))
	{
		// TODO:
		// For some input type, we may have to copy some of the previous object's property before deleting it

		// Delete the previous object
		CurrentInputObjectWrapper->MarkAsGarbage();
		(*InputObjectsPtr)[AtIndex] = nullptr;
	}

	// Update the input object array with the newly created input object
	(*InputObjectsPtr)[AtIndex] = NewInputObject;

	// Mark the outer package as dirty, to ensure that the changes are saved when using OFPA / World partition
	MarkPackageDirty();
}

void
UHoudiniInput::SetInputObjectsNumber(const EHoudiniInputType& InType, const int32& InNewCount)
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(InType);
	if (!InputObjectsPtr)
		return;

	if (InputObjectsPtr->Num() == InNewCount)
	{
		// Nothing to do
		return;
	}

	if (InNewCount > InputObjectsPtr->Num())
	{
		// Simply add new default InputObjects
		InputObjectsPtr->SetNum(InNewCount);
	}
	else
	{
		// TODO: Check this case!
		// Do not destroy the input object themselves manually,
		// destroy the input object's nodes and reduce the array's size  
		for (int32 InObjIdx = InputObjectsPtr->Num() - 1; InObjIdx >= InNewCount; InObjIdx--)
		{
			UHoudiniInputObject* CurrentInputObject = (*InputObjectsPtr)[InObjIdx];
			if (!IsValid(CurrentInputObject))
				continue;

			if (bCanDeleteHoudiniNodes)
				CurrentInputObject->InvalidateData();

			/*/
			//FHoudiniInputTranslator::DestroyInput(Inputs[InputIdx]);
			CurrentObject->ConditionalBeginDestroy();
			(*InputObjectsPtr)[InObjIdx] = nullptr;
			*/
		}
		
		// Decrease the input object array size
		InputObjectsPtr->SetNum(InNewCount);
	}

	// Also delete the input's merge node when all the input objects are deleted.
	if (InNewCount == 0 && InputNodeId >= 0)
	{
		if (bCanDeleteHoudiniNodes)
			MarkInputNodeAsPendingDelete();
		InputNodeId = -1;
	}
}

void
UHoudiniInput::SetBoundSelectorObjectsNumber(const int32& InNewCount)
{
	TArray<TObjectPtr<AActor>>* BoundSelectorObjects = GetBoundSelectorObjectArray();

	if (!BoundSelectorObjects)
		return;

	if (BoundSelectorObjects->Num() == InNewCount)
		return;

	BoundSelectorObjects->SetNum(InNewCount);

	/*
	// For InNewCount <= BoundSelectorObjects->Num():
	// TODO: Not Needed?
	// Do not destroy the input object themselves manually,
	// destroy the input object's nodes and reduce the array's size
	for (int32 InObjIdx = WorldInputBoundSelectorObjects.Num() - 1; InObjIdx >= InNewCount; InObjIdx--)
	{
		UHoudiniInputObject* CurrentInputObject = WorldInputBoundSelectorObjects[InObjIdx];
		if (!CurrentInputObject)
			continue;

		CurrentInputObject->MarkInputNodesForDeletion();
	}
	*/
}

void
UHoudiniInput::SetBoundSelectorObjectAt(const int32& AtIndex, AActor* InActor)
{
	// Start by making sure we have the proper number of objects
	int32 NumIntObject = GetNumberOfBoundSelectorObjects();
	if (NumIntObject <= AtIndex)
	{
		// We need to resize the array
		SetBoundSelectorObjectsNumber(AtIndex + 1);
	}

	AActor* CurrentActor = GetBoundSelectorObjectAt(AtIndex);
	if (CurrentActor == InActor)
	{
		// Nothing to do
		return;
	}

	TArray<TObjectPtr<AActor>>* BoundSelectorObjects = GetBoundSelectorObjectArray();

	if (!BoundSelectorObjects)
		return;

	(*BoundSelectorObjects)[AtIndex] = InActor;
}

// Helper function indicating what classes are supported by an input type
TArray<const UClass*>
UHoudiniInput::GetAllowedClasses(const EHoudiniInputType& InInputType)
{
	TArray<const UClass*> AllowedClasses;
	switch (InInputType)
	{
		case EHoudiniInputType::Geometry:
			AllowedClasses.Add(UStaticMesh::StaticClass());
			AllowedClasses.Add(USkeletalMesh::StaticClass());
			AllowedClasses.Add(UAnimSequence::StaticClass());
			AllowedClasses.Add(UGeometryCollection::StaticClass());
			AllowedClasses.Add(UGeometryCollectionComponent::StaticClass());
			AllowedClasses.Add(AGeometryCollectionActor::StaticClass());
			AllowedClasses.Add(UBlueprint::StaticClass());
			AllowedClasses.Add(UDataTable::StaticClass());
			AllowedClasses.Add(UFoliageType_InstancedStaticMesh::StaticClass());
			break;

		case EHoudiniInputType::Curve:
			AllowedClasses.Add(USplineComponent::StaticClass());
			AllowedClasses.Add(UHoudiniSplineComponent::StaticClass());
			break;

		case EHoudiniInputType::World:
			AllowedClasses.Add(AActor::StaticClass());
			AllowedClasses.Add(UHoudiniAssetComponent::StaticClass());
			AllowedClasses.Add(ALandscapeProxy::StaticClass());
			break;

		default:
			break;
	}

	return AllowedClasses;
}

// Helper function indicating if an object is supported by an input type
bool 
UHoudiniInput::IsObjectAcceptable(const EHoudiniInputType& InInputType, const UObject* InObject)
{
	TArray<const UClass*> AllowedClasses = GetAllowedClasses(InInputType);
	for (auto CurClass : AllowedClasses)
	{
		if (InObject->IsA(CurClass))
			return true;
	}

	return false;
}

bool
UHoudiniInput::IsDataUploadNeeded()
{
	if (bDataUploadNeeded)
		return true;

	return HasChanged();
}

// Indicates if this input has changed and should be updated
bool 
UHoudiniInput::HasChanged()
{
	if (bHasChanged)
		return true;

	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(Type);
	if (!ensure(InputObjectsPtr))
		return false;

	for (auto CurrentInputObject : (*InputObjectsPtr))
	{
		if (CurrentInputObject && CurrentInputObject->HasChanged())
			return true;
	}

	return false;
}

bool
UHoudiniInput::IsTransformUploadNeeded()
{
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(Type);
	if (!ensure(InputObjectsPtr))
		return false;

	for (auto CurrentInputObject : (*InputObjectsPtr))
	{
		if (CurrentInputObject && CurrentInputObject->HasTransformChanged())
			return true;
	}

	return false;
}

// Indicates if this input needs to trigger an update
bool
UHoudiniInput::NeedsToTriggerUpdate()
{
	if (bNeedsToTriggerUpdate)
		return true;

	const TArray<TObjectPtr<UHoudiniInputObject>>* InputObjectsPtr = GetHoudiniInputObjectArray(Type);
	if (!ensure(InputObjectsPtr))
		return false;

	for (auto CurrentInputObject : (*InputObjectsPtr))
	{
		if (CurrentInputObject && CurrentInputObject->NeedsToTriggerUpdate())
			return true;
	}

	return false;
}

FString 
UHoudiniInput::GetNodeBaseName() const
{
	UHoudiniAssetComponent* HAC = Cast<UHoudiniAssetComponent>(GetOuter());
	FString NodeBaseName = HAC ? HAC->GetDisplayName() : TEXT("HoudiniAsset");

	// Unfortunately CreateInputNode always prefix with input_...
	if (IsObjectPathParameter())
		NodeBaseName += TEXT("_") + GetInputName();
	else
		NodeBaseName += TEXT("_input") + FString::FromInt(GetInputIndex());

	return NodeBaseName;
}

void
UHoudiniInput::OnTransformUIExpand(const int32& AtIndex)
{
#if WITH_EDITORONLY_DATA
	if (TransformUIExpanded.IsValidIndex(AtIndex))
	{
		TransformUIExpanded[AtIndex] = !TransformUIExpanded[AtIndex];
	}
	else
	{
		// We need to append values to the expanded array
		for (int32 Index = TransformUIExpanded.Num(); Index <= AtIndex; Index++)
		{
			TransformUIExpanded.Add(Index == AtIndex ? true : false);
		}
	}
#endif
}

bool
UHoudiniInput::IsTransformUIExpanded(const int32& AtIndex)
{
#if WITH_EDITORONLY_DATA
	return TransformUIExpanded.IsValidIndex(AtIndex) ? TransformUIExpanded[AtIndex] : false;
#else
	return false;
#endif
};

FTransform*
UHoudiniInput::GetTransformOffset(const int32& AtIndex)
{
	UHoudiniInputObject* InObject = GetHoudiniInputObjectAt(AtIndex);
	if (InObject)
		return &InObject->GetTransform();

	return nullptr;
}

void
UHoudiniInput::SetUserInputAndTransformRotators(int AtIndex, const FRotator& Rotator)
{
	UHoudiniInputObject* InObject = GetHoudiniInputObjectAt(AtIndex);
	if (!InObject)
		return;

	// Set both the User Input and the transform.
	InObject->UserInputRotator = Rotator;
	InObject->GetTransform().SetRotation(Rotator.Quaternion());
}

FRotator
UHoudiniInput::GetUserInputRotator(int AtIndex) const
{
	const UHoudiniInputObject* InObject = GetHoudiniInputObjectAt(AtIndex);
	if (InObject)
		return InObject->UserInputRotator;

	return FRotator::ZeroRotator;
}

FTransform
UHoudiniInput::GetTransformOffset(const int32& AtIndex) const
{
	const UHoudiniInputObject* InObject = GetHoudiniInputObjectAt(AtIndex);
	if (InObject)
		return InObject->GetTransform();

	return FTransform::Identity;
}

TOptional<float>
UHoudiniInput::GetPositionOffsetX(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetLocation().X;
}

TOptional<float>
UHoudiniInput::GetPositionOffsetY(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetLocation().Y;
}

TOptional<float>
UHoudiniInput::GetPositionOffsetZ(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetLocation().Z;
}

TOptional<float>
UHoudiniInput::GetUserInputRoll(int32 AtIndex) const
{
	return GetUserInputRotator(AtIndex).Roll;
}

TOptional<float>
UHoudiniInput::GetUserInputPitch(int32 AtIndex) const
{
	return GetUserInputRotator(AtIndex).Pitch;
}

TOptional<float>
UHoudiniInput::GetUserInputYaw(int32 AtIndex) const
{
	return GetUserInputRotator(AtIndex).Yaw;
}

TOptional<float>
UHoudiniInput::GetScaleOffsetX(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetScale3D().X;
}

TOptional<float>
UHoudiniInput::GetScaleOffsetY(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetScale3D().Y;
}

TOptional<float>
UHoudiniInput::GetScaleOffsetZ(int32 AtIndex) const
{
	return GetTransformOffset(AtIndex).GetScale3D().Z;
}

bool
UHoudiniInput::SetTransformOffsetAt(const float& Value, const int32& AtIndex, const int32& PosRotScaleIndex, const int32& XYZIndex)
{
	FTransform* Transform = GetTransformOffset(AtIndex);
	if (!Transform)
		return false;

	FRotator UIRotator = GetUserInputRotator(AtIndex);

	if (PosRotScaleIndex == 0)
	{
		FVector Position = Transform->GetLocation();
		if (Position[XYZIndex] == Value)
			return false;
		Position[XYZIndex] = Value;
		Transform->SetLocation(Position);
	}
	else if (PosRotScaleIndex == 1)
	{
	//	FRotator Rotator = Transform->Rotator();
		switch (XYZIndex)
		{
			case 0:
			{
				if (UIRotator.Roll == Value)
					return false;
				UIRotator.Roll = Value;
				break;
			}

			case 1:
			{
				if (UIRotator.Pitch == Value)
					return false;
				UIRotator.Pitch = Value;
				break;
			}

			case 2:
			{
				if (UIRotator.Yaw == Value)
					return false;
				UIRotator.Yaw = Value;
				break;
			}
		}
		SetUserInputAndTransformRotators(AtIndex, UIRotator);
	}
	else if (PosRotScaleIndex == 2)
	{
		FVector Scale = Transform->GetScale3D();
		if (Scale[XYZIndex] == Value)
			return false;

		Scale[XYZIndex] = Value;
		Transform->SetScale3D(Scale);
	}

	MarkChanged(true);
	bStaticMeshChanged = true;

	return true;
}

void
UHoudiniInput::SetAddRotAndScaleAttributes(const bool& InValue)
{
	if (InputSettings.bAddRotAndScaleAttributesOnCurves == InValue)
		return;

	InputSettings.bAddRotAndScaleAttributesOnCurves = InValue;

	// Mark all input obj as changed
	MarkAllInputObjectsChanged(true);
}

void
UHoudiniInput::SetUseLegacyInputCurve(const bool& InValue)
{
	if (InputSettings.bUseLegacyInputCurves == InValue)
		return;

	InputSettings.bUseLegacyInputCurves = InValue;

	// Mark all input obj as changed
	MarkAllInputObjectsChanged(true);
}


bool 
UHoudiniInput::HasLandscapeExportTypeChanged () const 
{
	if (Type != EHoudiniInputType::World)
		return false;

	return bLandscapeHasExportTypeChanged;
}

void 
UHoudiniInput::SetHasLandscapeExportTypeChanged(const bool InChanged) 
{
	if (Type != EHoudiniInputType::World)
		return;

	bLandscapeHasExportTypeChanged = InChanged;
}

bool
UHoudiniInput::UpdateWorldSelectionFromBoundSelectors()
{
	TArray<TObjectPtr<AActor>>* BoundSelectorObjects = GetBoundSelectorObjectArray();
	if (!BoundSelectorObjects)
		return false;

	// Build an array of the current selection's bounds
	TArray<FBox> AllBBox;
	for (auto CurrentActor : *BoundSelectorObjects)
	{
		if (!IsValid(CurrentActor))
			continue;

		AllBBox.Add(CurrentActor->GetComponentsBoundingBox(true, true));
	}

	//
	// Select all actors in our bound selectors bounding boxes
	//

	// Get our parent component/actor
	USceneComponent* ParentComponent = Cast<USceneComponent>(GetOuter());
	AActor* ParentActor = ParentComponent ? ParentComponent->GetOwner() : nullptr;

	//UWorld* editorWorld = GEditor->GetEditorWorldContext().World();
	UWorld* MyWorld = GetWorld();
	TArray<AActor*> NewSelectedActors;
	for (TActorIterator<AActor> ActorItr(MyWorld); ActorItr; ++ActorItr)
	{
		AActor *CurrentActor = *ActorItr;
		if (!IsValid(CurrentActor))
			continue;

		// Check that actor is currently not selected
		if (BoundSelectorObjects->Contains(CurrentActor))
			continue;

		// Ignore the SkySpheres?
		FString ClassName = CurrentActor->GetClass() ? CurrentActor->GetClass()->GetName() : FString();
		if (ClassName.Contains("BP_Sky_Sphere"))
			continue;

		// Hacky solution to prevent selecting the UE5 Sky sphere mesh
		FString ActorLabel = CurrentActor->GetActorNameOrLabel();
		if (ActorLabel.Contains("SkySphere") || ActorLabel.Contains("Sky_Sphere"))
		{
			continue;
		}

		// Don't allow selection of ourselves. Bad things happen if we do.
		if (ParentActor && (CurrentActor == ParentActor))
			continue;

		// For BrushActors, both the actor and its brush must be valid
		ABrush* BrushActor = Cast<ABrush>(CurrentActor);
		if (BrushActor)
		{
			if (!IsValid(BrushActor->Brush))
				continue;
		}

		FBox ActorBounds = CurrentActor->GetComponentsBoundingBox(true);
		for (auto InBounds : AllBBox)
		{
			// Check if both actor's bounds intersects
			if (!ActorBounds.Intersect(InBounds))
				continue;

			NewSelectedActors.Add(CurrentActor);
			break;
		}
	}
	
	return UpdateWorldSelection(NewSelectedActors);
}

bool
UHoudiniInput::UpdateWorldSelection(const TArray<AActor*>& InNewSelection)
{
	// TODO: Can we use a set for NewSelectedActors instead?
	TArray<AActor*> NewSelectedActors = InNewSelection;
	TArray<TObjectPtr<UHoudiniInputObject>>* InputObjects = GetHoudiniInputObjectArray(EHoudiniInputType::World);

	// Update our current selection with the new one
	// Keep actors that are still selected, remove the one that are not selected anymore
	bool bHasSelectionChanged = false;
	for (int32 Idx = InputObjects->Num() - 1; Idx >= 0; Idx--)
	{
		UHoudiniInputActor* InputActor = Cast<UHoudiniInputActor>((*InputObjects)[Idx]);
		AActor* CurActor = InputActor ? InputActor->GetActor() : nullptr;

		if (CurActor && NewSelectedActors.Contains(CurActor))
		{
			// The actor is still selected, remove it from the new selection
			NewSelectedActors.Remove(CurActor);
		}
		else
		{
			// That actor is no longer selected, remove itr from our current selection
			DeleteInputObjectAt(EHoudiniInputType::World, Idx);
			bHasSelectionChanged = true;
		}
	}

	if (NewSelectedActors.Num() > 0)
		bHasSelectionChanged = true;

	// Then add the newly selected Actors
	int32 InputObjectIdx = GetNumberOfInputObjects(EHoudiniInputType::World);
	int32 NewInputObjectNumber = InputObjectIdx + NewSelectedActors.Num();
	SetInputObjectsNumber(EHoudiniInputType::World, NewInputObjectNumber);
	for (const auto& CurActor : NewSelectedActors)
	{
		// Update the input objects from the valid selected actors array
		SetInputObjectAt(InputObjectIdx++, CurActor);
	}

	if (!bIsWorldInputBoundSelector && InputSettings.bLandscapeAutoSelectSplines && AddAllLandscapeSplineActorsForInputLandscapes())
		bHasSelectionChanged = true;
	
	MarkChanged(bHasSelectionChanged);

	return bHasSelectionChanged;
}


bool
UHoudiniInput::ContainsInputObject(const UObject* InObject, const EHoudiniInputType& InType) const
{
	if (!IsValid(InObject))
		return false;

	// Returns true if the object is one of our input object for the given type
	const TArray<TObjectPtr<UHoudiniInputObject>>* ObjectArray = GetHoudiniInputObjectArray(InType);
	if (!ObjectArray)
		return false;

	for (auto& CurrentInputObject : (*ObjectArray))
	{
		if (!IsValid(CurrentInputObject))
			continue;

		if (CurrentInputObject->GetObject() == InObject)
			return true;
	}

	return false;
}

void UHoudiniInput::ForAllHoudiniInputObjects(TFunctionRef<void(UHoudiniInputObject*)> Fn, const bool bFilterByInputType) const
{
	auto ShouldIncludeFn = [&](const EHoudiniInputType InInputType) -> bool
	{
		return !bFilterByInputType || (bFilterByInputType && GetInputType() == InInputType);
	};
	
	if (ShouldIncludeFn(EHoudiniInputType::Geometry))
	{
		for(UHoudiniInputObject* InputObject : GeometryInputObjects)
		{
			Fn(InputObject);
		}
	}

	if (ShouldIncludeFn(EHoudiniInputType::Curve))
	{
		for(UHoudiniInputObject* InputObject : CurveInputObjects)
		{
			Fn(InputObject);
		}
	}

	if (ShouldIncludeFn(EHoudiniInputType::World))
	{
		for(UHoudiniInputObject* InputObject : WorldInputObjects)
		{
			Fn(InputObject);
		}
	}
}

TArray<const TArray<TObjectPtr<UHoudiniInputObject>>*> UHoudiniInput::GetAllObjectArrays() const
{
	return { &GeometryInputObjects, &CurveInputObjects, &WorldInputObjects };
}

TArray<TArray<TObjectPtr<UHoudiniInputObject>>*> UHoudiniInput::GetAllObjectArrays()
{
	return { &GeometryInputObjects, &CurveInputObjects, &WorldInputObjects };
}

void UHoudiniInput::ForAllHoudiniInputObjectArrays(TFunctionRef<void(const TArray<TObjectPtr<UHoudiniInputObject>>&)> Fn) const
{
	TArray<const TArray<TObjectPtr<UHoudiniInputObject>>*> ObjectArrays = GetAllObjectArrays();
	for (const TArray<TObjectPtr<UHoudiniInputObject>>* ObjectArrayPtr : ObjectArrays)
	{
		if (!ObjectArrayPtr)
			continue;
		Fn(*ObjectArrayPtr);
	}
}

void UHoudiniInput::ForAllHoudiniInputObjectArrays(TFunctionRef<void(TArray<TObjectPtr<UHoudiniInputObject>>&)> Fn)
{
	TArray<TArray<TObjectPtr<UHoudiniInputObject>>*> ObjectArrays = GetAllObjectArrays();
	for (TArray<TObjectPtr<UHoudiniInputObject>>* ObjectArrayPtr : ObjectArrays)
	{
		if (!ObjectArrayPtr)
			continue;
		Fn(*ObjectArrayPtr);
	}
}

void UHoudiniInput::GetAllHoudiniInputObjects(TArray<UHoudiniInputObject*>& OutObjects) const
{
	OutObjects.Empty();
	auto AddInputObject = [&OutObjects](UHoudiniInputObject* InputObject)
	{
		if (InputObject)
			OutObjects.Add(InputObject);
	};
	ForAllHoudiniInputObjects(AddInputObject);
}

void UHoudiniInput::ForAllHoudiniInputSceneComponents(TFunctionRef<void(UHoudiniInputSceneComponent*)> Fn) const
{
	auto ProcessSceneComponent = [Fn](UHoudiniInputObject* InputObject)
	{
		if (!InputObject)
			return;
		UHoudiniInputSceneComponent* SceneComponentInput = Cast<UHoudiniInputSceneComponent>(InputObject);
		if (!SceneComponentInput)
			return;
		Fn(SceneComponentInput);
	};
	ForAllHoudiniInputObjects(ProcessSceneComponent);
}

void UHoudiniInput::GetAllHoudiniInputSceneComponents(TArray<UHoudiniInputSceneComponent*>& OutObjects) const
{
	OutObjects.Empty();
	auto AddSceneComponent = [&OutObjects](UHoudiniInputObject* InputObject)
	{
		if (!InputObject)
			return;
		UHoudiniInputSceneComponent* SceneComponentInput = Cast<UHoudiniInputSceneComponent>(InputObject);
		if (!SceneComponentInput)
			return;
		OutObjects.Add(SceneComponentInput);
	};
	ForAllHoudiniInputObjects(AddSceneComponent);
}

void UHoudiniInput::GetAllHoudiniInputSplineComponents(TArray<UHoudiniInputHoudiniSplineComponent*>& OutObjects) const
{
	OutObjects.Empty();
	auto AddSceneComponent = [&OutObjects](UHoudiniInputObject* InputObject)
	{
		if (!InputObject)
			return;
		UHoudiniInputHoudiniSplineComponent* SceneComponentInput = Cast<UHoudiniInputHoudiniSplineComponent>(InputObject);
		if (!SceneComponentInput)
			return;
		OutObjects.Add(SceneComponentInput);
	};
	ForAllHoudiniInputObjects(AddSceneComponent);
}


void UHoudiniInput::RemoveHoudiniInputObject(UHoudiniInputObject* InInputObject)
{
	if (!InInputObject)
		return;

	ForAllHoudiniInputObjectArrays([InInputObject](TArray<TObjectPtr<UHoudiniInputObject>>& ObjectArray) {
		ObjectArray.Remove(InInputObject);
	});

	return;
}

void UHoudiniInput::SetLandscapeAutoSelectSplines(const bool bInLandscapeAutoSelectSplines)
{
	if (bInLandscapeAutoSelectSplines == InputSettings.bLandscapeAutoSelectSplines)
		return;
	
	InputSettings.bLandscapeAutoSelectSplines = bInLandscapeAutoSelectSplines;
}

bool
UHoudiniInput::RemoveAllLandscapeSplineActorsForInputLandscapes()
{
	// Only support world inputs
	if (Type != EHoudiniInputType::World)
		return false;
	
	// First determine all selected landscapes
	const int32 NumWorldInputObjects = WorldInputObjects.Num();
	TSet<ALandscapeProxy const*> SelectedLandscapes;
	for (UHoudiniInputObject const* const InputObject : WorldInputObjects)
	{
		if (!IsValid(InputObject))
			continue;
		UHoudiniInputLandscape const* const InputLandscape = Cast<UHoudiniInputLandscape>(InputObject);
		if (!IsValid(InputLandscape))
			continue;
		ALandscapeProxy const* const InputLandscapeProxy = InputLandscape->GetLandscapeProxy();
		if (!IsValid(InputLandscapeProxy))
			continue;
		if (SelectedLandscapes.IsEmpty())
			SelectedLandscapes.Reserve(NumWorldInputObjects);
		SelectedLandscapes.Add(InputLandscapeProxy);
	}

	// If no landscapes are selected, then we don't have to deselect any landscape splines
	if (SelectedLandscapes.IsEmpty())
		return false;

	// Get the index of each landscape spline that is in our world input objects selection that belongs to one of
	// the selected landscapes
	TArray<int32> IndicesToRemove;
	for (int32 Index = 0; Index < NumWorldInputObjects; ++Index)
	{
		UHoudiniInputObject const* const InputObject = WorldInputObjects[Index];
		if (!IsValid(InputObject))
			continue;
		UHoudiniInputLandscapeSplineActor const* const InputObjectSplineActor = Cast<UHoudiniInputLandscapeSplineActor>(InputObject);
		if (!IsValid(InputObjectSplineActor))
			continue;
		ALandscapeSplineActor const* const SplineActor = InputObjectSplineActor->GetLandscapeSplineActor();
		if (!IsValid(SplineActor))
			continue;
		ULandscapeInfo const* const LandscapeInfo = SplineActor->GetLandscapeInfo();
		if (!IsValid(LandscapeInfo))
			continue;
		ALandscapeProxy const* const LandscapeProxy = LandscapeInfo->GetLandscapeProxy();
		if (!IsValid(LandscapeProxy))
			continue;
		if (!SelectedLandscapes.Contains(LandscapeProxy))
			continue;
		if (IndicesToRemove.IsEmpty())
			IndicesToRemove.Reserve(NumWorldInputObjects);
		IndicesToRemove.Add(Index);
	}

	// Remove the landscape splines from highest index to lowest index
	const int32 NumIndicesToRemove = IndicesToRemove.Num();
	if (NumIndicesToRemove <= 0)
		return false;
	
	for (int32 Index = NumIndicesToRemove - 1; Index >= 0; --Index)
	{
		const int32 IndexToRemove = IndicesToRemove[Index];
		DeleteInputObjectAt(EHoudiniInputType::World, IndexToRemove);
	}

	return true;
}

bool
UHoudiniInput::AddAllLandscapeSplineActorsForInputLandscapes()
{
#if WITH_EDITOR
	// Only support world inputs
	if (Type != EHoudiniInputType::World)
		return false;
	
	// Find the current input landscape and splines
	TSet<UHoudiniInputLandscape const*> InputLandscapes;
	TSet<ALandscapeSplineActor const*> InputLandscapeSpineActors;
	
	for (UHoudiniInputObject const* const InputObject : WorldInputObjects)
	{
		if (!IsValid(InputObject))
			continue;
		
		UHoudiniInputLandscape const* const LandscapeObject = Cast<UHoudiniInputLandscape>(InputObject);
		if (IsValid(LandscapeObject))
		{
			InputLandscapes.Add(LandscapeObject);
			continue;
		}
		
		UHoudiniInputLandscapeSplineActor const* const LandscapeSplineActor = Cast<UHoudiniInputLandscapeSplineActor>(InputObject);
		if (!IsValid(LandscapeSplineActor))
			continue;

		ALandscapeSplineActor const* const SplineActor = LandscapeSplineActor->GetLandscapeSplineActor();
		if (!IsValid(SplineActor))
			continue;

		InputLandscapeSpineActors.Add(SplineActor);
	}

	if (InputLandscapes.IsEmpty())
		return false;
	
	// Get all spline actors that belong to InputLandscape splines but that are not in InputLandscapeSplineActors
	TArray<ALandscapeSplineActor*> SplinesToAdd;
	for (UHoudiniInputLandscape const* const InputLandscape : InputLandscapes)
	{
		if (!IsValid(InputLandscape))
			continue;
		ALandscapeProxy const* const LandscapeProxy = InputLandscape->GetLandscapeProxy();
		if (!IsValid(LandscapeProxy))
			continue;
		ULandscapeInfo const* const LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
		if (!IsValid(LandscapeInfo))
			continue;
		LandscapeInfo->ForAllSplineActors([&SplinesToAdd, &InputLandscapeSpineActors](TScriptInterface<ILandscapeSplineInterface> InSplineActor)
		{
			ALandscapeSplineActor* const SplineActor = Cast<ALandscapeSplineActor>(InSplineActor.GetObject());
			if (IsValid(SplineActor) && !InputLandscapeSpineActors.Contains(SplineActor))
				SplinesToAdd.Add(SplineActor);
		});
	}

	// There are no new spline actors to add
	if (SplinesToAdd.IsEmpty())
		return false;

	// Add SplinesToAdd to the input object array
	const int32 NumInputObjects = GetNumberOfInputObjects();
	const int32 NumToAdd = SplinesToAdd.Num();
	SetInputObjectsNumber(GetInputType(), NumInputObjects + NumToAdd);
	for (int32 Index = 0; Index < NumToAdd; ++Index)
		SetInputObjectAt(NumInputObjects + Index, SplinesToAdd[Index]);

	return true;
#else
	return false;
#endif
}
