// Copyright 2025 - Roberto De Ioris

#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/LuaUnitTestState.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineBinaryTest_Simple, "LuaMachine.UnitTests.Binary.Simple", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineBinaryTest_Simple::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Inactive, false);

	ULuaUnitTestState* UnitTestState = ULuaState::CreateDynamicLuaState<ULuaUnitTestState>(TestWorld);

	UnitTestState->SetLuaValueFromGlobalName("test", FLuaValue(TArray<uint8>({ 100, 200, 201 })));

	FLuaValue LuaValue = UnitTestState->RunString("return test", "");

	TestTrue(TEXT("LuaValue.ToBytes() == {100, 200, 201}"), LuaValue.ToBytes() == TArray<uint8>({ 100, 200, 201 }));

	return true;
}

#if LUAMACHINE_LUAU
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineBinaryTest_Buffer, "LuaMachine.UnitTests.Binary.Buffer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineBinaryTest_Buffer::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Inactive, false);

	ULuaUnitTestState* UnitTestState = ULuaState::CreateDynamicLuaState<ULuaUnitTestState>(TestWorld);

	UnitTestState->SetLuaValueFromGlobalName("test", FLuaValue(TArray<uint8>({ 0x22, 0x33, 0x00, 0x55 })));

	FLuaValue LuaValue = UnitTestState->RunString("return buffer.readu32(buffer.fromstring(test), 0)", "");

	TestTrue(TEXT("LuaValue == 0x55003322"), LuaValue.ToInteger() == 0x55003322);

	return true;
}
#endif

#if LUAMACHINE_LUAJIT
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineBinaryTest_StringBuffer, "LuaMachine.UnitTests.Binary.StringBuffer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineBinaryTest_StringBuffer::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Inactive, false);

	ULuaUnitTestState* UnitTestState = ULuaState::CreateDynamicLuaState<ULuaUnitTestState>(TestWorld);

	UnitTestState->SetLuaValueFromGlobalName("test", FLuaValue(TArray<uint8>({ 0x00, 0x33, 0x55, 0x22 })));

	FLuaValue LuaValue = UnitTestState->RunString("local buffer = require(\"string.buffer\"); b = buffer.new(); b:put(test); b:skip(1); return b:get(2);", "");

	TestTrue(TEXT("LuaValue == 0x33"), LuaValue.ToBytes().Num() == 2 && LuaValue.ToBytes()[0] == 0x33 && LuaValue.ToBytes()[1] == 0x55);

	return true;
}
#endif

#endif