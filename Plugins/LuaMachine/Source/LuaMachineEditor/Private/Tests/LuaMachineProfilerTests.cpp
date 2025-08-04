// Copyright 2025 - Roberto De Ioris

#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/LuaUnitTestState.h"
#include "Misc/AutomationTest.h"

#if LUAMACHINE_LUAU
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineProfilerTest_Simple, "LuaMachine.UnitTests.Profiler.Simple", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineProfilerTest_Simple::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Inactive, false);

	ULuaUnitTestState* UnitTestState = ULuaState::CreateDynamicLuaState<ULuaUnitTestState>(TestWorld);

	UnitTestState->StartProfiler();

	FLuaValue LuaValue = UnitTestState->RunString("function one() return 100 end\nfunction two() one() end\nfunction three() two() end\nthree()\ntwo()\none()", "");

	TMap<FLuaProfiledStack, FLuaProfiledData> Profiled = UnitTestState->StopProfiler();

	TestTrue(TEXT("Profiled.Num() == 7"), Profiled.Num() == 7);

	return true;
}

#endif

#endif