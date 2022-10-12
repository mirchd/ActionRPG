#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

// 一个简单的C++加载屏幕，用于在首次加载游戏或转换地图时显示纹理。由于需要在加载主要ARPG游戏模块之前先加载，它是一个单独的模块。

/** Module interface for this game's loading screens */
class IActionRPGLoadingScreenModule : public IModuleInterface
{
public:
	/** Loads the module so it can be turned on */
	static inline IActionRPGLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IActionRPGLoadingScreenModule>("ActionRPGLoadingScreen");
	}

	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen(bool bPlayUntilStopped, float PlayTime) = 0;

	/** Stops the loading screen */
	virtual void StopInGameLoadingScreen() = 0;
};
