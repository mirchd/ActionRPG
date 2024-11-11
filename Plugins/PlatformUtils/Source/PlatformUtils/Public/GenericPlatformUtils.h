#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformApplicationMisc.h"

struct PLATFORMUTILS_API FGenericPlatformUtils
{
public:
	static void Init() {}
	static void Shutdown() {}
	/**
	* Return device network connected status
	*
	* @return - has network?
	*/
	static bool HasInternetConnection() { return true; };
	/**
	* Return persistent Unique Device ID without reset after app reinstall
	*
	* @return - Unique Device ID
	*/
	static FString GetPersistentUniqueDeviceId(){ return TEXT(""); }

	/**
	* Return Device ID. Should be unique but not guaranteed.
	*
	* @return - Device ID
	*/
	static FString GetDeviceId() { return TEXT(""); }
};