// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibPlatformUtilsMisc.h"
#include "PlatformUtilsMisc.h"

bool UFlibPlatformUtilsMisc::HasInternetConnection()
{
	return FPlatformUtilsMisc::HasInternetConnection();
}

FString UFlibPlatformUtilsMisc::GetPersistentUniqueDeviceId()
{
	return FPlatformUtilsMisc::GetPersistentUniqueDeviceId();
}

FString UFlibPlatformUtilsMisc::GetDeviceId()
{
	return FPlatformUtilsMisc::GetDeviceId();
}
