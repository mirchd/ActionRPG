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

#include "HoudiniInstanceTranslator.h"

#include "HoudiniEngine.h"
#include "HoudiniEngineRuntimeUtils.h"
#include "HoudiniEngineString.h"
#include "HoudiniEngineUtils.h"
#include "HoudiniEnginePrivatePCH.h"
#include "HoudiniGenericAttribute.h"
#include "HoudiniInstancedActorComponent.h"
#include "HoudiniMaterialTranslator.h"
#include "HoudiniMeshSplitInstancerComponent.h"
#include "HoudiniOutput.h"
#include "HoudiniStaticMeshComponent.h"
#include "HoudiniStaticMesh.h"
#include "HoudiniFoliageTools.h"
#include "HoudiniHLODLayerUtils.h"

//#include "HAPI/HAPI_Common.h"

#include "Engine/StaticMesh.h"
#include "ComponentReregisterContext.h"
#include "HoudiniMaterialTranslator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "InstancedFoliageActor.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	#include "GeometryCollection/GeometryCollectionComponent.h"
#else
	#include "GeometryCollectionEngine/Public/GeometryCollection/GeometryCollectionComponent.h"
#endif
#include "FoliageEditUtility.h"
#include "LevelInstance/LevelInstanceActor.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
#include "LevelInstance/LevelInstanceComponent.h"
#endif

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	#include "StaticMeshComponentLODInfo.h"
#endif

#if WITH_EDITOR
	//#include "ScopedTransaction.h"
	#include "LevelEditorViewport.h"
	#include "MeshPaintHelpers.h"
#endif
#include "HoudiniEngineAttributes.h"
#include "HoudiniFoliageUtils.h"
#include "HoudiniMeshTranslator.h"

#define LOCTEXT_NAMESPACE HOUDINI_LOCTEXT_NAMESPACE

// Fastrand is a faster alternative to std::rand()
// and doesn't oscillate when looking for 2 values like Unreal's.
inline int fastrand(int& nSeed)
{
	nSeed = (214013 * nSeed + 2531011);
	return (nSeed >> 16) & 0x7FFF;
}

//
bool
FHoudiniInstanceTranslator::PopulateInstancedOutputPartData(
	const FHoudiniGeoPartObject& InHGPO,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	FHoudiniInstancedOutputPartData& OutInstancedOutputPartData,
	TSet<UObject*>& OutInvisibleObjects)
{
	// Get if force to use HISM from attribute
	OutInstancedOutputPartData.bForceHISM = HasHISMAttribute(InHGPO.GeoId, InHGPO.PartId);

	// Should we create an instancer even for single instances?
	OutInstancedOutputPartData.bForceInstancer = HasForceInstancerAttribute(InHGPO.GeoId, InHGPO.PartId);

	// Extract the object and transforms for this instancer
	if (!GetInstancerObjectsAndTransforms(
			InHGPO,
			InAllOutputs,
			OutInstancedOutputPartData.OriginalInstancedObjects,
			OutInstancedOutputPartData.OriginalInstancedTransforms,
			OutInstancedOutputPartData.OriginalInstancedIndices,
			OutInstancedOutputPartData.SplitAttributeName,
			OutInstancedOutputPartData.SplitAttributeValues,
			OutInstancedOutputPartData.PerSplitAttributes,
			OutInvisibleObjects))
		return false;
	
	// Check if this is a No-Instancers ( unreal_split_instances )
	OutInstancedOutputPartData.bSplitMeshInstancer = IsSplitInstancer(InHGPO.GeoId, InHGPO.PartId);

	OutInstancedOutputPartData.bIsFoliageInstancer = IsFoliageInstancer(InHGPO.GeoId, InHGPO.PartId);

	// Extract the generic attributes
	GetGenericPropertiesAttributes(InHGPO.GeoId, InHGPO.PartId, OutInstancedOutputPartData.AllPropertyAttributes);

	// Check for per instance custom data
	GetPerInstanceCustomData(InHGPO.GeoId, InHGPO.PartId, OutInstancedOutputPartData);

	//Get the level path attribute on the instancer
	if (!FHoudiniEngineUtils::GetLevelPathAttribute(InHGPO.GeoId, InHGPO.PartId, OutInstancedOutputPartData.AllLevelPaths))
	{
		// No attribute specified
		OutInstancedOutputPartData.AllLevelPaths.Empty();
	}

	// Get the output name attribute
	if (!FHoudiniEngineUtils::GetOutputNameAttribute(InHGPO.GeoId, InHGPO.PartId,  OutInstancedOutputPartData.OutputNames))
	{
		// No attribute specified
		OutInstancedOutputPartData.OutputNames.Empty();
	}

	// Get the bake name attribute
	if (!FHoudiniEngineUtils::GetBakeNameAttribute(InHGPO.GeoId, InHGPO.PartId, OutInstancedOutputPartData.BakeNames))
	{
		// No attribute specified
		OutInstancedOutputPartData.BakeNames.Empty();
	}

	// See if we have a tile attribute
	if (!FHoudiniEngineUtils::GetTileAttribute(InHGPO.GeoId, InHGPO.PartId,  OutInstancedOutputPartData.TileValues))
	{
		// No attribute specified
		OutInstancedOutputPartData.TileValues.Empty();
	}

	// Get the bake actor attribute
	if (!FHoudiniEngineUtils::GetBakeActorAttribute(InHGPO.GeoId, InHGPO.PartId,  OutInstancedOutputPartData.AllBakeActorNames))
	{
		// No attribute specified
		OutInstancedOutputPartData.AllBakeActorNames.Empty();
	}

	// Get the bake actor class attribute
	if (!FHoudiniEngineUtils::GetBakeActorClassAttribute(InHGPO.GeoId, InHGPO.PartId,  OutInstancedOutputPartData.AllBakeActorClassNames))
	{
		// No attribute specified
		OutInstancedOutputPartData.AllBakeActorClassNames.Empty();
	}

	// Get the unreal_bake_folder attribute
	if (!FHoudiniEngineUtils::GetBakeFolderAttribute(InHGPO.GeoId, OutInstancedOutputPartData.AllBakeFolders, InHGPO.PartId))
	{
		// No attribute specified
		OutInstancedOutputPartData.AllBakeFolders.Empty();
	}

	// Get the bake outliner folder attribute
	if (!FHoudiniEngineUtils::GetBakeOutlinerFolderAttribute(InHGPO.GeoId, InHGPO.PartId,  OutInstancedOutputPartData.AllBakeOutlinerFolders))
	{
		// No attribute specified
		OutInstancedOutputPartData.AllBakeOutlinerFolders.Empty();
	}

	// See if we have instancer material overrides
	if (!GetMaterialOverridesFromAttributes(InHGPO.GeoId, InHGPO.PartId, 0, InHGPO.InstancerType, OutInstancedOutputPartData.MaterialAttributes))
		OutInstancedOutputPartData.MaterialAttributes.Empty();
	OutInstancedOutputPartData.DataLayers = FHoudiniDataLayerUtils::GetDataLayers(InHGPO.GeoId, InHGPO.PartId, HAPI_GROUPTYPE_POINT);
	OutInstancedOutputPartData.HLODLayers = FHoudiniHLODLayerUtils::GetHLODLayers(InHGPO.GeoId, InHGPO.PartId, HAPI_ATTROWNER_POINT);

	return true;
}

int
FHoudiniInstanceTranslator::CreateAllInstancersFromHoudiniOutputs(
	const TArray<UHoudiniOutput*>& InAllOutputs,
	UObject* InOuterComponent,
	const FHoudiniPackageParams& InPackageParms,
	const TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutputPartData>* InPreBuiltInstancedOutputPartData)
{
	return CreateAllInstancersFromHoudiniOutputs(
		InAllOutputs,
		InAllOutputs,
		InOuterComponent,
		InPackageParms,
		InPreBuiltInstancedOutputPartData);
}

int
FHoudiniInstanceTranslator::CreateAllInstancersFromHoudiniOutputs(
	const TArray<UHoudiniOutput*>& OutputsToUpdate,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	UObject* InOuterComponent,
	const FHoudiniPackageParams& InPackageParms,
	const TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutputPartData>* InPreBuiltInstancedOutputPartData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHoudiniInstanceTranslator::CreateAllInstancersFromHoudiniOutputs);
	int FoliageTypeCount = 0;

	USceneComponent* ParentComponent = Cast<USceneComponent>(InOuterComponent);
	if (!ParentComponent)
		return false;

    int InstanceCount = 0;
	for (auto Output : OutputsToUpdate)
	{
		if (Output->GetType() != EHoudiniOutputType::Instancer)
			continue;

		for(auto OutputObject : Output->GetOutputObjects())
		{
			// Calling RemoveFoliageTypeFromWorld() with null dirties every FoliageInstanceActor, even if it ends up not actually changing them. 
			if (!IsValid(OutputObject.Value.FoliageType))
				continue;

			for(auto & OutputComponent : OutputObject.Value.OutputComponents)
			{
				if (OutputComponent)
					FHoudiniFoliageUtils::RemoveFoliageTypeFromWorld(OutputComponent->GetWorld(), OutputObject.Value.FoliageType);
			}
		}


		bool bSuccess = FHoudiniInstanceTranslator::CreateAllInstancersFromHoudiniOutput(
			Output,
			InAllOutputs,
			InOuterComponent,
			InPackageParms,
			FoliageTypeCount,
			InPreBuiltInstancedOutputPartData);

		if (bSuccess)
			++InstanceCount;
	}

	if (FoliageTypeCount > 0)
	{
		FHoudiniEngineUtils::RepopulateFoliageTypeListInUI();
	}
	return InstanceCount;
}


bool
FHoudiniInstanceTranslator::CreateAllInstancersFromHoudiniOutput(
	UHoudiniOutput* InOutput,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	UObject* InOuterComponent,
	const FHoudiniPackageParams& InPackageParms,
	int & FoliageTypeCount,
	const TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutputPartData>* InPreBuiltInstancedOutputPartData
)
{
	if (!IsValid(InOutput))
		return false;

	if (!IsValid(InOuterComponent))
		return false;

	if (InOutput->Type == EHoudiniOutputType::GeometryCollection)
		return true;

	// Keep track of the previous cook's component to clean them up after
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniOutputObject> NewOutputObjects;
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniOutputObject> OldOutputObjects = InOutput->GetOutputObjects();

	TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutput>& InstancedOutputs = InOutput->GetInstancedOutputs();
	// Mark all the current instanced output as stale
	for (auto& InstOut : InstancedOutputs)
		InstOut.Value.bStale = true;

	USceneComponent* ParentComponent = Cast<USceneComponent>(InOuterComponent);
	if (!ParentComponent)
		return false;

	// The default SM to be used if the instanced object has not been found (when using attribute instancers)
	UStaticMesh * DefaultReferenceSM = FHoudiniEngine::Get().GetHoudiniDefaultReferenceMesh().Get();

	TSet<UObject*> InvisibleObjects;

	// Iterate on all of the output's HGPO, creating meshes as we go
	for (const FHoudiniGeoPartObject& CurHGPO : InOutput->HoudiniGeoPartObjects)
	{
		// Not an instancer, skip
		if (CurHGPO.Type != EHoudiniPartType::Instancer)
			continue;

		// Prepare this output object's output identifier
		FHoudiniOutputObjectIdentifier OutputIdentifier;
		OutputIdentifier.ObjectId = CurHGPO.ObjectId;
		OutputIdentifier.GeoId = CurHGPO.GeoId;
		OutputIdentifier.PartId = CurHGPO.PartId;
		OutputIdentifier.PartName = CurHGPO.PartName;

		FHoudiniInstancedOutputPartData InstancedOutputPartDataTmp;
		const FHoudiniInstancedOutputPartData* InstancedOutputPartDataPtr = nullptr;
		if (InPreBuiltInstancedOutputPartData)
		{
			InstancedOutputPartDataPtr = InPreBuiltInstancedOutputPartData->Find(OutputIdentifier);
		}
		if (!InstancedOutputPartDataPtr)
		{
			if (!PopulateInstancedOutputPartData(CurHGPO, InAllOutputs, InstancedOutputPartDataTmp, InvisibleObjects))
				continue;
			InstancedOutputPartDataPtr = &InstancedOutputPartDataTmp;
		}

		const FHoudiniInstancedOutputPartData& InstancedOutputPartData = *InstancedOutputPartDataPtr;
		
		/*
		TArray<UMaterialInterface*> InstancerMaterials;
		if (!InstancedOutputPartData.bMaterialOverrideNeedsCreateInstance)
		{
			if (!GetInstancerMaterials(InstancedOutputPartData.MaterialAttributes, InstancerMaterials))
				InstancerMaterials.Empty();
		}
		else
		{
			if (!GetInstancerMaterialInstances(InstancedOutputPartData.MaterialAttributes, CurHGPO, InPackageParms, InstancerMaterials))
				InstancerMaterials.Empty();
		}*/

		//
		// TODO: REFACTOR THIS!
		//
		// We create an instanced output per original object 
		// These original object can then potentially be replaced by variations
		// Each variations will create a instance component / OutputObject
		// Currently we process all original objects AND their variations at the same time
		// we should instead loop on the original objects
		//	- get their variations objects/transform 
		//  - create the appropriate instancer
		// This means modifying UpdateInstanceVariationsObjects so that it works using 
		// a single OriginalObject instead of using an array
		// Also, apply the same logic to UpdateChangedInstanceOutput
		//

		// Array containing all the variations objects for all the original objects
		TArray<TSoftObjectPtr<UObject>> VariationInstancedObjects;
		// Array containing all the variations transforms
		TArray<TArray<FTransform>> VariationInstancedTransforms;
		// Array indicate the original object index for each variation
		TArray<int32> VariationOriginalObjectIndices;
		// Array indicate the variation number for each variation
		TArray<int32> VariationIndices;
		// Update our variations using the instanced outputs
		UpdateInstanceVariationObjects(
			OutputIdentifier,
			InstancedOutputPartData.OriginalInstancedObjects,
			InstancedOutputPartData.OriginalInstancedTransforms,
			InstancedOutputPartData.OriginalInstancedIndices,
			InOutput->GetInstancedOutputs(),
			VariationInstancedObjects,
			VariationInstancedTransforms, 
			VariationOriginalObjectIndices,
			VariationIndices);

		// Preload objects so we can benefit from async compilation as much as possible
		for (int32 InstanceObjectIdx = 0; InstanceObjectIdx < VariationInstancedObjects.Num(); InstanceObjectIdx++)
		{
			UObject* InstancedObject = VariationInstancedObjects[InstanceObjectIdx].LoadSynchronous();
			if (IsValid(InstancedObject) && InstancedObject->IsA<UBlueprintGeneratedClass>())
			{
				// Crash fix from VA
				// UE5.5 seems to no longer be able to load/instantiate BPGenerated Classes in the Editor.
				// Instead, we should use its source BP instead. Warn the user and replace the class to instantiate.
				HOUDINI_LOG_WARNING(TEXT("Loading a BlueprintGeneratedClass is no longer supported. Loading its BlueprintClass instead - %s"), *InstancedObject->GetPathName());

				UBlueprintGeneratedClass * BPGenClass = Cast<UBlueprintGeneratedClass>(InstancedObject);
				UObject* SourceBPClass = nullptr;
				if (IsValid(BPGenClass) && IsValid(BPGenClass->ClassGeneratedBy))
				{
					SourceBPClass = BPGenClass->ClassGeneratedBy;					
				}

				VariationInstancedObjects[InstanceObjectIdx] = SourceBPClass;
			}
		}

		// Create the instancer components now
		for (int32 InstanceObjectIdx = 0; InstanceObjectIdx < VariationInstancedObjects.Num(); InstanceObjectIdx++)
		{
			UObject* InstancedObject = VariationInstancedObjects[InstanceObjectIdx].LoadSynchronous();
			if (!IsValid(InstancedObject))
				continue;

			if (!VariationInstancedTransforms.IsValidIndex(InstanceObjectIdx))
				continue;

			const TArray<FTransform>& InstancedObjectTransforms = VariationInstancedTransforms[InstanceObjectIdx];
			if (InstancedObjectTransforms.Num() <= 0)
				continue;

			// Get the original Index of that variations
			int32 VariationOriginalIndex = VariationOriginalObjectIndices[InstanceObjectIdx];

			// Find the matching instance output now
			FHoudiniInstancedOutput* FoundInstancedOutput = nullptr;
			{
				// Instanced output only use the original object index for their split identifier
				FHoudiniOutputObjectIdentifier InstancedOutputIdentifier = OutputIdentifier;
				InstancedOutputIdentifier.SplitIdentifier = FString::FromInt(VariationOriginalIndex);
				FoundInstancedOutput = InstancedOutputs.Find(InstancedOutputIdentifier);
			}

			// Update the split identifier for this object
			// We use both the original object index and the variation index: ORIG_VAR
			OutputIdentifier.SplitIdentifier = 
				FString::FromInt(VariationOriginalIndex)
				+ TEXT("_")
				+ FString::FromInt(VariationIndices[InstanceObjectIdx]);
				
			// Get the OutputObj for this variation
			FHoudiniOutputObject* OldOutputObject = OldOutputObjects.Find(OutputIdentifier);
			// See if we can find an preexisting objects for this obj	to try to reuse it
			TArray<USceneComponent*> OldInstancerComponents;
			TArray<AActor*> OldInstancerActors;

			const bool bIsProxyMesh = InstancedObject->IsA<UHoudiniStaticMesh>();
			if (OldOutputObject)
			{
				if (bIsProxyMesh)
				{
					OldInstancerComponents.Add(Cast<USceneComponent>(OldOutputObject->ProxyComponent));
				}
				else
				{
					for(auto Component : OldOutputObject->OutputComponents)
					    OldInstancerComponents.Add(Cast<USceneComponent>(Component));

					for (auto Actor : OldOutputObject->OutputActors)
						OldInstancerActors.Add(Actor.Get());
				}
			}

			// Get all the materials needed for this object
			// Multiple material slots are supported, as well as creating new material instances if needed
			TArray<UMaterialInterface*> VariationMaterials;
			// We need to get the point / prim indices of the split via InstancedOutputPartData.OriginalInstancedIndices
			// to access the material attributes from Houdini/HAPI
			int32 FirstOriginalIndex = 0;
			if (InstancedOutputPartData.OriginalInstancedIndices.IsValidIndex(VariationOriginalIndex))
			{
				const TArray<int32>& OriginalInstancerObjectIndices = InstancedOutputPartData.OriginalInstancedIndices[VariationOriginalIndex];
				if (OriginalInstancerObjectIndices.Num() > 0)
					FirstOriginalIndex = OriginalInstancerObjectIndices[0];
			}
			if (!GetAllInstancerMaterials(OutputIdentifier.GeoId, OutputIdentifier.PartId, FirstOriginalIndex, CurHGPO, InPackageParms, VariationMaterials))
				VariationMaterials.Empty();

			TArray<USceneComponent*> NewInstancerComponents;
			TArray<AActor*> NewInstancerActors;

			UFoliageType* FoliageTypeUsed = nullptr;
			UWorld * WorldUsed = nullptr;

			if (!CreateOrUpdateInstancer(
				InstancedObject,
				InstancedObjectTransforms,
				InstancedOutputPartData.AllPropertyAttributes,
				CurHGPO,
				InPackageParms,
				ParentComponent,
				OldInstancerComponents,
				NewInstancerComponents,
				OldInstancerActors,
				NewInstancerActors,
				InstancedOutputPartData.bSplitMeshInstancer,
				InstancedOutputPartData.bIsFoliageInstancer,
				VariationMaterials,
				InstancedOutputPartData.OriginalInstancedIndices[VariationOriginalIndex],
				FoliageTypeCount,
				FoliageTypeUsed,
				WorldUsed, 
				InstancedOutputPartData.bForceHISM,
				InstancedOutputPartData.bForceInstancer))
			{
				// TODO??
				continue;
			}

			if (NewInstancerComponents.IsEmpty() && NewInstancerActors.IsEmpty())
				continue;


			for(auto NewInstancerComponent : NewInstancerComponents)
			{
				if (InvisibleObjects.Contains(InstancedObject))
				{
					NewInstancerComponent->SetVisibleFlag(false);	
				}

				// Copy the per-instance custom data if we have any
				if (InstancedOutputPartData.PerInstanceCustomData.Num() > 0)
				{
				    UpdateChangedPerInstanceCustomData(
					    InstancedOutputPartData.PerInstanceCustomData[VariationOriginalIndex], NewInstancerComponent);

				    // See if the HiddenInGame property is overriden
				    bool bOverridesHiddenInGame = false;
				    for (auto& CurPropAttr : InstancedOutputPartData.AllPropertyAttributes)
				    {
					    if (CurPropAttr.AttributeName.Equals(TEXT("HiddenInGame"))
						    || CurPropAttr.AttributeName.Equals(TEXT("bHiddenInGame")))
					    {
						    bOverridesHiddenInGame = true;
					    }
				    }

				    // If the instanced object (by ref) wasn't found, hide the component in game
				    if (InstancedObject == DefaultReferenceSM)
					    NewInstancerComponent->SetHiddenInGame(true);
				    else
				    {
					    // Dont force the property if it is overriden by generic attributes
					    if (!bOverridesHiddenInGame)
						    NewInstancerComponent->SetHiddenInGame(false);
				    }
			    }
			}

			FHoudiniOutputObject& NewOutputObject = NewOutputObjects.FindOrAdd(OutputIdentifier);
			NewOutputObject.UserFoliageType = Cast<UFoliageType>(InstancedObject);
			NewOutputObject.FoliageType = FoliageTypeUsed;
			NewOutputObject.World = WorldUsed;

			if (bIsProxyMesh)
			{
				NewOutputObject.ProxyComponent = NewInstancerComponents.Num() > 0 ? NewInstancerComponents[0] : nullptr;
				NewOutputObject.ProxyObject = InstancedObject;
			}
			else
			{
				check(NewOutputObject.OutputComponents.Num() < 2); // Multiple components not supported yet.
				NewOutputObject.OutputComponents.Empty();
				for(auto NewComponent : NewInstancerComponents)
				    NewOutputObject.OutputComponents.Add(NewComponent);
				NewOutputObject.OutputObject = nullptr;
			}

			for(auto & ActorPtr : NewInstancerActors)
				NewOutputObject.OutputActors.Add(ActorPtr);

			// If this is not a new output object we have to clear the CachedAttributes and CachedTokens before
			// setting the new values (so that we do not re-use any values from the previous cook)
			NewOutputObject.CachedAttributes.Empty();
			NewOutputObject.CachedTokens.Empty();

			// Cache the level path, output name and tile attributes on the output object So they can be reused for baking
			int32 FirstOriginalInstanceIndex = 0;
			if(InstancedOutputPartData.OriginalInstancedIndices.IsValidIndex(VariationOriginalIndex) && InstancedOutputPartData.OriginalInstancedIndices[VariationOriginalIndex].Num() > 0)
				FirstOriginalInstanceIndex = InstancedOutputPartData.OriginalInstancedIndices[VariationOriginalIndex][0];

			if(InstancedOutputPartData.AllLevelPaths.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.AllLevelPaths[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_LEVEL_PATH, InstancedOutputPartData.AllLevelPaths[FirstOriginalInstanceIndex]);

			if(InstancedOutputPartData.OutputNames.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.OutputNames[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(FString(HAPI_UNREAL_ATTRIB_CUSTOM_OUTPUT_NAME_V2), InstancedOutputPartData.OutputNames[FirstOriginalInstanceIndex]);

			if(InstancedOutputPartData.BakeNames.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.BakeNames[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(FString(HAPI_UNREAL_ATTRIB_BAKE_NAME), InstancedOutputPartData.BakeNames[FirstOriginalInstanceIndex]);

			// TODO: Check! maybe accessed with just VariationOriginalIndex
			if(InstancedOutputPartData.TileValues.IsValidIndex(FirstOriginalInstanceIndex) && InstancedOutputPartData.TileValues[FirstOriginalInstanceIndex] >= 0)
			{
				// cache the tile attribute as a token on the output object
				NewOutputObject.CachedTokens.Add(TEXT("tile"), FString::FromInt(InstancedOutputPartData.TileValues[FirstOriginalInstanceIndex]));
			}

			if(InstancedOutputPartData.AllBakeActorNames.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.AllBakeActorNames[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_ACTOR, InstancedOutputPartData.AllBakeActorNames[FirstOriginalInstanceIndex]);

			if(InstancedOutputPartData.AllBakeActorClassNames.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.AllBakeActorClassNames[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_ACTOR_CLASS, InstancedOutputPartData.AllBakeActorClassNames[FirstOriginalInstanceIndex]);

			if(InstancedOutputPartData.HLODLayers.IsValidIndex(FirstOriginalInstanceIndex))
			{
				NewOutputObject.HLODLayers.Add(InstancedOutputPartData.HLODLayers[FirstOriginalInstanceIndex]);
			}

			if (InstancedOutputPartData.DataLayers.IsValidIndex(FirstOriginalInstanceIndex))
			{
				NewOutputObject.DataLayers = InstancedOutputPartData.DataLayers[FirstOriginalInstanceIndex].DataLayers;
			}

			// TODO: Check if we should apply the same logic to other cached attributes?
			// When using PDG, we have one bake folder per PDG output (array size 1)
			// However, the translator expects one BakeFolder per instance!
			// This causes variation 0 to use the proper bake folder, but other variations to end up in the default bake folder.
			// Use this fallback mechanism so that all bake instances end up in the same folder
			if(InstancedOutputPartData.AllBakeFolders.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.AllBakeFolders[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_FOLDER, InstancedOutputPartData.AllBakeFolders[FirstOriginalInstanceIndex]);
			else if (InstancedOutputPartData.AllBakeFolders.IsValidIndex(0) && !InstancedOutputPartData.AllBakeFolders[0].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_FOLDER, InstancedOutputPartData.AllBakeFolders[0]);

			if(InstancedOutputPartData.AllBakeOutlinerFolders.IsValidIndex(FirstOriginalInstanceIndex) && !InstancedOutputPartData.AllBakeOutlinerFolders[FirstOriginalInstanceIndex].IsEmpty())
				NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_OUTLINER_FOLDER, InstancedOutputPartData.AllBakeOutlinerFolders[FirstOriginalInstanceIndex]);

			if(InstancedOutputPartData.SplitAttributeValues.IsValidIndex(VariationOriginalIndex)
				&& !InstancedOutputPartData.SplitAttributeName.IsEmpty())
			{
				FString SplitValue = InstancedOutputPartData.SplitAttributeValues[VariationOriginalIndex];

				// Cache the split attribute both as attribute and token
				NewOutputObject.CachedAttributes.Add(InstancedOutputPartData.SplitAttributeName, SplitValue);
				NewOutputObject.CachedTokens.Add(InstancedOutputPartData.SplitAttributeName, SplitValue);

				// If we have a split name that is non-empty, override attributes that can differ by split based
				// on the split name
				if (!SplitValue.IsEmpty())
				{
					const FHoudiniInstancedOutputPerSplitAttributes* PerSplitAttributes = InstancedOutputPartData.PerSplitAttributes.Find(SplitValue);
					if (PerSplitAttributes)
					{
						if (!PerSplitAttributes->LevelPath.IsEmpty())
							NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_LEVEL_PATH, PerSplitAttributes->LevelPath);
						if (!PerSplitAttributes->BakeActorName.IsEmpty())
							NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_ACTOR, PerSplitAttributes->BakeActorName);
						if (!PerSplitAttributes->BakeActorClassName.IsEmpty())
							NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_ACTOR_CLASS, PerSplitAttributes->BakeActorClassName);
						if (!PerSplitAttributes->BakeOutlinerFolder.IsEmpty())
							NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_OUTLINER_FOLDER, PerSplitAttributes->BakeOutlinerFolder);
						if (!PerSplitAttributes->BakeFolder.IsEmpty())
							NewOutputObject.CachedAttributes.Add(HAPI_UNREAL_ATTRIB_BAKE_FOLDER, PerSplitAttributes->BakeFolder);
					}
				}
			}
		}
	}

	// Remove reused components from the old map to avoid their deletion
	for (const auto& CurNewPair : NewOutputObjects)
	{
		// Get the new Identifier / StaticMesh
		const FHoudiniOutputObjectIdentifier& OutputIdentifier = CurNewPair.Key;
		
		// See if we already had that pair in the old map
		FHoudiniOutputObject* FoundOldOutputObject = OldOutputObjects.Find(OutputIdentifier);
		if (!FoundOldOutputObject)
			continue;

		bool bKeep = false;
		for(UObject* NewComponent : CurNewPair.Value.OutputComponents)
		{
			for(UObject* FoundOldComponent : FoundOldOutputObject->OutputComponents)
			{
			    if (IsValid(FoundOldComponent))
			    {
				    bKeep = (FoundOldComponent == NewComponent);
			    }
			}
		}

		UObject* NewProxyComponent = CurNewPair.Value.ProxyComponent;
		if (NewProxyComponent)
		{
			UObject* FoundOldProxyComponent = FoundOldOutputObject->ProxyComponent;
			if (IsValid(FoundOldProxyComponent))
			{
				bKeep = (FoundOldProxyComponent == NewProxyComponent);
			}
		}

		if (bKeep)
		{
			// Remove the reused component from the old map to avoid its destruction
			OldOutputObjects.Remove(OutputIdentifier);
		}
	}

	// The Old map now only contains unused/stale components, delete them
	for (auto& OldPair : OldOutputObjects)
	{
		// Get the old Identifier / StaticMesh
		FHoudiniOutputObjectIdentifier& OutputIdentifier = OldPair.Key;
		for(UObject* OldComponent : OldPair.Value.OutputComponents)
		{
			bool bDestroy = true;
			if (IsValid(OldComponent) && OldComponent->IsA<UHierarchicalInstancedStaticMeshComponent>())
			{
				// When destroying a component, we have to be sure it's not an HISMC owned by an InstanceFoliageActor
				UHierarchicalInstancedStaticMeshComponent* HISMC = Cast<UHierarchicalInstancedStaticMeshComponent>(OldComponent);
				if (HISMC->GetOwner() && HISMC->GetOwner()->IsA<AInstancedFoliageActor>())
					bDestroy = false;
			}

			if(bDestroy)
				RemoveAndDestroyComponent(OldComponent, OldPair.Value.OutputObject);

		}
		OldPair.Value.OutputComponents.Empty();
		OldPair.Value.OutputObject = nullptr;

		UObject* OldProxyComponent = OldPair.Value.ProxyComponent;
		if (OldProxyComponent)
		{
			RemoveAndDestroyComponent(OldProxyComponent, OldPair.Value.ProxyObject);
			OldPair.Value.ProxyComponent = nullptr;
			OldPair.Value.ProxyObject = nullptr;
		}
	}
	OldOutputObjects.Empty();

	// We need to clean up the instanced outputs that are still marked as stale
	// See Bug #124444
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutput> NewInstancedOutputs;
	for (const auto& CurrentInstancedOutput : InstancedOutputs)
	{
		if (!CurrentInstancedOutput.Value.bStale)
			NewInstancedOutputs.Add(CurrentInstancedOutput);
	}
	InOutput->SetInstancedOutputs(NewInstancedOutputs);
	
	// Update the output's object map
	// Instancer do not create objects, clean the map
	InOutput->SetOutputObjects(NewOutputObjects);

	return true;
}


bool
FHoudiniInstanceTranslator::UpdateChangedInstancedOutput(
	FHoudiniInstancedOutput& InInstancedOutput,
	const FHoudiniOutputObjectIdentifier& InOutputIdentifier,
	UHoudiniOutput* InParentOutput,
	USceneComponent* InParentComponent,
	const FHoudiniPackageParams& InPackageParams)
{
	check(false); // This code doesn't work and isn't called. If you call it, you'll need to make it work.

	FHoudiniOutputObjectIdentifier OutputIdentifier;
	OutputIdentifier.ObjectId = InOutputIdentifier.ObjectId;
	OutputIdentifier.GeoId = InOutputIdentifier.GeoId;
	OutputIdentifier.PartId = InOutputIdentifier.PartId;
	OutputIdentifier.SplitIdentifier = InOutputIdentifier.SplitIdentifier;
	OutputIdentifier.PartName = InOutputIdentifier.PartName;

	// Get if force using HISM from attribute
	const bool bForceHISM = HasHISMAttribute(InOutputIdentifier.GeoId, InOutputIdentifier.PartId);

	// Should we create an instancer even for single instances?
	const bool bForceInstancer = HasForceInstancerAttribute(InOutputIdentifier.GeoId, InOutputIdentifier.PartId);

	TArray<UObject*> OriginalInstancedObjects;
	OriginalInstancedObjects.Add(InInstancedOutput.OriginalObject.LoadSynchronous());

	TArray<TArray<FTransform>> OriginalInstancedTransforms;
	OriginalInstancedTransforms.Add(InInstancedOutput.OriginalTransforms);

	TArray<TArray<int32>> OriginalInstanceIndices;
	OriginalInstanceIndices.Add(InInstancedOutput.OriginalInstanceIndices);

	// Update our variations using the changed instancedoutputs objects
	TArray<TSoftObjectPtr<UObject>> InstancedObjects;
	TArray<TArray<FTransform>> InstancedTransforms;
	TArray<int32> VariationOriginalObjectIndices;
	TArray<int32> VariationIndices;
	UpdateInstanceVariationObjects(
		OutputIdentifier,
		OriginalInstancedObjects,
		OriginalInstancedTransforms,
		OriginalInstanceIndices,
		InParentOutput->GetInstancedOutputs(),
		InstancedObjects,
		InstancedTransforms,
		VariationOriginalObjectIndices,
		VariationIndices);

	// Find the HGPO for this instanced output
	bool FoundHGPO = false;
	FHoudiniGeoPartObject HGPO;
	for (const auto& curHGPO : InParentOutput->GetHoudiniGeoPartObjects())
	{
		if (OutputIdentifier.Matches(curHGPO))
		{
			HGPO = curHGPO;
			FoundHGPO = true;
			break;
		}
	}

	if (!FoundHGPO)
	{
		// TODO check failure
		ensure(FoundHGPO);
	}

	// Extract the generic attributes for that HGPO
	TArray<FHoudiniGenericAttribute> AllPropertyAttributes;
	GetGenericPropertiesAttributes(OutputIdentifier.GeoId, OutputIdentifier.PartId, AllPropertyAttributes);

	// Check if this is a No-Instancers ( unreal_split_instances )
	bool bSplitMeshInstancer = IsSplitInstancer(OutputIdentifier.GeoId, OutputIdentifier.PartId);

	bool bIsFoliageInstancer = IsFoliageInstancer(OutputIdentifier.GeoId, OutputIdentifier.PartId);

	// Preload objects so we can benefit from async compilation as much as possible
	for (int32 InstanceObjectIdx = 0; InstanceObjectIdx < InstancedObjects.Num(); InstanceObjectIdx++)
	{
		InstancedObjects[InstanceObjectIdx].LoadSynchronous();
	}

	// Keep track of the new instancer component in order to be able to clean up the unused/stale ones after.
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniOutputObject>& OutputObjects = InParentOutput->GetOutputObjects();
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniOutputObject> ToDeleteOutputObjects = InParentOutput->GetOutputObjects();

	// Create the instancer components now
	for (int32 InstanceObjectIdx = 0; InstanceObjectIdx < InstancedObjects.Num(); InstanceObjectIdx++)
	{
		UObject* InstancedObject = InstancedObjects[InstanceObjectIdx].LoadSynchronous();
		if (!IsValid(InstancedObject))
			continue;

		if (!InstancedTransforms.IsValidIndex(InstanceObjectIdx))
			continue;

		const TArray<FTransform>& InstancedObjectTransforms = InstancedTransforms[InstanceObjectIdx];
		if (InstancedObjectTransforms.Num() <= 0)
			continue;

		// Get the original Index of that variations
		int32 VariationOriginalIndex = VariationOriginalObjectIndices[InstanceObjectIdx];

		// Update the split identifier for this object
		// We use both the original object index and the variation index: ORIG_VAR
		// the original object index is used for the instanced outputs split identifier
		OutputIdentifier.SplitIdentifier =
			FString::FromInt(VariationOriginalIndex)
			+ TEXT("_")
			+ FString::FromInt(VariationIndices[InstanceObjectIdx]);

		// See if we can find an preexisting component for this obj	to try to reuse it
		FHoudiniOutputObject* FoundOutputObject = OutputObjects.Find(OutputIdentifier);
		TArray<USceneComponent*> OldInstancerComponents;
		const bool bIsProxyMesh = InstancedObject->IsA<UHoudiniStaticMesh>();
		if (FoundOutputObject)
		{
			if (bIsProxyMesh)
			{
				OldInstancerComponents.Add(Cast<USceneComponent>(FoundOutputObject->ProxyComponent));
			}
			else
			{
				for (auto Component : FoundOutputObject->OutputComponents)
				{
					OldInstancerComponents.Add(Cast<USceneComponent>(Component));
				}
			}
		}

		// Get the material for this variation
		TArray<UMaterialInterface*> VariationMaterials;
		if (!GetAllInstancerMaterials(OutputIdentifier.GeoId, OutputIdentifier.PartId, VariationOriginalIndex, HGPO, InPackageParams, VariationMaterials))
			VariationMaterials.Empty();

		TArray<USceneComponent*> NewInstancerComponents;
		TArray<AActor*> OldInstancerActors;
		TArray<AActor*> NewInstancerActors;
		UFoliageType * FoliageTypeUsed = nullptr;
		UWorld * World;

		int32 FoliageCount = 0;
		if (!CreateOrUpdateInstancer(
			InstancedObject,
			InstancedObjectTransforms,
			AllPropertyAttributes, 
			HGPO,
			InPackageParams,
			InParentComponent,
			OldInstancerComponents,
			NewInstancerComponents,
			OldInstancerActors,
			NewInstancerActors,
			bSplitMeshInstancer,
			bIsFoliageInstancer,
			VariationMaterials,
			OriginalInstanceIndices[VariationOriginalIndex],
			FoliageCount,
			FoliageTypeUsed,
			World,
			bForceHISM,
			bForceInstancer))
		{
			// TODO??
			continue;
		}

		if (NewInstancerComponents.IsEmpty())
			continue;

		// Remove old components not used.
		TSet<USceneComponent *> ComponentsToRemove;
		for(auto NewComponent : NewInstancerComponents)
		{
		    if (ComponentsToRemove.Contains(NewComponent))
			    ComponentsToRemove.Remove(NewComponent);
		}
		for(auto Component : ComponentsToRemove)
		{
		    RemoveAndDestroyComponent(Component, nullptr);
		}

		if (!FoundOutputObject)
			FoundOutputObject = &OutputObjects.Add(OutputIdentifier);

		FoundOutputObject->OutputComponents.Empty();
		for(auto NewInstancerComponent : NewInstancerComponents)
			FoundOutputObject->OutputComponents.Add(NewInstancerComponent);

		FoundOutputObject->UserFoliageType = Cast<UFoliageType>(InstancedObject);
		FoundOutputObject->FoliageType = FoliageTypeUsed;

		// Remove this output object from the todelete map
		ToDeleteOutputObjects.Remove(OutputIdentifier);
	}

	// Clean up the output objects that are not "reused" by the instanced outs
	// The ToDelete map now only contains unused/stale components, delete them
	for (auto& ToDeletePair : ToDeleteOutputObjects)
	{
		// Get the old Identifier / StaticMesh
		FHoudiniOutputObjectIdentifier& ToDeleteIdentifier = ToDeletePair.Key;
		for(int Index = 0; Index < ToDeletePair.Value.OutputComponents.Num(); Index++)
		{
			UObject * OldComponent = ToDeletePair.Value.OutputComponents[Index];
		    if (OldComponent)
		    {
			    RemoveAndDestroyComponent(OldComponent, ToDeletePair.Value.OutputObject);
			    ToDeletePair.Value.OutputComponents[Index] = nullptr;
		    }
		}

		UObject* OldProxyComponent = ToDeletePair.Value.ProxyComponent;
		if (OldProxyComponent)
		{
			RemoveAndDestroyComponent(OldProxyComponent, ToDeletePair.Value.ProxyObject);
			ToDeletePair.Value.ProxyComponent = nullptr;
		}
		
		// Make sure the stale output object is not in the output map anymore
		OutputObjects.Remove(ToDeleteIdentifier);
	}
	ToDeleteOutputObjects.Empty();

	return true;
}


bool
FHoudiniInstanceTranslator::GetInstancerObjectsAndTransforms(
	const FHoudiniGeoPartObject& InHGPO,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	TArray<TObjectPtr<UObject>>& OutInstancedObjects,
	TArray<TArray<FTransform>>& OutInstancedTransforms,
	TArray<TArray<int32>>& OutInstancedIndices,
	FString& OutSplitAttributeName,
	TArray<FString>& OutSplitAttributeValues,
	TMap<FString, FHoudiniInstancedOutputPerSplitAttributes>& OutPerSplitAttributes,
	TSet<UObject*>& OutInvisibleObjects)
{
	TArray<UObject*> InstancedObjects;
	TArray<TArray<FTransform>> InstancedTransforms;
	TArray<TArray<int32>> InstancedIndices;

	TArray<FHoudiniGeoPartObject> InstancedHGPOs;
	TArray<TArray<FTransform>> InstancedHGPOTransforms;
	TArray<TArray<int32>> InstancedHGPOIndices;

	bool bSuccess = false;
	switch (InHGPO.InstancerType)
	{
		case EHoudiniInstancerType::GeometryCollection:
		case EHoudiniInstancerType::PackedPrimitive:
		{
			// Packed primitives instances
			bSuccess = GetPackedPrimitiveInstancerHGPOsAndTransforms(
				InHGPO,
				InstancedHGPOs,
				InstancedHGPOTransforms,
				InstancedHGPOIndices,
				OutSplitAttributeName,
				OutSplitAttributeValues,
				OutPerSplitAttributes);
		}
		break;

		case EHoudiniInstancerType::AttributeInstancer:
		{
			// "Modern" attribute instancer - "unreal_instance"
			bSuccess = GetAttributeInstancerObjectsAndTransforms(
				InHGPO,
				InstancedObjects,
				InstancedTransforms,
				InstancedIndices,
				OutSplitAttributeName,
				OutSplitAttributeValues,
				OutPerSplitAttributes);
		}
		break;

		case EHoudiniInstancerType::OldSchoolAttributeInstancer:
		{
			// Old school attribute override instancer - instance attribute w/ a HoudiniPath
			bSuccess = GetOldSchoolAttributeInstancerHGPOsAndTransforms(InHGPO, InAllOutputs, InstancedHGPOs, InstancedHGPOTransforms, InstancedHGPOIndices);
		}
		break;

		case EHoudiniInstancerType::ObjectInstancer:
		{
			// Old School object instancer
			bSuccess = GetObjectInstancerHGPOsAndTransforms(InHGPO, InAllOutputs, InstancedHGPOs, InstancedHGPOTransforms, InstancedHGPOIndices);
		}	
		break;
	}

	if (!bSuccess)
		return false;

	// Fetch the UOBject that correspond to the instanced parts
	// Attribute instancers don't need to do this since they refer UObjects directly
	if (InstancedHGPOs.Num() > 0)
	{
		for (int32 HGPOIdx = 0; HGPOIdx < InstancedHGPOs.Num(); HGPOIdx++)
		{
			const FHoudiniGeoPartObject& CurrentHGPO = InstancedHGPOs[HGPOIdx];

			// Get the UObject that was generated for that HGPO
			TArray<UObject*> ObjectsToInstance;
			for (const auto& Output : InAllOutputs)
			{
				if (!Output || Output->Type != EHoudiniOutputType::Mesh)
					continue;

				if (Output->OutputObjects.Num() <= 0)
					continue;

				for (const auto& OutObjPair : Output->OutputObjects)
				{					
					if (!OutObjPair.Key.Matches(CurrentHGPO))
						continue;

					const FHoudiniOutputObject& CurrentOutputObject = OutObjPair.Value;

					if (CurrentOutputObject.bIsImplicit)
						continue;

					// In the case of a single-instance we can use the proxy (if it is current)
					// FHoudiniOutputTranslator::UpdateOutputs doesn't allow proxies if there is more than one instance in an output
					if (InstancedHGPOTransforms[HGPOIdx].Num() <= 1 && CurrentOutputObject.bProxyIsCurrent 
						&& IsValid(CurrentOutputObject.ProxyObject))
					{
						ObjectsToInstance.Add(CurrentOutputObject.ProxyObject);
					}
					else if (IsValid(CurrentOutputObject.OutputObject))
					{
						ObjectsToInstance.Add(CurrentOutputObject.OutputObject);

						EHoudiniSplitType SplitType = FHoudiniMeshTranslator::GetSplitTypeFromSplitName(OutObjPair.Key.SplitIdentifier);
						if (SplitType == EHoudiniSplitType::InvisibleComplexCollider)
						{
							OutInvisibleObjects.Add(CurrentOutputObject.OutputObject);
						}

					}
				}
			}

			// Add the UObject and the HGPO transforms to the output arrays
			for (const auto& MatchingOutputObj : ObjectsToInstance)
			{
				InstancedObjects.Add(MatchingOutputObj);
				InstancedTransforms.Add(InstancedHGPOTransforms[HGPOIdx]);
				InstancedIndices.Add(InstancedHGPOIndices[HGPOIdx]);
			}
		}
	}
	   
	//
	if (InstancedObjects.Num() <= 0 || InstancedTransforms.Num() != InstancedObjects.Num()  || InstancedIndices.Num() != InstancedObjects.Num())
	{
		// TODO
		// Error / warning
		return false;
	}

	OutInstancedObjects = InstancedObjects;
	OutInstancedTransforms = InstancedTransforms;
	OutInstancedIndices = InstancedIndices;

	return true;
}


void
FHoudiniInstanceTranslator::UpdateInstanceVariationObjects(
	const FHoudiniOutputObjectIdentifier& InOutputIdentifier,
	const TArray<UObject*>& InOriginalObjects,
	const TArray<TArray<FTransform>>& InOriginalTransforms,
	const TArray<TArray<int32>>& InOriginalInstancedIndices,
	TMap<FHoudiniOutputObjectIdentifier, FHoudiniInstancedOutput>& InstancedOutputs,
	TArray<TSoftObjectPtr<UObject>>& OutVariationsInstancedObjects,
	TArray<TArray<FTransform>>& OutVariationsInstancedTransforms,
	TArray<int32>& OutVariationOriginalObjectIdx,
	TArray<int32>& OutVariationIndices)
{
	FHoudiniOutputObjectIdentifier Identifier = InOutputIdentifier;
	for (int32 InstObjIdx = 0; InstObjIdx < InOriginalObjects.Num(); InstObjIdx++)
	{
		UObject* OriginalObj = InOriginalObjects[InstObjIdx];
		if (!IsValid(OriginalObj))
			continue;

		// Build this output object's split identifier
		Identifier.SplitIdentifier = FString::FromInt(InstObjIdx);

		// Do we have an instanced output object for this one?
		FHoudiniInstancedOutput * FoundInstancedOutput = nullptr;
		for (auto& Iter : InstancedOutputs)
		{
			FHoudiniOutputObjectIdentifier& FoundIdentifier = Iter.Key;
			if (!(FoundIdentifier == Identifier))
				continue;

			// We found an existing instanced output for this identifier
			FoundInstancedOutput = &(Iter.Value);

			if (FoundIdentifier.bLoaded)
			{
				// The output object identifier we found is marked as loaded,
				// so uses old node IDs, we must update them, or the next cook
				// will fail to locate the output back
				FoundIdentifier.ObjectId = Identifier.ObjectId;
				FoundIdentifier.GeoId = Identifier.GeoId;
				FoundIdentifier.PartId = Identifier.PartId;
			}
		}

		if (!FoundInstancedOutput)
		{
			// Create a new one
			FHoudiniInstancedOutput CurInstancedOutput;
			CurInstancedOutput.OriginalObject = OriginalObj;
			CurInstancedOutput.OriginalObjectIndex = InstObjIdx;
			CurInstancedOutput.OriginalTransforms = InOriginalTransforms[InstObjIdx];
			CurInstancedOutput.OriginalInstanceIndices = InOriginalInstancedIndices[InstObjIdx];

			CurInstancedOutput.VariationObjects.Add(OriginalObj);
			CurInstancedOutput.VariationTransformOffsets.Add(FTransform::Identity);
			CurInstancedOutput.TransformVariationIndices.SetNumZeroed(InOriginalTransforms[InstObjIdx].Num());
			CurInstancedOutput.MarkChanged(false);
			CurInstancedOutput.bStale = false;

			// No variations, simply assign the object/transforms
			OutVariationsInstancedObjects.Add(OriginalObj);
			OutVariationsInstancedTransforms.Add(InOriginalTransforms[InstObjIdx]);
			OutVariationOriginalObjectIdx.Add(InstObjIdx);
			OutVariationIndices.Add(0);

			InstancedOutputs.Add(Identifier, CurInstancedOutput);
		}
		else
		{
			// Process the potential variations
			FHoudiniInstancedOutput& CurInstancedOutput = *FoundInstancedOutput;
			UObject *ReplacedOriginalObject = nullptr;
			if (CurInstancedOutput.OriginalObject != OriginalObj)
			{
				ReplacedOriginalObject = CurInstancedOutput.OriginalObject.LoadSynchronous();
				CurInstancedOutput.OriginalObject = OriginalObj;
			}

			CurInstancedOutput.OriginalTransforms = InOriginalTransforms[InstObjIdx];
			CurInstancedOutput.OriginalInstanceIndices = InOriginalInstancedIndices[InstObjIdx];

			// Shouldnt be needed...
			CurInstancedOutput.OriginalObjectIndex = InstObjIdx;

			// Remove any null or deleted variation objects
			TArray<int32> ObjsToRemove;
			for (int32 VarIdx = CurInstancedOutput.VariationObjects.Num() - 1; VarIdx >= 0; --VarIdx)
			{
				UObject* CurrentVariationObject = CurInstancedOutput.VariationObjects[VarIdx].LoadSynchronous();
				if (!IsValid(CurrentVariationObject) || (ReplacedOriginalObject && ReplacedOriginalObject == CurrentVariationObject))
				{
					ObjsToRemove.Add(VarIdx);
				}
			}
			if (ObjsToRemove.Num() > 0)
			{
				for (const int32 &VarIdx : ObjsToRemove)
				{
					CurInstancedOutput.VariationObjects.RemoveAt(VarIdx);
					CurInstancedOutput.VariationTransformOffsets.RemoveAt(VarIdx);
				}
				// Force a recompute of variation assignments
				CurInstancedOutput.TransformVariationIndices.SetNum(0);
			}

			// If we don't have variations, simply use the original object
			if (CurInstancedOutput.VariationObjects.Num() == 0)
			{
				// No variations? add the original one
				CurInstancedOutput.VariationObjects.Add(OriginalObj);
				CurInstancedOutput.VariationTransformOffsets.Add(FTransform::Identity);
				CurInstancedOutput.TransformVariationIndices.SetNum(0);
			}
			else
			{
				HOUDINI_LOG_WARNING(TEXT("Instance Variations are deprecated and will be removed in a future version. See documentation for more details."));
			}

			// If the number of transforms has changed since the previous cook, 
			// we need to recompute the variation assignments
			if (CurInstancedOutput.TransformVariationIndices.Num() != CurInstancedOutput.OriginalTransforms.Num())
				UpdateVariationAssignements(CurInstancedOutput);

			// Assign variations and their transforms
			for (int32 VarIdx = 0; VarIdx < CurInstancedOutput.VariationObjects.Num(); VarIdx++)
			{
				UObject* CurrentVariationObject = CurInstancedOutput.VariationObjects[VarIdx].LoadSynchronous();
				if (!IsValid(CurrentVariationObject))
					continue;

				// Get the transforms assigned to that variation
				TArray<FTransform> ProcessedTransforms;
				ProcessInstanceTransforms(CurInstancedOutput, VarIdx, ProcessedTransforms);
				if (ProcessedTransforms.Num() > 0)
				{
					OutVariationsInstancedObjects.Add(CurrentVariationObject);
					OutVariationsInstancedTransforms.Add(ProcessedTransforms);
					OutVariationOriginalObjectIdx.Add(InstObjIdx);
					OutVariationIndices.Add(VarIdx);
				}
			}

			CurInstancedOutput.MarkChanged(false);
			CurInstancedOutput.bStale = false;
		}
	}
}


void
FHoudiniInstanceTranslator::UpdateVariationAssignements(FHoudiniInstancedOutput& InstancedOutput)
{
	int32 TransformCount = InstancedOutput.OriginalTransforms.Num();
	InstancedOutput.TransformVariationIndices.SetNumZeroed(TransformCount);

	int32 VariationCount = InstancedOutput.VariationObjects.Num();
	if (VariationCount <= 1)
		return;

	int nSeed = 1234;	
	for (int32 Idx = 0; Idx < TransformCount; Idx++)
	{
		InstancedOutput.TransformVariationIndices[Idx] = fastrand(nSeed) % VariationCount;
	}	
}

void
FHoudiniInstanceTranslator::ProcessInstanceTransforms(
	FHoudiniInstancedOutput& InstancedOutput, const int32& VariationIdx, TArray<FTransform>& OutProcessedTransforms)
{
	if (!InstancedOutput.VariationObjects.IsValidIndex(VariationIdx))
		return;

	if (!InstancedOutput.VariationTransformOffsets.IsValidIndex(VariationIdx))
		return;

	bool bHasVariations = InstancedOutput.VariationObjects.Num() > 1;
	bool bHasTransformOffset = InstancedOutput.VariationTransformOffsets.IsValidIndex(VariationIdx)
		? !InstancedOutput.VariationTransformOffsets[VariationIdx].Equals(FTransform::Identity)
		: false;

	if (!bHasVariations && !bHasTransformOffset)
	{
		// We dont have variations or transform offset, so we can reuse the original transforms as is
		OutProcessedTransforms = InstancedOutput.OriginalTransforms;
		return;
	}

	if (bHasVariations)
	{
		// We simply need to extract the transforms for this variation		
		for (int32 TransformIndex = 0; TransformIndex < InstancedOutput.TransformVariationIndices.Num(); TransformIndex++)
		{
			if (InstancedOutput.TransformVariationIndices[TransformIndex] != VariationIdx)
				continue;

			OutProcessedTransforms.Add(InstancedOutput.OriginalTransforms[TransformIndex]);
		}
	}
	else
	{
		// No variations, we can reuse the original transforms
		OutProcessedTransforms = InstancedOutput.OriginalTransforms;
	}

	if (bHasTransformOffset)
	{
		// Get the transform offset for this variation
		FVector PositionOffset = InstancedOutput.VariationTransformOffsets[VariationIdx].GetLocation();
		FQuat RotationOffset = InstancedOutput.VariationTransformOffsets[VariationIdx].GetRotation();
		FVector ScaleOffset = InstancedOutput.VariationTransformOffsets[VariationIdx].GetScale3D();

		FTransform CurrentTransform = FTransform::Identity;
		for (int32 TransformIndex = 0; TransformIndex < OutProcessedTransforms.Num(); TransformIndex++)
		{
			CurrentTransform = OutProcessedTransforms[TransformIndex];

			// Compute new rotation and scale.
			FVector Position = CurrentTransform.GetLocation() + PositionOffset;
			FQuat TransformRotation = CurrentTransform.GetRotation() * RotationOffset;
			FVector TransformScale3D = CurrentTransform.GetScale3D() * ScaleOffset;

			// Make sure inverse matrix exists - seems to be a bug in Unreal when submitting instances.
			// Happens in blueprint as well.
			// We want to make sure the scale is not too small, but keep negative values! (Bug 90876)
			if (FMath::Abs(TransformScale3D.X) < HAPI_UNREAL_SCALE_SMALL_VALUE)
				TransformScale3D.X = (TransformScale3D.X > 0) ? HAPI_UNREAL_SCALE_SMALL_VALUE : -HAPI_UNREAL_SCALE_SMALL_VALUE;

			if (FMath::Abs(TransformScale3D.Y) < HAPI_UNREAL_SCALE_SMALL_VALUE)
				TransformScale3D.Y = (TransformScale3D.Y > 0) ? HAPI_UNREAL_SCALE_SMALL_VALUE : -HAPI_UNREAL_SCALE_SMALL_VALUE;

			if (FMath::Abs(TransformScale3D.Z) < HAPI_UNREAL_SCALE_SMALL_VALUE)
				TransformScale3D.Z = (TransformScale3D.Z > 0) ? HAPI_UNREAL_SCALE_SMALL_VALUE : -HAPI_UNREAL_SCALE_SMALL_VALUE;

			CurrentTransform.SetLocation(Position);
			CurrentTransform.SetRotation(TransformRotation);
			CurrentTransform.SetScale3D(TransformScale3D);

			if (CurrentTransform.IsValid())
				OutProcessedTransforms[TransformIndex] = CurrentTransform;
		}
	}
}

bool
FHoudiniInstanceTranslator::GetPackedPrimitiveInstancerHGPOsAndTransforms(
	const FHoudiniGeoPartObject& InHGPO,
	TArray<FHoudiniGeoPartObject>& OutInstancedHGPO,
	TArray<TArray<FTransform>>& OutInstancedTransforms,
	TArray<TArray<int32>>& OutInstancedIndices,
	FString& OutSplitAttributeName,
	TArray<FString>& OutSplitAttributeValue,
	TMap<FString, FHoudiniInstancedOutputPerSplitAttributes>& OutPerSplitAttributes)
{
	if (InHGPO.InstancerType != EHoudiniInstancerType::PackedPrimitive && InHGPO.InstancerType != EHoudiniInstancerType::GeometryCollection)
		return false;

	// Get transforms for each instance
	TArray<HAPI_Transform> InstancerPartTransforms;
	InstancerPartTransforms.SetNumZeroed(InHGPO.PartInfo.InstanceCount);
	HOUDINI_CHECK_ERROR_RETURN(FHoudiniApi::GetInstancerPartTransforms(
		FHoudiniEngine::Get().GetSession(), InHGPO.GeoId, InHGPO.PartInfo.PartId,
		HAPI_RSTORDER_DEFAULT, InstancerPartTransforms.GetData(), 0, InHGPO.PartInfo.InstanceCount), false);

	// Convert the transform to Unreal's coordinate system
	TArray<FTransform> InstancerUnrealTransforms;
	InstancerUnrealTransforms.SetNum(InstancerPartTransforms.Num());
	for (int32 InstanceIdx = 0; InstanceIdx < InstancerPartTransforms.Num(); InstanceIdx++)
	{
		const auto& InstanceTransform = InstancerPartTransforms[InstanceIdx];
		FHoudiniEngineUtils::TranslateHapiTransform(InstanceTransform, InstancerUnrealTransforms[InstanceIdx]);
	}

	// Get the part ids for parts being instanced
	TArray<HAPI_PartId> InstancedPartIds;
	InstancedPartIds.SetNumZeroed(InHGPO.PartInfo.InstancedPartCount);
	HOUDINI_CHECK_ERROR_RETURN(FHoudiniApi::GetInstancedPartIds(
		FHoudiniEngine::Get().GetSession(), InHGPO.GeoId, InHGPO.PartInfo.PartId,
		InstancedPartIds.GetData(), 0, InHGPO.PartInfo.InstancedPartCount), false);

	// See if the user has specified an attribute for splitting the instances
	// and get the values
	FString SplitAttribName = FString();
	TArray<FString> AllSplitAttributeValues;
	bool bHasSplitAttribute = GetInstancerSplitAttributesAndValues(
		InHGPO.GeoId, InHGPO.PartId, HAPI_ATTROWNER_PRIM, SplitAttribName, AllSplitAttributeValues);

	// Get the level path attribute on the instancer
	TArray<FString> AllLevelPaths;
	const bool bHasLevelPaths = FHoudiniEngineUtils::GetLevelPathAttribute(
		InHGPO.GeoId, InHGPO.PartId, AllLevelPaths, HAPI_ATTROWNER_PRIM);

	// Get the bake actor attribute
	TArray<FString> AllBakeActorNames;
	const bool bHasBakeActorNames = FHoudiniEngineUtils::GetBakeActorAttribute(
		InHGPO.GeoId, InHGPO.PartId,  AllBakeActorNames, HAPI_ATTROWNER_PRIM);

	// Get the bake actor class attribute
	TArray<FString> AllBakeActorClassNames;
	const bool bHasBakeActorClassNames = FHoudiniEngineUtils::GetBakeActorClassAttribute(
		InHGPO.GeoId, InHGPO.PartId,  AllBakeActorClassNames, HAPI_ATTROWNER_PRIM);

	// Get the unreal_bake_folder attribute
	TArray<FString> AllBakeFolders;
	const bool bHasBakeFolders = FHoudiniEngineUtils::GetBakeFolderAttribute(
		InHGPO.GeoId, HAPI_ATTROWNER_PRIM, AllBakeFolders, InHGPO.PartId);

	// Get the bake outliner folder attribute
	TArray<FString> AllBakeOutlinerFolders;
	const bool bHasBakeOutlinerFolders = FHoudiniEngineUtils::GetBakeOutlinerFolderAttribute(
		InHGPO.GeoId, InHGPO.PartId,AllBakeOutlinerFolders, HAPI_ATTROWNER_PRIM);

	const bool bHasAnyPerSplitAttributes = bHasLevelPaths || bHasBakeActorNames || bHasBakeOutlinerFolders || bHasBakeFolders;

	for (const auto& InstancedPartId : InstancedPartIds)
	{
		// Create a GeoPartObject corresponding to the instanced part
		FHoudiniGeoPartObject InstancedHGPO;
		InstancedHGPO.AssetId = InHGPO.AssetId;
		InstancedHGPO.AssetName = InHGPO.AssetName;
		InstancedHGPO.ObjectId = InHGPO.ObjectId;
		InstancedHGPO.ObjectName = InHGPO.ObjectName;
		InstancedHGPO.GeoId = InHGPO.GeoId;
		InstancedHGPO.PartId = InstancedPartId;
		InstancedHGPO.PartName = InHGPO.PartName;
		InstancedHGPO.TransformMatrix = InHGPO.TransformMatrix;		

		// TODO: Copy more cached data?

		OutInstancedHGPO.Add(InstancedHGPO);
		OutInstancedTransforms.Add(InstancerUnrealTransforms);

		TArray<int32> Indices;
		Indices.SetNum(InstancerUnrealTransforms.Num());
		for (int32 Index = 0; Index < Indices.Num(); ++Index)
		{
			Indices[Index] = Index;
		}

		OutInstancedIndices.Add(Indices);
	}

	// If we don't need to split the instances, we're done
	if (!bHasSplitAttribute)
		return true;

	// TODO: Optimize this!
	// Split the instances using the split attribute's values
	
	// Move the output arrays to temp arrays
	TArray<FHoudiniGeoPartObject> UnsplitInstancedHGPOs = OutInstancedHGPO;
	TArray<TArray<FTransform>> UnsplitInstancedTransforms = OutInstancedTransforms;
	TArray<TArray<int32>> UnsplitInstancedIndices = OutInstancedIndices;

	// Empty the output arrays
	OutInstancedHGPO.Empty();
	OutInstancedTransforms.Empty();
	OutInstancedIndices.Empty();
	OutSplitAttributeValue.Empty();
	for (int32 ObjIdx = 0; ObjIdx < UnsplitInstancedHGPOs.Num(); ObjIdx++)
	{
		// Map of split values to transform arrays
		TMap<FString, TArray<FTransform>> SplitTransformMap;
		TMap<FString, TArray<int32>> SplitIndicesMap;

		TArray<FTransform>& CurrentTransforms = UnsplitInstancedTransforms[ObjIdx];
		TArray<int32>& CurrentIndices = UnsplitInstancedIndices[ObjIdx];

		int32 NumInstances = CurrentTransforms.Num();
		if (AllSplitAttributeValues.Num() != NumInstances || CurrentIndices.Num() != NumInstances)
			continue;

		// Split the transforms using the split values
		for (int32 InstIdx = 0; InstIdx < NumInstances; InstIdx++)
		{
			const FString& SplitAttrValue = AllSplitAttributeValues[InstIdx];
			SplitTransformMap.FindOrAdd(SplitAttrValue).Add(CurrentTransforms[InstIdx]);
			SplitIndicesMap.FindOrAdd(SplitAttrValue).Add(CurrentIndices[InstIdx]);
			
			// Record attributes for any split value we have not yet seen
			if (bHasAnyPerSplitAttributes)
			{
				FHoudiniInstancedOutputPerSplitAttributes& PerSplitAttributes = OutPerSplitAttributes.FindOrAdd(SplitAttrValue);
				if (bHasLevelPaths && PerSplitAttributes.LevelPath.IsEmpty() && AllLevelPaths.IsValidIndex(InstIdx))
				{
					PerSplitAttributes.LevelPath = AllLevelPaths[InstIdx];
				}
				if (bHasBakeActorNames && PerSplitAttributes.BakeActorName.IsEmpty() && AllBakeActorNames.IsValidIndex(InstIdx))
				{
					PerSplitAttributes.BakeActorName = AllBakeActorNames[InstIdx];
				}
				if (bHasBakeFolders && PerSplitAttributes.BakeFolder.IsEmpty() && AllBakeFolders.IsValidIndex(InstIdx))
				{
					PerSplitAttributes.BakeFolder = AllBakeFolders[InstIdx];
				}
				if (bHasBakeOutlinerFolders && PerSplitAttributes.BakeOutlinerFolder.IsEmpty() && AllBakeOutlinerFolders.IsValidIndex(InstIdx))
				{
					PerSplitAttributes.BakeOutlinerFolder = AllBakeOutlinerFolders[InstIdx];
				}
			}
		}

		// Add the objects, transform, split values to the final arrays
		for (auto& Iterator : SplitTransformMap)
		{
			OutSplitAttributeValue.Add(Iterator.Key);
			OutInstancedHGPO.Add(UnsplitInstancedHGPOs[ObjIdx]);
			OutInstancedTransforms.Add(Iterator.Value);
			OutInstancedIndices.Add(SplitIndicesMap[Iterator.Key]);
		}
	}

	OutSplitAttributeName = SplitAttribName;

	return true;
}


bool
FHoudiniInstanceTranslator::GetAttributeInstancerObjectsAndTransforms(
	const FHoudiniGeoPartObject& InHGPO,
	TArray<UObject*>& OutInstancedObjects,
	TArray<TArray<FTransform>>& OutInstancedTransforms,
	TArray<TArray<int32>>& OutInstancedIndices,
	FString& OutSplitAttributeName,
	TArray<FString>& OutSplitAttributeValue,
	TMap<FString, FHoudiniInstancedOutputPerSplitAttributes>& OutPerSplitAttributes)
{
	if (InHGPO.InstancerType != EHoudiniInstancerType::AttributeInstancer)
		return false;

	// Look for the unreal instance attribute
	HAPI_AttributeInfo AttribInfo;
	FHoudiniApi::AttributeInfo_Init(&AttribInfo);

	// instance attribute on points
	bool is_override_attr = false;
	HAPI_Result Result = FHoudiniApi::GetAttributeInfo(
		FHoudiniEngine::Get().GetSession(),
		InHGPO.GeoId, InHGPO.PartId,
		HAPI_UNREAL_ATTRIB_INSTANCE, HAPI_ATTROWNER_POINT, &AttribInfo);
	
	// unreal_instance attribute on points
	if (Result != HAPI_RESULT_SUCCESS || !AttribInfo.exists)
	{
		is_override_attr = true;
		Result = FHoudiniApi::GetAttributeInfo(
			FHoudiniEngine::Get().GetSession(),
			InHGPO.GeoId, InHGPO.PartId,
			HAPI_UNREAL_ATTRIB_INSTANCE_OVERRIDE, HAPI_ATTROWNER_POINT, &AttribInfo);
	}

	// unreal_instance attribute on detail
	if (Result != HAPI_RESULT_SUCCESS || !AttribInfo.exists)
	{
		is_override_attr = true;
		Result = FHoudiniApi::GetAttributeInfo(
			FHoudiniEngine::Get().GetSession(),
			InHGPO.GeoId, InHGPO.PartId,
			HAPI_UNREAL_ATTRIB_INSTANCE_OVERRIDE, HAPI_ATTROWNER_DETAIL, &AttribInfo);
	}

	// Attribute does not exist.
	if (Result != HAPI_RESULT_SUCCESS || !AttribInfo.exists)
		return false;

	// Get the instance transforms
	TArray<FTransform> InstancerUnrealTransforms;
	if (!HapiGetInstanceTransforms(InHGPO, InstancerUnrealTransforms))
	{
		// failed to get instance transform
		return false;
	}

	// Get the settings indicating if we want to use a default object when the referenced mesh is invalid
	bool bDefaultObjectEnabled = true;
	const UHoudiniRuntimeSettings * HoudiniRuntimeSettings = GetDefault< UHoudiniRuntimeSettings >();
	if (HoudiniRuntimeSettings)
	{
		bDefaultObjectEnabled = HoudiniRuntimeSettings->bShowDefaultMesh;
	}
	
	// See if the user has specified an attribute for splitting the instances, and get the values
	FString SplitAttribName = FString();
	TArray<FString> AllSplitAttributeValues;
	bool bHasSplitAttribute = GetInstancerSplitAttributesAndValues(
		InHGPO.GeoId, InHGPO.PartId, HAPI_ATTROWNER_POINT, SplitAttribName, AllSplitAttributeValues);

	// Get the level path attribute on the instancer
	TArray<FString> AllLevelPaths;
	const bool bHasLevelPaths = FHoudiniEngineUtils::GetLevelPathAttribute(
		InHGPO.GeoId, InHGPO.PartId, AllLevelPaths, HAPI_ATTROWNER_POINT);

	// Get the bake actor attribute
	TArray<FString> AllBakeActorNames;
	const bool bHasBakeActorNames = FHoudiniEngineUtils::GetBakeActorAttribute(
		InHGPO.GeoId, InHGPO.PartId,  AllBakeActorNames, HAPI_ATTROWNER_POINT);

	// Get the bake actor class attribute
	TArray<FString> AllBakeActorClassNames;
	const bool bHasBakeActorClassNames = FHoudiniEngineUtils::GetBakeActorClassAttribute(
		InHGPO.GeoId, InHGPO.PartId,  AllBakeActorClassNames, HAPI_ATTROWNER_POINT);

	// Get the unreal_bake_folder attribute
	TArray<FString> AllBakeFolders;
	const bool bHasBakeFolders = FHoudiniEngineUtils::GetBakeFolderAttribute(
		InHGPO.GeoId, HAPI_ATTROWNER_POINT, AllBakeFolders, InHGPO.PartId);

	// Get the bake outliner folder attribute
	TArray<FString> AllBakeOutlinerFolders;
	const bool bHasBakeOutlinerFolders = FHoudiniEngineUtils::GetBakeOutlinerFolderAttribute(
		InHGPO.GeoId, InHGPO.PartId,AllBakeOutlinerFolders, HAPI_ATTROWNER_POINT);

	const bool bHasAnyPerSplitAttributes = bHasLevelPaths || bHasBakeActorNames || bHasBakeOutlinerFolders || bHasBakeFolders;

	// Array used to store the split values per objects
	// Will only be used if we have a split attribute
	TArray<TArray<FString>> SplitAttributeValuesPerObject;

	if (AttribInfo.owner == HAPI_ATTROWNER_DETAIL)
	{
		// If the attribute is on the detail, then its value is applied to all points
		TArray<FString> DetailInstanceValues;
		if (!FHoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(
			InHGPO.GeoId,
			InHGPO.PartId,
			is_override_attr ? HAPI_UNREAL_ATTRIB_INSTANCE_OVERRIDE : HAPI_UNREAL_ATTRIB_INSTANCE,
			AttribInfo,
			DetailInstanceValues))
		{
			// This should not happen - attribute exists, but there was an error retrieving it.
			return false;
		}

		if (DetailInstanceValues.Num() <= 0)
		{
			// No values specified.
			return false;
		}

		// Attempt to load specified asset.
		const FString & AssetName = DetailInstanceValues[0];
		UObject * AttributeObject = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetName, nullptr, LOAD_None, nullptr);

		while (UObjectRedirector* Redirector = Cast<UObjectRedirector>(AttributeObject))
			AttributeObject = Redirector->DestinationObject;

		if (!AttributeObject)
		{
			// See if the ref is a class that we can instantiate
			UClass* FoundClass = FHoudiniEngineRuntimeUtils::GetClassByName(AssetName);

			if (FoundClass != nullptr)
			{
				// TODO: ensure we'll be able to create an actor from this class! 
				AttributeObject = FoundClass;
			}
		}

		if (!AttributeObject && bDefaultObjectEnabled)
		{
			HOUDINI_LOG_WARNING(TEXT("Failed to load instanced object '%s', using default instance mesh (hidden in game)."), *(AssetName));

			// Couldn't load the referenced object, use the default reference mesh
			UStaticMesh * DefaultReferenceSM = FHoudiniEngine::Get().GetHoudiniDefaultReferenceMesh().Get();
			if (!IsValid(DefaultReferenceSM))
			{
				HOUDINI_LOG_WARNING(TEXT("Failed to load the default instance mesh."));
				return false;
			}
			AttributeObject = DefaultReferenceSM;
		}

		// Attach the objectPtr/transforms/bHiddenInGame if the attributeObject is created successfully
		// (with either the actual referenced object or the default placeholder object)
		if (AttributeObject)
		{
			OutInstancedObjects.Add(AttributeObject);
			OutInstancedTransforms.Add(InstancerUnrealTransforms);

			TArray<int32> Indices;
			Indices.SetNum(InstancerUnrealTransforms.Num());
			for (int32 Index = 0; Index < Indices.Num(); ++Index)
			{
				Indices[Index] = Index;
			}

			OutInstancedIndices.Add(Indices);

			if(bHasSplitAttribute)
				SplitAttributeValuesPerObject.Add(AllSplitAttributeValues);
		}
	}
	else
	{
		// Attribute is on points, so we may have different values for each of them
		TArray<FString> PointInstanceValues;
		if (!FHoudiniEngineUtils::HapiGetAttributeDataAsStringFromInfo(
			InHGPO.GeoId,
			InHGPO.PartId,
			is_override_attr ? HAPI_UNREAL_ATTRIB_INSTANCE_OVERRIDE : HAPI_UNREAL_ATTRIB_INSTANCE,
			AttribInfo,
			PointInstanceValues))
		{
			// This should not happen - attribute exists, but there was an error retrieving it.
			return false;
		}

		// The attribute is on points, so the number of points must match number of transforms.
		if (!ensure(PointInstanceValues.Num() == InstancerUnrealTransforms.Num()))
		{
			// This should not happen, we have mismatch between number of instance values and transforms.
			return false;
		}

		// If instance attribute exists on points, we need to get all the unique values.
		// This will give us all the unique object we want to instance
		TMap<FString, UObject *> ObjectsToInstance;
		for (const auto& Iter : PointInstanceValues)
		{
			if (!ObjectsToInstance.Contains(Iter))
			{
				// To avoid trying to load an object that fails multiple times,
				// still add it to the array if null so we can still skip further attempts
				UObject* AttributeObject = StaticFindObjectSafe(UObject::StaticClass(), nullptr, *Iter);
				if (!IsValid(AttributeObject))
					AttributeObject = StaticLoadObject(
						UObject::StaticClass(), nullptr, *Iter, nullptr, LOAD_None, nullptr);

				while (UObjectRedirector* Redirector = Cast<UObjectRedirector>(AttributeObject))
					AttributeObject = Redirector->DestinationObject;

				if (!AttributeObject)
				{
					UClass* FoundClass = FHoudiniEngineRuntimeUtils::GetClassByName(Iter);
					if (FoundClass != nullptr)
					{
						// TODO: ensure we'll be able to create an actor from this class!
						AttributeObject = FoundClass;
					}
				}

				ObjectsToInstance.Add(Iter, AttributeObject);
			}
		}

		// Iterates through all the unique objects and get their corresponding transforms
		bool Success = false;
		for (auto Iter : ObjectsToInstance)
		{
			bool bHiddenInGame = false;
			// Check that we managed to load this object
			UObject * AttributeObject = Iter.Value;

			if (!AttributeObject && bDefaultObjectEnabled) 
			{
				HOUDINI_LOG_WARNING(
					TEXT("Failed to load instanced object '%s', use default mesh (hidden in game)."), *(Iter.Key));

				// If failed to load this object, add default reference mesh
				UStaticMesh * DefaultReferenceSM = FHoudiniEngine::Get().GetHoudiniDefaultReferenceMesh().Get();
				if (IsValid(DefaultReferenceSM))
				{
					AttributeObject = DefaultReferenceSM;
					bHiddenInGame = true;
				}
				else// Failed to load default reference mesh object
				{
					HOUDINI_LOG_WARNING(TEXT("Failed to load default mesh."));
					continue;
				}
			}

			if (!AttributeObject)
				continue;

			if (!bHasSplitAttribute)
			{
				// No Split attribute:
				// Extract the transform values that correspond to this object, and add them to the output arrays
				const FString & InstancePath = Iter.Key;
				TArray<FTransform> ObjectTransforms;
				TArray<int32> ObjectIndices;

				for (int32 Idx = 0; Idx < PointInstanceValues.Num(); ++Idx)
				{
					if (InstancePath.Equals(PointInstanceValues[Idx]))
					{
						ObjectTransforms.Add(InstancerUnrealTransforms[Idx]);
						ObjectIndices.Add(Idx);
					}
				}

				OutInstancedObjects.Add(AttributeObject);
				OutInstancedTransforms.Add(ObjectTransforms);
				OutInstancedIndices.Add(ObjectIndices);
				Success = true;
			}
			else
			{
				// We have a split attribute:
				// Extract the transform values and split attribute values for this object,
				// add them to the output arrays, and we will process the splits after
				const FString & InstancePath = Iter.Key;
				TArray<FTransform> ObjectTransforms;
				TArray<int32> ObjectIndices;
				TArray<FString> ObjectSplitValues;
				for (int32 Idx = 0; Idx < PointInstanceValues.Num(); ++Idx)
				{
					if (InstancePath.Equals(PointInstanceValues[Idx]))
					{
						ObjectTransforms.Add(InstancerUnrealTransforms[Idx]);
						ObjectIndices.Add(Idx);
						ObjectSplitValues.Add(AllSplitAttributeValues[Idx]);
					}
				}

				OutInstancedObjects.Add(AttributeObject);
				OutInstancedTransforms.Add(ObjectTransforms);
				OutInstancedIndices.Add(ObjectIndices);
				SplitAttributeValuesPerObject.Add(ObjectSplitValues);
				Success = true;
			}
		}

		if (!Success) 
			return false;
	}

	// If we don't need to split the instances, we're done
	if (!bHasSplitAttribute)
		return true;

	// Split the instances one more time, this time using the split values
	
	// Move the output arrays to temp arrays
	TArray<UObject*> UnsplitInstancedObjects = OutInstancedObjects;
	TArray<TArray<FTransform>> UnsplitInstancedTransforms = OutInstancedTransforms;
	TArray<TArray<int32>> UnsplitInstancedIndices = OutInstancedIndices;

	// Empty the output arrays
	OutInstancedObjects.Empty();
	OutInstancedTransforms.Empty();
	OutInstancedIndices.Empty();

	// TODO: Output the split values as well!
	OutSplitAttributeValue.Empty();
	for (int32 ObjIdx = 0; ObjIdx < UnsplitInstancedObjects.Num(); ObjIdx++)
	{
		UObject* InstancedObject = UnsplitInstancedObjects[ObjIdx];

		// Map of split values to transform arrays
		TMap<FString, TArray<FTransform>> SplitTransformMap;
		TMap<FString, TArray<int32>> SplitIndicesMap;

		TArray<FTransform>& CurrentTransforms = UnsplitInstancedTransforms[ObjIdx];
		TArray<int32>& CurrentIndices = UnsplitInstancedIndices[ObjIdx];
		TArray<FString>& CurrentSplits = SplitAttributeValuesPerObject[ObjIdx];

		int32 NumInstances = CurrentTransforms.Num();
		if (CurrentSplits.Num() != NumInstances || CurrentIndices.Num() != NumInstances)
			continue;

		// Split the transforms using the split values
		for (int32 InstIdx = 0; InstIdx < NumInstances; InstIdx++)
		{
			const FString& SplitAttrValue = CurrentSplits[InstIdx];
			SplitTransformMap.FindOrAdd(SplitAttrValue).Add(CurrentTransforms[InstIdx]);
			SplitIndicesMap.FindOrAdd(SplitAttrValue).Add(CurrentIndices[InstIdx]);

			int OriginalIndex = CurrentIndices[InstIdx];

			// Record attributes for any split value we have not yet seen
			FHoudiniInstancedOutputPerSplitAttributes& PerSplitAttributes = OutPerSplitAttributes.FindOrAdd(SplitAttrValue);
			if (bHasAnyPerSplitAttributes)
			{
				if (bHasLevelPaths && PerSplitAttributes.LevelPath.IsEmpty() && AllLevelPaths.IsValidIndex(OriginalIndex))
				{
					PerSplitAttributes.LevelPath = AllLevelPaths[OriginalIndex];
				}
				if (bHasBakeActorNames && PerSplitAttributes.BakeActorName.IsEmpty() && AllBakeActorNames.IsValidIndex(OriginalIndex))
				{
					PerSplitAttributes.BakeActorName = AllBakeActorNames[OriginalIndex];
				}
				if (bHasBakeFolders && PerSplitAttributes.BakeFolder.IsEmpty() && AllBakeFolders.IsValidIndex(OriginalIndex))
				{
					PerSplitAttributes.BakeFolder = AllBakeFolders[OriginalIndex];
				}
				if (bHasBakeOutlinerFolders && PerSplitAttributes.BakeOutlinerFolder.IsEmpty() && AllBakeOutlinerFolders.IsValidIndex(OriginalIndex))
				{
					PerSplitAttributes.BakeOutlinerFolder = AllBakeOutlinerFolders[OriginalIndex];
				}
			}

			PerSplitAttributes.DataLayers = FHoudiniDataLayerUtils::GetDataLayers(InHGPO.GeoId, InHGPO.PartId, HAPI_GroupType::HAPI_GROUPTYPE_POINT, OriginalIndex);
			PerSplitAttributes.HLODLayers = FHoudiniHLODLayerUtils::GetHLODLayers(InHGPO.GeoId, InHGPO.PartId, HAPI_ATTROWNER_POINT, OriginalIndex);

		}

		// Add the objects, transform, split values to the final arrays
		for (auto& Iterator : SplitTransformMap)
		{
			OutSplitAttributeValue.Add(Iterator.Key);
			OutInstancedObjects.Add(InstancedObject);
			OutInstancedTransforms.Add(Iterator.Value);
			OutInstancedIndices.Add(SplitIndicesMap[Iterator.Key]);
		}
	}

	OutSplitAttributeName = SplitAttribName;

	return true;
}


bool
FHoudiniInstanceTranslator::GetOldSchoolAttributeInstancerHGPOsAndTransforms(
	const FHoudiniGeoPartObject& InHGPO,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	TArray<FHoudiniGeoPartObject>& OutInstancedHGPO,
	TArray<TArray<FTransform>>& OutInstancedTransforms,
	TArray<TArray<int32>>& OutInstancedIndices)
{
	if (InHGPO.InstancerType != EHoudiniInstancerType::OldSchoolAttributeInstancer)
		return false;

	// Get the instance transforms
	TArray<FTransform> InstancerUnrealTransforms;
	if (!HapiGetInstanceTransforms(InHGPO, InstancerUnrealTransforms))
	{
		// failed to get instance transform
		return false;
	}

	// Get the objects IDs to instanciate
	int32 NumPoints = InHGPO.PartInfo.PointCount;
	TArray<HAPI_NodeId> InstancedObjectIds;
	InstancedObjectIds.SetNumUninitialized(NumPoints);
	HOUDINI_CHECK_ERROR_RETURN( FHoudiniApi::GetInstancedObjectIds(
		FHoudiniEngine::Get().GetSession(), 
		InHGPO.GeoId, InstancedObjectIds.GetData(), 0, NumPoints), false);

	// Find the set of instanced object ids and locate the corresponding parts
	TSet<int32> UniqueInstancedObjectIds(InstancedObjectIds);
	
	// Locate all the HoudiniGeoPartObject that corresponds to the instanced object IDs
	for (int32 InstancedObjectId : UniqueInstancedObjectIds)
	{
		// Get the parts that correspond to that object Id
		TArray<FHoudiniGeoPartObject> PartsToInstance;
		for (const auto& Output : InAllOutputs)
		{
			if (!Output || Output->Type != EHoudiniOutputType::Mesh)
				continue;
			
			for (const auto& OutHGPO : Output->HoudiniGeoPartObjects)
			{
				if (OutHGPO.Type != EHoudiniPartType::Mesh)
					continue;

				if (OutHGPO.bIsInstanced)
					continue;

				if (InstancedObjectId != OutHGPO.ObjectId)
					continue;

				PartsToInstance.Add(OutHGPO);
			}
		}

		// Extract only the transforms that correspond to that specific object ID
		TArray<FTransform> InstanceTransforms;
		TArray<int32> InstanceIndices;
		for (int32 Ix = 0; Ix < InstancedObjectIds.Num(); ++Ix)
		{
			if ((InstancedObjectIds[Ix] == InstancedObjectId) && (InstancerUnrealTransforms.IsValidIndex(Ix)))
			{
				InstanceTransforms.Add(InstancerUnrealTransforms[Ix]);
				InstanceIndices.Add(Ix);
			}
		}

		// Add the instanced parts and their transforms to the output arrays
		for (const auto& PartToInstance : PartsToInstance)
		{
			OutInstancedHGPO.Add(PartToInstance);
			OutInstancedTransforms.Add(InstanceTransforms);
			OutInstancedIndices.Add(InstanceIndices);
		}
	}

	if(OutInstancedHGPO.Num() > 0 && OutInstancedTransforms.Num() > 0 && OutInstancedIndices.Num() > 0)
		return true;

	return false;
}


bool
FHoudiniInstanceTranslator::GetObjectInstancerHGPOsAndTransforms(
	const FHoudiniGeoPartObject& InHGPO,
	const TArray<UHoudiniOutput*>& InAllOutputs,
	TArray<FHoudiniGeoPartObject>& OutInstancedHGPO,
	TArray<TArray<FTransform>>& OutInstancedTransforms,
	TArray<TArray<int32>>& OutInstancedIndices)
{
	if (InHGPO.InstancerType != EHoudiniInstancerType::ObjectInstancer)
		return false;

	if (InHGPO.ObjectInfo.ObjectToInstanceID < 0)
		return false;

	// Get the instance transforms
	TArray<FTransform> InstancerUnrealTransforms;
	if (!HapiGetInstanceTransforms(InHGPO, InstancerUnrealTransforms))
	{
		// failed to get instance transform
		return false;
	}

	// Get the parts that correspond to that Object Id
	TArray<FHoudiniGeoPartObject> PartsToInstance;
	for (const auto& Output : InAllOutputs)
	{
		if (!Output || Output->Type != EHoudiniOutputType::Mesh)
			continue;

		for (const auto& OutHGPO : Output->HoudiniGeoPartObjects)
		{
			if (OutHGPO.Type != EHoudiniPartType::Mesh)
				continue;

			/*
			// But the instanced geo is actually not marked as instanced
			if (!OutHGPO.bIsInstanced)
				continue;
			*/

			if (InHGPO.ObjectInfo.ObjectToInstanceID != OutHGPO.ObjectId)
				continue;

			PartsToInstance.Add(OutHGPO);
		}
	}

	// Add found HGPO and transforms to the output arrays
	for (auto& InstanceHGPO : PartsToInstance)
	{
		InstanceHGPO.TransformMatrix = InHGPO.TransformMatrix;

		// TODO:
		//InstanceHGPO.UpdateCustomName();

		OutInstancedHGPO.Add(InstanceHGPO);
		OutInstancedTransforms.Add(InstancerUnrealTransforms);

		TArray<int32> Indices;
		Indices.SetNum(InstancerUnrealTransforms.Num());
		for (int32 Index = 0; Index < Indices.Num(); ++Index)
		{
			Indices[Index] = Index;
		}

		OutInstancedIndices.Add(Indices);
	}

	return true;
}

InstancerComponentType GetComponentsType(USceneComponent* Component)
{
	InstancerComponentType ComponentType = InstancerComponentType::Invalid;

    if (Component != nullptr)
	{
		if (Component->IsA<UFoliageInstancedStaticMeshComponent>())
			ComponentType = Foliage;
		else if (Component->GetOwner() && Component->GetOwner()->IsA<AInstancedFoliageActor>())
			ComponentType = Foliage;
		else if (Component->IsA<UHierarchicalInstancedStaticMeshComponent>())
			ComponentType = HierarchicalInstancedStaticMeshComponent;
		else if (Component->IsA<UInstancedStaticMeshComponent>())
			ComponentType = InstancedStaticMeshComponent;
		else if (Component->IsA<UHoudiniMeshSplitInstancerComponent>())
			ComponentType = MeshSplitInstancerComponent;
		else if (Component->IsA<UHoudiniInstancedActorComponent>())
			ComponentType = HoudiniInstancedActorComponent;
		else if (Component->IsA<UStaticMeshComponent>())
			ComponentType = StaticMeshComponent;
		else if (Component->IsA<UHoudiniStaticMeshComponent>())
			ComponentType = HoudiniStaticMeshComponent;
		else if (Component->IsA<UGeometryCollectionComponent>())
			ComponentType = GeometryCollectionComponent;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		else if (Component->IsA<ULevelInstanceComponent>())
			ComponentType = LevelInstance;
#endif
	}
	return ComponentType;
}

InstancerComponentType GetComponentsType(TArray<USceneComponent*>& Components)
{
	if (Components.Num() == 0)
	    return InstancerComponentType::Invalid;

	InstancerComponentType ComponentType = GetComponentsType(Components[0]);

	for(int Index = 0; Index < Components.Num(); Index++)
	{
        int OtherType = GetComponentsType(Components[Index]);
		check(OtherType == ComponentType);
	}
	return ComponentType;

}

bool
FHoudiniInstanceTranslator::CreateOrUpdateInstancer(
	UObject* InstancedObject,
	const TArray<FTransform>& InstancedObjectTransforms,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	const FHoudiniPackageParams& InPackageParams,
	USceneComponent* ParentComponent,
	TArray<USceneComponent*>& OldComponents,
	TArray<USceneComponent*>& NewComponents,
	TArray<AActor*>& OldActors,
	TArray<AActor*>& NewActors,
	bool InIsSplitMeshInstancer,
	bool InIsFoliageInstancer,
	const TArray<UMaterialInterface *>& InstancerMaterials,
	const TArray<int32>& OriginalInstancerObjectIndices,
	int32& FoliageTypeCount,
	UFoliageType*& FoliageTypeUsed,
	UWorld*& WorldUsed,
	bool bForceHISM,
	bool bForceInstancer)
{
	// See if we can reuse the old component
	InstancerComponentType OldType = GetComponentsType(OldComponents);

	// Geometry collections only have one component for all instancers and is rebuilt in HoudiniGeometryCollectionTrnaslator.
	if (OldType == GeometryCollectionComponent && !OldComponents.IsEmpty())
	{
		for(auto OldComponent : OldComponents)
		{
			RemoveAndDestroyComponent(OldComponent, nullptr);
		}
		OldComponents.Empty();
	}

	// See what type of component we want to create
	InstancerComponentType NewType = InstancerComponentType::Invalid;

	UStaticMesh* StaticMesh = Cast<UStaticMesh>(InstancedObject);

	UFoliageType* FoliageType = Cast<UFoliageType>(InstancedObject);
	if (IsValid(FoliageType))
	{
		StaticMesh = Cast<UStaticMesh>(FoliageType->GetSource());
	}

	UWorld * World = Cast<UWorld>(InstancedObject);

	UHoudiniStaticMesh * HSM = nullptr;
	if (!StaticMesh && !FoliageType)
		HSM = Cast<UHoudiniStaticMesh>(InstancedObject);

	if (IsValid(FoliageType))
	{
		// We must test for foliage type first, or FT will be considered as meshes
		NewType = Foliage;
	}
	else if(IsValid(StaticMesh))
	{
		const bool bMustUseInstancerComponent = InstancedObjectTransforms.Num() > 1 || bForceInstancer;
		if (InIsFoliageInstancer)
			NewType = Foliage;
		else if (InIsSplitMeshInstancer)
			NewType = MeshSplitInstancerComponent;
		// It is recommended to avoid putting Nanite mesh in HISM since they have their own LOD mechanism.
		// Will also improve performance by avoiding access to the render data to fetch the LOD count which could
		// trigger an async mesh wait until it has been computed.
		else if (!StaticMesh->NaniteSettings.bEnabled && (bForceHISM || (bMustUseInstancerComponent && StaticMesh->GetNumLODs() > 1)))
			NewType = HierarchicalInstancedStaticMeshComponent;
		else if (bMustUseInstancerComponent)
			NewType = InstancedStaticMeshComponent;
		else
			NewType = StaticMeshComponent;
	}
	else if (IsValid(HSM))
	{
		if (InstancedObjectTransforms.Num() == 1)
			NewType = HoudiniStaticMeshComponent;
		else
		{
			HOUDINI_LOG_ERROR(TEXT("More than one instance transform encountered for UHoudiniStaticMesh: %s"), *(HSM->GetPathName()));
			NewType = Invalid;
			return false;
		}
	}
	else if (IsValid(World))
	{
		if (InIsFoliageInstancer)
		{
			HOUDINI_LOG_ERROR(TEXT("Cannot use a level instance as foliage"));
			return false;
		}
		NewType = LevelInstance;
	}
	else
	{
		NewType = HoudiniInstancedActorComponent;
	}

	if (OldType == NewType)
	{
		NewComponents = OldComponents;
	}

	if (NewComponents.Num() == 0)
		NewComponents.Add(nullptr);

	// First valid index in the original instancer part 
	// This should be used to access attributes that are store for the whole part, not split
	// (ie, GenericProperty Attributes)
	int32 FirstOriginalIndex = OriginalInstancerObjectIndices.Num() > 0 ? OriginalInstancerObjectIndices[0] : 0;

	bool bCheckRenderState = false;
	bool bSuccess = false;
	switch (NewType)
	{
		case InstancedStaticMeshComponent:
		case HierarchicalInstancedStaticMeshComponent:
		{
			// Create an Instanced Static Mesh Component
			bSuccess = CreateOrUpdateInstancedStaticMeshComponent(
				StaticMesh, InstancedObjectTransforms, AllPropertyAttributes, InstancerGeoPartObject, ParentComponent, NewComponents[0], InstancerMaterials, bForceHISM, FirstOriginalIndex);
			bCheckRenderState = true;
		}
		break;

		case MeshSplitInstancerComponent:
		{
			bSuccess = CreateOrUpdateMeshSplitInstancerComponent(
				StaticMesh, InstancedObjectTransforms, AllPropertyAttributes, InstancerGeoPartObject, ParentComponent, NewComponents[0], InstancerMaterials);
		}
		break;

		case HoudiniInstancedActorComponent:
		{
			bSuccess = CreateOrUpdateInstancedActorComponent(
				InstancedObject, InstancedObjectTransforms, OriginalInstancerObjectIndices, AllPropertyAttributes, &InstancerGeoPartObject, ParentComponent, NewComponents[0]);
		}
		break;

		case StaticMeshComponent:
		{
			// Create a Static Mesh Component
			bSuccess = CreateOrUpdateStaticMeshComponent(
				StaticMesh, InstancedObjectTransforms, FirstOriginalIndex, AllPropertyAttributes, InstancerGeoPartObject, ParentComponent, NewComponents[0], InstancerMaterials);
			bCheckRenderState = true;
		}
		break;

		case HoudiniStaticMeshComponent:
		{
			// Create a Houdini Static Mesh Component
			bSuccess = CreateOrUpdateHoudiniStaticMeshComponent(
				HSM, InstancedObjectTransforms, FirstOriginalIndex, AllPropertyAttributes, InstancerGeoPartObject, ParentComponent, NewComponents[0], InstancerMaterials);
		}
		break;

		case Foliage:
		{
			bSuccess = CreateOrUpdateFoliageInstances(
				StaticMesh, FoliageType, WorldUsed, InstancedObjectTransforms, FirstOriginalIndex, AllPropertyAttributes, InstancerGeoPartObject, InPackageParams, FoliageTypeCount, ParentComponent, FoliageTypeUsed, NewComponents, InstancerMaterials);

		}
		break;
		case LevelInstance:
		{
			NewComponents.Empty();

			// Create a Houdini Static Mesh Component
			bSuccess = CreateOrUpdateLevelInstanceActors(
				World, InstancedObjectTransforms, FirstOriginalIndex, AllPropertyAttributes, InstancerGeoPartObject, ParentComponent, NewActors, InstancerMaterials);
		}
		break;
	}

	for(auto NewComponentToSet : NewComponents)
	{
		// UE5: Make sure we update/recreate the Component's render state
	    // after the update or the mesh component will not be rendered!
	    if (bCheckRenderState)
	    {
		    UMeshComponent* const NewMeshComponent = Cast<UMeshComponent>(NewComponentToSet);
		    if (IsValid(NewMeshComponent))
		    {
			    if (NewMeshComponent->IsRenderStateCreated())
			    {
				    // Need to send this to render thread at some point
				    NewMeshComponent->MarkRenderStateDirty();
			    }
			    else if (NewMeshComponent->ShouldCreateRenderState())
			    {
				    // If we didn't have a valid StaticMesh assigned before
				    // our render state might not have been created so
				    // do it now.
				    NewMeshComponent->RecreateRenderState_Concurrent();
			    }
		    }
	    }

		NewComponentToSet->SetMobility(ParentComponent->Mobility);

		if (NewType != Foliage && NewType != LevelInstance)
		    NewComponentToSet->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform);

	    // For single instance, that generates a SMC, the transform is already set on the component
	    // TODO: Should cumulate transform in that case?
	    if(NewType != StaticMeshComponent && NewType != HoudiniStaticMeshComponent && NewType != LevelInstance)
			NewComponentToSet->SetRelativeTransform(InstancerGeoPartObject.TransformMatrix);

	    // Only register if we have a valid component
	    if (NewComponentToSet->GetOwner() && NewComponentToSet->GetWorld())
			NewComponentToSet->RegisterComponent();

	}

	//
	// If the old components couldn't be reused, dettach/ destroy them.
	//

	TSet<USceneComponent*> ComponentsToRemove(OldComponents);
	for(auto Component : NewComponents)
	{
	    if (ComponentsToRemove.Contains(Component))
		    ComponentsToRemove.Remove(Component);
	}
	for (auto Component : ComponentsToRemove)
	{
		RemoveAndDestroyComponent(Component, nullptr);
	}

	//
	// If the old actors couldn't be reused, dettach/ destroy them.
	//
	TSet<AActor*> ActorsToRemove(OldActors);
	for (auto NewActor : NewActors)
	{
		if (ActorsToRemove.Contains(NewActor))
			ActorsToRemove.Remove(NewActor);
	}

	for (auto Actor : ActorsToRemove)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		if (IsValid(Actor) && Actor->IsA<ALevelInstance>())
		{
			Actor->Destroy();
		}
#endif
	}

	return bSuccess;
}

bool
FHoudiniInstanceTranslator::CreateOrUpdateInstancedStaticMeshComponent(
	UStaticMesh* InstancedStaticMesh,
	const TArray<FTransform>& InstancedObjectTransforms,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	USceneComponent* ParentComponent,
	USceneComponent*& CreatedInstancedComponent,
	TArray<UMaterialInterface*> InstancerMaterials,
	const bool & bForceHISM,
	const int32& InstancerObjectIdx)
{
	if (!InstancedStaticMesh)
		return false;

	if (!IsValid(ParentComponent))
		return false;

	UObject* ComponentOuter = ParentComponent;
	if (IsValid(ParentComponent->GetOwner()))
		ComponentOuter = ParentComponent->GetOwner();

	bool bCreatedNewComponent = false;
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(CreatedInstancedComponent);
	if (!IsValid(InstancedStaticMeshComponent))
	{
		// It is recommended to avoid putting Nanite mesh in HISM since they have their own LOD mecanism.
		// Will also improve performance by avoiding access to the render data to fetch the LOD count which could
		// trigger an async mesh wait until it has been computed.
		if (!InstancedStaticMesh->NaniteSettings.bEnabled && (InstancedStaticMesh->GetNumLODs() > 1 || bForceHISM))
		{
			// If the mesh has LODs, use Hierarchical ISMC
			InstancedStaticMeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(
				ComponentOuter, UHierarchicalInstancedStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);
		}
		else
		{
			// If the mesh doesnt have LOD, we can use a regular ISMC
			InstancedStaticMeshComponent = NewObject<UInstancedStaticMeshComponent>(
				ComponentOuter, UInstancedStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);
		}

		// Change the creation method so the component is listed in the details panels
		if (InstancedStaticMeshComponent)
			FHoudiniEngineRuntimeUtils::AddOrSetAsInstanceComponent(InstancedStaticMeshComponent);

		bCreatedNewComponent = true;
	}

	if (!InstancedStaticMeshComponent)
		return false;
	
	FHoudiniEngineUtils::KeepOrClearComponentTags(InstancedStaticMeshComponent, &InstancerGeoPartObject);

	InstancedStaticMeshComponent->SetStaticMesh(InstancedStaticMesh);

	if (InstancedStaticMeshComponent->GetBodyInstance())
	{
		InstancedStaticMeshComponent->GetBodyInstance()->bAutoWeld = false;
	}

	InstancedStaticMeshComponent->OverrideMaterials.Empty();
	if (InstancerMaterials.Num() > 0)
	{
		int32 MeshMaterialCount = InstancedStaticMesh->GetStaticMaterials().Num();
		for (int32 Idx = 0; Idx < MeshMaterialCount; ++Idx)
		{
			if (InstancerMaterials.IsValidIndex(Idx) && IsValid(InstancerMaterials[Idx]))
				InstancedStaticMeshComponent->SetMaterial(Idx, InstancerMaterials[Idx]);
		}
	}

	int32 NumOldInstances = InstancedStaticMeshComponent->GetInstanceCount();
	int32 NumNewInstances = InstancedObjectTransforms.Num();
	if (NumOldInstances == NumNewInstances)
	{
		// For efficiency, try to reuse the existing buffer.
		InstancedStaticMeshComponent->BatchUpdateInstancesTransforms(0, InstancedObjectTransforms, false, true);
	}
	else
	{
		// Clear old instances, add new ones.
		InstancedStaticMeshComponent->ClearInstances();
		InstancedStaticMeshComponent->AddInstances(InstancedObjectTransforms, false);
	}

	// Apply generic attributes if we have any
	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(InstancedStaticMeshComponent, AllPropertyAttributes, InstancerObjectIdx);

	// Assign the new ISMC / HISMC to the output component if we created a new one
	if(bCreatedNewComponent)
		CreatedInstancedComponent = InstancedStaticMeshComponent;

	return true;
}

bool
FHoudiniInstanceTranslator::CreateOrUpdateInstancedActorComponent(
	UObject* InstancedObject,
	const TArray<FTransform>& InstancedObjectTransforms,
	const TArray<int32>& OriginalInstancerObjectIndices,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject* InstancerHGPO,
	USceneComponent* ParentComponent,
	USceneComponent*& CreatedInstancedComponent)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHoudiniInstanceTranslator::CreateInstancedActorInstancer);

	if (!InstancedObject)
		return false;

	if (!IsValid(ParentComponent))
		return false;

	UObject* ComponentOuter = ParentComponent;
	if (IsValid(ParentComponent->GetOwner()))
		ComponentOuter = ParentComponent->GetOwner();

	bool bCreatedNewComponent = false;
	UHoudiniInstancedActorComponent* InstancedActorComponent = Cast<UHoudiniInstancedActorComponent>(CreatedInstancedComponent);
	if (!IsValid(InstancedActorComponent))
	{
		// If the mesh doesnt have LOD, we can use a regular ISMC
		InstancedActorComponent = NewObject<UHoudiniInstancedActorComponent>(
			ComponentOuter, UHoudiniInstancedActorComponent::StaticClass(), NAME_None, RF_Transactional);
		
		// Change the creation method so the component is listed in the details panels
		FHoudiniEngineRuntimeUtils::AddOrSetAsInstanceComponent(InstancedActorComponent);

		bCreatedNewComponent = true;
	}

	if (!InstancedActorComponent)
		return false;

	FHoudiniEngineUtils::KeepOrClearComponentTags(InstancedActorComponent, InstancerHGPO);

	// See if the instanced object has changed
	bool bInstancedObjectHasChanged = (InstancedObject != InstancedActorComponent->GetInstancedObject());
	if (bInstancedObjectHasChanged)
	{
		// All actors will need to be respawned, invalidate all of them
		InstancedActorComponent->ClearAllInstances();

		// Update the HIAC's instanced asset
		InstancedActorComponent->SetInstancedObject(InstancedObject);
	}

	// Get the level where we want to spawn the actors
	ULevel* SpawnLevel = ParentComponent->GetOwner() ? ParentComponent->GetOwner()->GetLevel() : nullptr;
	if (!SpawnLevel)
		return false;

	// Set the number of needed instances
	InstancedActorComponent->SetNumberOfInstances(InstancedObjectTransforms.Num());

	AActor* ReferenceActor = nullptr;
	for (int32 Idx = 0; Idx < InstancedObjectTransforms.Num(); Idx++)
	{
		// if we already have an actor, we can reuse it
		const FTransform& CurTransform = InstancedObjectTransforms[Idx];

		// Get the current instance
		// If null, we need to create a new one, else we can reuse the actor
		AActor* CurInstance = InstancedActorComponent->GetInstancedActorAt(Idx);
		if (!IsValid(CurInstance))
		{
			CurInstance = SpawnInstanceActor(CurTransform, SpawnLevel, InstancedActorComponent, ReferenceActor);
			InstancedActorComponent->SetInstanceAt(Idx, CurTransform, CurInstance);
		}
		else
		{
			// We can simply update the actor's transform
			InstancedActorComponent->SetInstanceTransformAt(Idx, CurTransform);	
		}

		if (!ReferenceActor)
			ReferenceActor = CurInstance;

		// Keep or clear tags on the instanced actor
		FHoudiniEngineUtils::KeepOrClearActorTags(CurInstance, true, true, InstancerHGPO);

		// Update the generic properties for that instance if any
		FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(CurInstance, AllPropertyAttributes, OriginalInstancerObjectIndices[Idx]);
	}

	// Update generic properties for the component managing the instances
	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(InstancedActorComponent, AllPropertyAttributes);

	// Make sure Post edit change is called on all generated actors
	TArray<AActor*> NewActors = InstancedActorComponent->GetInstancedActors();
	for (auto& CurActor : NewActors)
	{
		if (CurActor)
			CurActor->PostEditChange();
	}

	// Assign the new ISMC / HISMC to the output component if we created a new one
	if (bCreatedNewComponent)
	{
		CreatedInstancedComponent = InstancedActorComponent;
	}

	return true;
}

// Create or update a MSIC
bool 
FHoudiniInstanceTranslator::CreateOrUpdateMeshSplitInstancerComponent(
	UStaticMesh* InstancedStaticMesh,
	const TArray<FTransform>& InstancedObjectTransforms,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	USceneComponent* ParentComponent,
	USceneComponent*& CreatedInstancedComponent,
	const TArray<UMaterialInterface *>& InInstancerMaterials)
{
	if (!InstancedStaticMesh)
		return false;

	if (!IsValid(ParentComponent))
		return false;

	UObject* ComponentOuter = ParentComponent;
	if (IsValid(ParentComponent->GetOwner()))
		ComponentOuter = ParentComponent->GetOwner();

	bool bCreatedNewComponent = false;
	UHoudiniMeshSplitInstancerComponent* MeshSplitComponent = Cast<UHoudiniMeshSplitInstancerComponent>(CreatedInstancedComponent);
	if (!IsValid(MeshSplitComponent))
	{
		// If the mesh doesn't have LOD, we can use a regular ISMC
		MeshSplitComponent = NewObject<UHoudiniMeshSplitInstancerComponent>(
			ComponentOuter, UHoudiniMeshSplitInstancerComponent::StaticClass(), NAME_None, RF_Transactional);

		// Change the creation method so the component is listed in the details panels
		FHoudiniEngineRuntimeUtils::AddOrSetAsInstanceComponent(MeshSplitComponent);

		bCreatedNewComponent = true;
	}

	if (!MeshSplitComponent)
		return false;

	// Write a deprecation warning for mesh split instancer... 
	HOUDINI_LOG_WARNING(TEXT("MeshSplitInstancers are deprecated in Houdini 20.0 - we recommand switching to attribute instancers and the unreal_split_attr attribute instead."));

	MeshSplitComponent->SetStaticMesh(InstancedStaticMesh);
	MeshSplitComponent->SetOverrideMaterials(InInstancerMaterials);
	
	FHoudiniEngineUtils::KeepOrClearComponentTags(MeshSplitComponent, &InstancerGeoPartObject);

	// Now add the instances
	MeshSplitComponent->SetInstanceTransforms(InstancedObjectTransforms);

	// Check for instance colors
	TArray<FLinearColor> InstanceColorOverrides;
	bool ColorOverrideAttributeFound = false;

	// Look for the unreal_instance_color attribute on points	
	HAPI_AttributeInfo AttributeInfo;
	FHoudiniApi::AttributeInfo_Init(&AttributeInfo);
	if (HAPI_RESULT_SUCCESS == FHoudiniApi::GetAttributeInfo(
		FHoudiniEngine::Get().GetSession(), InstancerGeoPartObject.GeoId, InstancerGeoPartObject.PartId,
		HAPI_UNREAL_ATTRIB_INSTANCE_COLOR, HAPI_AttributeOwner::HAPI_ATTROWNER_POINT, &AttributeInfo))
	{
		ColorOverrideAttributeFound = AttributeInfo.exists;
	}
	
	// Look for the unreal_instance_color attribute on prims? (why? original code)
	if (!ColorOverrideAttributeFound)
	{
		if (HAPI_RESULT_SUCCESS == FHoudiniApi::GetAttributeInfo(
			FHoudiniEngine::Get().GetSession(), InstancerGeoPartObject.GeoId, InstancerGeoPartObject.PartId,
			HAPI_UNREAL_ATTRIB_INSTANCE_COLOR, HAPI_AttributeOwner::HAPI_ATTROWNER_PRIM, &AttributeInfo))
		{
			ColorOverrideAttributeFound = AttributeInfo.exists;
		}
	}

	if (ColorOverrideAttributeFound)
	{
		if (AttributeInfo.tupleSize == 4)
		{
			// Allocate sufficient buffer for data.
			InstanceColorOverrides.SetNumZeroed(AttributeInfo.count);

			if (HAPI_RESULT_SUCCESS != FHoudiniApi::GetAttributeFloatData(
				FHoudiniEngine::Get().GetSession(), InstancerGeoPartObject.GeoId, InstancerGeoPartObject.PartId,
				HAPI_UNREAL_ATTRIB_INSTANCE_COLOR, &AttributeInfo, -1, (float*)InstanceColorOverrides.GetData(), 0, AttributeInfo.count))
			{
				InstanceColorOverrides.Empty();
			}
		}
		else if (AttributeInfo.tupleSize == 3)
		{
			// Allocate sufficient buffer for data.
			TArray<float> FloatValues;			
			FloatValues.SetNumZeroed(AttributeInfo.count * AttributeInfo.tupleSize);
			if (HAPI_RESULT_SUCCESS == FHoudiniApi::GetAttributeFloatData(
				FHoudiniEngine::Get().GetSession(), InstancerGeoPartObject.GeoId, InstancerGeoPartObject.PartId,
				HAPI_UNREAL_ATTRIB_INSTANCE_COLOR, &AttributeInfo, -1, (float*)FloatValues.GetData(), 0, AttributeInfo.count))
			{

				// Allocate sufficient buffer for data.
				InstanceColorOverrides.SetNumZeroed(AttributeInfo.count);

				// Convert float to FLinearColors
				for (int32 ColorIdx = 0; ColorIdx < InstanceColorOverrides.Num(); ColorIdx++)
				{
					InstanceColorOverrides[ColorIdx].R = FloatValues[ColorIdx * AttributeInfo.tupleSize + 0];
					InstanceColorOverrides[ColorIdx].G = FloatValues[ColorIdx * AttributeInfo.tupleSize + 1];
					InstanceColorOverrides[ColorIdx].B = FloatValues[ColorIdx * AttributeInfo.tupleSize + 2];
					InstanceColorOverrides[ColorIdx].A = 1.0;
				}
				FloatValues.Empty();
			}
		}
		else
		{
			HOUDINI_LOG_WARNING(TEXT(HAPI_UNREAL_ATTRIB_INSTANCE_COLOR " must be a float[4] or float[3] prim/point attribute"));
		}
	}

	// if we have vertex color overrides, apply them now
#if WITH_EDITOR
	if (InstanceColorOverrides.Num() > 0)
	{
		// Convert the color attribute to FColor
		TArray<FColor> InstanceColors;
		InstanceColors.SetNumUninitialized(InstanceColorOverrides.Num());
		for (int32 ix = 0; ix < InstanceColors.Num(); ++ix)
		{
			InstanceColors[ix] = InstanceColorOverrides[ix].GetClamped().ToFColor(false);
		}

		// Apply them to the instances
		auto & Instances = MeshSplitComponent->GetInstancesForWrite();
		for (int32 InstIndex = 0; InstIndex < Instances.Num(); InstIndex++)
		{
			UStaticMeshComponent* CurSMC = Instances[InstIndex];
			if (!IsValid(CurSMC))
				continue;

			if (!InstanceColors.IsValidIndex(InstIndex))
				continue;

			MeshPaintHelpers::FillStaticMeshVertexColors(CurSMC, -1, InstanceColors[InstIndex], FColor::White);

			//CurSMC->UnregisterComponent();
			//CurSMC->ReregisterComponent();

			{
				// We're only changing instanced vertices on this specific mesh component, so we
				// only need to detach our mesh component
				FComponentReregisterContext ComponentReregisterContext(CurSMC);
				for (auto& CurLODData : CurSMC->LODData)
				{
					BeginInitResource(CurLODData.OverrideVertexColors);
				}
			}

			//FIXME: How to get rid of the warning about fixup vertex colors on load?
			//SMC->FixupOverrideColorsIfNecessary();
		}
	}
#endif

	// Apply generic attributes if we have any
	// TODO: Handle variations w/ index
	// TODO: Optimize
	// Loop on attributes first, then components,
	// if failing to find the attrib on a component, skip the rest
	if (AllPropertyAttributes.Num() > 0)
	{
		auto & Instances = MeshSplitComponent->GetInstancesForWrite();
		for (int32 InstIndex = 0; InstIndex < Instances.Num(); InstIndex++)
		{
			UStaticMeshComponent* CurSMC = Instances[InstIndex];
			if (!IsValid(CurSMC))
				continue;

			FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(CurSMC, AllPropertyAttributes, InstIndex);
		}
	}

	// Assign the new ISMC / HISMC to the output component if we created a new one
	if (bCreatedNewComponent)
		CreatedInstancedComponent = MeshSplitComponent;

	// TODO:
	// We want to make this invisible if it's a collision instancer.
	//CreatedInstancedComponent->SetVisibility(!InstancerGeoPartObject.bIsCollidable);

	return true;
}

bool
FHoudiniInstanceTranslator::CreateOrUpdateStaticMeshComponent(
	UStaticMesh* InstancedStaticMesh,
	const TArray<FTransform>& InstancedObjectTransforms,
	const int32& InOriginalIndex,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	USceneComponent* ParentComponent,
	USceneComponent*& CreatedInstancedComponent,
	TArray<UMaterialInterface*> InstancerMaterials)
{
	if (!InstancedStaticMesh)
		return false;

	if (!IsValid(ParentComponent))
		return false;

	UObject* ComponentOuter = ParentComponent;
	if (IsValid(ParentComponent->GetOwner()))
		ComponentOuter = ParentComponent->GetOwner();

	bool bCreatedNewComponent = false;
	UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(CreatedInstancedComponent);
	if (!IsValid(SMC))
	{
		// Create a new StaticMeshComponent
		SMC = NewObject<UStaticMeshComponent>(
			ComponentOuter, UStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);

		// Change the creation method so the component is listed in the details panels
		FHoudiniEngineRuntimeUtils::AddOrSetAsInstanceComponent(SMC);

		bCreatedNewComponent = true;
	}

	if (!SMC)
		return false;

	SMC->SetStaticMesh(InstancedStaticMesh);
	SMC->GetBodyInstance()->bAutoWeld = false;
	
	FHoudiniEngineUtils::KeepOrClearComponentTags(SMC, &InstancerGeoPartObject);

	SMC->OverrideMaterials.Empty();
	if (InstancerMaterials.Num() > 0)
	{
		int32 MeshMaterialCount = InstancedStaticMesh->GetStaticMaterials().Num();
		for (int32 Idx = 0; Idx < MeshMaterialCount; ++Idx)
		{
			if (InstancerMaterials.IsValidIndex(Idx) && IsValid(InstancerMaterials[Idx]))
				SMC->SetMaterial(Idx, InstancerMaterials[Idx]);
		}
	}

	// Now add the instances Transform
	if (InstancedObjectTransforms.Num() > 0)
	{
		SMC->SetRelativeTransform(InstancedObjectTransforms[0]);
	}

	// Apply generic attributes if we have any
	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(SMC, AllPropertyAttributes, InOriginalIndex);

	// Assign the new ISMC / HISMC to the output component if we created a new one
	if (bCreatedNewComponent)
		CreatedInstancedComponent = SMC;

	// TODO:
	// We want to make this invisible if it's a collision instancer.
	//CreatedInstancedComponent->SetVisibility(!InstancerGeoPartObject.bIsCollidable);

	return true;
}

bool
FHoudiniInstanceTranslator::CreateOrUpdateHoudiniStaticMeshComponent(
	UHoudiniStaticMesh* InstancedProxyStaticMesh,
	const TArray<FTransform>& InstancedObjectTransforms,
	const int32& InOriginalIndex,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	USceneComponent* ParentComponent,
	USceneComponent*& CreatedInstancedComponent,
	TArray<UMaterialInterface*> InstancerMaterials)
{
	if (!InstancedProxyStaticMesh)
		return false;

	if (!IsValid(ParentComponent))
		return false;

	UObject* ComponentOuter = ParentComponent;
	if (IsValid(ParentComponent->GetOwner()))
		ComponentOuter = ParentComponent->GetOwner();

	bool bCreatedNewComponent = false;
	UHoudiniStaticMeshComponent* HSMC = Cast<UHoudiniStaticMeshComponent>(CreatedInstancedComponent);
	if (!IsValid(HSMC))
	{
		// Create a new StaticMeshComponent
		HSMC = NewObject<UHoudiniStaticMeshComponent>(
			ComponentOuter, UHoudiniStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);

		// Change the creation method so the component is listed in the details panels
		FHoudiniEngineRuntimeUtils::AddOrSetAsInstanceComponent(HSMC);

		bCreatedNewComponent = true;
	}

	if (!HSMC)
		return false; 

	HSMC->SetMesh(InstancedProxyStaticMesh);

	FHoudiniEngineUtils::KeepOrClearComponentTags(HSMC, &InstancerGeoPartObject);
	
	HSMC->OverrideMaterials.Empty();
	if (InstancerMaterials.Num() > 0)
	{
		int32 MeshMaterialCount = InstancedProxyStaticMesh->GetStaticMaterials().Num();
		for (int32 Idx = 0; Idx < MeshMaterialCount; ++Idx)
		{
			if (InstancerMaterials.IsValidIndex(Idx) && IsValid(InstancerMaterials[Idx]))
				HSMC->SetMaterial(Idx, InstancerMaterials[Idx]);
		}
	}

	// Now add the instances Transform
	HSMC->SetRelativeTransform(InstancedObjectTransforms[0]);

	// Apply generic attributes if we have any
	// TODO: Handle variations w/ index
	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(HSMC, AllPropertyAttributes, InOriginalIndex);

	// Assign the new  HSMC to the output component if we created a new one
	if (bCreatedNewComponent)
		CreatedInstancedComponent = HSMC;

	// TODO:
	// We want to make this invisible if it's a collision instancer.
	//CreatedInstancedComponent->SetVisibility(!InstancerGeoPartObject.bIsCollidable);

	return true;
}


bool
FHoudiniInstanceTranslator::CreateOrUpdateFoliageInstances(
	UStaticMesh* InstancedStaticMesh,
	UFoliageType* InFoliageType,
	UWorld*& WorldUsed,
	const TArray<FTransform>& InstancedObjectTransforms,
	const int32& FirstOriginalIndex,
	const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
	const FHoudiniGeoPartObject& InstancerGeoPartObject,
	const FHoudiniPackageParams& InPackageParams,
	int & FoliageTypeCount,
	USceneComponent* ParentComponent,
	UFoliageType*& CookedFoliageType,
	TArray<USceneComponent*>& NewInstancedComponents,
	TArray<UMaterialInterface*> InstancerMaterials)
{
	HOUDINI_CHECK_RETURN(IsValid(InstancedStaticMesh) || IsValid(InFoliageType), false);
	HOUDINI_CHECK_RETURN(IsValid(ParentComponent), false);

	AActor* OwnerActor = ParentComponent->GetOwner();
	HOUDINI_CHECK_RETURN(IsValid(OwnerActor), false);

	// We want to spawn the foliage in the same level as the parent HDA
	// as spawning in the current level may cause reference issue later on.
	ULevel* DesiredLevel = OwnerActor->GetLevel();
	HOUDINI_CHECK_RETURN(IsValid(DesiredLevel), false);

	WorldUsed = DesiredLevel->GetWorld();
	HOUDINI_CHECK_RETURN(IsValid(WorldUsed), false);

    // Previously, (pre 2023) we used to try to find existing foliage types in the current world, but this is dangerous
	// because it can trash the users data if they non-HDA foliage. This can get fairly confusing if there are two HDA
	// in the same level, and doesn't make it clear what is baked where. So always create a custom foliage type.

    FHoudiniPackageParams FoliageTypePackageParams =  InPackageParams;

	if (InFoliageType)
	{
	    CookedFoliageType = FHoudiniFoliageTools::DuplicateFoliageType(FoliageTypePackageParams, FoliageTypeCount, InFoliageType);
	}
	else
	{
		CookedFoliageType = FHoudiniFoliageTools::CreateFoliageType(FoliageTypePackageParams, FoliageTypeCount, InstancedStaticMesh);
	}

	++FoliageTypeCount;

	// Set material overrides on the cooked foliage type
	if (InstancerMaterials.Num() > 0)
	{
		UFoliageType_InstancedStaticMesh* const CookedMeshFoliageType = Cast<UFoliageType_InstancedStaticMesh>(CookedFoliageType);
		if (IsValid(CookedMeshFoliageType))
		{
			UStaticMesh const* const FoliageMesh = CookedMeshFoliageType->GetStaticMesh();
			const int32 MeshMaterialSlotCount = IsValid(FoliageMesh) ? FoliageMesh->GetStaticMaterials().Num() : 0;
			const int32 MaterialOverrideSlotCount = FMath::Min(InstancerMaterials.Num(), MeshMaterialSlotCount);
			for (int32 Idx = 0; Idx < MaterialOverrideSlotCount; ++Idx)
			{
				if (IsValid(InstancerMaterials[Idx]))
				{
					if (!CookedMeshFoliageType->OverrideMaterials.IsValidIndex(Idx))
						CookedMeshFoliageType->OverrideMaterials.SetNum(Idx + 1);
					CookedMeshFoliageType->OverrideMaterials[Idx] = InstancerMaterials[Idx];
				}
				else if (CookedMeshFoliageType->OverrideMaterials.IsValidIndex(Idx) && CookedMeshFoliageType->OverrideMaterials[Idx])
				{
					CookedMeshFoliageType->OverrideMaterials[Idx] = nullptr;
				}
			}
		}
	}
	
	FTransform HoudiniAssetTransform = ParentComponent->GetComponentTransform();
	
	TArray<FFoliageInstance> FoliageInstances;
	FoliageInstances.SetNum(InstancedObjectTransforms.Num());

	//for (auto CurrentTransform : InstancedObjectTransforms)
	for(int32 n = 0; n < InstancedObjectTransforms.Num(); n++)
	{
		// Instances transforms are relative to the HDA, 
		// But we need world transform for the Foliage Types
		FTransform CurrentTransform = InstancedObjectTransforms[n] * HoudiniAssetTransform;

		FoliageInstances[n].Location = CurrentTransform.GetLocation();
		FoliageInstances[n].Rotation = CurrentTransform.GetRotation().Rotator();
		FoliageInstances[n].DrawScale3D = (FVector3f)CurrentTransform.GetScale3D();
	}

	TArray<FFoliageAttachmentInfo> AttachmentTypes = 
		FHoudiniFoliageTools::GetAttachmentInfo(InstancerGeoPartObject.GeoId, InstancerGeoPartObject.PartId, FoliageInstances.Num());

	FHoudiniFoliageTools::SpawnFoliageInstances(WorldUsed, CookedFoliageType, FoliageInstances, AttachmentTypes);

	// Clear the returned component. This should be set, but doesn't make in world partition.
	// In future, this should be an array of components.
	NewInstancedComponents.Empty();

	TArray<FFoliageInfo*> FoliageInfos = FHoudiniFoliageTools::GetAllFoliageInfo(DesiredLevel->GetWorld(), CookedFoliageType);
	for(FFoliageInfo * FoliageInfo : FoliageInfos)
	{

	    UHierarchicalInstancedStaticMeshComponent* FoliageHISMC = FoliageInfo->GetComponent();	
	    if (IsValid(FoliageHISMC))
	    {
		    // TODO: This was due to a bug in UE4.22-20, check if still needed! 
		    FoliageHISMC->BuildTreeIfOutdated(true, true);

	    	FHoudiniEngineUtils::KeepOrClearComponentTags(FoliageHISMC, &InstancerGeoPartObject);

	        NewInstancedComponents.Add(FoliageHISMC);

			FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(FoliageHISMC, AllPropertyAttributes, FirstOriginalIndex);
	    }
	}

	// Try to apply generic properties attributes
	// either on the instancer, mesh or foliage type
	// TODO: Use proper atIndex!!

	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(InstancedStaticMesh, AllPropertyAttributes, FirstOriginalIndex);
	FHoudiniEngineUtils::UpdateGenericPropertiesAttributes(CookedFoliageType, AllPropertyAttributes, FirstOriginalIndex);

	// TODO:
	// We want to make this invisible if it's a collision instancer.
	//CreatedInstancedComponent->SetVisibility(!InstancerGeoPartObject.bIsCollidable);

	return true;
}

bool
FHoudiniInstanceTranslator::CreateOrUpdateLevelInstanceActors(
		UWorld* LevelInstanceWorld,
		const TArray<FTransform>& InstancedObjectTransforms,
		const int32& InOriginalIndex,
		const TArray<FHoudiniGenericAttribute>& AllPropertyAttributes,
		const FHoudiniGeoPartObject& InstancerGeoPartObject,
		USceneComponent* ParentComponent,
		TArray<AActor*> & NewInstanceActors,
		TArray<UMaterialInterface*> InstancerMaterials)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	UWorld* SpawnWorld = ParentComponent->GetWorld();

	for(int Index = 0; Index < InstancedObjectTransforms.Num(); Index++)
	{
		FTransform HoudiniAssetTransform = ParentComponent->GetComponentTransform();
		FTransform CurrentTransform = InstancedObjectTransforms[Index] * HoudiniAssetTransform;
		FString Name = FString::Printf(TEXT("%s_%d_%d_%d_%d"),
			*InstancerGeoPartObject.ObjectName,
			InstancerGeoPartObject.ObjectId,
			InstancerGeoPartObject.GeoId,
			InstancerGeoPartObject.PartId,
			Index);

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Name = FName(Name);
		SpawnInfo.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		SpawnInfo.Owner = ParentComponent->GetOwner();
		ALevelInstance* LevelInstance = Cast<ALevelInstance>(SpawnWorld->SpawnActor(ALevelInstance::StaticClass(), &CurrentTransform, SpawnInfo));
		LevelInstance->bDefaultOutlinerExpansionState = false;
		LevelInstance->SetWorldAsset(LevelInstanceWorld);
		LevelInstance->LoadLevelInstance();
		LevelInstance->SetActorLabel(Name);
		LevelInstance->AttachToActor(ParentComponent->GetOwner(), FAttachmentTransformRules::KeepWorldTransform);
		NewInstanceActors.Add(LevelInstance);

	}
	return true;
#else
	return false;
#endif
}

bool
FHoudiniInstanceTranslator::HapiGetInstanceTransforms(
	const FHoudiniGeoPartObject& InHGPO, 
	TArray<FTransform>& OutInstancerUnrealTransforms)
{
	// Get the instance transforms	
	int32 PointCount = InHGPO.PartInfo.PointCount;
	if (PointCount <= 0)
		return false;

	TArray<HAPI_Transform> InstanceTransforms;
	InstanceTransforms.SetNum(PointCount);
	for (int32 Idx = 0; Idx < InstanceTransforms.Num(); Idx++)
		FHoudiniApi::Transform_Init(&(InstanceTransforms[Idx]));

	if (HAPI_RESULT_SUCCESS != FHoudiniApi::GetInstanceTransformsOnPart(
		FHoudiniEngine::Get().GetSession(),
		InHGPO.GeoId, InHGPO.PartId, HAPI_SRT,
		&InstanceTransforms[0], 0, PointCount))
	{
		InstanceTransforms.SetNum(0);

		// TODO: Warning? error?
		return false;
	}

	// Convert the transform to Unreal's coordinate system
	OutInstancerUnrealTransforms.SetNumZeroed(InstanceTransforms.Num());
	for (int32 InstanceIdx = 0; InstanceIdx < InstanceTransforms.Num(); InstanceIdx++)
	{
		const auto& InstanceTransform = InstanceTransforms[InstanceIdx];
		FHoudiniEngineUtils::TranslateHapiTransform(InstanceTransform, OutInstancerUnrealTransforms[InstanceIdx]);
	}

	return true;
}

bool
FHoudiniInstanceTranslator::GetGenericPropertiesAttributes(
	const int32& InGeoNodeId, 
	const int32& InPartId, 
	TArray<FHoudiniGenericAttribute>& OutPropertyAttributes)
{
	// List all the generic property detail attributes ...
	int32 FoundCount = FHoudiniEngineUtils::GetGenericAttributeList(
		(HAPI_NodeId)InGeoNodeId, (HAPI_PartId)InPartId, HAPI_UNREAL_ATTRIB_GENERIC_UPROP_PREFIX, OutPropertyAttributes, HAPI_ATTROWNER_DETAIL);

	// .. then get all the values for the primitive property attributes
	FoundCount += FHoudiniEngineUtils::GetGenericAttributeList(
		(HAPI_NodeId)InGeoNodeId, (HAPI_PartId)InPartId, HAPI_UNREAL_ATTRIB_GENERIC_UPROP_PREFIX, OutPropertyAttributes, HAPI_ATTROWNER_PRIM, -1);

	// .. then finally, all values for point uproperty attributes
	// TODO: !! get the correct Index here?
	FoundCount += FHoudiniEngineUtils::GetGenericAttributeList(
		(HAPI_NodeId)InGeoNodeId, (HAPI_PartId)InPartId, HAPI_UNREAL_ATTRIB_GENERIC_UPROP_PREFIX, OutPropertyAttributes, HAPI_ATTROWNER_POINT, -1);

	return FoundCount > 0;
}

bool
FHoudiniInstanceTranslator::RemoveAndDestroyComponent(UObject* InComponent, UObject* InFoliageObject)
{
	if (!IsValid(InComponent))
		return false;

	UFoliageInstancedStaticMeshComponent* FISMC = Cast<UFoliageInstancedStaticMeshComponent>(InComponent);
	if (IsValid(FISMC))
	{
		// Make sure foliage our foliage instances have been removed
		USceneComponent* ParentComponent = Cast<USceneComponent>(FISMC->GetOuter());
		if (IsValid(ParentComponent))
			CleanupFoliageInstances(FISMC, InFoliageObject, ParentComponent);

		// do not delete FISMC that still have instances left
		// as we have cleaned up our instances before, these have been hand-placed
		if (FISMC->GetInstanceCount() > 0)
			return false;
	}

	USceneComponent* SceneComponent = Cast<USceneComponent>(InComponent);
	if (IsValid(SceneComponent))
	{
		/*
		UE5: DEPRECATED
		if (SceneComponent->IsA(UGeometryCollectionComponent::StaticClass()))
		{
			UActorComponent * DebugDrawComponent = SceneComponent->GetOwner()->FindComponentByClass(UGeometryCollectionDebugDrawComponent::StaticClass());
			if (DebugDrawComponent)
			{
				RemoveAndDestroyComponent(DebugDrawComponent, nullptr);
			}
		}
		*/
		
		// Remove from the HoudiniAssetActor
		if (SceneComponent->GetOwner())
			SceneComponent->GetOwner()->RemoveOwnedComponent(SceneComponent);

		SceneComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		SceneComponent->UnregisterComponent();
		SceneComponent->DestroyComponent();

		return true;
	}

	return false;
}

bool
FHoudiniInstanceTranslator::GetMaterialOverridesFromAttributes(
	const int32& InGeoNodeId,
	const int32& InPartId,
	const int32& InAttributeIndex,
	const EHoudiniInstancerType InInstancerType,
	TArray<FHoudiniMaterialInfo>& OutMaterialAttributes)
{	
	const HAPI_AttributeOwner AttribOwner = InInstancerType == EHoudiniInstancerType::AttributeInstancer ? HAPI_ATTROWNER_POINT : HAPI_ATTROWNER_PRIM;

	// Get the part info
	HAPI_PartInfo PartInfo;
	FHoudiniApi::PartInfo_Init(&PartInfo);
	HOUDINI_CHECK_ERROR_RETURN(
		FHoudiniApi::GetPartInfo(FHoudiniEngine::Get().GetSession(), InGeoNodeId, InPartId, &PartInfo), false);

	// Get all the part's attribute names
	int32 NumAttribs = PartInfo.attributeCounts[AttribOwner];
	TArray<HAPI_StringHandle> AttribNameHandles;
	AttribNameHandles.SetNum(NumAttribs);
	HOUDINI_CHECK_ERROR_RETURN(
		FHoudiniApi::GetAttributeNames(FHoudiniEngine::Get().GetSession(), InGeoNodeId, InPartId, AttribOwner, AttribNameHandles.GetData(), NumAttribs), false);

	// Extract the attribute names' strings
	TArray<FString> AllAttribNames;
	FHoudiniEngineString::SHArrayToFStringArray(AttribNameHandles, AllAttribNames);

	// Remove all unneeded attributes, only keep valid materials attr
	for (int32 Idx = AllAttribNames.Num() - 1; Idx >= 0; Idx--)
	{
		if (AllAttribNames[Idx].StartsWith(HAPI_UNREAL_ATTRIB_MATERIAL_INSTANCE))
			continue;

		if(AllAttribNames[Idx].StartsWith(HAPI_UNREAL_ATTRIB_MATERIAL))
			continue;

		if (AllAttribNames[Idx].StartsWith(HAPI_UNREAL_ATTRIB_MATERIAL_FALLBACK))
			continue;

		// Not a valid mat, remove from the array
		AllAttribNames.RemoveAt(Idx);
	}

	// Now look for different material attributes in the found attributes
	bool bFoundMaterialAttributes = false;
	// TODO: We need to turn this to an array in order to support a mix of material AND material instances
	//OutMaterialOverrideNeedsCreateInstance = false;

	if (AllAttribNames.IsEmpty())
		return false;

	TArray<FString> MaterialInstanceAttributes;
	TArray<FString> MaterialAttributes;

	// Get material instances overrides attributes
	if (GetMaterialOverridesFromAttributes(InGeoNodeId, InPartId, InAttributeIndex, HAPI_UNREAL_ATTRIB_MATERIAL_INSTANCE, AllAttribNames, MaterialInstanceAttributes))
		bFoundMaterialAttributes = true;

	// Get the "main" material override attributes
	if(GetMaterialOverridesFromAttributes(InGeoNodeId, InPartId, InAttributeIndex, HAPI_UNREAL_ATTRIB_MATERIAL, AllAttribNames, MaterialAttributes))
		bFoundMaterialAttributes = true;

	// If we haven't found anything, try the fallback attribute
	if (!bFoundMaterialAttributes && GetMaterialOverridesFromAttributes(InGeoNodeId, InPartId, InAttributeIndex, HAPI_UNREAL_ATTRIB_MATERIAL_FALLBACK, AllAttribNames, MaterialAttributes))
		bFoundMaterialAttributes = true;

	// We couldnt find any mat attribute? early return
	if (!bFoundMaterialAttributes)
	{
		OutMaterialAttributes.Empty();
		return false;
	}

	// Fetch material instance parameters (detail + AttribOwner) specified via attributes
	TArray<FHoudiniGenericAttribute> AllMatParams;
	FHoudiniMaterialTranslator::GetMaterialParameterAttributes(
		InGeoNodeId, InPartId, AttribOwner, AllMatParams, InAttributeIndex);

	// Consolidate the final material (or material instance) selection into OutMaterialAttributes
	// Use unreal_material if non-empty. If empty, fallback to unreal_material_instance.
	const int32 MaxNumSlots = FMath::Max(MaterialInstanceAttributes.Num(), MaterialAttributes.Num());
	OutMaterialAttributes.Reset(MaxNumSlots);
	for (int32 MatIdx = 0; MatIdx < MaxNumSlots; ++MatIdx)
	{
		FHoudiniMaterialInfo& MatInfo = OutMaterialAttributes.AddDefaulted_GetRef();
		MatInfo.MaterialIndex = MatIdx;
		// unreal_material takes precedence. If it is missing / empty, check unreal_material_instance
		if (MaterialAttributes.IsValidIndex(MatIdx) && !MaterialAttributes[MatIdx].IsEmpty())
		{
			MatInfo.MaterialObjectPath = MaterialAttributes[MatIdx];
		}
		else if (MaterialInstanceAttributes.IsValidIndex(MatIdx) && !MaterialInstanceAttributes[MatIdx].IsEmpty())
		{
			MatInfo.MaterialObjectPath = MaterialInstanceAttributes[MatIdx];
			MatInfo.bMakeMaterialInstance = true;
			// Get any material parameters for the instance, specified via attributes.
			// We use 0 for the index because we only loaded a specific index's attribute values into AllMatParams for
			// AttribOwner. So the underlying FHoudiniGenericAttribute only contains one entry per attribute.
			FHoudiniMaterialTranslator::GetMaterialParameters(MatInfo, AllMatParams, 0);
		}
	}
	
	return true;
}

bool
FHoudiniInstanceTranslator::GetMaterialOverridesFromAttributes(
	const int32& InGeoNodeId, 
	const int32& InPartId, 
	const int32& InAttributeIndex,
	const FString& InAttributeName,
	const TArray<FString>& InAllAttribNames,
	TArray<FString>& OutMaterialAttributes)
{
	// See if the "materialX" attributes were added as zero-based or not by searching for "material0"
	// If they are not (so the attributes starts at unreal_material1). then we'll need to decrement the idx
	FString MatZero = InAttributeName;
	MatZero += "0";
	bool bZeroBased = false;
	if (InAllAttribNames.Contains(MatZero))
		bZeroBased = true;

	bool bFoundMatAttributes = false;
	int32 PrefixLength = InAttributeName.Len();
	TArray<FString> MatName;
	for (const FString& AttribName : InAllAttribNames)
	{
		if (!AttribName.StartsWith(InAttributeName))
		{
			continue;
		}

		FString Fragment = AttribName.Mid(PrefixLength);

		int32 OverrideIdx = -1;
		if (Fragment == "")
		{
			// The attribute is exactly "unreal_material", use it as the default mat (index 0)
			OverrideIdx = 0;
		}
		else if (!Fragment.IsNumeric())
		{
			continue;
		}
		else
		{
			OverrideIdx = FCString::Atoi(*Fragment);
			if (!bZeroBased)
			{
				OverrideIdx--;
			}
		}

		if (OverrideIdx < 0)
		{
			continue;
		}

		// Increase the size of the array with empty materials
		while (OutMaterialAttributes.Num() <= OverrideIdx)
		{
			OutMaterialAttributes.Add("");
		}

		FHoudiniHapiAccessor Accessor(InGeoNodeId, InPartId, TCHAR_TO_ANSI(*AttribName));
		bool Res = Accessor.GetAttributeData(HAPI_ATTROWNER_INVALID, MatName, InAttributeIndex, 1);

		if (!Res)
		{
			HOUDINI_LOG_WARNING(TEXT("[FHoudiniInstanceTranslator::GetMaterialOverridesFromAttributes]: Failed to get material override index %d."), OverrideIdx);
			continue;
		}

		OutMaterialAttributes[OverrideIdx] = MatName[0];
		bFoundMatAttributes = true;
	}

	return bFoundMatAttributes;
}

bool
FHoudiniInstanceTranslator::GetInstancerMaterials(
	const TArray<FHoudiniMaterialInfo>& MaterialAttributes, TArray<UMaterialInterface*>& OutInstancerMaterials)
{
	// Use a map to avoid attempting to load the object for each instance
	TMap<FHoudiniMaterialIdentifier, UMaterialInterface*> MaterialMap;

	// Non-instanced materials check material attributes one by one
	const int32 NumSlots = MaterialAttributes.Num();
	OutInstancerMaterials.SetNumZeroed(NumSlots);
	for (int32 MatIdx = 0; MatIdx < NumSlots; ++MatIdx)
	{
		const FHoudiniMaterialInfo& CurrentMatInfo = MaterialAttributes[MatIdx];
		// Only process cases where we are not making material instances
		if (CurrentMatInfo.bMakeMaterialInstance)
			continue;

		const FHoudiniMaterialIdentifier MaterialIdentifier = CurrentMatInfo.MakeIdentifier();
		
		UMaterialInterface* CurrentMaterialInterface = nullptr;
		UMaterialInterface** FoundMaterial = MaterialMap.Find(MaterialIdentifier);
		if (!FoundMaterial)
		{
			// See if we can find a material interface that matches the attribute
			CurrentMaterialInterface = Cast<UMaterialInterface>(
				StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *CurrentMatInfo.MaterialObjectPath, nullptr, LOAD_NoWarn, nullptr));

			// Check validity
			if (!IsValid(CurrentMaterialInterface))
				CurrentMaterialInterface = nullptr;

			// Add what we found to the material map to avoid unnecessary loads
			MaterialMap.Add(MaterialIdentifier, CurrentMaterialInterface);
		}
		else
		{
			// Reuse what we previously found
			CurrentMaterialInterface = *FoundMaterial;
		}
		
		OutInstancerMaterials[MatIdx] = CurrentMaterialInterface;
	}

	return true;
}

bool 
FHoudiniInstanceTranslator::GetInstancerMaterialInstances(
	const TArray<FHoudiniMaterialInfo>& MaterialAttribute,
	const FHoudiniGeoPartObject& InHGPO,
	const FHoudiniPackageParams& InPackageParams,
	TArray<UMaterialInterface*>& OutInstancerMaterials)
{
	TMap<FHoudiniMaterialIdentifier, FHoudiniMaterialInfo> MaterialInstanceOverrides;
	TArray<FHoudiniMaterialIdentifier> MaterialIdentifiers;
	for (const FHoudiniMaterialInfo& MatInfo : MaterialAttribute)
	{
		if (!MatInfo.bMakeMaterialInstance)
		{
			MaterialIdentifiers.Add(FHoudiniMaterialIdentifier());
			continue;
		}
		const FHoudiniMaterialIdentifier MaterialIdentifier = MatInfo.MakeIdentifier();
		MaterialIdentifiers.Add(MaterialIdentifier);
		MaterialInstanceOverrides.Add(MaterialIdentifier, MatInfo);
	}

	// We have no material instances to create
	if (MaterialInstanceOverrides.Num() <= 0)
		return true;
	
	TArray<UPackage*> MaterialAndTexturePackages;
	TMap<FHoudiniMaterialIdentifier, TObjectPtr<UMaterialInterface>> InputAssignmentMaterials;
	TMap<FHoudiniMaterialIdentifier, TObjectPtr<UMaterialInterface>> OutputAssignmentMaterials;
	static constexpr bool bForceRecookAll = false;
	bool bSuccess = false;
	if (FHoudiniMaterialTranslator::CreateMaterialInstances(
			InHGPO,
			InPackageParams,
			MaterialInstanceOverrides,
			MaterialAndTexturePackages,
			InputAssignmentMaterials,
			OutputAssignmentMaterials,
			bForceRecookAll))
	{
		bSuccess = true;
		// Make sure that the OutInstancerMaterials array is the correct size
		OutInstancerMaterials.SetNumZeroed(MaterialAttribute.Num());
		const int32 NumSlots = MaterialIdentifiers.Num();
		for (int32 SlotIdx = 0; SlotIdx < NumSlots; ++SlotIdx)
		{
			const FHoudiniMaterialIdentifier& MaterialIdentifier = MaterialIdentifiers[SlotIdx];
			// skip the invalid ids (non material instance)
			if (!MaterialIdentifier.IsValid())
				continue;
			TObjectPtr<UMaterialInterface>* Material = OutputAssignmentMaterials.Find(MaterialIdentifier);
			if (!Material || !IsValid(*Material))
			{
				OutInstancerMaterials[SlotIdx] = nullptr;
				bSuccess = false;
			}
			else
			{
				OutInstancerMaterials[SlotIdx] = *Material;
			}
		}
	}

	return bSuccess;
}

bool
FHoudiniInstanceTranslator::GetAllInstancerMaterials(
	const int32& InGeoNodeId, 
	const int32& InPartId,
	const int32& InOriginalIndex,
	const FHoudiniGeoPartObject& InHGPO, 
	const FHoudiniPackageParams& InPackageParams, 
	TArray<UMaterialInterface*>& OutInstancerMaterials)
{
	// Get all the material attributes for that variation
	TArray<FHoudiniMaterialInfo> MaterialAttributes;
	FHoudiniInstanceTranslator::GetMaterialOverridesFromAttributes(
		InGeoNodeId, InPartId, InOriginalIndex, InHGPO.InstancerType, MaterialAttributes);

	// Get the materials (for which we don't create material instances)
	// OutInstancerMaterials is grown to the same length as MaterialAttributes (# slots). Sets materials in
	// corresponding slots.
	OutInstancerMaterials.SetNumZeroed(MaterialAttributes.Num());
	bool bSuccess = GetInstancerMaterials(MaterialAttributes, OutInstancerMaterials);

	// Get/create the material instances (if any were specified, see FHoudiniMaterialInfo.bMakeMaterialInstace
	// OutInstancerMaterials is grown to the same length as MaterialAttributes (# slots). Sets material instances
	// in corresponding slots.
	bSuccess &= GetInstancerMaterialInstances(MaterialAttributes, InHGPO, InPackageParams, OutInstancerMaterials);

	return bSuccess;
}


bool
FHoudiniInstanceTranslator::IsSplitInstancer(const int32& InGeoId, const int32& InPartId)
{
	bool bSplitMeshInstancer = false;
	HAPI_AttributeOwner Owner = HAPI_ATTROWNER_DETAIL;
	bSplitMeshInstancer = FHoudiniEngineUtils::HapiCheckAttributeExists(
		InGeoId, InPartId, HAPI_UNREAL_ATTRIB_SPLIT_INSTANCES, Owner);

	if (!bSplitMeshInstancer)
	{
		// Try on primitive
		Owner = HAPI_ATTROWNER_PRIM;
		bSplitMeshInstancer = FHoudiniEngineUtils::HapiCheckAttributeExists(InGeoId, InPartId, HAPI_UNREAL_ATTRIB_SPLIT_INSTANCES, Owner);
	}

	if (!bSplitMeshInstancer)
		return false;

	// Add deprecation warning for 20.0
	HOUDINI_LOG_WARNING(TEXT("MeshSplitInstancers are deprecated in Houdini 20.0 - we recommand switching to attribute instancers and the unreal_split_attr attribute instead."));

	TArray<int32> IntData;
	FHoudiniHapiAccessor Accessor(InGeoId, InPartId, HAPI_UNREAL_ATTRIB_SPLIT_INSTANCES);
	bool bSuccess = Accessor.GetAttributeData(Owner, IntData, 0, 1);

	if (!bSuccess || IntData.IsEmpty())
	{
		return false;
	}
	
	return (IntData[0] != 0);
}

bool
FHoudiniInstanceTranslator::IsFoliageInstancer(const int32& InGeoId, const int32& InPartId)
{
	bool bIsFoliageInstancer = false;
	HAPI_AttributeOwner Owner = HAPI_ATTROWNER_DETAIL;
	bIsFoliageInstancer = FHoudiniEngineUtils::HapiCheckAttributeExists(
		InGeoId, InPartId, HAPI_UNREAL_ATTRIB_FOLIAGE_INSTANCER, Owner);

	if (!bIsFoliageInstancer)
	{
		// Try on primitive
		Owner = HAPI_ATTROWNER_PRIM;
		bIsFoliageInstancer = FHoudiniEngineUtils::HapiCheckAttributeExists(
			InGeoId, InPartId, HAPI_UNREAL_ATTRIB_FOLIAGE_INSTANCER, Owner);
	}

	if (!bIsFoliageInstancer)
	{
		// Finally, try on points
		Owner = HAPI_ATTROWNER_POINT;
		bIsFoliageInstancer = FHoudiniEngineUtils::HapiCheckAttributeExists(
			InGeoId, InPartId, HAPI_UNREAL_ATTRIB_FOLIAGE_INSTANCER, Owner);
	}

	if (!bIsFoliageInstancer)
		return false;

	TArray<int32> IntData;

	// Get the first attribute value as Int
	FHoudiniHapiAccessor Accessor(InGeoId, InPartId, HAPI_UNREAL_ATTRIB_FOLIAGE_INSTANCER);

	bool bSuccess = Accessor.GetAttributeData(Owner, IntData, 0, 1);

	if (!bSuccess || IntData.IsEmpty())
		return false;

	return (IntData[0] != 0);
}


AActor*
FHoudiniInstanceTranslator::SpawnInstanceActor(
	const FTransform& InTransform,
	ULevel* InSpawnLevel,
	UHoudiniInstancedActorComponent* InIAC,
	AActor* InReferenceActor,
	const FName Name)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FHoudiniInstanceTranslator::SpawnInstanceActor);

	if (!IsValid(InIAC))
		return nullptr;

	UObject* InstancedObject = InIAC->GetInstancedObject();
	if (!IsValid(InstancedObject))
		return nullptr;

	AActor* NewActor = nullptr;

	UWorld* SpawnWorld = InSpawnLevel->GetWorld();
	UClass* InstancedActorClass = InIAC->GetInstancedActorClass();
	if (InstancedActorClass == nullptr || SpawnWorld == nullptr)
	{
#if WITH_EDITOR
		// Try to spawn a new actor for the given transform
		GEditor->ClickLocation = InTransform.GetTranslation();
		GEditor->ClickPlane = FPlane(GEditor->ClickLocation, FVector::UpVector);

		// Using this function lets unreal find the appropriate actor class for us
		// We only use it for the first instanced actors just to get the best actor class for that object
		// Once we have that class - it is much faster (~25x) to just use SpawnActor instead
		TArray<AActor*> NewActors = FLevelEditorViewportClient::TryPlacingActorFromObject(InSpawnLevel, InstancedObject, false, RF_Transactional, nullptr, Name);
		if (NewActors.Num() > 0)
		{
			if (IsValid(NewActors[0]))
			{
				NewActor = NewActors[0];
			}
		}

		// Set the instanced actor class on the IAC so we can reuse it
		if(NewActor)
			InIAC->SetInstancedActorClass(NewActor->GetClass());
#endif
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags = RF_Transactional;
		//SpawnParams.Owner = ComponentOuter;
		SpawnParams.OverrideLevel = InSpawnLevel;
		SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		SpawnParams.Template = nullptr;
		SpawnParams.bNoFail = true;
		// We need to use the previously instantiated actor as template when instantiating a decal material.
		SpawnParams.Template = InReferenceActor;

		NewActor = SpawnWorld->SpawnActor(InstancedActorClass, &InTransform, SpawnParams);
	}

	// Make sure that the actor was spawned in the proper level
	FHoudiniEngineUtils::MoveActorToLevel(NewActor, InSpawnLevel);

	return NewActor;
}


void 
FHoudiniInstanceTranslator::CleanupFoliageInstances(
	UHierarchicalInstancedStaticMeshComponent* InFoliageHISMC,
	UObject* InInstancedObject,
	USceneComponent* InParentComponent)
{
	if (!IsValid(InFoliageHISMC))
		return;

	UStaticMesh* FoliageSM = InFoliageHISMC->GetStaticMesh();
	if (!IsValid(FoliageSM))
		return;

	// If we are a foliage HISMC, then our owner is an Instanced Foliage Actor,
	// if it is not, then we are just a "regular" HISMC
	AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(InFoliageHISMC->GetOwner());
	if (!IsValid(InstancedFoliageActor))
		return;

	// Get the Foliage Type
	UFoliageType *FoliageType = Cast<UFoliageType>(InInstancedObject);
	if (!IsValid(FoliageType))
	{
		// Try to get the foliage type for the instanced mesh from the actor
		FoliageType = InstancedFoliageActor->GetLocalFoliageTypeForSource(InInstancedObject);

		if (!IsValid(FoliageType))
			return;
	}

	// Clean up the instances previously generated for that component
	InstancedFoliageActor->DeleteInstancesForComponent(InParentComponent, FoliageType);

	// Remove the foliage type if it doesn't have any more instances
	if(InFoliageHISMC->GetInstanceCount() == 0)
		InstancedFoliageActor->RemoveFoliageType(&FoliageType, 1);

	return;
}


FString
FHoudiniInstanceTranslator::GetInstancerTypeFromComponent(UObject* InObject)
{
	USceneComponent* InComponent = Cast<USceneComponent>(InObject);

	FString InstancerType = TEXT("Instancer");
	if (IsValid(InComponent))
	{
		if (InComponent->IsA<UHoudiniMeshSplitInstancerComponent>())
		{
			InstancerType = TEXT("(Split Instancer)");
		}
		else if (InComponent->IsA<UHoudiniInstancedActorComponent>())
		{
			InstancerType = TEXT("(Actor Instancer)");
		}
		else if (InComponent->IsA<UHierarchicalInstancedStaticMeshComponent>())
		{
			if (InComponent->GetOwner() && InComponent->GetOwner()->IsA<AInstancedFoliageActor>())
				InstancerType = TEXT("(Foliage Instancer)");
			else
				InstancerType = TEXT("(Hierarchical Instancer)");
		}
		else if (InComponent->IsA<UInstancedStaticMeshComponent>())
		{
			InstancerType = TEXT("(Mesh Instancer)");
		}
		else if (InComponent->IsA<UStaticMeshComponent>())
		{
			InstancerType = TEXT("(Static Mesh Component)");
		}
	}

	return InstancerType;
}

bool
FHoudiniInstanceTranslator::GetInstancerSplitAttributesAndValues(
	const int32& InGeoId,
	const int32& InPartId,
	const HAPI_AttributeOwner& InSplitAttributeOwner,
	FString& OutSplitAttributeName,
	TArray<FString>& OutAllSplitAttributeValues)
{
	// See if the user has specified an attribute to split the instancers.
	bool bHasSplitAttribute = false;
	//FString SplitAttribName = FString();
	OutSplitAttributeName = FString();

	// Look for the unreal_split_attr attribute
	// This attribute indicates the name of the point attribute that we'll use to split the instances further

	TArray<FString> StringData;

	FHoudiniHapiAccessor Accessor(InGeoId, InPartId, HAPI_UNREAL_ATTRIB_SPLIT_ATTR);
	bHasSplitAttribute = Accessor.GetAttributeData(InSplitAttributeOwner, 1, StringData, 0, 1);

	if (!bHasSplitAttribute || StringData.Num() <= 0)
		return false;

	OutSplitAttributeName = StringData[0];

	// We have specified a split attribute, try to get its values.
	OutAllSplitAttributeValues.Empty();
	if (!OutSplitAttributeName.IsEmpty())
	{
		Accessor.Init(InGeoId, InPartId, TCHAR_TO_ANSI(*OutSplitAttributeName));
		bool bSplitAttrFound = Accessor.GetAttributeData(InSplitAttributeOwner, 1, OutAllSplitAttributeValues);

		if (!bSplitAttrFound || OutAllSplitAttributeValues.Num() <= 0)
		{
			// We couldn't properly get the point values
			bHasSplitAttribute = false;
		}
	}
	else
	{
		// We couldn't properly get the split attribute
		bHasSplitAttribute = false;
	}

	if (!bHasSplitAttribute)
	{
		// Clean up everything to ensure that we'll ignore the split attribute
		OutAllSplitAttributeValues.Empty();
		OutSplitAttributeName = FString();
	}

	return bHasSplitAttribute;
}

bool 
FHoudiniInstanceTranslator::HasHISMAttribute(const HAPI_NodeId& GeoId, const HAPI_NodeId& PartId) 
{
	bool bHISM = false;

	TArray<int32> IntData;
	IntData.Empty();

	FHoudiniHapiAccessor Accessor(GeoId, PartId, HAPI_UNREAL_ATTRIB_HIERARCHICAL_INSTANCED_SM);
	bool bSuccess = Accessor.GetAttributeData(HAPI_ATTROWNER_INVALID, 1,  IntData, 0, 1);

	if (!bSuccess)
		return false;

	if (IntData.Num() <= 0)
		return false;

	return IntData[0] != 0;
}

bool 
FHoudiniInstanceTranslator::HasForceInstancerAttribute(const HAPI_NodeId& GeoId, const HAPI_NodeId& PartId) 
{
	bool bHISM = false;
	TArray<int32> IntData;
	IntData.Empty();

	FHoudiniHapiAccessor Accessor(GeoId, PartId, HAPI_UNREAL_ATTRIB_FORCE_INSTANCER);
	bool bSuccess = Accessor.GetAttributeData(HAPI_ATTROWNER_INVALID, 1, IntData, 0, 1);

	if (!bSuccess)
		return false;

	if (IntData.Num() <= 0)
		return false;

	return IntData[0] != 0;
}

void
FHoudiniInstancedOutputPartData::BuildFlatInstancedTransformsAndObjectPaths()
{
	NumInstancedTransformsPerObject.Empty(OriginalInstancedTransforms.Num());
	// We expect to have one or more entries per object
	OriginalInstancedTransformsFlat.Empty(OriginalInstancedTransforms.Num());
	for (const TArray<FTransform>& Transforms : OriginalInstancedTransforms)
	{
		NumInstancedTransformsPerObject.Add(Transforms.Num());
		OriginalInstancedTransformsFlat.Append(Transforms);
	}

	OriginalInstanceObjectPackagePaths.Empty(OriginalInstancedObjects.Num());
	for (const UObject* Obj : OriginalInstancedObjects)
	{
		if (IsValid(Obj))
		{
			OriginalInstanceObjectPackagePaths.Add(Obj->GetPathName());
		}
		else
		{
			OriginalInstanceObjectPackagePaths.Add(FString());
		}
	}

	NumInstancedIndicesPerObject.Empty(OriginalInstancedIndices.Num());
	// We expect to have one or more entries per object
	OriginalInstancedIndicesFlat.Empty(OriginalInstancedIndices.Num());
	for (const TArray<int32>& InstancedIndices : OriginalInstancedIndices)
	{
		NumInstancedIndicesPerObject.Add(InstancedIndices.Num());
		OriginalInstancedIndicesFlat.Append(InstancedIndices);
	}

	NumPerInstanceCustomDataPerObject.Empty(PerInstanceCustomData.Num());
	// We expect to have one or more entries per object
	PerInstanceCustomDataFlat.Empty(PerInstanceCustomData.Num());
	for (const TArray<float>& PerInstanceCustomDataArray : PerInstanceCustomData)
	{
		NumPerInstanceCustomDataPerObject.Add(PerInstanceCustomDataArray.Num());
		PerInstanceCustomDataFlat.Append(PerInstanceCustomDataArray);
	}
}

void
FHoudiniInstancedOutputPartData::BuildOriginalInstancedTransformsAndObjectArrays()
{
	{
		const int32 NumObjects = NumInstancedTransformsPerObject.Num();
		OriginalInstancedTransforms.SetNum(NumObjects);
		for (int32 n = 0; n < OriginalInstancedTransforms.Num(); n++)
			OriginalInstancedTransforms[n] = TArray<FTransform>();

		int32 ObjectIndexOffset = 0;
		for (int32 ObjIndex = 0; ObjIndex < NumObjects; ++ObjIndex)
		{
			TArray<FTransform>& Transforms = OriginalInstancedTransforms[ObjIndex];
			const int32 NumInstances = NumInstancedTransformsPerObject[ObjIndex];
			Transforms.Reserve(NumInstances);
			for (int32 Index = 0; Index < NumInstances; ++Index)
			{
				Transforms.Add(OriginalInstancedTransformsFlat[ObjectIndexOffset + Index]);
			}
			ObjectIndexOffset += NumInstances;
		}
		NumInstancedTransformsPerObject.Empty();
		OriginalInstancedTransformsFlat.Empty();
	}

	OriginalInstancedObjects.Empty(OriginalInstanceObjectPackagePaths.Num());
	for (const FString& PackageFullPath : OriginalInstanceObjectPackagePaths)
	{
		FString PackagePath;
		FString PackageName;
		const bool bDidSplit = PackageFullPath.Split(TEXT("."), &PackagePath, &PackageName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (!bDidSplit)
			PackagePath = PackageFullPath;
	
		UPackage* Package = FindPackage(nullptr, *PackagePath);
		if (!IsValid(Package))
		{
			// Editor might have picked up the package yet, try to load it
			Package = LoadPackage(nullptr, *PackagePath, LOAD_NoWarn);
		}
		if (IsValid(Package))
		{
			OriginalInstancedObjects.Add(FindObject<UObject>(Package, *PackageName));
		}
		else
		{
			OriginalInstancedObjects.Add(nullptr);
		}
	}
	OriginalInstanceObjectPackagePaths.Empty();

	{
		const int32 NumObjects = NumInstancedIndicesPerObject.Num();
		OriginalInstancedIndices.SetNumUninitialized(NumObjects);
		for (int32 n = 0; n < OriginalInstancedIndices.Num(); n++)
			OriginalInstancedIndices[n] = TArray<int32>();

		int32 ObjectIndexOffset = 0;
		for (int32 EntryIndex = 0; EntryIndex < NumObjects; ++EntryIndex)
		{
			TArray<int32>& InstancedIndices = OriginalInstancedIndices[EntryIndex];
			const int32 NumInstancedIndices = NumInstancedIndicesPerObject[EntryIndex];
			InstancedIndices.Reserve(NumInstancedIndices);
			for (int32 Index = 0; Index < NumInstancedIndices; ++Index)
			{
				InstancedIndices.Add(OriginalInstancedIndicesFlat[ObjectIndexOffset + Index]);
			}
			ObjectIndexOffset += NumInstancedIndices;
		}
		NumInstancedIndicesPerObject.Empty();
		OriginalInstancedIndicesFlat.Empty();
	}

	{
		const int32 NumObjects = NumPerInstanceCustomDataPerObject.Num();
		PerInstanceCustomData.SetNumUninitialized(NumObjects);
		for (int32 n = 0; n < PerInstanceCustomData.Num(); n++)
			PerInstanceCustomData[n] = TArray<float>();

		int32 ObjectIndexOffset = 0;
		for (int32 EntryIndex = 0; EntryIndex < NumObjects; ++EntryIndex)
		{
			TArray<float>& PerInstanceCustomDataArray = PerInstanceCustomData[EntryIndex];
			const int32 NumPerInstanceCustomData = NumPerInstanceCustomDataPerObject[EntryIndex];
			PerInstanceCustomDataArray.Reserve(NumPerInstanceCustomData);
			for (int32 Index = 0; Index < NumPerInstanceCustomData; ++Index)
			{
				PerInstanceCustomDataArray.Add(PerInstanceCustomDataFlat[ObjectIndexOffset + Index]);
			}
			ObjectIndexOffset += NumPerInstanceCustomData;
		}
		NumPerInstanceCustomDataPerObject.Empty();
		PerInstanceCustomDataFlat.Empty();
	}
}

bool
FHoudiniInstanceTranslator::GetPerInstanceCustomData(
	const int32& InGeoNodeId,
	const int32& InPartId,
	FHoudiniInstancedOutputPartData& OutInstancedOutputPartData)
{
	// Initialize sizes to zero
	OutInstancedOutputPartData.PerInstanceCustomData.SetNum(0);

	// First look for the number of custom floats
	// If we dont have the attribute, or it is set to zero, we dont have PerInstanceCustomData
	// HAPI_UNREAL_ATTRIB_INSTANCE_NUM_CUSTOM_FLOATS "unreal_num_custom_floats"	

	TArray<int32> CustomFloatsArray;

	FHoudiniHapiAccessor Accessor(InGeoNodeId, InPartId, HAPI_UNREAL_ATTRIB_INSTANCE_NUM_CUSTOM_FLOATS);
	bool bSuccess = Accessor.GetAttributeData(HAPI_ATTROWNER_INVALID, CustomFloatsArray);

	if (!bSuccess)
		return false;

	if (CustomFloatsArray.Num() <= 0)
		return false;

	int32 NumCustomFloats = 0;
	for (int32 CustomFloatCount : CustomFloatsArray)
	{
		NumCustomFloats = FMath::Max(NumCustomFloats, CustomFloatCount);
	}

	if (NumCustomFloats <= 0)
		return false;

	// We do have custom float, now read the per instance custom data
	// They are stored in attributes that uses the  "unreal_per_instance_custom" prefix
	// ie, unreal_per_instance_custom0, unreal_per_instance_custom1 etc...
	// We do not supprot tuples/arrays attributes for now.
	TArray<TArray<float>> AllCustomDataAttributeValues;
	AllCustomDataAttributeValues.SetNum(NumCustomFloats);

	// Read the custom data attributes
	int32 NumInstance = 0;
	for (int32 nIdx = 0; nIdx < NumCustomFloats; nIdx++)
	{
		// Build the custom data attribute
		FString CurrentAttr = TEXT(HAPI_UNREAL_ATTRIB_INSTANCE_CUSTOM_DATA_PREFIX) + FString::FromInt(nIdx);
		
		// TODO? Tuple values Array attributes?
		Accessor.Init(InGeoNodeId, InPartId, TCHAR_TO_ANSI(*CurrentAttr));
		bSuccess = Accessor.GetAttributeData(HAPI_ATTROWNER_INVALID, 1, AllCustomDataAttributeValues[nIdx]);

		// Retrieve the custom data values
		if (!bSuccess)
		{
			// Skip, we'll fill the values with zeros later on
			continue;
		}

		if (NumInstance < AllCustomDataAttributeValues[nIdx].Num())
			NumInstance = AllCustomDataAttributeValues[nIdx].Num();

		if (NumInstance != AllCustomDataAttributeValues[nIdx].Num())
		{
			HOUDINI_LOG_ERROR(TEXT("Instancer: Invalid number of Per-Instance Custom data attributes, ignoring..."));
			return false;
		}
	}

	// Check sizes
	if (AllCustomDataAttributeValues.Num() != NumCustomFloats)
	{
		HOUDINI_LOG_ERROR(TEXT("Instancer: Number of Per-Instance Custom data attributes don't match the number of custom floats, ignoring..."));
		return false;
	}

	OutInstancedOutputPartData.PerInstanceCustomData.SetNum(OutInstancedOutputPartData.OriginalInstancedObjects.Num());

	for (int32 ObjIdx = 0; ObjIdx < OutInstancedOutputPartData.OriginalInstancedObjects.Num(); ++ObjIdx)
	{
		OutInstancedOutputPartData.PerInstanceCustomData[ObjIdx].Reset();
	}

	for(int32 ObjIdx = 0; ObjIdx < OutInstancedOutputPartData.OriginalInstancedObjects.Num(); ++ObjIdx)
	{
		const TArray<int32>& InstanceIndices = OutInstancedOutputPartData.OriginalInstancedIndices[ObjIdx];
		
		if (InstanceIndices.Num() == 0)
		{
			continue;
		}

		// Perform some validation
		int32 NumCustomFloatsForInstance = CustomFloatsArray[InstanceIndices[0]];
		for (int32 InstIdx : InstanceIndices)
		{
			if (CustomFloatsArray[InstIdx] != NumCustomFloatsForInstance)
			{
				NumCustomFloatsForInstance = -1;
				break;
			}
		}

		if (NumCustomFloatsForInstance == -1)
		{
			continue;
		}

		// Now that we have read all the custom data values, we need to "interlace" them
		// in the final per-instance custom data array, fill missing values with zeroes
		TArray<float>& PerInstanceCustomData = OutInstancedOutputPartData.PerInstanceCustomData[ObjIdx];
		PerInstanceCustomData.Reserve(InstanceIndices.Num() * NumCustomFloatsForInstance);

		if(NumCustomFloatsForInstance == 0)
		{
			continue;
		}
		
		for (int32 InstIdx : InstanceIndices)
		{
			for (int32 nCustomIdx = 0; nCustomIdx < NumCustomFloatsForInstance; ++nCustomIdx)
			{
				float CustomData = (InstIdx < AllCustomDataAttributeValues[nCustomIdx].Num() ? AllCustomDataAttributeValues[nCustomIdx][InstIdx] : 0.0f);
				PerInstanceCustomData.Add(CustomData);
			}
		}
	}

	return true;
}


bool
FHoudiniInstanceTranslator::UpdateChangedPerInstanceCustomData(
	const TArray<float>& InPerInstanceCustomData,
	USceneComponent* InComponentToUpdate)
{
	// Checks
	UInstancedStaticMeshComponent* ISMC = Cast<UInstancedStaticMeshComponent>(InComponentToUpdate);
	if (!IsValid(ISMC))
		return false;

	// No Custom data to add/remove
	if (ISMC->NumCustomDataFloats == 0 && InPerInstanceCustomData.Num() == 0)
		return false;

	// We can copy the per instance custom data if we have any
	// TODO: Properly extract only needed values!
	int32 InstanceCount = ISMC->GetInstanceCount();
	int32 NumCustomFloats = InPerInstanceCustomData.Num() / InstanceCount;

	if (NumCustomFloats * InstanceCount != InPerInstanceCustomData.Num())
	{
		ISMC->NumCustomDataFloats = 0;
		ISMC->PerInstanceSMCustomData.Reset();
		return false;
	}

	ISMC->NumCustomDataFloats = NumCustomFloats;

	// Clear out and reinit to 0 the PerInstanceCustomData array
	ISMC->PerInstanceSMCustomData.SetNumZeroed(InstanceCount * NumCustomFloats);

	// Behaviour copied From UInstancedStaticMeshComponent::SetCustomData()
	// except we modify all the instance/custom values at once
	ISMC->Modify();

	// MemCopy
	const int32 NumToCopy = FMath::Min(ISMC->PerInstanceSMCustomData.Num(), InPerInstanceCustomData.Num());
	if (NumToCopy > 0)
	{
		FMemory::Memcpy(&ISMC->PerInstanceSMCustomData[0], InPerInstanceCustomData.GetData(), NumToCopy * InPerInstanceCustomData.GetTypeSize());
	}

	// Force recreation of the render data when proxy is created
	//NewISMC->InstanceUpdateCmdBuffer.Edit();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	// TODO:5.4 ?? fix me!!
#else
	// Cant call the edit function above because the function is defined in a different cpp file than the .h it is declared in...
	ISMC->InstanceUpdateCmdBuffer.NumEdits++;
#endif
	
	ISMC->MarkRenderStateDirty();
	
	return true;
}

#undef LOCTEXT_NAMESPACE