// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGGameInstanceBase.h"

/** 访问ChunkDownloader，以及一些用于管理资产和委托的有用工具 */
#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Serialization/JsonSerializerMacros.h"

#include "RPGAssetManager.h"
#include "RPGSaveGame.h"
#include "Items/RPGItem.h"
#include "Kismet/GameplayStatics.h"

URPGGameInstanceBase::URPGGameInstanceBase()
	: SaveSlot(TEXT("SaveGame"))
	, SaveUserIndex(0)
{}

void URPGGameInstanceBase::Init()
{
	Super::Init();

	/** 执行以下步骤可确保ChunkDownloader已初始化，准备开始下载内容，并告知其他函数清单的状态。 */

	// https://docs.unrealengine.com/5.3/en-US/hosting-a-manifest-and-assets-for-chunkdownloader-in-unreal-engine/
	/** 测试固定值 */
	// DefaultGame.ini
	// [/Script/Plugins.ChunkDownloader PatchingLive] +CdnBaseUrls=127.0.0.1/PatchingCDN
	/** 正式版本 */
	// 使用HTTP请求获取 ContentBuildID。该函数使用这些变量中的信息来向你的网站请求清单。
	const FString DeploymentName = "PatchingLive";
	const FString ContentBuildId = "PatchingKey";

	// 用选定平台初始化文件块下载器
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
	// 所需平台名称 TargetDownloadsInFlight赋予值 8 ，该值设置ChunkDownloader同时处理的最大下载数量。
	Downloader->Initialize("Windows", 8);

	// 加载缓存的版本ID 检查磁盘上是否已经下载文件，如果它们是最新清单文件，则ChunkDownloader可以跳过下载流程。
	Downloader->LoadCachedBuild(DeploymentName);

	// 更新版本清单文件
	// 输出操作成功或失败的回调
	TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {
		bIsDownloadManifestUpToDate = true;
	};
	// 下载清单文件的更新版本
	Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
}

void URPGGameInstanceBase::Shutdown()
{
	Super::Shutdown();

	// 关闭ChunkDownloader 停止当前正在进行的所有ChunkDownloader下载，然后清理并卸载该模块
	FChunkDownloader::Shutdown();
}

void URPGGameInstanceBase::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
	// 获取ChunkDownloader的引用
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	// 获取加载统计结构体
	FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

	// 获取已下载和要下载的的字节
	BytesDownloaded = LoadingStats.BytesDownloaded;
	TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

	// 获取已挂载文件块数和要下载的文件块数
	ChunksMounted = LoadingStats.ChunksMounted;
	TotalChunksToMount = LoadingStats.TotalChunksToMount;

	// 使用以上统计信息计算下载和挂载百分比
	DownloadPercent = ((float)BytesDownloaded / (float)TotalBytesToDownload) * 100.0f;
	MountPercent = ((float)ChunksMounted / (float)TotalChunksToMount) * 100.0f;
}

bool URPGGameInstanceBase::PatchGame()
{
	// 确保下载清单是最新的
	// 检查清单是否是当前最新的。如果你尚未初始化ChunkDownloader并成功获取清单的新副本，则 bIsDownloadManifestUpToDate 将为false，并且此函数将返回false，表示无法开始补丁。
	if (bIsDownloadManifestUpToDate)
	{
		// 获取文件块下载器 引用
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		// 报告当前文件块状态
		// 遍历下载列表并检查每个文件块的状态。
		for (int32 ChunkID : ChunkDownloadList)
		{
			int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
			UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
		}

		TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {
			// 当每个单独的文件块完成下载时，将调用 当每个文件块成功下载或下载失败时，它将输出一条消息。
			OnDownloadComplete(bSuccess);
		};
		// 开始下载所需文件块，这些文件块在 ChunkDownloadList 中列出。在调用此函数之前，必须用你想要的文件块ID填充此列表
		Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

		// 启动加载模式
		TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {
			// 所有文件块下载完毕后，就会触发
			OnLoadingModeComplete(bSuccess);
		};
		// 加载模式将告知ChunkDownloader开始监视其下载状态。
		// 可以在不调用加载模式的情况下在后台被动下载文件块，使用它将输出下载统计信息，使你可以创建一个可以跟踪用户下载进度的UI。
		// 下载整批文件块时，你还可以使用该回调函数运行特定功能。
		Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
		
		return true;
	}

	// 我们无法联系服务器验证清单，因此你无法修补
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

	return false;
}

// 清单更新完成后，此函数将用作异步回调。
void URPGGameInstanceBase::OnManifestUpdateComplete(bool bSuccess)
{
	// GameInstance可以全局识别此补丁阶段已完成
	bIsDownloadManifestUpToDate = bSuccess;
}

// 当打包文件已成功下载到用户的设备上时，它将运行。
void URPGGameInstanceBase::OnDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("Download Complete"));

		// 获取文件块下载器
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();
		// 用于发出的请求
		FJsonSerializableArrayInt DownloadedChunks;

		for (int32 ChunkID : ChunkDownloadList)
		{
			DownloadedChunks.Add(ChunkID);
		}

		// 挂载文件块
		// 输出是否已成功应用补丁
		TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {
			OnMountComplete(bSuccess);
		};
		// 开始挂载已下载文件块
		Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

		// 如果下载成功，则该函数激活值为true的 OnPatchComplete 委托
		OnPatchComplete.Broadcast(true);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Load process failed"));

		// 调用委托
		// 如果失败，则会以 false 值激活
		OnPatchComplete.Broadcast(false);
	}
}

void URPGGameInstanceBase::OnLoadingModeComplete(bool bSuccess)
{
	// 将传递给 OnDownloadComplete ，后者将在后续步骤中继续挂载文件块。
	OnDownloadComplete(bSuccess);
}

void URPGGameInstanceBase::OnMountComplete(bool bSuccess)
{
	// 将指示所有文件块均已完成挂载，并且内容可用。
	OnPatchComplete.Broadcast(bSuccess);
}

void URPGGameInstanceBase::AddDefaultInventory(URPGSaveGame* SaveGame, bool bRemoveExtra)
{
	// If we want to remove extra, clear out the existing inventory
	if (bRemoveExtra)
	{
		SaveGame->InventoryData.Reset();
	}

	// Now add the default inventory, this only adds if not already in hte inventory
	for (const TPair<FPrimaryAssetId, FRPGItemData>& Pair : DefaultInventory)
	{
		if (!SaveGame->InventoryData.Contains(Pair.Key))
		{
			SaveGame->InventoryData.Add(Pair.Key, Pair.Value);
		}
	}
}

bool URPGGameInstanceBase::IsValidItemSlot(FRPGItemSlot ItemSlot) const
{
	if (ItemSlot.IsValid())
	{
		const int32* FoundCount = ItemSlotsPerType.Find(ItemSlot.ItemType);

		if (FoundCount)
		{
			return ItemSlot.SlotNumber < *FoundCount;
		}
	}
	return false;
}

URPGSaveGame* URPGGameInstanceBase::GetCurrentSaveGame()
{
	return CurrentSaveGame;
}

void URPGGameInstanceBase::SetSavingEnabled(bool bEnabled)
{
	bSavingEnabled = bEnabled;
}

bool URPGGameInstanceBase::LoadOrCreateSaveGame()
{
	URPGSaveGame* LoadedSave = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SaveSlot, SaveUserIndex) && bSavingEnabled)
	{
		LoadedSave = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, SaveUserIndex));
	}

	return HandleSaveGameLoaded(LoadedSave);
}

bool URPGGameInstanceBase::HandleSaveGameLoaded(USaveGame* SaveGameObject)
{
	bool bLoaded = false;

	if (!bSavingEnabled)
	{
		// If saving is disabled, ignore passed in object
		SaveGameObject = nullptr;
	}

	// Replace current save, old object will GC out
	CurrentSaveGame = Cast<URPGSaveGame>(SaveGameObject);

	if (CurrentSaveGame)
	{
		// Make sure it has any newly added default inventory
		AddDefaultInventory(CurrentSaveGame, false);
		bLoaded = true;
	}
	else
	{
		// This creates it on demand
		CurrentSaveGame = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

		AddDefaultInventory(CurrentSaveGame, true);
	}

	OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	OnSaveGameLoadedNative.Broadcast(CurrentSaveGame);

	return bLoaded;
}

void URPGGameInstanceBase::GetSaveSlotInfo(FString& SlotName, int32& UserIndex) const
{
	SlotName = SaveSlot;
	UserIndex = SaveUserIndex;
}

bool URPGGameInstanceBase::WriteSaveGame()
{
	if (bSavingEnabled)
	{
		if (bCurrentlySaving)
		{
			// Schedule another save to happen after current one finishes. We only queue one save
			bPendingSaveRequested = true;
			return true;
		}

		// Indicate that we're currently doing an async save
		bCurrentlySaving = true;

		// This goes off in the background
		UGameplayStatics::AsyncSaveGameToSlot(GetCurrentSaveGame(), SaveSlot, SaveUserIndex, FAsyncSaveGameToSlotDelegate::CreateUObject(this, &URPGGameInstanceBase::HandleAsyncSave));
		return true;
	}
	return false;
}

void URPGGameInstanceBase::ResetSaveGame()
{
	// Call handle function with no loaded save, this will reset the data
	HandleSaveGameLoaded(nullptr);
}

void URPGGameInstanceBase::HandleAsyncSave(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	ensure(bCurrentlySaving);
	bCurrentlySaving = false;

	if (bPendingSaveRequested)
	{
		// Start another save as we got a request while saving
		bPendingSaveRequested = false;
		WriteSaveGame();
	}
}
