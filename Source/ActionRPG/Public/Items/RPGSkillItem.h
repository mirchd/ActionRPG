// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 技能（Skills）：是指特殊的攻击（例如火球），玩家可以装备和使用，可造成直接和区域作用伤害。

#include "Items/RPGItem.h"
#include "RPGSkillItem.generated.h"

/** Native base class for skills, should be blueprinted */
UCLASS()
class ACTIONRPG_API URPGSkillItem : public URPGItem
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGSkillItem()
	{
		ItemType = URPGAssetManager::SkillItemType;
	}
};