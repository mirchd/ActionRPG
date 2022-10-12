// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 药剂（Potions）：是一种一次性消耗品，用于治疗或补充更多法力。

#include "Items/RPGItem.h"
#include "RPGPotionItem.generated.h"

/** Native base class for potions, should be blueprinted */
UCLASS()
class ACTIONRPG_API URPGPotionItem : public URPGItem
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGPotionItem()
	{
		ItemType = URPGAssetManager::PotionItemType;
	}
};