#pragma once

#include "IOS/IOSPlatformUtils.h"

#import <Reachability/Reachability.h>
#import <SSKeychain/SSKeychain.h>

void FIOSPlatformUtils::Init()
{

}
void FIOSPlatformUtils::Shutdown()
{

}

bool FIOSPlatformUtils::HasInternetConnection()
{
	Reachability *reachability = [Reachability reachabilityForInternetConnection];
	NetworkStatus networkStatus = [reachability currentReachabilityStatus];
	return networkStatus != NotReachable;
}


FString FIOSPlatformUtils::GetPersistentUniqueDeviceId()
{
	NSString *AppName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString*)kCFBundleNameKey];
	NSString *PersistentUUID = [SSKeychain passwordForService : AppName account : @"incoding"];

	if (PersistentUUID == nil)
	{
		PersistentUUID = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
		[SSKeychain setPassword : PersistentUUID forService : AppName account : @"incoding"];
	}

	FString resultId(PersistentUUID);

	while (true)
	{
		int32 foundPos=-1;
		if (resultId.FindChar('-', foundPos))
		{
			resultId.RemoveAt(foundPos);
		}
		else
		{
			break;
		}
	}
	return resultId;
}

FString FIOSPlatformUtils::GetDeviceId()
{
	return FPlatformUtilsMisc::GetPersistentUniqueDeviceId();
}
