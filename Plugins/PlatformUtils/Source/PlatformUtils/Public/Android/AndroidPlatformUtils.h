#pragma once

#include "GenericPlatformUtils.h"

// engine header
#include "CoreMinimal.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"

class PLATFORMUTILS_API FJavaHelperEx
{
public:
	// Converts the java string to FString and calls DeleteLocalRef on the passed-in java string reference
	static FString FStringFromLocalRef(JNIEnv* Env, jstring JavaString);

	// Converts the java string to FString and calls DeleteGlobalRef on the passed-in java string reference
	static FString FStringFromGlobalRef(JNIEnv* Env, jstring JavaString);

	// Converts the java string to FString, does NOT modify the passed-in java string reference
	static FString FStringFromParam(JNIEnv* Env, jstring JavaString);
};

struct PLATFORMUTILS_API FAndroidPlatformUtils:public FGenericPlatformUtils
{
public:
	static void Init();
	static void Shutdown();

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

	// JNI Methods
	static jmethodID HasInternetConnectionMethod;
	static jmethodID GetDeviceIdMethod;
	static jmethodID GetAndroidDeviceIdMethod;
	static jmethodID GetMacAddressMethod;
	static jmethodID GetFakeDeviceIDMethod;
};

typedef FAndroidPlatformUtils FPlatformUtilsMisc;