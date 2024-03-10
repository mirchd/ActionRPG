#pragma once

#include "HAL/Platform.h"

#if ENGINE_MINOR_VERSION > 22
	#include COMPILED_PLATFORM_HEADER(PlatformUtils.h)
#else
	#if PLATFORM_ANDROID
	#include "Android/AndroidPlatformUtils.h"	
	#elif PLATFORM_IOS	
	#include "IOS/IOSPlatformUtils.h"	
	#elif PLATFORM_WINDOWS	
	#include "Windows/WindowsPlatformUtils.h"	
	#elif PLATFORM_MAC	
	#include "Mac/MacPlatformUtils.h"	
	#endif 
#endif