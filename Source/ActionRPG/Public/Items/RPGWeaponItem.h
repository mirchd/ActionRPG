// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 武器（Weapons）：是一种玩家可以装备的蓝图，可造成近战伤害。

#include "Items/RPGItem.h"
#include "RPGWeaponItem.generated.h"

/** Native base class for weapons, should be blueprinted */
UCLASS()
class ACTIONRPG_API URPGWeaponItem : public URPGItem
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGWeaponItem()
	{
		ItemType = URPGAssetManager::WeaponItemType;
	}

	/** Weapon actor to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AActor> WeaponActor;
};