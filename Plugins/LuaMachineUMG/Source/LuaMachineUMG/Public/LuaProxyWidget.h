// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LuaUserDataInterface.h"
#include "Components/Widget.h"
#include "LuaProxyWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUAMACHINEUMG_API ULuaProxyWidget : public UObject, public ILuaUserDataInterface
{
	GENERATED_BODY()

public:
	FLuaValue LuaMetaMethodIndex_Implementation(const FString& Key) override;

	bool LuaMetaMethodNewIndex_Implementation(const FString& Key, FLuaValue Value) override;

	FLuaValue LuaMetaMethodToString_Implementation() override;

	UPROPERTY()
	UWidget* Widget;

	UPROPERTY()
	TSet<class ULuaProxySlot*> Proxies;

	class ULuaState* GetLuaState();
	
};
