// Copyright 2018-2025 - Roberto De Ioris


#include "LuauBlueprintFunctionLibrary.h"
#include "LuaState.h"

#if LUAMACHINE_LUAU
#undef check
#include "ThirdParty/luau/Luau/Frontend.h"
#define check(expr) UE_CHECK_IMPL(expr)
#include "ThirdParty/luau/Luau/BuiltinDefinitions.h"
#endif

bool ULuauBlueprintFunctionLibrary::LuauAnalyze(const FString& Code, const FString& ModuleName, const bool bLint, TArray<FLuauAnalysisResult>& Results)
{
#if LUAMACHINE_LUAU
	Luau::FrontendOptions FrontendOptions;
	FrontendOptions.runLintChecks = bLint;

	Luau::NullConfigResolver NullConfigResolver;

	struct LuaMachineLuauFileResolver : Luau::FileResolver
	{
		LuaMachineLuauFileResolver(const FString& InCode) : Code(InCode)
		{
		}

		std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& Name) override
		{
			return Luau::SourceCode{ TCHAR_TO_UTF8(*Code), Luau::SourceCode::Script };
		}

		const FString Code;
	};

	LuaMachineLuauFileResolver FileResolver(Code);

	Luau::Frontend Frontend(&FileResolver, &NullConfigResolver, FrontendOptions);
	Luau::registerBuiltinGlobals(Frontend, Frontend.globals);
	Luau::freeze(Frontend.globals.globalTypes);

	Frontend.parse(TCHAR_TO_UTF8(*ModuleName));
#undef check
	Luau::CheckResult CheckResult = Frontend.check(TCHAR_TO_UTF8(*ModuleName));
#define check(expr) UE_CHECK_IMPL(expr)

	for (const Luau::TypeError& LuaTypeError : CheckResult.errors)
	{
		FLuauAnalysisResult Result;
		Result.StartLine = LuaTypeError.location.begin.line;
		Result.StartColumn = LuaTypeError.location.begin.column;
		Result.EndLine = LuaTypeError.location.end.line;
		Result.EndColumn = LuaTypeError.location.end.column;
		Result.bLint = false;
		Result.bWarning = false;
		Result.LintCode = -1;
		Result.Message = UTF8_TO_TCHAR(Luau::toString(LuaTypeError, Luau::TypeErrorToStringOptions{ Frontend.fileResolver }).c_str());
		Results.Add(MoveTemp(Result));
	}
	
	if (bLint)
	{
		for (const Luau::LintWarning& LuaLintWarning : CheckResult.lintResult.errors)
		{
			FLuauAnalysisResult Result;
			Result.StartLine = LuaLintWarning.location.begin.line;
			Result.StartColumn = LuaLintWarning.location.begin.column;
			Result.EndLine = LuaLintWarning.location.end.line;
			Result.EndColumn = LuaLintWarning.location.end.column;
			Result.bLint = true;
			Result.bWarning = false;
			Result.LintCode = LuaLintWarning.code;
			Result.Message = UTF8_TO_TCHAR(LuaLintWarning.text.c_str());
			Results.Add(MoveTemp(Result));
		}
	
		for (const Luau::LintWarning& LuaLintWarning : CheckResult.lintResult.warnings)
		{
			FLuauAnalysisResult Result;
			Result.StartLine = LuaLintWarning.location.begin.line;
			Result.StartColumn = LuaLintWarning.location.begin.column;
			Result.EndLine = LuaLintWarning.location.end.line;
			Result.EndColumn = LuaLintWarning.location.end.column;
			Result.bLint = true;
			Result.bWarning = true;
			Result.LintCode = LuaLintWarning.code;
			Result.Message = UTF8_TO_TCHAR(LuaLintWarning.text.c_str());
			Results.Add(MoveTemp(Result));
		}
	}

	return Results.Num() == 0;
#else
	UE_LOG(LogLuaMachine, Error, TEXT("The current Lua VM is not Luau, Analyzer cannot be used"));
	return false;
#endif
}

bool ULuauBlueprintFunctionLibrary::LuauAnalyzeLuaCode(ULuaCode* LuaCode, const FString& ModuleName, const bool bLint, TArray<FLuauAnalysisResult>& Results)
{
	if (!LuaCode)
	{
		return false;
	}

	return LuauAnalyze(LuaCode->Code.ToString(), ModuleName, bLint, Results);
}