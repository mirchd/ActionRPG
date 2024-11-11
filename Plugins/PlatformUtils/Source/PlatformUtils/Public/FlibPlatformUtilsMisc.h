// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// engine header
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlibPlatformUtilsMisc.generated.h"

/**
 * 
 */
UCLASS()
class PLATFORMUTILS_API UFlibPlatformUtilsMisc : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	* Return device network connected status
	*
	* @return - has network?
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category="PlatformUtils")
		static bool HasInternetConnection();
	/**
	* Return persistent Unique Device ID without reset after app reinstall
	*
	* @return - Unique Device ID
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "PlatformUtils")
		static FString GetPersistentUniqueDeviceId();

	/**
	* Return Device ID. Should be unique but not guaranteed.
	*
	* @return - Device ID
	*/
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "PlatformUtils")
		static FString GetDeviceId();
};
