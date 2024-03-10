#pragma once


#include "Android/AndroidPlatformUtils.h"
#include "Android/AndroidPlatform.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidJavaEnv.h"
// engine header
#include "Kismet/KismetSystemLibrary.h"

jmethodID FAndroidPlatformUtils::HasInternetConnectionMethod;
jmethodID FAndroidPlatformUtils::GetDeviceIdMethod;
jmethodID FAndroidPlatformUtils::GetAndroidDeviceIdMethod;
jmethodID FAndroidPlatformUtils::GetMacAddressMethod;
jmethodID FAndroidPlatformUtils::GetFakeDeviceIDMethod;

void FAndroidPlatformUtils::Init()
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		HasInternetConnectionMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_HasInternetConnected", "()Z", false);
		GetDeviceIdMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetDeviceId", "()Ljava/lang/String;", false);
		GetAndroidDeviceIdMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetAndroidDeviceId", "()Ljava/lang/String;", false);
		GetMacAddressMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetMacAddress", "()Ljava/lang/String;", false);
		GetFakeDeviceIDMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetFakeDeviceID", "()Ljava/lang/String;", false);
	}
}
void FAndroidPlatformUtils::Shutdown()
{

}

bool FAndroidPlatformUtils::HasInternetConnection()
{
	bool bResult = false;
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		bResult = FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, FAndroidPlatformUtils::HasInternetConnectionMethod);
	}
	return bResult;
}
FString FAndroidPlatformUtils::GetPersistentUniqueDeviceId()
{
	return FPlatformUtilsMisc::GetDeviceId();
}

FString FAndroidPlatformUtils::GetDeviceId()
{
	FString ResultDeviceId = FString("");
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		ResultDeviceId = FJavaHelperEx::FStringFromLocalRef(Env, (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, FAndroidPlatformUtils::GetDeviceIdMethod));
	}
	return ResultDeviceId;
}


FString FJavaHelperEx::FStringFromLocalRef(JNIEnv* Env, jstring JavaString)
{
	FString ReturnString = FStringFromParam(Env, JavaString);

	if (Env && JavaString)
	{
		Env->DeleteLocalRef(JavaString);
	}

	return ReturnString;
}

FString FJavaHelperEx::FStringFromGlobalRef(JNIEnv* Env, jstring JavaString)
{
	FString ReturnString = FStringFromParam(Env, JavaString);

	if (Env && JavaString)
	{
		Env->DeleteGlobalRef(JavaString);
	}

	return ReturnString;
}

FString FJavaHelperEx::FStringFromParam(JNIEnv* Env, jstring JavaString)
{
	if (!Env || !JavaString || Env->IsSameObject(JavaString, NULL))
	{
		return {};
	}

	const auto chars = Env->GetStringUTFChars(JavaString, 0);
	FString ReturnString(UTF8_TO_TCHAR(chars));
	Env->ReleaseStringUTFChars(JavaString, chars);
	return ReturnString;
}
