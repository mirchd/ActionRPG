// Fill out your copyright notice in the Description page of Project Settings.


#include "LuaProxyWidget.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "LuaDelegate.h"
#include "LuaProxySlot.h"
#include "LuaState.h"

ULuaState* ULuaProxyWidget::GetLuaState()
{
	return Cast<ULuaState>(GetOuter());
}

FLuaValue ULuaProxyWidget::LuaMetaMethodToString_Implementation()
{
	return FString::Printf(TEXT("LuaProxyWidget@%p"), this);
}

FLuaValue ULuaProxyWidget::LuaMetaMethodIndex_Implementation(const FString& Key)
{
	if (Key == "SetContent")
	{
		FLuaValue SetContentWidget = FLuaValue([this](TArray<FLuaValue> LuaArgs) -> FLuaValueOrError {
			if (!Widget->IsA<UContentWidget>())
			{
				return FString("SetContent can be called only on ContentWidget instances");
			}
			if (!LuaArgs.IsValidIndex(0) || !LuaArgs[0].Object || !LuaArgs[0].Object->IsA<ULuaProxyWidget>())
			{
				return FString("Expected first argument to be a widget");
			}

			UPanelSlot* Slot = Cast<UContentWidget>(Widget)->SetContent(Cast<ULuaProxyWidget>(LuaArgs[0].Object)->Widget);
			if (Slot)
			{
				ULuaProxySlot* NewProxySlot = NewObject<ULuaProxySlot>(GetLuaState());
				NewProxySlot->Slot = Slot;
				Proxies.Add(NewProxySlot);
				return FLuaValue(NewProxySlot);
			}

			return FLuaValue();
			});

		return SetContentWidget;
	}
	else if (Key == "AddChild")
	{
		FLuaValue SetContentWidget = FLuaValue([this](TArray<FLuaValue> LuaArgs) -> FLuaValueOrError {
			if (!Widget->IsA<UPanelWidget>())
			{
				return FString("AddChild can be called only on PanelWidget instances");
			}
			if (!LuaArgs.IsValidIndex(0) || !LuaArgs[0].Object || !LuaArgs[0].Object->IsA<ULuaProxyWidget>())
			{
				return FString("Expected first argument to be a widget");
			}

			UPanelSlot* Slot = Cast<UPanelWidget>(Widget)->AddChild(Cast<ULuaProxyWidget>(LuaArgs[0].Object)->Widget);
			if (Slot)
			{
				ULuaProxySlot* NewProxySlot = NewObject<ULuaProxySlot>(GetLuaState());
				NewProxySlot->Slot = Slot;
				Proxies.Add(NewProxySlot);
				return FLuaValue(NewProxySlot);
			}
			return FLuaValue();
			});

		return SetContentWidget;
	}
	else if (Key == "ColorAndOpacity" || Key == "Text" || Key == "CheckedState" || Key == "BrushColor" || Key == "Brush")
	{
		return GetLuaState()->GetLuaValueFromProperty(Widget, *Key);
	}

	return FLuaValue();
}

bool ULuaProxyWidget::LuaMetaMethodNewIndex_Implementation(const FString& Key, FLuaValue Value)
{
	bool bSuccess = false;
	if (Key.StartsWith("On"))
	{
		bSuccess = GetLuaState()->SetPropertyFromLuaValue(Widget, *Key, Value);
	}
	else if (Key == "ColorAndOpacity" || Key == "Text" || Key == "CheckedState" || Key == "BrushColor" || Key == "Brush")
	{
		bSuccess = GetLuaState()->SetPropertyFromLuaValue(Widget, *Key, Value);
	}

	if (bSuccess)
	{
		Widget->SynchronizeProperties();
	}

	return bSuccess;
}