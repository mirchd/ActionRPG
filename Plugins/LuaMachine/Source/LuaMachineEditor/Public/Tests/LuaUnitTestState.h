// Copyright 2025 - Roberto De Ioris

#pragma once

#include "CoreMinimal.h"
#include "LuaState.h"
#include "LuaUnitTestState.generated.h"

/**
 *
 */
UCLASS()
class ULuaUnitTestState : public ULuaState
{
	GENERATED_BODY()
public:

	ULuaUnitTestState()
	{
		MaxMemoryUsage = 8192;
		bLogError = true;
		StepCount = 0;

		Table.Add("lambda001", FLuaValue::NewLambda([](TArray<FLuaValue> Args) { return FLuaValue("Hello Test"); }));
		Table.Add("lambda002", FLuaValue::NewLambda([this](TArray<FLuaValue> Args) { return Table["lambda001"]; }));
		Table.Add("lambda003", FLuaValue::NewLambda([](TArray<FLuaValue> Args) { return FString("!!!ERROR!!!"); }));
		Table.Add("dummy", FLuaValue::Function(GET_FUNCTION_NAME_CHECKED(ULuaUnitTestState, DummyFunction)));
	}

	void ReceiveLuaSingleStepHook_Implementation(const FLuaDebug& LuaDebug) override
	{
		StepCount++;
	}

	int32 StepCount;

	UFUNCTION()
	FLuaValue DummyFunction();
};
