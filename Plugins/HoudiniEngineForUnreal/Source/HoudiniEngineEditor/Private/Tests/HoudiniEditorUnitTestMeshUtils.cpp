#include "HoudiniEditorUnitTestMeshUtils.h"

#include "HoudiniEditorUnitTestUtils.h"
#include "FileHelpers.h"
#include "HoudiniAsset.h"
#include "HoudiniPublicAPIAssetWrapper.h"
#include "HoudiniPublicAPIInputTypes.h"
#include "HoudiniAssetActor.h"
#include "HoudiniEditorTestProxyMesh.h"
#include "HoudiniEngineBakeUtils.h"
#include "HoudiniEngineEditorUtils.h"

#include "HoudiniParameter.h"
#include "HoudiniParameterInt.h"
#include "HoudiniPDGAssetLink.h"
#include "Landscape.h"
#include "StaticMeshResources.h"
#include "AssetRegistry/AssetRegistryModule.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "HoudiniEditorTestUtils.h"

#include "Misc/AutomationTest.h"
#include "HoudiniAssetActorFactory.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HoudiniParameterToggle.h"
#include "HoudiniEngineOutputStats.h"
#include "HoudiniPDGManager.h"

FHoudiniTestMeshData FHoudiniEditorUnitTestMeshUtils::GetExpectedMeshData()
{
	FHoudiniTestMeshData Result;

	for (int Index = 0; Index < 8; Index++)
	{
		FVector3f Vertex;
		Vertex.X = Index & 1 ? -1.0f : +1.0f;
		Vertex.Y = Index & 2 ? -1.0f : +1.0f;
		Vertex.Z = Index & 4 ? -1.0f : +1.0f;
		Vertex *= 800.0f;


		Result.VertexPositions.Add(Vertex);
		Result.VertexColors.Add(FColor(188, 188, 188, 255)); // Expected color is 0.5 (grey) but gamma corrected.
		Result.NumTriangles = 6 * 2;
	}
	return Result;
}

FHoudiniTestMeshData FHoudiniEditorUnitTestMeshUtils::ExtractMeshData(UHoudiniStaticMesh& Mesh)
{
	FHoudiniTestMeshData Result;
	Result.VertexPositions = Mesh.GetVertexPositions();
	Result.VertexColors = Mesh.GetVertexInstanceColors();
	Result.NumTriangles = Mesh.GetNumTriangles();
	return Result;

}

FHoudiniTestMeshData FHoudiniEditorUnitTestMeshUtils::ExtractMeshData(UStaticMesh& Mesh, int LOD)
{
	FHoudiniTestMeshData Result;

	FStaticMeshLODResources& LODResource = Mesh.GetRenderData()->LODResources[LOD];
	int NumUnrealVertices = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices();

	Result.VertexPositions.SetNum(NumUnrealVertices);
	Result.VertexColors.SetNum(NumUnrealVertices);

	for (int Index = 0; Index < NumUnrealVertices; Index++)
	{
		Result.VertexPositions[Index] = LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition(Index);
		Result.VertexColors[Index] = LODResource.VertexBuffers.ColorVertexBuffer.VertexColor(Index);
	}

	Result.NumTriangles = LODResource.GetNumTriangles();
	return Result;

}

TArray<FString> FHoudiniEditorUnitTestMeshUtils::CheckMesh(FHoudiniTestMeshData& ExpectedMesh, FHoudiniTestMeshData& ActualData)
{
	TArray<FString> Errors;

	if (ExpectedMesh.VertexPositions.Num() != ActualData.VertexPositions.Num())
	{
		FString Error = FString::Printf(TEXT("Expected %d vertices, not %d"), ExpectedMesh.VertexPositions.Num(), ActualData.VertexPositions.Num());

		Errors.Add(Error);
		return Errors;
	}

	// The following comparison is O(n^2). This could be optimized greatly, but our test meshes are currently very simple.
	// The order of the vertices will likely be different between UE and Houdini, so have to do a comparison.

	if (ExpectedMesh.VertexPositions.Num() > 100)
	{
		FString Error = FString::Printf(TEXT("TOO MANY VERTICES! Optimize this function or do something else."));
		Errors.Add(Error);
		return Errors;
	}

	int bFoundCount = 0;;

	// Keep track of which actual vertex was used
	TArray<bool> bUsed;
	bUsed.SetNum(ExpectedMesh.VertexPositions.Num());


	for(int ExpectedVertexIndex = 0; ExpectedVertexIndex < ExpectedMesh.VertexPositions.Num(); ExpectedVertexIndex++)
	{
		for (int ActualVertexIndex = 0; ActualVertexIndex < ActualData.VertexPositions.Num(); ActualVertexIndex++)
		{
				if (bUsed[ActualVertexIndex])
					continue;

				if (ExpectedMesh.VertexPositions[ExpectedVertexIndex] == ActualData.VertexPositions[ActualVertexIndex] &&
						ExpectedMesh.VertexColors[ExpectedVertexIndex] == ActualData.VertexColors[ActualVertexIndex])
				{
					bFoundCount++;
					bUsed[ActualVertexIndex] = true;
					break;
				}
		}
	}

	if (bFoundCount != ExpectedMesh.VertexPositions.Num())
	{
		Errors.Add(FString("Could not match all vertices"));
		Errors.Add(ExpectedMesh.ToString());
		Errors.Add(ActualData.ToString());
	}

	return Errors;
}


FString FHoudiniTestMeshData::ToString()
{
	FString Result;

	for(int Index = 0; Index < VertexPositions.Num(); Index++)
	{
		FString Vertex = FString::Printf(TEXT("P = %.1f %.1f %.1f, C = %d %d %d %d\n"),
			VertexPositions[Index].X, VertexPositions[Index].Y, VertexPositions[Index].Z,
			VertexColors[Index].R, VertexColors[Index].G, VertexColors[Index].B, VertexColors[Index].A);

		Result += Vertex;
	}
	return Result;
}


#endif