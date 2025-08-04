// Fill out your copyright notice in the Description page of Project Settings.


#include "LuaUserWidget.h"
#include "LuaProxyWidget.h"
#include "Blueprint/WidgetTree.h"

FLuaValue ULuaUserWidget::LuaMetaMethodToString_Implementation()
{
	return FString::Printf(TEXT("LuaUserWidget@%p"), this);
}

FLuaValue ULuaUserWidget::LuaMetaMethodIndex_Implementation(const FString& Key)
{
	if (Key.StartsWith("Create"))
	{
		const FString WidgetClassName = Key.Mid(6);

		UClass* WidgetClass = FindFirstObject<UClass>(*WidgetClassName);
		if (!WidgetClass)
		{
			UE_LOG(LogLuaMachine, Error, TEXT("%s is an invalid UWidget class name"), *WidgetClassName);
			return FLuaValue();
		}

		if (!WidgetClass->IsChildOf<UWidget>())
		{
			UE_LOG(LogLuaMachine, Error, TEXT("%s is not a UWidget"), *WidgetClassName);
			return FLuaValue();
		}

		FLuaValue CreateNewWidget = FLuaValue([this, WidgetClass](TArray<FLuaValue>) -> FLuaValueOrError {
			UWidget* NewWidget = WidgetTree->ConstructWidget<UWidget>(WidgetClass);
			ULuaProxyWidget* NewProxyWidget = NewObject<ULuaProxyWidget>(OwningLuaState);
			NewProxyWidget->Widget = NewWidget;
			Proxies.Add(NewProxyWidget);
			return FLuaValue(NewProxyWidget);
			});

		return CreateNewWidget;

	}
	else if (Key == "SetRoot")
	{
		FLuaValue SetRootWidget = FLuaValue([this](TArray<FLuaValue> LuaArgs) -> FLuaValueOrError {
			if (!LuaArgs.IsValidIndex(0) || !LuaArgs[0].Object || !LuaArgs[0].Object->IsA<ULuaProxyWidget>())
			{
				return FString("Expected first argument to be a widget");
			}

			if (WidgetTree->RootWidget)
			{
				RemoveFromParent();
			}
			WidgetTree->RootWidget = Cast<ULuaProxyWidget>(LuaArgs[0].Object)->Widget;
			AddToViewport();
			return FLuaValue();
			});

		return SetRootWidget;
	}

	return FLuaValue();
}