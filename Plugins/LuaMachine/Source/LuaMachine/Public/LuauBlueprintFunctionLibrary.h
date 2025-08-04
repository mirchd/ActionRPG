// Copyright 2018-2025 - Roberto De Ioris

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LuaCode.h"
#include "LuauBlueprintFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FLuauAnalysisResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	int32 StartLine = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	int32 StartColumn = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	int32 EndLine = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	int32 EndColumn = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	bool bLint = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	bool bWarning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	int32 LintCode = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Luau")
	FString Message;
};

/**
 * 
 */
UCLASS()
class LUAMACHINE_API ULuauBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Luau")
	static bool LuauAnalyze(const FString& Code, const FString& ModuleName, const bool bLint, TArray<FLuauAnalysisResult>& Results);
	
	UFUNCTION(BlueprintCallable, Category="Luau")
	static bool LuauAnalyzeLuaCode(ULuaCode* LuaCode, const FString& ModuleName, const bool bLint, TArray<FLuauAnalysisResult>& Results);
};
