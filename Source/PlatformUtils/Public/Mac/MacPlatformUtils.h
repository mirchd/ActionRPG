#pragma once

#include "GenericPlatformUtils.h"
#include "CoreMinimal.h"

struct PLATFORMUTILS_API FMacPlatformUtils:public FGenericPlatformUtils
{
public:
	static void Init() {}
	static void Shutdown() {}

	/**
	* Return device network connected status
	*
	* @return - has network?
	*/
	static bool HasInternetConnection();

	/**
	* Return persistent Unique Device ID without reset after app reinstall
	*
	* @return - Unique Device ID
	*/
	static FString GetPersistentUniqueDeviceId();

	/**
	* Return Device ID. Should be unique but not guaranteed.
	*
	* @return - Device ID
	*/
	static FString GetDeviceId();
};

typedef FMacPlatformUtils FPlatformUtilsMisc;