#include "StreetMap.h"
#include "EditorFramework/AssetImportData.h"

UStreetMap::UStreetMap()
{
#if WITH_EDITORONLY_DATA
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		AssetImportData = NewObject<UAssetImportData>( this, TEXT( "AssetImportData" ) );
	}
#endif
}


void UStreetMap::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
#if WITH_EDITORONLY_DATA
	if( AssetImportData )
	{
		Context.AddTag(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif

	Super::GetAssetRegistryTags(Context);
}
