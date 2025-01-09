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

#include "HoudiniEditorColliderTests.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "HoudiniEditorTestUtils.h"

#include "Misc/AutomationTest.h"

FString FHoudiniEditorColliderTests::EquivalenceTestMapName = TEXT("Colliders");
FString FHoudiniEditorColliderTests::TestHDAPath = TEXT("/Game/TestHDAs/Colliders/");

IMPLEMENT_SIMPLE_HOUDINI_AUTOMATION_TEST(HoudiniEditorColliderTest_Colliders_Common, "Houdini.Editor.Colliders.Colliders_Common", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool HoudiniEditorColliderTest_Colliders_Common::RunTest(const FString & Parameters)
{
	FHoudiniEditorTestUtils::InitializeTests(this, [this]
	{
		const FString MapName = FHoudiniEditorColliderTests::EquivalenceTestMapName;
		const FString ActorName = TEXT("Colliders_Common");
		const FString HDAAssetPath = FHoudiniEditorColliderTests::TestHDAPath + TEXT("Colliders_Common");
		const FHoudiniActorTestSettings Settings = {};

		FHoudiniEditorTestUtils::RunOrSetupDifferentialTest(this, MapName, HDAAssetPath, ActorName, Settings, nullptr);
	});

	return true;
}


#endif

