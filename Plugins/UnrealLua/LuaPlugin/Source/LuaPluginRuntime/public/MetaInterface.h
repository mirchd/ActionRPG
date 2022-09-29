#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"
#include "UObject/ScriptMacros.h"
#include "LuaApi.h"
#include "TableUtil.h"
#include "LuaDelegateSingle.h"
#include "LuaArrayHelper.h"
#include "Runtime/Launch/Resources/Version.h"

struct LuaBaseBpInterface
{
	virtual ~LuaBaseBpInterface() {};
};

struct LuaBasePropertyInterface:public LuaBaseBpInterface
{
	virtual FProperty* GetProperty() = 0;

	virtual void push(lua_State* inL, const void* ValuePtr) = 0;
	virtual void push_ret(lua_State* inL, const void* ValuePtr) = 0;
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) = 0;
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) = 0;
	
	virtual void push_container(lua_State* inL, const void* ContainerPtr) = 0;
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) = 0;
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) = 0;
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) = 0;

};

static TSharedPtr<LuaBasePropertyInterface> CreatePropertyInterface(lua_State*inL, FProperty* Property);

struct LuaUBoolProperty :public LuaBasePropertyInterface
{
	FBoolProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUBoolProperty(){}
	LuaUBoolProperty(lua_State*inL, FBoolProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		bool value = popiml<bool>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushboolean(inL, (bool)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		bool value = popiml<bool>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUIntProperty :public LuaBasePropertyInterface
{
	FIntProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUIntProperty(){}
	LuaUIntProperty(lua_State*inL, FIntProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		int32 value = popiml<int>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		int32 value = popiml<int>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUInt8Property :public LuaBasePropertyInterface
{
	FInt8Property* Property;
	//Some hook
	virtual FProperty* GetProperty() { return Property; }
	virtual ~LuaUInt8Property() {}
	LuaUInt8Property(lua_State*inL, FInt8Property* InProperty) :Property(InProperty)
	{

	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr)
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr)
	{
		int8 value = popiml<int8>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr)
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int8)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		int8 value = popiml<int8>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUUInt16Property :public LuaBasePropertyInterface
{
	FUInt16Property* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUUInt16Property(){}
	LuaUUInt16Property(lua_State*inL, FUInt16Property* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		uint16 value = popiml<uint16>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		uint16 value = popiml<uint16>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUInt16Property :public LuaBasePropertyInterface
{
	FInt16Property* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUInt16Property(){}
	LuaUInt16Property(lua_State*inL, FInt16Property* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		int16 value = popiml<int16>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int16)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		int16 value = popiml<int16>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUUInt32Property :public LuaBasePropertyInterface
{
	FUInt32Property* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUUInt32Property(){}
	LuaUUInt32Property(lua_State*inL, FUInt32Property* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		uint32 value = popiml<uint32>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		uint32 value = popiml<uint32>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUInt64Property :public LuaBasePropertyInterface
{
	FInt64Property* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUInt64Property(){}
	LuaUInt64Property(lua_State*inL, FInt64Property* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		int64 value = popiml<int64>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		int64 value = popiml<int64>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUUInt64Property :public LuaBasePropertyInterface
{
	FUInt64Property* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUUInt64Property(){}
	LuaUUInt64Property(lua_State*inL, FUInt64Property* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		uint64 value = popiml<uint64>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (uint64)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		uint64 value = popiml<uint64>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUFloatProperty :public LuaBasePropertyInterface
{
	FFloatProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUFloatProperty(){}
	LuaUFloatProperty(lua_State*inL, FFloatProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		float value = popiml<float>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (float)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		float value = popiml<float>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUDoubleProperty :public LuaBasePropertyInterface
{
	FDoubleProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUDoubleProperty(){}
	LuaUDoubleProperty(lua_State*inL, FDoubleProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		double value = popiml<double>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushnumber(inL, (double)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		double value = popiml<double>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUObjectPropertyBase :public LuaBasePropertyInterface
{
	FObjectPropertyBase* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUObjectPropertyBase(){}
	LuaUObjectPropertyBase(lua_State*inL, FObjectPropertyBase* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUObjectProperty :public LuaBasePropertyInterface
{
	FObjectProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUObjectProperty(){}
	LuaUObjectProperty(lua_State*inL, FObjectProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUClassProperty :public LuaBasePropertyInterface
{
	FClassProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUClassProperty(){}
	LuaUClassProperty(lua_State*inL, FClassProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUStrProperty :public LuaBasePropertyInterface
{
	FStrProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUStrProperty(){}
	LuaUStrProperty(lua_State*inL, FStrProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue(ValuePtr))));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue(ValuePtr))));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue(ValuePtr))));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		FString value = popiml<FString>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue_InContainer(ContainerPtr))));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue_InContainer(ContainerPtr))));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*(Property->GetPropertyValue_InContainer(ContainerPtr))));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		FString value = popiml<FString>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUNameProperty :public LuaBasePropertyInterface
{
	FNameProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUNameProperty(){}
	LuaUNameProperty(lua_State*inL, FNameProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		FName result = (FName)Property->GetPropertyValue(ValuePtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FName result = (FName)Property->GetPropertyValue(ValuePtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		FName result = (FName)Property->GetPropertyValue(ValuePtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		FName value = popiml<FName>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		FName result = (FName)Property->GetPropertyValue_InContainer(ContainerPtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		FName result = (FName)Property->GetPropertyValue_InContainer(ContainerPtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		FName result = (FName)Property->GetPropertyValue_InContainer(ContainerPtr);
		ue_lua_pushstring(inL, TCHAR_TO_UTF8(*result.ToString()));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		FName value = popiml<FName>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUTextProperty :public LuaBasePropertyInterface
{
	FTextProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUTextProperty(){}
	LuaUTextProperty(lua_State*inL, FTextProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushstruct_nogc(inL, "FText", "FText_nogc", (void*)ValuePtr);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FText* Text_ptr = (FText*)ValuePtr;
		UTableUtil::push_ret(inL, *Text_ptr);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		if (!ue_lua_isuserdata(inL, LuaStackIndex))
		{
			FText* Text_ptr = (FText*)ValuePtr;
			UTableUtil::push(inL, Text_ptr->ToString());
		}
		else
		{
			void* DestPtr = tovoid(inL, LuaStackIndex);
			Property->CopyCompleteValueFromScriptVM(DestPtr, ValuePtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		FText value = popiml<FText>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushstruct_nogc(inL, "FText", "FText_nogc", (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		FText* Text_ptr = (FText*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
		UTableUtil::push_ret(inL, *Text_ptr);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		if (!ue_lua_isuserdata(inL, LuaStackIndex))
		{
			FText* Text_ptr = (FText*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
			UTableUtil::push(inL, Text_ptr->ToString());
		}
		else
		{
			void* DestPtr = tovoid(inL, LuaStackIndex);
			Property->CopyCompleteValueFromScriptVM(DestPtr, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		FText value = popiml<FText>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUByteProperty :public LuaBasePropertyInterface
{
	FByteProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUByteProperty(){}
	LuaUByteProperty(lua_State*inL, FByteProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		int32 value = popiml<int>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		ue_lua_pushinteger(inL, (int32)Property->GetPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		int32 value = popiml<int>::pop(inL, LuaStackIndex);
		Property->SetPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUEnumProperty :public LuaBasePropertyInterface
{
	FEnumProperty* Property;
	TSharedPtr<LuaBasePropertyInterface> UnderlyingProperty;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUEnumProperty(){}
	LuaUEnumProperty(lua_State*inL, FEnumProperty* InProperty):Property(InProperty)
	{
		UnderlyingProperty = CreatePropertyInterface(inL, InProperty->GetUnderlyingProperty());
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		UnderlyingProperty->push(inL, ValuePtr);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		UnderlyingProperty->push(inL, ValuePtr);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		UnderlyingProperty->push(inL, ValuePtr);
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		UnderlyingProperty->pop(inL, LuaStackIndex, ValuePtr);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_container_novirtual(inL, ContainerPtr);
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_container_novirtual(inL, ContainerPtr);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		push_container_novirtual(inL, ContainerPtr);
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		pop_novirtual(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
};
struct LuaUStructProperty :public LuaBasePropertyInterface
{
	FStructProperty* Property;
	FString TypeName;
	FString TypeName_nogc;
	const char* PtrTypeName;
	const char* PtrTypeName_nogc;
	TSharedPtr<FTCHARToUTF8> PtrTypeName_Utf8;
	TSharedPtr<FTCHARToUTF8> PtrTypeName_nogc_Utf8;
// 	auto temp1 = FTCHARToUTF8((const TCHAR*)*TypeName);
// 	const char* name = (ANSICHAR*)temp1.Get();
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUStructProperty(){
	}
	LuaUStructProperty(lua_State*inL, FStructProperty* InProperty):Property(InProperty)
	{
		FString TempName;
		if (UUserDefinedStruct* BpStruct = Cast<UUserDefinedStruct>(Property->Struct))
		{
			UTableUtil::MayAddNewStructType(inL, BpStruct);
			TempName = BpStruct->GetName();
		}
		else
			TempName = Property->Struct->GetStructCPPName();
		TypeName = TempName;
		TypeName_nogc = TempName+"_nogc";
		PtrTypeName_Utf8 = MakeShareable(new FTCHARToUTF8((const TCHAR*)*TypeName));
		PtrTypeName_nogc_Utf8 = MakeShareable(new FTCHARToUTF8((const TCHAR*)*TypeName_nogc));
		PtrTypeName = PtrTypeName_Utf8->Get();
		PtrTypeName_nogc = PtrTypeName_nogc_Utf8->Get();
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushstruct_nogc(inL, PtrTypeName, PtrTypeName_nogc, (void*)ValuePtr);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		uint8* result = GetBpStructTempIns(TypeName, Property->GetSize());
		Property->InitializeValue(result);
		Property->CopyCompleteValueFromScriptVM(result, ValuePtr);
		pushstruct_nogc(inL, PtrTypeName, PtrTypeName_nogc, result);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		void* DestPtr = tostruct(inL, LuaStackIndex);
		if(DestPtr)
			Property->CopyCompleteValueFromScriptVM(DestPtr, ValuePtr);
		else
			ensureAlwaysMsgf(0, TEXT("Bug"));
		lua_pushvalue(inL, LuaStackIndex);
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		void* value = tostruct(inL, LuaStackIndex);
		if (value)
			Property->CopyCompleteValue(ValuePtr, value);
		else
			ensureAlwaysMsgf(0, TEXT("Bug"));
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_rawget(inL, -2);
			if (!lua_isnil(inL, -1))
			{
				return;
			}
		}
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_pushvalue(inL, -3);
			lua_rawset(inL, -3);
			lua_pop(inL, 1);
		}
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushstruct_nogc(inL, PtrTypeName, PtrTypeName_nogc, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		void* ValuePtr = (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
		uint8* result = GetBpStructTempIns(TypeName, Property->GetSize());
		Property->InitializeValue(result);
		Property->CopyCompleteValueFromScriptVM(result, ValuePtr);
		pushstruct_temp(inL, PtrTypeName, PtrTypeName_nogc, result);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		void* ValuePtr = (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
		void* DestPtr = tostruct(inL, LuaStackIndex);
		if (DestPtr)
			Property->CopyCompleteValueFromScriptVM(DestPtr, ValuePtr);
		else
			ensureAlwaysMsgf(0, TEXT("Bug"));
		lua_pushvalue(inL, LuaStackIndex);
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		void* value = tostruct(inL, LuaStackIndex);
		if (value)
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>((void*)ContainerPtr), value);
		else
			ensureAlwaysMsgf(0, TEXT("Bug"));
	}
};

struct LuaUDelegateProperty :public LuaBasePropertyInterface
{
	FDelegateProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUDelegateProperty(){}
	LuaUDelegateProperty(lua_State*inL, FDelegateProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		FScriptDelegate* DelegatePtr = (FScriptDelegate*)Property->GetPropertyValuePtr(ValuePtr);
		ULuaDelegateSingle* NewOne = ULuaDelegateSingle::CreateInCppRef(DelegatePtr, Property->SignatureFunction);
		UTableUtil::push(inL, NewOne);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		check(0);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_rawget(inL, -2);
			if (!lua_isnil(inL, -1))
			{
				return;
			}
		}
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_pushvalue(inL, -3);
			lua_rawset(inL, -3);
			lua_pop(inL, 1);
		}
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		check(0);
	}
};
struct LuaUWeakObjectProperty :public LuaBasePropertyInterface
{
	FWeakObjectProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUWeakObjectProperty(){}
	LuaUWeakObjectProperty(lua_State*inL, FWeakObjectProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue(ValuePtr));
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue(ValuePtr, value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		pushuobject(inL, Property->GetObjectPropertyValue_InContainer(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		UObject* value = popiml<UObject*>::pop(inL, LuaStackIndex);
		Property->SetObjectPropertyValue_InContainer((void*)ContainerPtr, value);
	}
};
struct LuaUArrayProperty :public LuaBasePropertyInterface
{
	TSharedPtr<LuaBasePropertyInterface> InnerProperty;
	FArrayProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUArrayProperty(){}
	LuaUArrayProperty(lua_State*inL, FArrayProperty* InProperty):Property(InProperty)
	{
		InnerProperty = CreatePropertyInterface(inL, Property->Inner);
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushstruct_gc(inL, "ULuaArrayHelper", ULuaArrayHelper::GetHelperCPP_ValuePtr((void*)ValuePtr, Property));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FScriptArrayHelper result(Property, ValuePtr);
		lua_newtable(inL);
		for (int32 i = 0; i < result.Num(); ++i)
		{
			ue_lua_pushinteger(inL, i + 1);
// will bug for struct type
			InnerProperty->push_ret_container(inL, result.GetRawPtr(i));
			ue_lua_rawset(inL, -3);
		}
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		if (UnrealLua::IsGlueTArray(inL, LuaStackIndex))
		{
			void* Ptr = tovoid(inL, LuaStackIndex);
			ULuaArrayHelper::GlueArrCopyTo(Property, ValuePtr, Ptr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (ULuaArrayHelper* ArrHelper = (ULuaArrayHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex))
		{
			ArrHelper->CopyFrom(Property, ValuePtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (lua_istable(inL, LuaStackIndex))
		{
			FScriptArrayHelper Arr(Property, ValuePtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
			int table_len = lua_objlen(inL, -1);
			for (int i = 1; i <= FMath::Max(table_len, Arr.Num()); i++)
			{
				ue_lua_pushinteger(inL, i);
				if (i <= Arr.Num())
					InnerProperty->push_ret_container(inL, Arr.GetRawPtr(i-1));
				else
					ue_lua_pushnil(inL);
				ue_lua_rawset(inL, -3);
			}
		}
		else
		{
			ensureAlwaysMsgf(0, TEXT("not arr"));
			UnrealLua::ReportError(inL, "not arr");
		}
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		if (UnrealLua::IsGlueTArray(inL, LuaStackIndex))
		{
			void* ArrPtr = (void*)tovoid(inL, LuaStackIndex);
			ULuaArrayHelper::GlueArrCopyTo(Property, ArrPtr, ValuePtr);
		}
		else
		{
			ULuaArrayHelper* ArrHelper = (ULuaArrayHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex);
			if (ArrHelper)
			{
				ArrHelper->CopyTo(Property, ValuePtr);
			}
			else
			{
				ue_lua_pushvalue(inL, LuaStackIndex);
				int32 len = lua_objlen(inL, -1);
				FScriptArrayHelper result(Property, ValuePtr);
				result.Resize(len);
				ue_lua_pushnil(inL);
				while (ue_lua_next(inL, -2))
				{
					int32 i = ue_lua_tointeger(inL, -2) - 1;
					InnerProperty->pop_container(inL, -1, result.GetRawPtr(i));
					ue_lua_pop(inL, 1);
				}
				ue_lua_pop(inL, 1);
			}
		}
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_ret(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		push_ref(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		pop_novirtual(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
};
struct LuaUMapProperty :public LuaBasePropertyInterface
{
	TSharedPtr<LuaBasePropertyInterface> KeyProperty;
	TSharedPtr<LuaBasePropertyInterface> ValueProperty;
	int32 KeyOffset;
	int32 KeyPropertySize;
	FMapProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUMapProperty(){}
	LuaUMapProperty(lua_State*inL, FMapProperty* InProperty):Property(InProperty)
	{
		KeyProperty = CreatePropertyInterface(inL, Property->KeyProp);
		ValueProperty = CreatePropertyInterface(inL, Property->ValueProp);
		KeyOffset = 0;
		KeyPropertySize = Property->KeyProp->ElementSize * Property->KeyProp->ArrayDim;
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushstruct_gc(inL, "ULuaMapHelper", ULuaMapHelper::GetHelperCPP_ValuePtr((void*)ValuePtr, Property));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FScriptMapHelper result(Property, ValuePtr);
		lua_newtable(inL);
		for (int32 i = 0; i < result.Num(); ++i)
		{
			uint8* PairPtr = result.GetPairPtr(i);
			KeyProperty->push_ret_container(inL, PairPtr + KeyOffset);
			ValueProperty->push_ret_container(inL, PairPtr);
			lua_rawset(inL, -3);
		}
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		if (UnrealLua::IsGlueTMap(inL, LuaStackIndex))
		{
			void* ArrPtr = (void*)tovoid(inL, LuaStackIndex);
			ULuaMapHelper::GlueMapCopyTo(Property, ValuePtr, ArrPtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (ULuaMapHelper* Helper = (ULuaMapHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex))
		{
			Helper->CopyFrom(Property, (void*)ValuePtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (ue_lua_istable(inL, LuaStackIndex))
		{
			FScriptMapHelper result(Property, ValuePtr);
			FProperty* CurrKeyProp = Property->KeyProp;
			void* KeyStorageSpace = FMemory_Alloca(KeyPropertySize);
			CurrKeyProp->InitializeValue(KeyStorageSpace);

			ue_lua_newtable(inL);
			ue_lua_pushvalue(inL, LuaStackIndex);
			ue_lua_pushnil(inL);
			int i = 1;
			while (ue_lua_next(inL, -2) != 0)
			{
				ue_lua_pop(inL, 1);
				KeyProperty->pop_container(inL, -1, KeyStorageSpace);
				uint8* Result = result.FindValueFromHash(KeyStorageSpace);
				if (Result == nullptr)
				{
					ue_lua_pushvalue(inL, -1);
					ue_lua_rawseti(inL, -4, i);
					i++;
				}
			}
			CurrKeyProp->DestroyValue(KeyStorageSpace);

			ue_lua_pushnil(inL);
			while (ue_lua_next(inL, -3) != 0)
			{
				ue_lua_pushnil(inL);
				ue_lua_rawset(inL, -4);
			}
			ue_lua_remove(inL, -2);
			for (int32 j = 0; j < result.Num(); ++j)
			{
				uint8* PairPtr = result.GetPairPtr(j);
				KeyProperty->push_ret_container(inL, PairPtr + KeyOffset);
				ValueProperty->push_ret_container(inL, PairPtr);
				ue_lua_rawset(inL, -3);
			}
		}
		else
		{
			ensureAlwaysMsgf(0, TEXT("not map"));
			UnrealLua::ReportError(inL, "not map");
		}
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		if (UnrealLua::IsGlueTMap(inL, LuaStackIndex))
		{
			void* Ptr = (void*)tovoid(inL, LuaStackIndex);
			ULuaMapHelper::GlueMapCopyTo(Property, Ptr, ValuePtr);
		}
		else
		{
			ULuaMapHelper* Helper = (ULuaMapHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex);
			if (Helper)
			{
				Helper->CopyTo(Property, ValuePtr);
			}
			else if (ue_lua_istable(inL, LuaStackIndex))
			{
				ue_lua_pushvalue(inL, LuaStackIndex);
				FScriptMapHelper result(Property, ValuePtr);
				result.EmptyValues();
				ue_lua_pushnil(inL);
				while (ue_lua_next(inL, -2))
				{
					int32 i = result.AddDefaultValue_Invalid_NeedsRehash();
					uint8* PairPtr = result.GetPairPtr(i);
					KeyProperty->pop_container(inL, -2, PairPtr + KeyOffset);
					ValueProperty->pop_container(inL, -1, PairPtr);
					ue_lua_pop(inL, 1);
				}
				result.Rehash();
				ue_lua_pop(inL, 1);
			}
			else
			{
				ensureAlwaysMsgf(0, TEXT("not map"));
			}
		}
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_ret(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		push_ref(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		pop_novirtual(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
};
struct LuaUSetProperty :public LuaBasePropertyInterface
{
	TSharedPtr<LuaBasePropertyInterface> ElementProp;
	FSetProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUSetProperty(){}
	LuaUSetProperty(lua_State*inL, FSetProperty* InProperty):Property(InProperty)
	{
		ElementProp = CreatePropertyInterface(inL, Property->ElementProp);
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		pushstruct_gc(inL, "ULuaSetHelper", ULuaSetHelper::GetHelperCPP_ValuePtr((void*)ValuePtr, Property));
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FScriptSetHelper result(Property, ValuePtr);
		ue_lua_newtable(inL);
		for (int32 i = 0; i < result.Num(); ++i)
		{
			ElementProp->push_ret_container(inL, result.GetElementPtr(i));
			ue_lua_pushboolean(inL, true);
			ue_lua_rawset(inL, -3);
		}
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		if (UnrealLua::IsGlueTSet(inL, LuaStackIndex))
		{
			void* ArrPtr = (void*)tovoid(inL, LuaStackIndex);
			ULuaSetHelper::GlueSetCopyTo(Property, (void*)ValuePtr, ArrPtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (ULuaSetHelper* Helper = (ULuaSetHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex))
		{
			Helper->CopyFrom(Property, (void*)ValuePtr);
			ue_lua_pushvalue(inL, LuaStackIndex);
		}
		else if (ue_lua_istable(inL, LuaStackIndex))
		{
			ue_lua_newtable(inL);
			ue_lua_pushvalue(inL, LuaStackIndex);
			ue_lua_pushnil(inL);
			int i = 1;
			FScriptSetHelper result(Property, ValuePtr);
			FProperty* CurrKeyProp = Property->ElementProp;
			const int32 KeyPropertySize = CurrKeyProp->ElementSize * CurrKeyProp->ArrayDim;
			void* KeyStorageSpace = FMemory_Alloca(KeyPropertySize);
			CurrKeyProp->InitializeValue(KeyStorageSpace);

			uint8* keyptr = nullptr;
			int32 Index = result.FindElementIndexFromHash(KeyStorageSpace);
			if (Index != INDEX_NONE)
			{
				keyptr = result.GetElementPtr(Index);
			}
			while (ue_lua_next(inL, -2) != 0)
			{
				ue_lua_pop(inL, 1);
				ElementProp->pop_container(inL, -1, KeyStorageSpace);
				uint8* NewKeyptr = nullptr;
				int32 NewIndex = result.FindElementIndexFromHash(KeyStorageSpace);
				if (NewIndex != INDEX_NONE)
				{
					NewKeyptr = result.GetElementPtr(NewIndex);
				}
				if (NewKeyptr == nullptr)
				{
					ue_lua_pushvalue(inL, -1);
					ue_lua_rawseti(inL, -4, i);
					i++;
				}
			}
			ue_lua_pushnil(inL);
			while (ue_lua_next(inL, -3) != 0)
			{
				ue_lua_pushnil(inL);
				ue_lua_rawset(inL, -4);
			}
			ue_lua_remove(inL, -2);

			for (int32 j = 0; j < result.Num(); ++j)
			{
				ElementProp->push_ret_container(inL, result.GetElementPtr(j));
				ue_lua_pushboolean(inL, true);
				ue_lua_rawset(inL, -3);
			}
		}
		else
		{
			ensureAlwaysMsgf(0, TEXT("not set"));
			UnrealLua::ReportError(inL, "not set");
		}
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		if (UnrealLua::IsGlueTSet(inL, LuaStackIndex))
		{
			void* ArrPtr = (void*)tovoid(inL, LuaStackIndex);
			ULuaSetHelper::GlueSetCopyTo(Property, ArrPtr, (void*)ValuePtr);
		}
		else {
			ULuaSetHelper* Helper = (ULuaSetHelper*)UnrealLua::IsCppPtr(inL, LuaStackIndex);
			if (Helper)
			{
				Helper->CopyTo(Property, (void*)ValuePtr);
			}
			else if (ue_lua_istable(inL, LuaStackIndex))
			{
				ue_lua_pushvalue(inL, LuaStackIndex);
				FScriptSetHelper result(Property, ValuePtr);
				result.EmptyElements();
				ue_lua_pushnil(inL);
				while (ue_lua_next(inL, -2))
				{
					int32 i = result.AddDefaultValue_Invalid_NeedsRehash();
					uint8* ElementPtr = result.GetElementPtr(i);
					ElementProp->pop_container(inL, -2, ElementPtr);
					ue_lua_pop(inL, 1);
				}
				result.Rehash();
				ue_lua_pop(inL, 1);
			}
			else
			{
				ensureAlwaysMsgf(0, TEXT("not set"));
			}
		}
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_ret(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		push_ref(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		pop_novirtual(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
};
struct LuaUInterfaceProperty :public LuaBasePropertyInterface
{
	FInterfaceProperty* Property;
	//Some hook
	virtual FProperty* GetProperty(){return Property;}
	virtual ~LuaUInterfaceProperty(){}
	LuaUInterfaceProperty(lua_State*inL, FInterfaceProperty* InProperty):Property(InProperty)
	{
		
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr) 
	{
		FScriptInterface* result = (FScriptInterface*)Property->GetPropertyValuePtr(ValuePtr);
		pushuobject(inL, (void*)result->GetObject());
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		FScriptInterface* result = (FScriptInterface*)Property->GetPropertyValuePtr(ValuePtr);
		pushuobject(inL, (void*)result->GetObject());
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		FScriptInterface* result = (FScriptInterface*)Property->GetPropertyValuePtr(ValuePtr);
		pushuobject(inL, (void*)result->GetObject());
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) 
	{
		FScriptInterface* result = (FScriptInterface*)Property->GetPropertyValuePtr(ValuePtr);
		UObject* value = (UObject*)touobject(inL, LuaStackIndex);
		result->SetObject(value);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr) 
	{
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_ret(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		push_ref(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		pop_novirtual(inL, LuaStackIndex, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	}
};

#if ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23)
struct LuaUMulticastInlineDelegateProperty :public LuaBasePropertyInterface
{
	FMulticastInlineDelegateProperty* Property;
	//Some hook
	virtual FProperty* GetProperty() { return Property; }
	virtual ~LuaUMulticastInlineDelegateProperty() {}
	LuaUMulticastInlineDelegateProperty(lua_State*inL, FMulticastInlineDelegateProperty* InProperty) :Property(InProperty)
	{
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr)
	{
		UFunction* FunSig = Property->SignatureFunction;
		auto delegateproxy = NewObject<ULuaDelegateMulti>();
		delegateproxy->Init((void*)ValuePtr, FunSig);
		pushuobject(inL, (void*)delegateproxy);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr)
	{
		check(0);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr)
	{
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_rawget(inL, -2);
			if (!lua_isnil(inL, -1))
			{
				return;
			}
		}
		push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_pushvalue(inL, -3);
			lua_rawset(inL, -3);
			lua_pop(inL, 1);
		}
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		void* ValuePtr = (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
		push_novirtual(inL, ValuePtr);
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		check(0);
	}
};

struct LuaUMulticastSparseDelegateProperty :public LuaBasePropertyInterface
{
	FMulticastSparseDelegateProperty* Property;
	//Some hook
	virtual FProperty* GetProperty() { return Property; }
	virtual ~LuaUMulticastSparseDelegateProperty() {}
	LuaUMulticastSparseDelegateProperty(lua_State*inL, FMulticastSparseDelegateProperty* InProperty) :Property(InProperty)
	{
	}
	virtual void push(lua_State* inL, const void* ValuePtr) override
	{
		push_novirtual(inL, ValuePtr);
	}
	void push_novirtual(lua_State* inL, const void* ValuePtr)
	{
		check(0);
	}
	virtual void push_ret(lua_State* inL, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
	{
		check(0);
	}
	virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
	{
		pop_novirtual(inL, LuaStackIndex, ValuePtr);
	}
	void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr)
	{
		check(0);
	}
	void push_container_novirtual(lua_State* inL, const void* ContainerPtr)
	{
		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_rawget(inL, -2);
			if (!lua_isnil(inL, -1))
			{
				return;
			}
		}

		auto delegateproxy = NewObject<ULuaDelegateMulti>();
		delegateproxy->Init(Property, (UObject*)ContainerPtr);
		pushuobject(inL, (void*)delegateproxy);

		lua_pushvalue(inL, lua_upvalueindex(2));
		if (!lua_isnil(inL, -1))
		{
			lua_pushvalue(inL, 1);
			lua_pushvalue(inL, -3);
			lua_rawset(inL, -3);
			lua_pop(inL, 1);
		}
	}
	virtual void push_container(lua_State* inL, const void* ContainerPtr) override
	{
		push_container_novirtual(inL, ContainerPtr);
	}
	virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
	{
		check(0);
	}
	virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
	{
		pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
	}
	void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
	{
		check(0);
	}
};
#else
struct LuaUMulticastDelegateProperty : public LuaBasePropertyInterface
{
	FMulticastDelegateProperty* Property;
//Some hook
virtual FProperty* GetProperty() { return Property; }
virtual ~LuaUMulticastDelegateProperty() {}
LuaUMulticastDelegateProperty(lua_State*inL, FMulticastDelegateProperty* InProperty) :Property(InProperty)
{

}
virtual void push(lua_State* inL, const void* ValuePtr) override
{
	push_novirtual(inL, ValuePtr);
}
void push_novirtual(lua_State* inL, const void* ValuePtr)
{
	UFunction* FunSig = Property->SignatureFunction;
	auto delegateproxy = NewObject<ULuaDelegateMulti>();
	delegateproxy->Init((void*)ValuePtr, FunSig);
	pushuobject(inL, (void*)delegateproxy);
}
virtual void push_ret(lua_State* inL, const void* ValuePtr) override
{
	check(0);
}
virtual void push_ref(lua_State* inL, int32 LuaStackIndex, const void* ValuePtr) override
{
	check(0);
}
virtual void pop(lua_State* inL, int32 LuaStackIndex, void* ValuePtr) override
{
	pop_novirtual(inL, LuaStackIndex, ValuePtr);
}
void pop_novirtual(lua_State* inL, int32 LuaStackIndex, void* ValuePtr)
{
	check(0);
}
void push_container_novirtual(lua_State* inL, const void* ContainerPtr)
{
	lua_pushvalue(inL, lua_upvalueindex(2));
	if (!lua_isnil(inL, -1))
	{
		lua_pushvalue(inL, 1);
		lua_rawget(inL, -2);
		if (!lua_isnil(inL, -1))
		{
			return;
		}
	}
	push_novirtual(inL, (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr));
	lua_pushvalue(inL, lua_upvalueindex(2));
	if (!lua_isnil(inL, -1))
	{
		lua_pushvalue(inL, 1);
		lua_pushvalue(inL, -3);
		lua_rawset(inL, -3);
		lua_pop(inL, 1);
	}
}
virtual void push_container(lua_State* inL, const void* ContainerPtr) override
{
	void* ValuePtr = (void*)Property->ContainerPtrToValuePtr<uint8>(ContainerPtr);
	push_novirtual(inL, ValuePtr);
}
virtual void push_ret_container(lua_State* inL, const void* ContainerPtr) override
{
	check(0);
}
virtual void push_ref_container(lua_State* inL, int32 LuaStackIndex, const void* ContainerPtr) override
{
	check(0);
}
virtual void pop_container(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr) override
{
	pop_container_novirtual(inL, LuaStackIndex, ContainerPtr);
}
void pop_container_novirtual(lua_State* inL, int32 LuaStackIndex, const void * ContainerPtr)
{
	check(0);
}
};
#endif

#define NEWPROPERTY_INTERFACE(Type)\
	static LuaBasePropertyInterface* NewPropertyInterfaceBy##Type(lua_State*inL, FProperty* Property)\
	{\
		return new Lua##Type(inL, (Type*)Property);\
	}


NEWPROPERTY_INTERFACE(FBoolProperty)
NEWPROPERTY_INTERFACE(FIntProperty)
NEWPROPERTY_INTERFACE(FInt8Property)
NEWPROPERTY_INTERFACE(FUInt16Property)
NEWPROPERTY_INTERFACE(FInt16Property)
NEWPROPERTY_INTERFACE(FUInt32Property)
NEWPROPERTY_INTERFACE(FInt64Property)
NEWPROPERTY_INTERFACE(FUInt64Property)
NEWPROPERTY_INTERFACE(FFloatProperty)
NEWPROPERTY_INTERFACE(FDoubleProperty)
NEWPROPERTY_INTERFACE(FObjectPropertyBase)
NEWPROPERTY_INTERFACE(FObjectProperty)
NEWPROPERTY_INTERFACE(FClassProperty)
NEWPROPERTY_INTERFACE(FStrProperty)
NEWPROPERTY_INTERFACE(FNameProperty)
NEWPROPERTY_INTERFACE(FTextProperty)
NEWPROPERTY_INTERFACE(FByteProperty)
NEWPROPERTY_INTERFACE(FEnumProperty)
NEWPROPERTY_INTERFACE(FStructProperty)
#if ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23)
NEWPROPERTY_INTERFACE(FMulticastInlineDelegateProperty)
NEWPROPERTY_INTERFACE(FMulticastSparseDelegateProperty)
#else
NEWPROPERTY_INTERFACE(FMulticastDelegateProperty)
#endif
NEWPROPERTY_INTERFACE(FDelegateProperty)
NEWPROPERTY_INTERFACE(FWeakObjectProperty)
NEWPROPERTY_INTERFACE(FArrayProperty)
NEWPROPERTY_INTERFACE(FMapProperty)
NEWPROPERTY_INTERFACE(FSetProperty)
NEWPROPERTY_INTERFACE(FInterfaceProperty)

#define MAP_CREATEFUNC_TOTYPE(Type) TheMap.Add(Type::StaticClass(), NewPropertyInterfaceBy##Type);\

static TMap<UClass*, TFunction< LuaBasePropertyInterface*(lua_State*, FProperty*)> > GetCreateFuncMap()
{
	TMap <UClass*, TFunction< LuaBasePropertyInterface*(lua_State*, FProperty*)>> TheMap;

	MAP_CREATEFUNC_TOTYPE(FBoolProperty)
		MAP_CREATEFUNC_TOTYPE(FIntProperty)
		MAP_CREATEFUNC_TOTYPE(FInt8Property)
		MAP_CREATEFUNC_TOTYPE(FUInt16Property)
		MAP_CREATEFUNC_TOTYPE(FInt16Property)
		MAP_CREATEFUNC_TOTYPE(FUInt32Property)
		MAP_CREATEFUNC_TOTYPE(FInt64Property)
		MAP_CREATEFUNC_TOTYPE(FUInt64Property)
		MAP_CREATEFUNC_TOTYPE(FFloatProperty)
		MAP_CREATEFUNC_TOTYPE(FDoubleProperty)
		MAP_CREATEFUNC_TOTYPE(FObjectPropertyBase)
		MAP_CREATEFUNC_TOTYPE(FObjectProperty)
		MAP_CREATEFUNC_TOTYPE(FClassProperty)
		MAP_CREATEFUNC_TOTYPE(FStrProperty)
		MAP_CREATEFUNC_TOTYPE(FNameProperty)
		MAP_CREATEFUNC_TOTYPE(FTextProperty)
		MAP_CREATEFUNC_TOTYPE(FByteProperty)
		MAP_CREATEFUNC_TOTYPE(FEnumProperty)
		MAP_CREATEFUNC_TOTYPE(FStructProperty)
#if ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 23)
 		MAP_CREATEFUNC_TOTYPE(FMulticastInlineDelegateProperty)
		MAP_CREATEFUNC_TOTYPE(FMulticastSparseDelegateProperty)
#else
		MAP_CREATEFUNC_TOTYPE(FMulticastDelegateProperty)
#endif
		MAP_CREATEFUNC_TOTYPE(FDelegateProperty)
		MAP_CREATEFUNC_TOTYPE(FWeakObjectProperty)
		MAP_CREATEFUNC_TOTYPE(FArrayProperty)
		MAP_CREATEFUNC_TOTYPE(FMapProperty)
		MAP_CREATEFUNC_TOTYPE(FSetProperty)
		MAP_CREATEFUNC_TOTYPE(FInterfaceProperty)

		return TheMap;
}

static LuaBasePropertyInterface* CreatePropertyInterfaceRaw(lua_State*inL, FProperty* Property)
{
	static TMap<UClass*, TFunction< LuaBasePropertyInterface*(lua_State*, FProperty*)>> CreateFuncs = GetCreateFuncMap();
	if (auto* ProcessFunc = CreateFuncs.Find(Property->GetClass()))
	{
		return (*ProcessFunc)(inL, Property);
	}
	else
	{
		if (FStructProperty* p = Cast<FStructProperty>(Property))
		{
			return NewPropertyInterfaceByUStructProperty(inL, p);
		}

		else if (FObjectPropertyBase* p = Cast<FObjectPropertyBase>(Property))
		{
			return NewPropertyInterfaceByUObjectPropertyBase(inL, p);
		}
		else
		{
			ensureAlwaysMsgf(0, TEXT("Some type didn't process"));
		}
	}
	return nullptr;
}

TSharedPtr<LuaBasePropertyInterface> CreatePropertyInterface(lua_State*inL, FProperty* Property)
{
	return MakeShareable(CreatePropertyInterfaceRaw(inL, Property));
}

#define BufferReserveCount 3
struct LuaUFunctionInterface : public LuaBaseBpInterface
{
	struct IncGaurd
	{
		IncGaurd(int32& InCount) :Count(InCount) {}
		~IncGaurd() { ++Count; }
		int32& Count;
	};
	struct DecGaurd
	{
		DecGaurd(int32& InCount) :Count(InCount) {}
		~DecGaurd() { --Count; }
		int32& Count;
	};
	~LuaUFunctionInterface(){
		if(PersistBuffer)
			FMemory::Free(PersistBuffer);

		while (OutParms)
		{
			FOutParmRec *NextOut = OutParms->NextOutParm;
			FMemory::Free(OutParms);
			OutParms = NextOut;
		}
	}
	LuaUFunctionInterface(lua_State*inL, UFunction* Function)
	{
		ReEnterCount = 0;
		IsStatic = (Function->FunctionFlags & FUNC_Static)!=0;
		TheFunc = Function;
		int32 ArgIndex;
		if (IsStatic) 
		{
			ArgIndex = 1;
			DefaultObj = ((UClass*)Function->GetOuter())->GetDefaultObject();
		}
		else
		{
			ArgIndex = 2;
		}
		StartIndex = ArgIndex;
		FOutParmRec *NowOutParmRec = nullptr;

		if(GetBufferSize() > 0)
			PersistBuffer = (uint8*)FMemory::Malloc(GetBufferSize());

		IsNativeFunc = Function->HasAnyFunctionFlags(FUNC_Native);
		for (TFieldIterator<FProperty> It(Function); It && (It->GetPropertyFlags() & (CPF_Parm)); ++It)
		{
			auto Prop = *It;
			bool HasAdd = false;
			TSharedPtr<LuaBasePropertyInterface> PropInterface = CreatePropertyInterface(inL, Prop);
			InitAndDestroyParams.Add(PropInterface);
			if ((Prop->GetPropertyFlags() & (CPF_ReturnParm)))
			{
				ReturnValues.Insert(PropInterface, 0);
				continue;
			}
			if (!HasAdd && Prop->GetPropertyFlags() & CPF_ReferenceParm)
			{
				HasAdd = true;
				RefParams.Add(PropInterface);
				StackIndexs.Add(ArgIndex);
			}

			if (Prop->GetPropertyFlags() & CPF_OutParm)
			{
				if (IsNativeFunc)
				{
					FOutParmRec *Out = (FOutParmRec*)FMemory::Malloc(sizeof(FOutParmRec), alignof(FOutParmRec));
					Out->PropAddr = Prop->ContainerPtrToValuePtr<uint8>(PersistBuffer);
					Out->Property = Prop;
					Out->NextOutParm = nullptr;
					if (NowOutParmRec)
					{
						NowOutParmRec->NextOutParm = Out;
						NowOutParmRec = Out;
					}
					else
					{
						OutParms = Out;
						NowOutParmRec = Out;
					}
				}

				if (!HasAdd)
				{
					if (IsNativeFunc)
					{
						RefParams.Add(PropInterface);
						StackIndexs.Add(ArgIndex);
					}
					else
					{
						ReturnValues.Insert(CreatePropertyInterface(inL, Prop), 0);
						continue;
					}
				}
			}
			Params.Add(PropInterface);
			++ArgIndex;

		}
		ReturnCount = ReturnValues.Num() + RefParams.Num();

		ReturnValueAddressOffset = TheFunc->ReturnValueOffset != MAX_uint16 ? TheFunc->ReturnValueOffset : 0;
		ReturnValueAddress = (uint8*)PersistBuffer + ReturnValueAddressOffset;
		UClass *OClass = Function->GetOuterUClass();
		if (OClass != UInterface::StaticClass() && OClass->HasAnyClassFlags(CLASS_Interface))
		{}
	}

	int32 GetBufferSize() const
	{
		return TheFunc->ParmsSize;
	}
	uint8* GetBuffer()
	{
		return PersistBuffer;
	}
	void InitBuffer(uint8* Buffer)
	{
		for (auto& Itf : InitAndDestroyParams)
		{
			Itf->GetProperty()->InitializeValue_InContainer(Buffer);
		}
	}

	void DestroyBuffer(uint8* Buffer)
	{
		for (auto& Itf : InitAndDestroyParams)
		{
			Itf->GetProperty()->DestroyValue_InContainer(Buffer);
		}
	}

	bool BuildTheBuffer(lua_State*inL, uint8* Buffer)
	{	
		int32 LuaTop = ue_lua_gettop(inL);
#if LuaDebug
		if (Params.Num() + StartIndex - 1 > LuaTop)
		{
			ensureAlwaysMsgf(0, TEXT("arguments is not enough"));
			return false;
		}
#endif
		for (int i = 0; i < Params.Num()  ; i++)
		{
			Params[i]->pop_container(inL, StartIndex + i, Buffer);
		}
		return true;
	}

	template<bool bFastCallNative>
	bool Call(lua_State*inL, uint8* Buffer, UObject* Ptr = nullptr)
	{
// 		auto Gaurd = DecGaurd(ReEnterCount);
		if (Ptr == nullptr)
		{
			if (IsStatic)
			{
#if LuaDebug
				if (DefaultObj == nullptr)
				{
					ensureAlwaysMsgf(0, TEXT("Bug"));
					return false;
				}
#endif
				Ptr = DefaultObj;
			}
			else
			{
				Ptr = (UObject*)touobject(inL, 1);
			}
		}
		if (Ptr)
		{
			if (bFastCallNative)
			{
				FFrame NewStack(Ptr, TheFunc, Buffer, nullptr, TheFunc->Children);
				NewStack.OutParms = OutParms;
				TheFunc->Invoke(Ptr, NewStack, ReturnValueAddress);
			}
			else
			{
				Ptr->ProcessEvent(TheFunc, Buffer);
			}
		}
		else 
		{
			ensureAlwaysMsgf(0, TEXT("Ptr Can't be null"));
			return false;
		}
		return true;
	}

	int32 PushRet(lua_State*inL, uint8* Buffer)
	{
		for (auto& Itf : ReturnValues)
		{
			Itf->push_ret_container(inL, Buffer);
		}
		for (int i = 0; i < RefParams.Num(); i++)
		{
			RefParams[i]->push_ref_container(inL, StackIndexs[i], Buffer);
		}
		return ReturnCount;
	}

	int32 JustCall(lua_State* inL)
	{
// 		uint8* Buffer = (uint8*)FMemory_Alloca(GetBufferSize());
		++ReEnterCount;
		DecGaurd g(ReEnterCount);
		bool IsReinto = ReEnterCount>1;
		uint8* Buffer = PersistBuffer;

		if(IsReinto)
			Buffer = (uint8*)FMemory_Alloca(GetBufferSize());

		InitBuffer(Buffer);
		int32 Count;
		bool SusscessCall = false;
		if (BuildTheBuffer(inL, Buffer))
		{
			if (IsNativeFunc && ! IsReinto)
				SusscessCall = Call<true>(inL, Buffer);
			else
				SusscessCall = Call<false>(inL, Buffer);
		}

		if(SusscessCall)
			Count = PushRet(inL, Buffer);
		else
			Count = 0;

		DestroyBuffer(Buffer);
		return Count;
	}
	
	bool IsStatic;
	UObject* DefaultObj;
	UFunction* TheFunc;
	UFunction* ActualFunc = nullptr;
	TArray<int32> StackIndexs;
	int32 StartIndex;
	TArray<TSharedPtr<LuaBasePropertyInterface>, TInlineAllocator<6>> Params;
	TArray<TSharedPtr<LuaBasePropertyInterface>, TInlineAllocator<6>> InitAndDestroyParams;
	TArray<TSharedPtr<LuaBasePropertyInterface>, TInlineAllocator<2>> ReturnValues;
	TArray<TSharedPtr<LuaBasePropertyInterface>, TInlineAllocator<6>> RefParams;
	int32 ReturnCount;
	int32 ReEnterCount;
	uint8* PersistBuffer = nullptr;
	uint8* ReturnValueAddress = nullptr;
	FOutParmRec* OutParms = nullptr;
	int32 ReturnValueAddressOffset = 0;
	bool IsNativeFunc;
};

struct MuldelegateBpInterface
{
	bool HasReturn;
	int ParamCount;
	TSharedPtr<LuaBasePropertyInterface> ReturnValue;
	TArray<TSharedPtr<LuaBasePropertyInterface>> Params;
	MuldelegateBpInterface(UFunction* Function)
	{
		HasReturn = false;
		for (TFieldIterator<FProperty> ParamIt(Function); ParamIt; ++ParamIt)
		{
			FProperty* Param = *ParamIt;
			if (Param->GetName() != "ReturnValue")
			{
				Params.Add(CreatePropertyInterface(nullptr, Param));
			}
			else
			{
				ReturnValue = CreatePropertyInterface(nullptr, Param);
				HasReturn = true;
			}
		}
		ParamCount = Params.Num();
	}
	void Call(lua_State*inL, void* Buffer)
	{
		for (auto& Itf : Params)
		{
			Itf->push_container(inL, Buffer);
		}
		if (lua_pcall(inL, ParamCount, HasReturn ? 1 : 0, 0))
		{
#if LuaDebug
			FString error = lua_tostring(inL, -1);
			ensureAlwaysMsgf(0, *error);
			UnrealLua::ReportError(inL, error);
#endif
			UTableUtil::log(lua_tostring(inL, -1));
		}

		if (HasReturn)
			ReturnValue->pop_container(inL, -1, Buffer);
	}

};