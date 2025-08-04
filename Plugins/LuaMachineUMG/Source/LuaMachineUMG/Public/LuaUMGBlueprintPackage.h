// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LuaBlueprintPackage.h"
#include "LuaUMGBlueprintPackage.generated.h"

/**
 * 
 */
UCLASS()
class LUAMACHINEUMG_API ULuaUMGBlueprintPackage : public ULuaBlueprintPackage
{
	GENERATED_BODY()

public:
	ULuaUMGBlueprintPackage();

	UFUNCTION()
	FLuaValue CreateUserWidget();

	UFUNCTION()
	FLuaValue LoadTextureAsBrush(FLuaValue TexturePath);
	
};
