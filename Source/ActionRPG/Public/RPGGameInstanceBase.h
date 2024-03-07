// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Engine/GameInstance.h"
#include "RPGGameInstanceBase.generated.h"

class URPGItem;
class URPGSaveGame;

// 动态组播委托：该委托输出一个布尔值，该布尔值将告知你补丁下载操作是否成功。委托通常用于响应异步操作，例如下载或安装文件。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);

// 它是 GameInstance 的游戏特定子类，是大多数游戏所必需的。由于在整个游戏中只声明一个游戏实例，它很适合存储全局Gameplay数据。
/**
 * Base class for GameInstance, should be blueprinted
 * Most games will need to make a game-specific subclass of GameInstance
 * Once you make a blueprint subclass of your native subclass you will want to set it to be the default in project settings
 */
UCLASS()
class ACTIONRPG_API URPGGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()

public:
	// Constructor
	URPGGameInstanceBase();

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/** virtual function to allow custom GameInstances an opportunity to do cleanup when shutting down */
	virtual void Shutdown() override;

	UFUNCTION(BlueprintPure, Category="Patching|Stats")
	void GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

	// 启动游戏补丁过程。如果补丁清单不是最新的，则返回false
	// 此函数提供了蓝图的一种公开的补丁过程启动方式。它返回布尔值指示成功还是失败。这是下载管理和其他类型异步任务中的通用模式。
	UFUNCTION(BlueprintCallable, Category = "Patching")
	bool PatchGame();

	/**
	 * Adds the default inventory to the inventory array
	 * @param InventoryArray Inventory to modify
	 * @param RemoveExtra If true, remove anything other than default inventory
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void AddDefaultInventory(URPGSaveGame* SaveGame, bool bRemoveExtra = false);

	/** Returns true if this is a valid inventory slot */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool IsValidItemSlot(FRPGItemSlot ItemSlot) const;

	/** Returns the current save game, so it can be used to initialize state. Changes are not written until WriteSaveGame is called */
	UFUNCTION(BlueprintCallable, Category = Save)
	URPGSaveGame* GetCurrentSaveGame();

	/** Sets rather save/load is enabled. If disabled it will always count as a new character */
	UFUNCTION(BlueprintCallable, Category = Save)
	void SetSavingEnabled(bool bEnabled);

	/** Synchronously loads a save game. If it fails, it will create a new one for you. Returns true if it loaded, false if it created one */
	UFUNCTION(BlueprintCallable, Category = Save)
	bool LoadOrCreateSaveGame();

	/** Handle the final setup required after loading a USaveGame object using AsyncLoadGameFromSlot. Returns true if it loaded, false if it created one */
	UFUNCTION(BlueprintCallable, Category = Save)
	bool HandleSaveGameLoaded(USaveGame* SaveGameObject);

	/** Gets the save game slot and user index used for inventory saving, ready to pass to GameplayStatics save functions */
	UFUNCTION(BlueprintCallable, Category = Save)
	void GetSaveSlotInfo(FString& SlotName, int32& UserIndex) const;

	/** Writes the current save game object to disk. The save to disk happens in a background thread*/
	UFUNCTION(BlueprintCallable, Category = Save)
	bool WriteSaveGame();

	/** Resets the current save game to it's default. This will erase player data! This won't save to disk until the next WriteSaveGame */
	UFUNCTION(BlueprintCallable, Category = Save)
	void ResetSaveGame();

public:

	// 委托
	// 补丁过程成功或失败时触发 提供了一个在补丁操作完成后与蓝图挂接的位置
	UPROPERTY(BlueprintAssignable, Category = "Patching")
	FPatchCompleteDelegate OnPatchComplete;

	/** List of inventory items to add to new players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FPrimaryAssetId, FRPGItemData> DefaultInventory;

	/** Number of slots for each type of item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FPrimaryAssetType, int32> ItemSlotsPerType;

	/** The slot name used for saving */
	UPROPERTY(BlueprintReadWrite, Category = Save)
	FString SaveSlot;

	/** The platform-specific user index */
	UPROPERTY(BlueprintReadWrite, Category = Save)
	int32 SaveUserIndex;

	/** Delegate called when the save game has been loaded/reset */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnSaveGameLoaded OnSaveGameLoaded;

	/** Native delegate for save game load/reset */
	FOnSaveGameLoadedNative OnSaveGameLoadedNative;

protected:

	// 要尝试和下载的文件块ID列表
	// 将使用此列表保存后续要下载的所有文件块ID。在开发设置中，可以根据需要使用资产列表对此进行初始化。但是出于测试目的，只需公开默认值，以使用 蓝图编辑器 填写它们。
	UPROPERTY(EditDefaultsOnly, Category="Patching")
	TArray<int32> ChunkDownloadList;

	/** The current save game object */
	UPROPERTY()
	URPGSaveGame* CurrentSaveGame;

	/** Rather it will attempt to actually save to disk */
	UPROPERTY()
	bool bSavingEnabled;

	/** True if we are in the middle of doing a save */
	UPROPERTY()
	bool bCurrentlySaving;

	/** True if another save was requested during a save */
	UPROPERTY()
	bool bPendingSaveRequested;

	/** 用我们网站上托管的清单文件追踪本地清单文件是否为最新文件。*/
	bool bIsDownloadManifestUpToDate;


protected:
	/** Called when the async save happens */
	virtual void HandleAsyncSave(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	/** 在文件块下载进程完成时调用 */
	void OnManifestUpdateComplete(bool bSuccess);

	// 在文件块下载进程完成时调用
	void OnDownloadComplete(bool bSuccess);

	// ChunkDownloader加载模式完成时调用
	void OnLoadingModeComplete(bool bSuccess);

	// ChunkDownloader完成挂载文件块时调用
	void OnMountComplete(bool bSuccess);
};