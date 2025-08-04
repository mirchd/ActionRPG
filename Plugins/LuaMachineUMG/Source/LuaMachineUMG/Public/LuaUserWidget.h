// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LuaUserDataInterface.h"
#include "LuaState.h"
#include "LuaUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUAMACHINEUMG_API ULuaUserWidget : public UUserWidget, public ILuaUserDataInterface
{
	GENERATED_BODY()

public:
	FLuaValue LuaMetaMethodIndex_Implementation(const FString& Key) override;

	FLuaValue LuaMetaMethodToString_Implementation() override;

	UPROPERTY()
	ULuaState* OwningLuaState;

	UPROPERTY()
	TSet<class ULuaProxyWidget*> Proxies;
	
};
