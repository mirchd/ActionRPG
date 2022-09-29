#pragma once

#include "ActionRPG.h"
#include "RPGBlueprintLibrary.generated.h"

/**
 * Game-specific blueprint library
 * Most games will need to implement one or more blueprint function libraries to expose their native code to blueprints
 */
UCLASS()
class URPGBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Show the native loading screen, such as on a map transfer. If bPlayUntilStopped is false, it will be displayed for PlayTime and automatically stop */
	UFUNCTION(BlueprintCallable, Category = Loading)
	static void PlayLoadingScreen(bool bPlayUntilStopped, float PlayTime);

	/** Turns off the native loading screen if it is visible. This must be called if bPlayUntilStopped was true */
	UFUNCTION(BlueprintCallable, Category = Loading)
	static void StopLoadingScreen();

	/** Returns true if this is being run from an editor preview */
	UFUNCTION(BlueprintCallable, Category = Loading)
	static bool IsInEditor();

	//Returns the project version set in the 'Project Settings' > 'Description' section of the editor
	UFUNCTION(BlueprintPure, Category = "Project")
	static FString GetProjectVersion();
};
