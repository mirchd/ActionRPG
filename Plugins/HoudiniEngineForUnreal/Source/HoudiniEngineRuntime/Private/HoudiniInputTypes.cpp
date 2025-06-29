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


#include "HoudiniInputTypes.h"
#include "HoudiniEngineRuntimeCommon.h"
#include "HoudiniInput.h"
#include "HoudiniRuntimeSettings.h"


FHoudiniInputObjectSettings::FHoudiniInputObjectSettings()
	: KeepWorldTransform(EHoudiniXformType::Auto)
	, bImportAsReference(false)
	, bImportAsReferenceRotScaleEnabled(true)
	, bImportAsReferenceBboxEnabled(true)
	, bImportAsReferenceMaterialEnabled(true)
	, bExportMainGeometry(true)
	, bExportLODs(false)
	, bExportSockets(false)
	, bPreferNaniteFallbackMesh(false)
	, bExportColliders(false)
	, bExportMaterialParameters(false)
	, bAddRotAndScaleAttributesOnCurves(false)
	, bUseLegacyInputCurves(false)
	, UnrealSplineResolution(0.0f)
	, LandscapeExportType(EHoudiniLandscapeExportType::Heightfield)
	, bLandscapeExportSelectionOnly(false)
	, bLandscapeAutoSelectComponent(false)
	, bLandscapeExportMaterials(false)
	, bLandscapeExportLighting(false)
	, bLandscapeExportNormalizedUVs(false)
	, bLandscapeExportTileUVs(false)
	, bLandscapeAutoSelectSplines(false)
	, bLandscapeSplinesExportControlPoints(false)
	, bLandscapeSplinesExportLeftRightCurves(false)
	, bLandscapeSplinesExportSplineMeshComponents(false)
	, bMergeSplineMeshComponents(true)
	, bExportHeightDataPerEditLayer(true)
	, bExportPaintLayersPerEditLayer(false)
	, bExportMergedPaintLayers(true)
	, bExportLevelInstanceContent(true)
{
	UHoudiniRuntimeSettings const* const HoudiniRuntimeSettings = GetDefault<UHoudiniRuntimeSettings>();
	if (IsValid(HoudiniRuntimeSettings))
	{
		UnrealSplineResolution = HoudiniRuntimeSettings->MarshallingSplineResolution;
		bAddRotAndScaleAttributesOnCurves = HoudiniRuntimeSettings->bAddRotAndScaleAttributesOnCurves;
		bUseLegacyInputCurves = HoudiniRuntimeSettings->bUseLegacyInputCurves;
		bPreferNaniteFallbackMesh = HoudiniRuntimeSettings->bPreferNaniteFallbackMesh;
	}
}

FHoudiniInputObjectSettings::FHoudiniInputObjectSettings(UHoudiniInput const* InInput)
	: FHoudiniInputObjectSettings()
{
	InInput->CopyInputSettingsTo(*this);
}
