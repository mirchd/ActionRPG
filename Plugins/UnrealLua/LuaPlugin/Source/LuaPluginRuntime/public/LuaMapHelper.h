// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/UnrealType.h"
#include "LuaDelegateMulti.h"

class LUAPLUGINRUNTIME_API ULuaMapHelper
{
public:
	ULuaMapHelper();
	void* Obj;

	FMapProperty* Property;
		
	void Init(void* _Obj, FMapProperty* _Property);
	void Init_ValuePtr(void* _Obj, FMapProperty* _Property);

	template<class K, class V>
	TMap<K, V>* ValuePtr()
	{
		return (TMap<K, V>*)(Obj);
	}

	static int32 __index(lua_State* inL);
	static int32 __newindex(lua_State* inL);

	static ULuaMapHelper* GetHelper(UObject* _Obj, const FName& PropertyName);

	static ULuaMapHelper* GetHelperCPP(void* _Obj, FMapProperty* Property);
	static ULuaMapHelper* GetHelperCPP_ValuePtr(void* _Obj, FMapProperty* Property);

	static void Copy(FScriptMapHelper& SrcMapHelper, FScriptMapHelper& DestMapHelper, FMapProperty* p);
	void CopyTo(FMapProperty* p, void* ptr);
	void CopyFrom(FMapProperty* p, void* ptr);
	static void GlueMapCopyTo(FMapProperty* p, const void* src, const void* dest);

	int32 Num();

	int32 Get(lua_State* inL);

	void Set(lua_State* inL);

	void Add(lua_State* inL);

	void Reset();

	int32 Table(lua_State* inL);

	static int32 __pairs(lua_State* inL);
	static int32 travel(lua_State* inL);
};
