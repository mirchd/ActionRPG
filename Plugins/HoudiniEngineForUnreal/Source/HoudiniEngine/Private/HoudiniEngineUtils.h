/*
* Copyright (c) <2021> Side Effects Software Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. The name of Side Effects Software may not be used to endorse or
*    promote products derived from this software without specific prior
*    written permission.
*
* THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
* NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "HAPI/HAPI_Common.h"
#include "HoudiniEnginePrivatePCH.h"
#include "EngineUtils.h"
#include "Misc/Optional.h"
#include <string>

#include "HoudiniGenericAttribute.h"
#include "HoudiniOutput.h"
#include "HoudiniPackageParams.h"
#include "Containers/UnrealString.h"
#include "HoudiniEngineString.h"

class FString;
class UStaticMesh;
class UHoudiniAsset;
class UHoudiniAssetComponent;

struct FHoudiniPartInfo;
struct FHoudiniMeshSocket;
struct FHoudiniGeoPartObject;

struct FRawMesh;

enum class EHoudiniCurveType : int8;
enum class EHoudiniCurveMethod : int8;
enum class EHoudiniInstancerType : uint8;

#define H_DEPRECATED_OLD_ATTRIBUTE_API(Version, Message)  [[deprecated(Message " Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.")]]

class FHoudiniParameterWidgetMetaData : public ISlateMetaData
{
public:
	SLATE_METADATA_TYPE(FHoudiniParameterWidgetMetaData, ISlateMetaData)

	FHoudiniParameterWidgetMetaData(const FString& UniqueName, const uint32 Index)
		: UniqueName(UniqueName)
		, Index(Index)
	{
	}

	bool operator==(const FHoudiniParameterWidgetMetaData& Other) const
	{
		return UniqueName == Other.UniqueName && Index == Other.Index;
	}

	const FString UniqueName;
	const uint32 Index;
};

struct HOUDINIENGINE_API FHoudiniEngineUtils
{
	friend struct FUnrealMeshTranslator;

	public:

		// Force deletes an Unreal Object without throwing up warning dialogs.
		static void ForceDeleteObject(UObject* Object);

		// Load libHAPI and return handle to it, also store location of loaded libHAPI in passed argument.
		static void* LoadLibHAPI(FString& StoredLibHAPILocation);

		// Return true if module has been properly initialized.
		static bool IsInitialized();

		// Return type of license used.
		static bool GetLicenseType(FString & LicenseType);

		// Cook the specified node id
		// if the cook options are null, the defualt one will be used
		// if bWaitForCompletion is true, this call will be blocking until the cook is finished
		static bool HapiCookNode(const HAPI_NodeId& InNodeId, HAPI_CookOptions* InCookOptions = nullptr, const bool& bWaitForCompletion = false);

		// Wrapper for CommitGeo - adds a profiler scope wrapper
		static HAPI_Result HapiCommitGeo(const HAPI_NodeId& InNodeId);

		// Return a specified HAPI status string.
		static const FString GetStatusString(HAPI_StatusType status_type, HAPI_StatusVerbosity verbosity);

		// HAPI : Return the string that corresponds to the given string handle.
		static FString HapiGetString(int32 StringHandle);

		// Return a string representing cooking result.
		static const FString GetCookResult();

		// Return a string indicating cook state.
		static const FString GetCookState();

		// Return a string error description.
		static const FString GetErrorDescription();

		// Return a string description of error from a given error code.
		static const FString GetErrorDescription(HAPI_Result Result);

		// Return a string description for a Houdini Engine session connection error.
		static const FString GetConnectionError();

		// Helper function used to indicate to all HAC that they need to be instantiated in the new HE session
		// Needs to be call after starting/restarting/connecting/session syncing a HE session..
		static void MarkAllHACsAsNeedInstantiation();

		// Return the errors, warning and messages on a specified node
		static const FString GetNodeErrorsWarningsAndMessages(const HAPI_NodeId& InNodeId);

		static const FString GetCookLog(TArray<UHoudiniAssetComponent*>& InHACs);

		static const FString GetAssetHelp(UHoudiniAssetComponent* HoudiniAssetComponent);

		static const FString GetAssetHelpURL(UHoudiniAssetComponent* HoudiniAssetComponent);

		// Updates the Object transform of a Houdini Asset Component
		static bool UploadHACTransform(UHoudiniAssetComponent* HAC);

		// Convert FString to std::string
		static void ConvertUnrealString(const FString & UnrealString, std::string& String);

		// Wrapper for the CreateNode function
		// As HAPI_CreateNode is an async call, this function actually waits for the node creation to be done before returning
		static HAPI_Result CreateNode(
			const HAPI_NodeId& InParentNodeId, 
			const FString& InOperatorName,
			const FString& InNodeLabel,
			const HAPI_Bool& bInCookOnCreation, 
			HAPI_NodeId* OutNewNodeId);

		static int32 HapiGetCookCount(const HAPI_NodeId& InNodeId);

		// HAPI : Retrieve the asset node's object transform. **/
		static bool HapiGetAssetTransform(const HAPI_NodeId& InNodeId, FTransform& OutTransform);

		// HAPI : Translate HAPI transform to Unreal one.
		static void TranslateHapiTransform(const HAPI_Transform & HapiTransform, FTransform & UnrealTransform);

		// HAPI : Translate HAPI Euler transform to Unreal one.
		static void TranslateHapiTransform(const HAPI_TransformEuler & HapiTransformEuler, FTransform & UnrealTransform);

		// HAPI : Translate Unreal transform to HAPI one.
		static void TranslateUnrealTransform(const FTransform & UnrealTransform, HAPI_Transform & HapiTransform);

		// HAPI : Translate Unreal transform to HAPI Euler one.
		static void TranslateUnrealTransform(const FTransform & UnrealTransform, HAPI_TransformEuler & HapiTransformEuler);
		
		// Translate an array of float position values from Houdini to Unreal
		static void ConvertHoudiniPositionToUnrealVector(const TArray<float>& InRawData, TArray<FVector>& OutVectorData);
		static FVector3f ConvertHoudiniPositionToUnrealVector3f(const FVector3f& InVector);

		// Translate an array of float scale values from Houdini to Unreal
		static void ConvertHoudiniScaleToUnrealVector(const TArray<float>& InRawData, TArray<FVector>& OutVectorData);

		// Translate an array of float quaternion rotation values from Houdini to Unreal
		static void ConvertHoudiniRotQuatToUnrealVector(const TArray<float>& InRawData, TArray<FVector>& OutVectorData);

		// Translate an array of float euler rotation values from Houdini to Unreal
		static void ConvertHoudiniRotEulerToUnrealVector(const TArray<float>& InRawData, TArray<FVector>& OutVectorData);

		// Return true if asset is valid.
		static bool IsHoudiniNodeValid(const HAPI_NodeId& AssetId);

		// HAPI : Retrieve HAPI_ObjectInfo's from given asset node id.
		static bool HapiGetObjectInfos(const HAPI_NodeId& InNodeId, TArray<HAPI_ObjectInfo>& OutObjectInfos, TArray<HAPI_Transform>& OutObjectTransforms);

		// Traverse from the Child up to the Root node to determine whether the ChildNode is fully visible
		// inside the RootNode.
		// - The Obj node itself is visible
		// - All parent nodes are visible
		// - Only has Object subnet parents (if we find a parent with non-Object nodetype then it's not visible).
		static bool IsObjNodeFullyVisible(const TSet<HAPI_NodeId>& AllObjectIds, const HAPI_NodeId& RootNodeId, const HAPI_NodeId& ChildNodeId);

		static bool IsSopNode(const HAPI_NodeId& NodeId);
		
		static bool ContainsSopNodes(const HAPI_NodeId& NodeId);

		// Get the output index of InNodeId (assuming InNodeId is an Output node).
		// This is done by getting the value of the outputidx parameter on
		// InNodeId.
		// Returns false if outputidx could not be found/read. Sets OutOutputIndex to the
		// value of the outputidx parameter.
		static bool GetOutputIndex(const HAPI_NodeId& InNodeId, int32& OutOutputIndex);

		static bool GatherAllAssetOutputs(
			HAPI_NodeId InAssetId,
			bool bUseOutputNodes,
			bool bOutputTemplatedGeos,
			bool bGatherEditableCurves,
			TArray<HAPI_NodeId>& OutOutputNodes); 

		// Get the immediate output geo infos for the given Geometry object network.
		// Find immediate Display and output nodes (if enabled).
	    // If bIgnoreOutputNodes is false, only Display nodes will be retrieved.
		// If bIgnoreOutputNodes is true, any output nodes will take precedence over display nodes.
		static bool GatherImmediateOutputGeoInfos(
			const int& InNodeId,
			const bool bUseOutputNodes,
			const bool bGatherTemplateNodes,
			TArray<HAPI_GeoInfo>& OutGeoInfos,
			TSet<HAPI_NodeId>& OutForceNodesCook);

		// HAPI: Retrieve absolute path to the given Node
		static bool HapiGetAbsNodePath(const HAPI_NodeId& InNodeId, FString& OutPath);

		// HAPI: Retrieve Path to the given Node, relative to the given Node
		static bool HapiGetNodePath(const HAPI_NodeId& InNodeId, const HAPI_NodeId& InRelativeToNodeId, FString& OutPath);

		// HAPI: Retrieve the relative for the given HGPO Node
		static bool HapiGetNodePath(const FHoudiniGeoPartObject& InHGPO, FString& OutPath);

		// HAPI : Return all group names for a given Geo. 
		static bool HapiGetGroupNames(
			const HAPI_NodeId GeoId, 
			const HAPI_PartId PartId,
			const HAPI_GroupType GroupType, 
			bool isPackedPrim,
			TArray<FString>& OutGroupNames );

		// HAPI : Retrieve group membership.
		static bool HapiGetGroupMembership(
			const HAPI_NodeId& GeoId, const HAPI_PartInfo& PartInfo,
			const HAPI_GroupType& GroupType, const FString & GroupName,
			TArray<int32>& OutGroupMembership, bool& OutAllEquals);

		static bool HapiGetGroupMembership(
			HAPI_NodeId GeoId, HAPI_PartId PartId,
			const HAPI_GroupType& GroupType, const FString& GroupName,
			int32 & OutGroupMembership, int Start = 0, int Length = 1);

		// HAPI : Given vertex list, retrieve new vertex list for a specified group.
		// Return number of processed valid index vertices for this split.
		static int32 HapiGetVertexListForGroup(
			const HAPI_NodeId& GeoId,
			const HAPI_PartInfo& PartInfo,
			const FString& GroupName,
			const TArray<int32>& FullVertexList,
			TArray<int32>& NewVertexList,
			TArray<int32>& AllVertexList,
			TArray<int32>& AllFaceList,
			TArray<int32>& AllGroupFaceIndices,
			int32& FirstValidVertex,
			int32& FirstValidPrim,
			const bool& isPackedPrim);

		// HAPI : Get attribute data as float.
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static bool HapiGetAttributeDataAsFloat(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char * InAttribName,
			HAPI_AttributeInfo& OutAttributeInfo,
			TArray<float>& OutData,
			int32 InTupleSize = 0,
			HAPI_AttributeOwner InOwner = HAPI_ATTROWNER_INVALID,
			const int32& InStartIndex = 0,
			const int32& InCount = -1);

		// HAPI : Get attribute data as Integer.
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static bool HapiGetAttributeDataAsInteger(
			const HAPI_NodeId InGeoId,
			const HAPI_PartId InPartId,
			const char * InAttribName,
			HAPI_AttributeInfo& OutAttributeInfo,
			TArray<int32>& OutData,
			const int32 InTupleSize = 0,
			const HAPI_AttributeOwner& InOwner = HAPI_ATTROWNER_INVALID,
			const int32 InStartIndex = 0,
			const int32 InCount = -1);

		// HAPI : Get attribute data as strings.
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static bool HapiGetAttributeDataAsString(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char * InAttribName,
			HAPI_AttributeInfo& OutAttributeInfo,
			TArray<FString>& OutData,
			int32 InTupleSize = 0,
			HAPI_AttributeOwner InOwner = HAPI_ATTROWNER_INVALID,
			const int32& InStartIndex = 0,
			const int32& InCount = -1);

		// HAPI : Get attribute data as strings.
		static bool HapiGetAttributeDataAsStringFromInfo(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char * InAttribName,
			HAPI_AttributeInfo& InAttributeInfo,
			TArray<FString>& OutData,
			const int32& InStartIndex = 0,
			const int32& InCount = -1);

		// HAPI : Check if given attribute exists.
		static bool HapiCheckAttributeExists(
			const HAPI_NodeId& GeoId,
			const HAPI_PartId& PartId,
			const char * AttribName,
			HAPI_AttributeOwner Owner = HAPI_ATTROWNER_INVALID);

		// HAPI: Returns all the attributes of a given type for a given owner
		static int32 HapiGetAttributeOfType(
			const HAPI_NodeId& GeoId,
			const HAPI_NodeId& PartId,
			const HAPI_AttributeOwner& AttributeOwner,
			const HAPI_AttributeTypeInfo& AttributeType,
			TArray<HAPI_AttributeInfo>& MatchingAttributesInfo,
			TArray<FString>& MatchingAttributesName);

		// HAPI: Gets either a int or a int array
		static bool HapiGetAttributeIntOrIntArray(
			const HAPI_NodeId& GeoId,
			const HAPI_NodeId& PartId,
			const FString & AttribName,
			const HAPI_AttributeOwner& AttributeOwner,
			HAPI_AttributeInfo& OutAttributeInfo,
			TArray<int32>& OutData
		);
	
		// HAPI: Gets either a float or a float array
		static bool HapiGetAttributeFloatOrFloatArray(
			const HAPI_NodeId& GeoId,
			const HAPI_NodeId& PartId,
			const FString & AttribName,
			const HAPI_AttributeOwner& AttributeOwner,
			HAPI_AttributeInfo& OutAttributeInfo,
			TArray<float>& OutData
		);

		// Retreives the first value of an attribute. OutData is left unchanged
		// if there is an error, so you can initialize it with a default.
		static bool HapiGetFirstAttributeValueAsInteger(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char* InAttribName,
			const HAPI_AttributeOwner InAttribOwner,
			int32 & OutData);

		// Retreives the first value of an attribute. OutData is left unchanged
		// if there is an error.
		static bool HapiGetFirstAttributeValueAsFloat(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char* InAttribName,
			const HAPI_AttributeOwner InAttribOwner,
			float& OutData);

		// Retreives the first value of an attribute. OutData is left unchanged
		// if there is an error.
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static bool HapiGetFirstAttributeValueAsString(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const char* InAttribName,
			const HAPI_AttributeOwner InAttribOwner,
			FString& OutData);


		// HAPI : Look for a parameter by name and returns its index. Returns -1 if not found.
		static HAPI_ParmId HapiFindParameterByName(
			const HAPI_NodeId& InNodeId, const std::string& InParmName, HAPI_ParmInfo& OutFoundParmInfo);

		// HAPI : Look for a parameter by tag and returns its index. Returns -1 if not found.
		static HAPI_ParmId HapiFindParameterByTag(
			const HAPI_NodeId& InNodeId, const std::string& InParmTag, HAPI_ParmInfo& OutFoundParmInfo);

		// Returns true is the given Geo-Part is an attribute instancer
		static bool IsAttributeInstancer(
			const HAPI_NodeId& GeoId, const HAPI_PartId& PartId, EHoudiniInstancerType& OutInstancerType);

		static bool IsValidDataTable(
			const HAPI_NodeId& GeoId, const HAPI_PartId& PartId);

		// Returns true if the given Geo-Part is a landscape spline
		static bool IsLandscapeSpline(const HAPI_NodeId& GeoId, const HAPI_PartId& PartId);

		// HAPI : Return a give node's parent ID, -1 if none
		static HAPI_NodeId HapiGetParentNodeId(const HAPI_NodeId& NodeId);

		// HAPI : Marshaling, disconnect input asset from a given slot.
		static bool HapiDisconnectAsset(HAPI_NodeId HostAssetId, int32 InputIndex);

		// Destroy asset, returns the status.
		static bool DestroyHoudiniAsset(const HAPI_NodeId& AssetId);

		// Deletes the specified HAPI node by id.
		static bool DeleteHoudiniNode(const HAPI_NodeId& InNodeId);

		// Loads an HDA file and returns its AssetLibraryId
		static bool LoadHoudiniAsset(
			const UHoudiniAsset * HoudiniAsset,
			HAPI_AssetLibraryId & OutAssetLibraryId);
		
		// Returns the name of the available subassets in a loaded HDA
		static bool GetSubAssetNames(
			const HAPI_AssetLibraryId& AssetLibraryId,
			TArray< HAPI_StringHandle > & OutAssetNames);

		static bool OpenSubassetSelectionWindow(
			TArray<HAPI_StringHandle>& AssetNames, HAPI_StringHandle& OutPickedAssetName );

		// Returns the name of a Houdini asset.
		static bool GetHoudiniAssetName(const HAPI_NodeId& AssetNodeId, FString & NameString);

		// Gets preset data for a given asset.
		static bool GetAssetPreset(const HAPI_NodeId& AssetNodeId, TArray<int8>& PresetBuffer);

		// HAPI : Set asset transform.
		static bool HapiSetAssetTransform(const HAPI_NodeId& AssetNodeId, const FTransform & Transform);

		// TODO: Move me somewhere else
		static void AssignUniqueActorLabelIfNeeded(UHoudiniAssetComponent* HAC);

		// Triggers an update the details panel
		// Will use an AsyncTask if we're not in the game thread
		// NOTE: Prefer using IDetailLayoutBuilder::ForceRefreshDetails() instead.
		static void UpdateEditorProperties(const bool bInForceFullUpdate);

		// Triggers an update the details panel
		static void UpdateBlueprintEditor(UHoudiniAssetComponent* HAC);

		// Check if the Houdini asset component is being cooked
		static bool IsHoudiniAssetComponentCooking(UObject* InObj);

		// Helper function to set float attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeFloatData(
			const TArray<float>& InFloatData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo,
			bool bAttemptRunLengthEncoding = false);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeFloatData(
			const float* InFloatData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo,
			bool bAttemptRunLengthEncoding = false);

		// Helper function for setting unique float values
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeFloatUniqueData(
			const float InFloatData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set Int attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeIntData(
			const TArray<int32>& InIntData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo,
            bool bAttemptRunLengthEncoding = false);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeIntData(
			const int32* InIntData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo,
            bool bAttemptRunLengthEncoding = false);

		// Helper function for setting unique int values
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeIntUniqueData(
			const int32 InIntData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set unsigned Int attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUIntData(
			const TArray<int64>& InIntData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUIntData(
			const int64* InIntData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set signed int8 attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt8Data(
			const TArray<int8>& InByteData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt8Data(
			const int8* InByteData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set Byte attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUInt8Data(
			const TArray<uint8>& InByteData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUInt8Data(
			const uint8* InByteData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set signed int16 attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt16Data(
			const TArray<int16>& InShortData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt16Data(
			const int16* InShortData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set uint16 attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUInt16Data(
			const TArray<int32>& InShortData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUInt16Data(
			const int32* InShortData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set Int64 attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt64Data(
			const TArray<int64>& InInt64Data,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeInt64Data(
			const int64* InInt64Data,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set unsigned Int64 attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeUInt64Data(
			const TArray<int64>& InInt64Data,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set Double attribute data
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeDoubleData(
			const TArray<double>& InDoubleData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeDoubleData(
			const double* InDoubleData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set Vertex Lists
		// The data will be sent in chunks if too large for thrift
		static HAPI_Result HapiSetVertexList(
			const TArray<int32>& InVertexListData,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId);

		// Helper function to set Face Counts
		// The data will be sent in chunks if too large for thrift
		static HAPI_Result HapiSetFaceCounts(
			const TArray<int32>& InFaceCounts,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId);

		// Helper function to set attribute string data for a single FString
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeStringUniqueData(
			const FString& InString,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		// Helper function to set attribute string data for a FString array
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeStringData(
			const TArray<FString>& InStringArray,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeStringMap(
			const FHoudiniEngineIndexedStringMap& InIndexedStringMap,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);

		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeStringArrayData(
			const TArray<FString>& InStringArray,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo,
			const TArray<int>& SizesFixedArray);

		// Helper function to set attribute dict data for a FString array
		// The data will be sent in chunks if too large for thrift
		H_DEPRECATED_OLD_ATTRIBUTE_API(20.5, "Use FHoudiniHapiAccessor instead.")
		static HAPI_Result HapiSetAttributeDictionaryData(
			const TArray<FString>& InStringArray,
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const FString& InAttributeName,
			const HAPI_AttributeInfo& InAttributeInfo);


		// Helper function to set Heightfield data
		// The data will be sent in chunks if too large for thrift
		static HAPI_Result HapiSetHeightFieldData(
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			const TArray<float>& InFloatValues,
			const FString& InHeightfieldName);

		static bool HapiGetParameterDataAsString(
			const HAPI_NodeId& NodeId,
			const std::string& ParmName,
			const FString& DefaultValue,
			FString& OutValue);

		static bool HapiGetParameterDataAsInteger(
			const HAPI_NodeId& NodeId, 
			const std::string& ParmName,
			const int32& DefaultValue,
			int32 & OutValue);

		static bool HapiGetParameterDataAsFloat(
			const HAPI_NodeId& NodeId,
			const std::string& ParmName,
			const float& DefaultValue,
			float& OutValue);

		// Returns a list of all the generic attributes for a given attribute owner
		static int32 GetGenericAttributeList(
			const HAPI_NodeId& InGeoNodeId,
			const HAPI_PartId& InPartId,
			const FString& InGenericAttributePrefix,
			TArray<FHoudiniGenericAttribute>& OutFoundAttributes,
			const HAPI_AttributeOwner& AttributeOwner,
			const int32& InAttribIndex = -1);

		// Helper functions for generic property attributes
		static bool GetGenericPropertiesAttributes(
			const HAPI_NodeId& InGeoNodeId,
			const HAPI_PartId& InPartId,
			const bool InFindDetailAttributes, // if true, find default attributes
			const int32& InFirstValidPrimIndex, // If not INDEX_NONE, look for primitive attribute
			const int32& InFirstValidVertexIndex, // If this is not INDEX_NONE, look for vertex attribute
			const int32& InFirstValidPointIndex, // If this is not INDEX_NONE, look for point attribute
			TArray<FHoudiniGenericAttribute>& OutPropertyAttributes);

		static bool UpdateGenericPropertiesAttributes(
			UObject* InObject,
			const TArray<FHoudiniGenericAttribute>& InAllPropertyAttributes,
			const int32& AtIndex = 0,
			const bool bInDeferPostEditChangePropertyCalls=false,
			const FHoudiniGenericAttribute::FFindPropertyFunctionType& InProcessFunction=nullptr);

		// Helper function for setting a generic attribute on geo (UE -> HAPI)
		static bool SetGenericPropertyAttribute(
			const HAPI_NodeId& InGeoNodeId,
			const HAPI_PartId& InPartId,
			const FHoudiniGenericAttribute& InPropertyAttribute);

		// Helper functions to retrieve the default tag values from the actor CDO.
		static TArray<FName> GetDefaultActorTags(const AActor* InActor);
		// Helper functions to retrieve the default tag values from the component CDO.
		static TArray<FName> GetDefaultComponentTags(const UActorComponent* InComponent);
	
		// Helper to add actor tags from the generic attributes and to the OutActorTags array.
		// The ApplyTags* helpers are typically used during Bake phases to reapply tags since the structure
		// of outputs typically change.
		static void ApplyTagsToActorOnly(const TArray<FHoudiniGenericAttribute>& GenericPropertyAttributes, TArray<FName>& OutActorTags);
		static void ApplyTagsToActorAndComponents(AActor* InActor, bool bKeepActorTags, const TArray<FHoudiniGenericAttribute>& GenericPropertyAttributes);

		// Helpers to check whether KeepTags is enabled in any of the HGPOs
		static bool IsKeepTagsEnabled(const TArray<FHoudiniGeoPartObject>& InHGPOs);
		static bool IsKeepTagsEnabled(const FHoudiniGeoPartObject* InHGPO);
	
		// Helper to clear a component's tags based on the KeepTags settings in a set of HGPOs
		static void KeepOrClearComponentTags(UActorComponent* ActorComponent, const TArray<FHoudiniGeoPartObject>& InHGPOs);
		static void KeepOrClearComponentTags(UActorComponent* ActorComponent, const FHoudiniGeoPartObject* InHGPO);
		static void KeepOrClearComponentTags(UActorComponent* ActorComponent, bool bKeepTags);
	
		// Helper to reset all the actor component's tags based on the KeepTags settings in a set of HGPOs
		static void KeepOrClearActorTags(AActor* Actor, bool bApplyToActor, bool bApplyToComponents, const FHoudiniGeoPartObject* InHGPO);

		/*
		// Tries to update values for all the UProperty attributes to apply on the object.
		static void ApplyUPropertyAttributesOnObject(
			UObject* MeshComponent, const TArray< UGenericAttribute >& UPropertiesToModify );
		*/
		/*
		static bool TryToFindInStructProperty(
			UObject* Object, FString UPropertyNameToFind, UStructProperty* ArrayProperty, UProperty*& FoundProperty, void*& StructContainer );
		*/		
		/*
		static bool TryToFindInArrayProperty(
			UObject* Object, FString UPropertyNameToFind, UArrayProperty* ArrayProperty, UProperty*& FoundProperty, void*& StructContainer );
		*/

		static void AddHoudiniMetaInformationToPackage(
			UPackage* Package, UObject* Object, const FString& Key, const FString& Value);

		// Adds the HoudiniLogo mesh to a Houdini Asset Component
		static bool AddHoudiniLogoToComponent(UHoudiniAssetComponent* HAC);

		// Removes the default Houdini logo mesh from a HAC
		static bool RemoveHoudiniLogoFromComponent(UHoudiniAssetComponent* HAC);

		// Indicates if a HAC has the Houdini logo mesh
		static bool HasHoudiniLogo(UHoudiniAssetComponent* HAC);

		// 
		static HAPI_PartInfo ToHAPIPartInfo(const FHoudiniPartInfo& InHPartInfo);

		//
		static int32 AddMeshSocketsToArray_Group(
			const HAPI_NodeId& GeoId, const HAPI_PartId& PartId,
			TArray<FHoudiniMeshSocket>& AllSockets, const bool& isPackedPrim);

		//
		static int32 AddMeshSocketsToArray_DetailAttribute(
			const HAPI_NodeId& GeoId, const HAPI_PartId& PartId,
			TArray<FHoudiniMeshSocket>& AllSockets, const bool& isPackedPrim);

		static bool AddMeshSocketsToStaticMesh(
			UStaticMesh* StaticMesh,
			TArray<FHoudiniMeshSocket >& AllSockets,
			const bool& CleanImportSockets);

		// 
		static bool CreateGroupsFromTags(
			const HAPI_NodeId& NodeId, const HAPI_PartId& PartId, const TArray<FName>& Tags);

		//
		static bool CreateAttributesFromTags(
			const HAPI_NodeId& NodeId, const HAPI_PartId& PartId, const TArray<FName>& Tags);

		// Helper function to access the "unreal_level_path" attribute
		static bool GetLevelPathAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutLevelPath,
			HAPI_AttributeOwner InAttributeOwner=HAPI_AttributeOwner::HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the "unreal_level_path" attribute
		static bool GetLevelPathAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& OutLevelPath,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Helper function to access the custom output name attribute
		static bool GetOutputNameAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutOutputName,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the custom output name attribute
		static bool GetOutputNameAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& OutOutputName,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Helper function to access the custom bake name attribute
		static bool GetBakeNameAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutBakeName,
			const HAPI_AttributeOwner& InAttribOwner = HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the custom bake name attribute
		static bool GetBakeNameAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId, 
			FString& OutBakeName,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Helper function to access the "tile" attribute
		static bool GetTileAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<int32>& OutTileValue,
			const HAPI_AttributeOwner& InAttribOwner = HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the "tile" attribute
		static bool GetTileAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			int32& OutTileValue,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		static bool GetEditLayerName(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& EditLayerName,
			const HAPI_AttributeOwner& InAttribOwner = HAPI_ATTROWNER_INVALID);

		static bool HasEditLayerName(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			const HAPI_AttributeOwner& InAttribOwner = HAPI_ATTROWNER_INVALID);

		// Helper function to access the "unreal_temp_folder" attribute
		static bool GetTempFolderAttribute(
			const HAPI_NodeId& InNodeId,
			const HAPI_AttributeOwner& InAttributeOwner,
			TArray<FString>& OutTempFolder,
			const HAPI_PartId& InPartId=0,
			const int32& InStart=0,
			const int32& InCount=-1);

		// Helper function to access the "unreal_temp_folder" attribute
		// We check for a primitive attribute first, if the primitive attribute does not exist, we check for a
		// detail attribute.
		static bool GetTempFolderAttribute(
			const HAPI_NodeId& InNodeId,
			FString& OutTempFolder,
			const HAPI_PartId& InPartId=0,
			const int32& InPrimIndex=0);

		// Helper function to access the "unreal_bake_folder" attribute
		static bool GetBakeFolderAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_AttributeOwner& InAttributeOwner,
			TArray<FString>& OutBakeFolder,
			const HAPI_PartId& InPartId = 0,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the "unreal_bake_folder" attribute
		// We check for a primitive attribute first, if the primitive attribute does not exist, we check for a
		// detail attribute.
		static bool GetBakeFolderAttribute(
			const HAPI_NodeId& InGeoId,
			TArray<FString>& OutBakeFolder,
			const HAPI_PartId& InPartId = 0,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the "unreal_bake_folder" attribute
		// We check for a primitive attribute first, if the primitive attribute does not exist, we check for a
		// detail attribute.
		static bool GetBakeFolderAttribute(
			const HAPI_NodeId InGeoId,
			const HAPI_PartId InPartId,
			FString& OutBakeFolder,
			const int32 InPrimIndex = 0);

		// Helper function to access the bake output actor attribute (unreal_bake_actor)
		static bool GetBakeActorAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutBakeActorNames,
			const HAPI_AttributeOwner& InAttributeOwner = HAPI_AttributeOwner::HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the bake output actor attribute (unreal_bake_actor)
		static bool GetBakeActorAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& OutBakeActorName,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Helper function to access the bake output actor attribute (unreal_bake_actor_class)
		static bool GetBakeActorClassAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutBakeActorClassNames,
			const HAPI_AttributeOwner& InAttributeOwner = HAPI_AttributeOwner::HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the bake output actor attribute (unreal_bake_actor_class)
		static bool GetBakeActorClassAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& OutBakeActorClassName,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Helper function to access the bake output actor attribute (unreal_bake_outliner_folder)
		static bool GetBakeOutlinerFolderAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			TArray<FString>& OutBakeOutlinerFolders,
			const HAPI_AttributeOwner& InAttributeOwner = HAPI_AttributeOwner::HAPI_ATTROWNER_INVALID,
			const int32& InStart = 0,
			const int32& InCount = -1);

		// Helper function to access the bake output actor attribute (unreal_bake_outliner_folder)
		static bool GetBakeOutlinerFolderAttribute(
			const HAPI_NodeId& InGeoId,
			const HAPI_PartId& InPartId,
			FString& OutBakeOutlinerFolder,
			const int32& InPointIndex = 0,
			const int32& InPrimIndex = 0);

		// Adds the "unreal_level_path" primitive attribute
		static bool AddLevelPathAttribute(
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			ULevel* InLevel,
			const int32& InCount,
			const HAPI_AttributeOwner& InAttributeOwner = HAPI_AttributeOwner::HAPI_ATTROWNER_PRIM);

		// Adds the "unreal_actor_path" primitive attribute
		static bool AddActorPathAttribute(
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			AActor* InActor,
			const int32& InCount,
			const HAPI_AttributeOwner& InAttributeOwner = HAPI_AttributeOwner::HAPI_ATTROWNER_PRIM);

		// Adds the landscape type primitive attribute based off InActor's type.
	    // (currently just ALandscapeStreamingProxy)
		static bool AddLandscapeTypeAttribute(
			const HAPI_NodeId& InNodeId,
			const HAPI_PartId& InPartId,
			AActor* InActor,
			const int32& InCount);

		// Helper function used to extract a const char* from a FString
		// !! Allocates memory using malloc that will need to be freed afterwards!
		static char * ExtractRawString(const FString& Name);

		// Frees memory allocated by ExtractRawString()
		static void FreeRawStringMemory(const char*& InRawString);

		// Frees memory allocated by ExtractRawString()
		static void FreeRawStringMemory(TArray<const char*>& InRawStringArray);

		// Make sure a string complies with Houdini's internal variable naming convention (group, attr etc..)
		static bool SanitizeHAPIVariableName(FString& String);

		// Ensures a string is a valid variable (mainly for attributes name)
		// Reproduces the behaviour of UT_String::forceValidVariableName()
		// Return true if a change occured, the string is modified inline
		//static bool ForceValidVariableNameInline(FString& InOutString);

		/** How many GUID symbols are used for package component name generation. **/
		static const int32 PackageGUIDComponentNameLength;

		/** How many GUID symbols are used for package item name generation. **/
		static const int32 PackageGUIDItemNameLength;

		/** Helper routine to check invalid lightmap faces. **/
		static bool ContainsInvalidLightmapFaces(const FRawMesh & RawMesh, int32 LightmapSourceIdx);

		// Helper function for creating a temporary Slate notification.
		static void CreateSlateNotification(
			const FString& NotificationString,
			const float& NotificationExpire = HAPI_UNREAL_NOTIFICATION_EXPIRE,
			const float& NotificationFadeOut = HAPI_UNREAL_NOTIFICATION_FADEOUT);

		static FString GetHoudiniEnginePluginDir();

		// -------------------------------------------------
		// UWorld and UPackage utilities
		// -------------------------------------------------

		// Find actor in a given world by label or name
		template<class T>
		static T* FindActorInWorldByLabelOrName(UWorld* InWorld, FString ActorLabelOrName, EActorIteratorFlags Flags = EActorIteratorFlags::AllActors)
		{
			T* OutActor = nullptr;
			for (TActorIterator<T> ActorIt(InWorld, T::StaticClass(), Flags); ActorIt; ++ActorIt)
			{
				OutActor = *ActorIt;
				if (!OutActor)
					continue;

				// Try the label first, then the name
				if (OutActor->GetActorLabel() == ActorLabelOrName)
					return OutActor;
				if (OutActor->GetFName().ToString() == ActorLabelOrName)
					return OutActor;
			}
			return nullptr;
		}

		// Find actor in a given world by label
		template<class T>
		static T* FindActorInWorldByLabel(UWorld* InWorld, FString ActorLabel, EActorIteratorFlags Flags = EActorIteratorFlags::AllActors)
		{
			T* OutActor = nullptr;
			for (TActorIterator<T> ActorIt(InWorld, T::StaticClass(), Flags); ActorIt; ++ActorIt)
			{
				OutActor = *ActorIt;
				if (!OutActor)
					continue;
				if (OutActor->GetActorLabel() == ActorLabel)
					return OutActor;
			}
			return nullptr;
		}

		// Find actor in a given world by name
		template<class T>
		static T* FindActorInWorld(UWorld* InWorld, FName ActorName, EActorIteratorFlags Flags = EActorIteratorFlags::AllActors)
		{
			T* OutActor = nullptr;
			for (TActorIterator<T> ActorIt(InWorld, T::StaticClass(), Flags); ActorIt; ++ActorIt)
			{
				OutActor = *ActorIt;
				if (!OutActor)
					continue;
				if (OutActor->GetFName().Compare(ActorName)==0)
					return OutActor;
			}
			return nullptr;
		}

		// Find an actor by name 
		static UWorld* FindWorldInPackage(const FString& PackagePath, bool bCreatedMissingPackage, bool& bOutPackageCreated);

		// Determine the appropriate world and level in which to spawn a new actor. 
		static bool FindWorldAndLevelForSpawning(
			UWorld* CurrentWorld,
			const FString& PackagePath,
			bool bCreateMissingPackage,
			UWorld*& OutWorld,
			ULevel*& OutLevel,
			bool& bOutPackageCreated,
			bool& bPackageInWorld);

		template <class T>
		static T* SpawnActorInLevel(UWorld* InWorld, ULevel* InLevel)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.OverrideLevel = InLevel;
			return InWorld->SpawnActor<T>(SpawnParams);
		}

		// Force the AssetRegistry to recursively rescan a path for
		// any new packages that it may not know about, starting at the directory
		// in which the given world package is located. This is typically useful
		// for WorldComposition to detect new packages immediately after they
		// were created.
		static void RescanWorldPath(UWorld* InWorld);

		// -------------------------------------------------
		// Actor Utilities
		// -------------------------------------------------

		// Find in actor that belongs to the given outer matching the specified name.
		// If the actor doesn't match the type, or is in a PendingKill state, rename it
		// so that a new actor can be created with the given name.
		// Note that if an actor with the give name was found, it will be returned via `OutFoundActor`.
		static AActor* FindOrRenameInvalidActorGeneric(UClass* Class, UWorld* InWorld, const FString& InName, AActor*& OutFoundActor);

		template<class T>
		static T* FindOrRenameInvalidActor(UWorld* InWorld, const FString& InName, AActor*& OutFoundActor)
		{
			return Cast<T>( FindOrRenameInvalidActorGeneric(T::StaticClass(), InWorld, InName, OutFoundActor) );
		}

		// Finds actors with the same Name, but without the post fix number.
		static TArray<AActor*> FindActorsWithNameNoNumber(UClass* InClass, UWorld* InWorld, const FString & InName);

		// Moves an actor to the specified level
		static bool MoveActorToLevel(AActor* InActor, ULevel* InDesiredLevel);
	
		// -------------------------------------------------
		// Debug Utilities
		// -------------------------------------------------

		// Log debug info for the given package
		static void LogPackageInfo(const FString& InLongPackageName);
		static void LogPackageInfo(const UPackage* InPackage);

		static void LogWorldInfo(const FString& InLongPackageName);
		static void LogWorldInfo(const UWorld* InWorld);

		static FString HapiGetEventTypeAsString(const HAPI_PDG_EventType& InEventType);
		static FString HapiGetWorkItemStateAsString(const HAPI_PDG_WorkItemState& InWorkItemState);

		static TArray<FString> GetAttributeNames(const HAPI_Session* Session, HAPI_NodeId Node, HAPI_PartId PartId, HAPI_AttributeOwner Owner);
		static TMap< HAPI_AttributeOwner, TArray<FString>> GetAllAttributeNames(const HAPI_Session * Session, HAPI_NodeId Node, HAPI_PartId PartId);

		// -------------------------------------------------
		// Generic naming / pathing utilities
		// -------------------------------------------------

		static bool RenameObject(UObject* Object, const TCHAR* NewName = nullptr, UObject* NewOuter = nullptr, ERenameFlags Flags = REN_None);

		// Rename the actor to a unique / generated name.
		static FName RenameToUniqueActor(AActor* InActor, const FString& InName);

		// Safely rename the actor by ensuring that there aren't any existing objects left
		// in the actor's outer with the same name. If an existing object was found, rename it and return it.
		static UObject* SafeRenameActor(AActor* InActor, const FString& InName, bool UpdateLabel=true);

		// Validates InPath by converting it to an absolute path for the platform and then calling FPaths::ValidatePath.
		static bool ValidatePath(const FString& InPath, FText* OutInvalidPathReason=nullptr);

		static bool DoesFolderExist(const FString& InPath);

		// -------------------------------------------------
		// PackageParam utilities
		// -------------------------------------------------

		// Helper for populating FHoudiniPackageParams.
		// If bAutomaticallySetAttemptToLoadMissingPackages is true, then
		// OutPackageParams.bAttemptToLoadMissingPackages is set to true in EPackageReplaceMode::CreateNewAssets mode.
		static void FillInPackageParamsForBakingOutput(
			FHoudiniPackageParams& OutPackageParams,
			const FHoudiniOutputObjectIdentifier& InIdentifier,
			const FString &BakeFolder,
			const FString &ObjectName,
			const FString &HoudiniAssetName,
			const FString &HoudiniAssetActorName,
			EPackageReplaceMode InReplaceMode=EPackageReplaceMode::ReplaceExistingAssets,
			bool bAutomaticallySetAttemptToLoadMissingPackages=true,
			const TOptional<FGuid>& InComponentGuid=TOptional<FGuid>());

		// Helper for populating FHoudiniPackageParams when baking. This includes configuring the resolver to
		// resolve the object name and unreal_bake_folder and setting these resolved values on the PackageParams.
		// If bAutomaticallySetAttemptToLoadMissingPackages is true, then
		// OutPackageParams.bAttemptToLoadMissingPackages is set to true in EPackageReplaceMode::CreateNewAssets mode.
		// If InHoudiniAssetName or InHoudiniAssetActorName is blank, then the values are determined via
		// HoudiniAssetComponent.
		static void FillInPackageParamsForBakingOutputWithResolver(
			UWorld* const InWorldContext,
			const UHoudiniAssetComponent* HoudiniAssetComponent,
			const FHoudiniOutputObjectIdentifier& InIdentifier,
			const FHoudiniOutputObject& InOutputObject,
			const bool bInHasPreviousBakeData,
			const FString &InDefaultObjectName,
			FHoudiniPackageParams& OutPackageParams,
			FHoudiniAttributeResolver& OutResolver,
			const FString &InDefaultBakeFolder=FString(),
			EPackageReplaceMode InReplaceMode=EPackageReplaceMode::ReplaceExistingAssets,
			const FString& InHoudiniAssetName=TEXT(""),
			const FString& InHoudiniAssetActorName=TEXT(""),
			bool bAutomaticallySetAttemptToLoadMissingPackages=true,
			bool bInSkipObjectNameResolutionAndUseDefault=false,
			bool bInSkipBakeFolderResolutionAndUseDefault=false);

		// Helper for updating FHoudiniPackageParams for temp outputs. This includes configuring the resolver to
		// resolve the unreal_temp_folder and setting the resolved values on the PackageParams.
		// If bAutomaticallySetAttemptToLoadMissingPackages is true, then
		// OutPackageParams.bAttemptToLoadMissingPackages is set to true in EPackageReplaceMode::CreateNewAssets mode.
		static void UpdatePackageParamsForTempOutputWithResolver(
			const FHoudiniPackageParams& InPackageParams,
			const UWorld* InWorldContext,
			const UObject* OuterComponent,
			const TMap<FString, FString>& InCachedAttributes,
			const TMap<FString, FString>& InCachedTokens,
			FHoudiniPackageParams& OutPackageParams,
			FHoudiniAttributeResolver& OutResolver,
			bool bInSkipTempFolderResolutionAndUseDefault=false);

		// -------------------------------------------------
		template <typename T>
		static TArray<TObjectPtr<T>> ToObjectPtr(const TArray<T*>& In)
		{
			TArray<TObjectPtr<T>> Result;
			Result.Reserve(In.Num());
			for(T* Ptr : In)
				Result.Add(TObjectPtr<T>(Ptr));
			return Result;
		}

		template <typename T>
		static TSet<TObjectPtr<T>> ToObjectPtr(const TSet<T*>& In)
		{
			TSet<TObjectPtr<T>> Result;
			Result.Reserve(In.Num());
			for(T* Ptr : In)
				Result.Add(TObjectPtr<T>(Ptr));
			return Result;
		}

		template <typename T>
		static TSet<T*> RemoveObjectPtr(const TSet<TObjectPtr<T>>& In)
		{
			TSet<T*> Result;
			Result.Reserve(In.Num());
			for(T* Ptr : In)
				Result.Add(Ptr);
			return Result;
		}

		// -------------------------------------------------

		// -------------------------------------------------
		// Houdini Engine debug functions
		// -------------------------------------------------

		static FString DumpNode(HAPI_NodeId NodeId);
		static void DumpPart(HAPI_NodeId NodeId, HAPI_PartId PartId, FStringBuilderBase& Output);
		static void DumpNode(const FString & NodePath);
		static FString CurveTypeToString(HAPI_CurveType CurveType);
		static FString StorageTypeToString(HAPI_StorageType StorageType);
		static FString AttributeTypeToString(HAPI_AttributeTypeInfo AttributeType);
		static FString PartTypeToString(HAPI_PartType PartType);
		static FString NodeTypeToString(HAPI_NodeType NodeType);
		static FString DumpAttribute(HAPI_NodeId NodeId, HAPI_PartId PartId, HAPI_AttributeOwner Owner, const FString& Name);
		static FString RSTOrderToString(HAPI_RSTOrder RstOrder);
		static FString HapiTransformToString(HAPI_Transform Transform);

		// -------------------------------------------------
		// Foliage utilities
		// -------------------------------------------------

		// If the foliage editor mode is active, repopulate the list of foliage types in the UI.
		// NOTE: this is a currently a bit of a hack: we deactive and reactive the foliage mode (if it was active),
		// since the relevant functions are not API exported.
		// Returns true if the list was repopulated.
		static bool RepopulateFoliageTypeListInUI();

		// -------------------------------------------------
		// Landscape utilities
		// -------------------------------------------------

		// Iterate over the input objects and gather only the landscape inputs.
		static void GatherLandscapeInputs(UHoudiniAssetComponent* HAC, TArray<ALandscapeProxy*>& AllInputLandscapes);


		static UHoudiniAssetComponent* GetOuterHoudiniAssetComponent(const UObject* Obj);

		// Helper to create an input node (similar to the HAPI version, but allows for specifying a parent node id
		static HAPI_Result CreateInputNode(const FString& InNodeLabel, HAPI_NodeId& OutNodeId, const int32 InParentNodeId=-1);

		// Helper to connect two nodes together
		// Connects InNodeIdToConnect's OutputIndex to InNodeId's InputIndex
		// (similar to the HAPI function, but allows for specifying a XformType for the created object merge when the two nodes aren't in the same subnet)
		static bool HapiConnectNodeInput(const int32& InNodeId, const int32& InputIndex, const int32& InNodeIdToConnect, const int32& OutputIndex, const int32& InXFormType);


		// -------------------------------------------------
		// JSON Utilities
		// -------------------------------------------------
		static FString JSONToString(const TSharedPtr<FJsonObject>& JSONObject);
		static bool JSONFromString(const ::FString& JSONString, TSharedPtr<FJsonObject>& OutJSONObject);

		// -------------------------------------------------
		// Mesh Attribute Utilities
		// -------------------------------------------------

		// Retrieve Houdini UV sets from the given Mesh part GeoId/PartId.
		static bool UpdateMeshPartUVSets(
			const int GeoId,
			const int PartId,
			const bool& bRemoveUnused,
			TArray<TArray<float>>& OutPartUVSets,
			TArray<HAPI_AttributeInfo>& OutAttribInfoUVSets);

		template <typename DataType>
		static TArray<int> RunLengthEncode(const DataType* Data, int TupleSize, int Count, float MaxCompressionRatio = 0.25f, int MaxPackets = 500)
		{
			// Run length encode the data.
			// If this function returns an empty array it means the desired compression ratio could not be met.

			auto CompareTuple = [TupleSize](const DataType* StartA, const DataType* StartB)
			{
				for (int Index = 0; Index < TupleSize; Index++)
				{
					if (StartA[Index] != StartB[Index])
						return false;
				}
				return true;
			};

			TArray<int> EncodedData;
			if (Count == 0)
				return EncodedData;

			// Guess of size needed.
			EncodedData.Reserve(MaxPackets);

			// The first run always begins on element zero.
			int Start = 0;
			EncodedData.Add(Start);

			// Created a run length encoded array based off the input data. eg.
			// [ 0, 0, 0, 1, 1, 2, 3 ] will return [ 0, 3, 5, 6]

			for (int Index = 0; Index < Count * TupleSize; Index += TupleSize)
			{
				if (!CompareTuple(&Data[Start], &Data[Index]))
				{
					// The value changed, so start a new run
					if (EncodedData.Num() == MaxPackets)
						return {};
					Start = Index;
					EncodedData.Add(Start / TupleSize);
				}
			}

			// Check we've made a decent compression ratio. If not return an empty array.
			float Ratio = float(EncodedData.Num() / float(Count));
			if (Ratio > MaxCompressionRatio)
				EncodedData.SetNum(0);

			return EncodedData;
		}
	protected:
		
		// Computes the XX.YY.ZZZ version string using HAPI_Version
		static FString ComputeVersionString(bool ExtraDigit);

#if PLATFORM_WINDOWS
		// Attempt to locate libHAPI on Windows in the registry. Return handle if located and return location.
		static void* LocateLibHAPIInRegistry(
			const FString& HoudiniInstallationType, FString& StoredLibHAPILocation, bool LookIn32bitRegistry);
#endif

		// Triggers an update the details panel
		static void UpdateEditorProperties_Internal(const bool bInForceFullUpdate);

		// Trigger an update of the Blueprint Editor on the game thread
		static void UpdateBlueprintEditor_Internal(UHoudiniAssetComponent* HAC);

	private:

		/** 
		 * Gets FHoudiniParameterWidgetMetaData from focused widget if it exists and has DetailsView
		 * as a parent.
		 *
		 * @see UpdateEditorProperties_Internal
		 * @see FocusUsingParameterWidgetMetaData
		 */
		static TSharedPtr<FHoudiniParameterWidgetMetaData> GetFocusedParameterWidgetMetaData(
			TSharedPtr<IDetailsView> DetailsView);

		/**
		 * Sets user focus on a descendant widget that has matching parameter widget metadata.
		 *
		 * This is used for a hack needed to maintain user focus on parameter widgets after the
		 * Details panel is forcibly refreshed.
		 *
		 * @param AncestorWidget All descendant widgets will be searched for matching metadata.
		 * @param ParameterWidgetMetaData Should be unique to the widget which we want to select.
		 * @return true if a widget was successfully selected, false otherwise.
		 *
		 * @see UpdateEditorProperties_Internal
		 */
		static bool FocusUsingParameterWidgetMetaData(
			TSharedRef<SWidget> AncestorWidget,
			const FHoudiniParameterWidgetMetaData& ParameterWidgetMetaData);
};

