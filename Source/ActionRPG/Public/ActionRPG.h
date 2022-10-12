// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 这是项目中每个CPP文件都包含的第一个模块标头。这里包含大多数类所需的共享引擎标头以及全局定义。

// ----------------------------------------------------------------------------------------------------------------
// This header is included by all headers in the project so it's a good place to declare common includes
// We include EngineMinimal and the subset of engine headers used by most of our classes
// We don't want to include "Engine.h" as that pulls in too many classes we don't need and slows compile time
// ----------------------------------------------------------------------------------------------------------------

#include "EngineMinimal.h"
#include "Engine/Engine.h"
//#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "RPGTypes.h"

ACTIONRPG_API DECLARE_LOG_CATEGORY_EXTERN(LogActionRPG, Log, All);

