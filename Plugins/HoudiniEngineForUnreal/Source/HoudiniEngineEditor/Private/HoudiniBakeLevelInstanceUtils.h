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
#include "HoudiniAssetComponent.h"
#include "HoudiniEngineBakeUtils.h"
#include "HoudiniEngineOutputStats.h"
#include "HoudiniOutput.h"

class UHoudiniOutput;
class ILevelInstanceInterface;
class AActor;
struct FHoudiniBakedOutput;
struct FHoudiniLevelInstanceParams;
struct FHoudiniEngineBakedActor;

class FHoudiniBakeLevelInstanceUtils
{
public:

	static bool CreateLevelInstances(
		UHoudiniAssetComponent * HAC, 
		const TArray<FHoudiniEngineBakedActor> & BakedActors,
		const FString& BakeFolder,
		FHoudiniBakedObjectData& BakedObjectData);

protected:
	static ILevelInstanceInterface* CreateLevelInstance(
		const FHoudiniLevelInstanceParams& Params, 
		TArray<AActor*>& Actors,
		const FString & BakeFolder,
		FHoudiniBakedObjectData& BakedObjectData);

	static const UHoudiniOutput* GetHoudiniObject(const FHoudiniOutputObjectIdentifier Id, const TArray<UHoudiniOutput*>& CookedOutputs);

};
