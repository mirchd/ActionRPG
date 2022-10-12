// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/GameStateBase.h"
#include "RPGGameStateBase.generated.h"

// 游戏模式（Game Mode）和状态子类。对于ARPG，只有存根代码，因为大多数贴图特定的Gameplay逻辑是使用蓝图，但许多游戏使用C++代码制作各种模式和状态。

/** Base class for GameMode, should be blueprinted */
UCLASS()
class ACTIONRPG_API ARPGGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ARPGGameStateBase() {}
};

