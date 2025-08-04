// Fill out your copyright notice in the Description page of Project Settings.


#include "LuaUMGBlueprintPackage.h"
#include "LuaUserWidget.h"

ULuaUMGBlueprintPackage::ULuaUMGBlueprintPackage()
{
	Table.Add("create_user_widget", FLuaValue::Function(GET_FUNCTION_NAME_CHECKED(ULuaUMGBlueprintPackage, CreateUserWidget)));
	Table.Add("load_texture_as_brush", FLuaValue::Function(GET_FUNCTION_NAME_CHECKED(ULuaUMGBlueprintPackage, LoadTextureAsBrush)));
}

FLuaValue ULuaUMGBlueprintPackage::CreateUserWidget()
{
	UWorld* CurrentWorld = GetLuaStateInstance()->GetWorld();
	if (!CurrentWorld)
	{
		return FLuaValue();
	}
	ULuaUserWidget* NewUserWidget = CreateWidget<ULuaUserWidget>(CurrentWorld);
	NewUserWidget->OwningLuaState = GetLuaStateInstance();
	return NewUserWidget;
}

FLuaValue ULuaUMGBlueprintPackage::LoadTextureAsBrush(FLuaValue TexturePath)
{
	const FString TexturePathString = TexturePath.ToString();
	UObject* TextureObject = StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePathString);
	if (!TextureObject)
	{
		return FLuaValue();
	}

	UTexture2D* Texture = Cast<UTexture2D>(TextureObject);
	if (!Texture)
	{
		return FLuaValue();
	}

	const FIntPoint TextureSize = Texture->GetImportedSize();

	FSlateImageBrush ImageBrush(Texture, FVector2D(TextureSize.X, TextureSize.Y));

	return GetLuaStateInstance()->StructToLuaValue(ImageBrush);
}