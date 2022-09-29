#pragma once

#include "ActionRPG.h"
#include "Engine/GameInstance.h"
#include "RPGGameInstanceBase.generated.h"

class URPGSaveGame;

/**
 * Base class for GameInstance, should be blueprinted
 * Most games will need to make a game-specific subclass of GameInstance
 * Once you make a blueprint subclass of your native subclass you will want to set it to be the default in project settings
 */
 UCLASS()
 class ACTOINRPG_API URPGGameInstanceBase : public UGameInstance
 {
    GENERATED_BODY()

public:
    // Constructor
    URPGGameInstanceBase();

    /** The slot name used for saving */
    UPROPERTY(BlueprintReadWrite, Category = Save)
    FString SaveSlot;

    /** The platform-specific user index */
    UPROPERTY(BlueprintReadWrite, Category = Save)
    int32 SaveUserIndex;
 };

