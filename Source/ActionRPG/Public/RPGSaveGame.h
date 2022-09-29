#pragma once

#include "ActionRPG.h"
#include "GameFramework/SaveGame.h"
#include "RPGSaveGame.generated.h"

/** List of versions, native code will handle fixups for any old versions */
namespace ERPGSaveGameVersion
{
	enum type
	{
		// Initial version
		Initial,
		// Added Inventory
		AddedInventory,
		// Added ItemData to store count/level
		AddedItemData,

		// -----<new versions must be added before this line>-------------------------------------------------
		VersionPlusOne,
		LastestVersion = VersionPlusOne - 1
	};
}

/** Object that is written to and read from the save game archive, with a data version */
UCLASS(BlueprintType)
class ACTIONRPG_API URPGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGSaveGame()
	{

	}

protected:
	/** Deprecated way of storing items, this is read in but not saved out */
	
};

