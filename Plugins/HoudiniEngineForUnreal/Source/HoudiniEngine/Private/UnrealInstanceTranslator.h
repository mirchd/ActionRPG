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

#pragma once

#include "HAPI/HAPI_Common.h"

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

class UInstancedStaticMeshComponent;
class FUnrealObjectInputHandle;

struct HOUDINIENGINE_API FUnrealInstanceTranslator
{
	public:

		// HAPI : Marshaling, extract geometry and create input asset for it - return true on success
		static bool HapiCreateInputNodeForInstancer(
			UInstancedStaticMeshComponent* ISMC,
			const FString& InNodeName,
			HAPI_NodeId& OutCreatedNodeId,
			FUnrealObjectInputHandle& OutHandle,
			const bool& bExportLODs,
			const bool& bExportSockets,
			const bool& bExportColliders,
			const bool& bExportAsAttributeInstancer,
			bool bPreferNaniteFallbackMesh,
			bool bExportMaterialParameters,
			const bool& bInputNodesCanBeDeleted);
};