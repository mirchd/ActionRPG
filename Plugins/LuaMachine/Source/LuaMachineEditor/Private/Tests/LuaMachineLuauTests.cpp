// Copyright 2025 - Roberto De Ioris

#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/LuaUnitTestState.h"
#include "LuauBlueprintFunctionLibrary.h"
#include "Misc/AutomationTest.h"

#if LUAMACHINE_LUAU

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_AnalyzeSimple, "LuaMachine.UnitTests.Luau.AnalyzeSimple", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_AnalyzeSimple::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("local x : number = 1; x = \"test\"", "Test", false, Results);

	TestTrue(TEXT("bCodeCheck == false"), !bCodeCheck);
	TestTrue(TEXT("Results.Num() == 1"), Results.Num() == 1);
	TestTrue(TEXT("Results[0] == \"Type 'string' could not be converted into 'number'\""), Results[0].Message == "Type 'string' could not be converted into 'number'");

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_AnalyzeCorrect, "LuaMachine.UnitTests.Luau.AnalyzeCorrect", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_AnalyzeCorrect::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("local x : number = 1; x = 2", "Test", false, Results);

	TestTrue(TEXT("bCodeCheck == true"), bCodeCheck);
	TestTrue(TEXT("Results.Num() == 1"), Results.Num() == 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_LintUnusedLocal, "LuaMachine.UnitTests.Luau.LintUnusedLocal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_LintUnusedLocal::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("local x = 1", "Test", true, Results);

	TestTrue(TEXT("bCodeCheck == false"), !bCodeCheck);
	TestTrue(TEXT("Results.Num() == 1"), Results.Num() == 1);
	TestTrue(TEXT("Results[0].bLint == true"), Results[0].bLint);
	TestTrue(TEXT("Results[0].bWarning == true"), Results[0].bWarning);
	TestTrue(TEXT("Results[0].LintCode == 7"), Results[0].LintCode == 7);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_AnalyzeGlobalWithLint, "LuaMachine.UnitTests.Luau.AnalyzeGlobalWithLint", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_AnalyzeGlobalWithLint::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("x = 1; x = 2", "Test", true, Results);

	TestTrue(TEXT("bCodeCheck == true"), bCodeCheck);
	TestTrue(TEXT("Results.Num() == 1"), Results.Num() == 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_AnalyzeErrorWithLint, "LuaMachine.UnitTests.Luau.AnalyzeErrorWithLint", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_AnalyzeErrorWithLint::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("callme()", "Test", true, Results);

	TestTrue(TEXT("bCodeCheck == false"), !bCodeCheck);
	TestTrue(TEXT("Results.Num() == 1"), Results.Num() == 1);
	TestTrue(TEXT("Results[0].bLint == false"), !Results[0].bLint);
	TestTrue(TEXT("Results[0].Message ~= \"callme\""), Results[0].Message.Contains("callme"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLuaMachineLuauTest_AnalyzeEmpty, "LuaMachine.UnitTests.Luau.AnalyzeEmpty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLuaMachineLuauTest_AnalyzeEmpty::RunTest(const FString& Parameters)
{
	TArray<FLuauAnalysisResult> Results;
	const bool bCodeCheck = ULuauBlueprintFunctionLibrary::LuauAnalyze("", "Test", true, Results);

	TestTrue(TEXT("bCodeCheck == true"), bCodeCheck);

	return true;
}

#endif

#endif