#include "StreetMapComponent.h"
#include "StreetMapSceneProxy.h"
#include "NavigationSystem.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Public/StaticMeshResources.h"
#include "PolygonTools.h"
#include "PhysicsEngine/BodySetup.h"

#if WITH_EDITOR
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#endif //WITH_EDITOR

UStreetMapComponent::UStreetMapComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  StreetMap(nullptr),
	  CachedLocalBounds(ForceInit)
{
	// We make sure our mesh collision profile name is set to NoCollisionProfileName at initialization. 
	// Because we don't have collision data yet!
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	// We don't currently need to be ticked.  This can be overridden in a derived class though.
	PrimaryComponentTick.bCanEverTick = false;
	this->bAutoActivate = false;	// NOTE: Components instantiated through C++ are not automatically active, so they'll only tick once and then go to sleep!

	// We don't currently need InitializeComponent() to be called on us.  This can be overridden in a
	// derived class though.
	bWantsInitializeComponent = false;

	// Turn on shadows.  It looks better.
	CastShadow = true;

	// Our mesh is too complicated to be a useful occluder.
	bUseAsOccluder = false;

	// Our mesh can influence navigation.
	bCanEverAffectNavigation = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("/StreetMap/StreetMapDefaultMaterial"));
	StreetMapDefaultMaterial = DefaultMaterialAsset.Object;

}


FPrimitiveSceneProxy* UStreetMapComponent::CreateSceneProxy()
{
	FStreetMapSceneProxy* StreetMapSceneProxy = nullptr;

	if( HasValidMesh() )
	{
		StreetMapSceneProxy = new FStreetMapSceneProxy( this );
		StreetMapSceneProxy->Init( this, Vertices, Indices );
	}
	
	return StreetMapSceneProxy;
}


int32 UStreetMapComponent::GetNumMaterials() const
{
	// NOTE: This is a bit of a weird thing about Unreal that we need to deal with when defining a component that
	// can have materials assigned.  UPrimitiveComponent::GetNumMaterials() will return 0, so we need to override it 
	// to return the number of overridden materials, which are the actual materials assigned to the component.
	return HasValidMesh() ? GetNumMeshSections() : GetNumOverrideMaterials();
}


void UStreetMapComponent::SetStreetMap(class UStreetMap* NewStreetMap, bool bClearPreviousMeshIfAny /*= false*/, bool bRebuildMesh /*= false */)
{
	if (StreetMap != NewStreetMap)
	{
		StreetMap = NewStreetMap;

		if (bClearPreviousMeshIfAny)
			InvalidateMesh();

		if (bRebuildMesh)
			BuildMesh();
	}
}


bool UStreetMapComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{

	if (!CollisionSettings.bGenerateCollision || !HasValidMesh())
	{
		return false;
	}

	// Copy vertices data
	const int32 NumVertices = Vertices.Num();
	CollisionData->Vertices.Empty();
	CollisionData->Vertices.AddUninitialized(NumVertices);

	for (int32 VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
	{
		CollisionData->Vertices[VertexIndex] = Vertices[VertexIndex].Position;
	}

	// Copy indices data
	const int32 NumTriangles = Indices.Num() / 3;
	FTriIndices TempTriangle;
	for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles * 3; TriangleIndex += 3)
	{

		TempTriangle.v0 = Indices[TriangleIndex + 0];
		TempTriangle.v1 = Indices[TriangleIndex + 1];
		TempTriangle.v2 = Indices[TriangleIndex + 2];


		CollisionData->Indices.Add(TempTriangle);
		CollisionData->MaterialIndices.Add(0);
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;

	return HasValidMesh();
}


bool UStreetMapComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return HasValidMesh() && CollisionSettings.bGenerateCollision;
}


bool UStreetMapComponent::WantsNegXTriMesh()
{
	return false;
}


void UStreetMapComponent::CreateBodySetupIfNeeded(bool bForceCreation /*= false*/)
{
	if (StreetMapBodySetup == nullptr || bForceCreation == true)
	{
		// Creating new BodySetup Object.
		StreetMapBodySetup = NewObject<UBodySetup>(this);
		StreetMapBodySetup->BodySetupGuid = FGuid::NewGuid();
		StreetMapBodySetup->bDoubleSidedGeometry = CollisionSettings.bAllowDoubleSidedGeometry;

		// shapes per poly shape for collision (Not working in simulation mode).
		StreetMapBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
	}
}


void UStreetMapComponent::GenerateCollision()
{
	if (!CollisionSettings.bGenerateCollision || !HasValidMesh())
	{
		return;
	}

	// create a new body setup
	CreateBodySetupIfNeeded(true);


	if (GetCollisionProfileName() == UCollisionProfile::NoCollision_ProfileName)
	{
		SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}

	// Rebuild the body setup
	StreetMapBodySetup->InvalidatePhysicsData();
	StreetMapBodySetup->CreatePhysicsMeshes();
	UpdateNavigationIfNeeded();
}


void UStreetMapComponent::ClearCollision()
{

	if (StreetMapBodySetup != nullptr)
	{
		StreetMapBodySetup->InvalidatePhysicsData();
		StreetMapBodySetup = nullptr;
	}

	if (GetCollisionProfileName() != UCollisionProfile::NoCollision_ProfileName)
	{
		SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}

	UpdateNavigationIfNeeded();
}

class UBodySetup* UStreetMapComponent::GetBodySetup()
{
	if (CollisionSettings.bGenerateCollision == true)
	{
		// checking if we have a valid body setup. 
		// A new one is created only if a valid body setup is not found.
		CreateBodySetupIfNeeded();
		return StreetMapBodySetup;
	}

	if (StreetMapBodySetup != nullptr) StreetMapBodySetup = nullptr;

	return nullptr;
}

void UStreetMapComponent::GenerateMesh()
{
	/////////////////////////////////////////////////////////
	// Visual tweakables for generated Street Map mesh
	//
	const float RoadZ = MeshBuildSettings.RoadOffsetZ;
	const bool bWant3DBuildings = MeshBuildSettings.bWant3DBuildings;
	const float BuildingLevelFloorFactor = MeshBuildSettings.BuildingLevelFloorFactor;
	const bool bWantLitBuildings = MeshBuildSettings.bWantLitBuildings;
	const bool bWantBuildingBorderOnGround = !bWant3DBuildings;
	const float StreetThickness = MeshBuildSettings.StreetThickness;
	const FColor StreetColor = MeshBuildSettings.StreetColor.ToFColor( false );
	const float MajorRoadThickness = MeshBuildSettings.MajorRoadThickness;
	const FColor MajorRoadColor = MeshBuildSettings.MajorRoadColor.ToFColor( false );
	const float HighwayThickness = MeshBuildSettings.HighwayThickness;
	const FColor HighwayColor = MeshBuildSettings.HighwayColor.ToFColor( false );
	const float BuildingBorderThickness = MeshBuildSettings.BuildingBorderThickness;
	FLinearColor BuildingBorderLinearColor = MeshBuildSettings.BuildingBorderLinearColor;
	const float BuildingBorderZ = MeshBuildSettings.BuildingBorderZ;
	const FColor BuildingBorderColor( BuildingBorderLinearColor.ToFColor( false ) );
	const FColor BuildingFillColor( FLinearColor( BuildingBorderLinearColor * 0.33f ).CopyWithNewOpacity( 1.0f ).ToFColor( false ) );
	/////////////////////////////////////////////////////////


	CachedLocalBounds = FBox( ForceInit );
	Vertices.Reset();
	Indices.Reset();

	if( StreetMap != nullptr )
	{
		FBox3f MeshBoundingBox;
		MeshBoundingBox.Init();

		const auto& Roads = StreetMap->GetRoads();
		const auto& Nodes = StreetMap->GetNodes();
		const auto& Buildings = StreetMap->GetBuildings();

		for( const auto& Road : Roads )
		{
			float RoadThickness = StreetThickness;
			FColor RoadColor = StreetColor;
			switch( Road.RoadType )
			{
				case EStreetMapRoadType::Highway:
					RoadThickness = HighwayThickness;
					RoadColor = HighwayColor;
					break;
					
				case EStreetMapRoadType::MajorRoad:
					RoadThickness = MajorRoadThickness;
					RoadColor = MajorRoadColor;
					break;
					
				case EStreetMapRoadType::Street:
				case EStreetMapRoadType::Other:
					break;
					
				default:
					check( 0 );
					break;
			}
			
			for( int32 PointIndex = 0; PointIndex < Road.RoadPoints.Num() - 1; ++PointIndex )
			{
				AddThick2DLine( 
					FVector2f(Road.RoadPoints[ PointIndex ]),
					FVector2f(Road.RoadPoints[ PointIndex + 1 ]),
					RoadZ,
					RoadThickness,
					RoadColor,
					RoadColor,
					MeshBoundingBox );
			}
		}
		
		TArray< int32 > TempIndices;
		TArray< int32 > TriangulatedVertexIndices;
		TArray< FVector3f > TempPoints;
		for( int32 BuildingIndex = 0; BuildingIndex < Buildings.Num(); ++BuildingIndex )
		{
			const auto& Building = Buildings[ BuildingIndex ];

			// Building mesh (or filled area, if the building has no height)

			// Triangulate this building
			// @todo: Performance: Triangulating lots of building polygons is quite slow.  We could easily do this 
			//        as part of the import process and store tessellated geometry instead of doing this at load time.
			bool WindsClockwise;
			if( FPolygonTools::TriangulatePolygon( Building.BuildingPoints, TempIndices, /* Out */ TriangulatedVertexIndices, /* Out */ WindsClockwise ) )
			{
				// @todo: Performance: We could preprocess the building shapes so that the points always wind
				//        in a consistent direction, so we can skip determining the winding above.

				const int32 FirstTopVertexIndex = this->Vertices.Num();

				// calculate fill Z for buildings
				// either use the defined height or extrapolate from building level count
				float BuildingFillZ = 0.0f;
				if (bWant3DBuildings) {
					if (Building.Height > 0) {
						BuildingFillZ = Building.Height;
					}
					else if (Building.BuildingLevels > 0) {
						BuildingFillZ = (float)Building.BuildingLevels * BuildingLevelFloorFactor;
					}
				}		

				// Top of building
				{
					TempPoints.SetNum( Building.BuildingPoints.Num(), EAllowShrinking::No);
					for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
					{
						TempPoints[ PointIndex ] = FVector3f( FVector2f(Building.BuildingPoints[ ( Building.BuildingPoints.Num() - PointIndex ) - 1 ]), BuildingFillZ );
					}
					AddTriangles( TempPoints, TriangulatedVertexIndices, FVector3f::ForwardVector, FVector3f::UpVector, BuildingFillColor, MeshBoundingBox );
				}

				if( bWant3DBuildings && (Building.Height > KINDA_SMALL_NUMBER || Building.BuildingLevels > 0) )
				{
					// NOTE: Lit buildings can't share vertices beyond quads (all quads have their own face normals), so this uses a lot more geometry!
					if( bWantLitBuildings )
					{
						// Create edges for the walls of the 3D buildings
						for( int32 LeftPointIndex = 0; LeftPointIndex < Building.BuildingPoints.Num(); ++LeftPointIndex )
						{
							const int32 RightPointIndex = ( LeftPointIndex + 1 ) % Building.BuildingPoints.Num();

							TempPoints.SetNum( 4, EAllowShrinking::No);

							const int32 TopLeftVertexIndex = 0;
							TempPoints[ TopLeftVertexIndex ] = FVector3f( FVector2f(Building.BuildingPoints[ WindsClockwise ? RightPointIndex : LeftPointIndex ]), BuildingFillZ );

							const int32 TopRightVertexIndex = 1;
							TempPoints[ TopRightVertexIndex ] = FVector3f( FVector2f(Building.BuildingPoints[ WindsClockwise ? LeftPointIndex : RightPointIndex ]), BuildingFillZ );

							const int32 BottomRightVertexIndex = 2;
							TempPoints[ BottomRightVertexIndex ] = FVector3f( FVector2f(Building.BuildingPoints[ WindsClockwise ? LeftPointIndex : RightPointIndex ]), 0.0f );

							const int32 BottomLeftVertexIndex = 3;
							TempPoints[ BottomLeftVertexIndex ] = FVector3f( FVector2f(Building.BuildingPoints[ WindsClockwise ? RightPointIndex : LeftPointIndex ]), 0.0f );


							TempIndices.SetNum( 6, EAllowShrinking::No);

							TempIndices[ 0 ] = BottomLeftVertexIndex;
							TempIndices[ 1 ] = TopLeftVertexIndex;
							TempIndices[ 2 ] = BottomRightVertexIndex;

							TempIndices[ 3 ] = BottomRightVertexIndex;
							TempIndices[ 4 ] = TopLeftVertexIndex;
							TempIndices[ 5 ] = TopRightVertexIndex;

							const FVector3f FaceNormal = FVector3f::CrossProduct( ( TempPoints[ 0 ] - TempPoints[ 2 ] ).GetSafeNormal(), ( TempPoints[ 0 ] - TempPoints[ 1 ] ).GetSafeNormal() );
							const FVector3f ForwardVector = FVector3f::UpVector;
							const FVector3f UpVector = FaceNormal;
							AddTriangles( TempPoints, TempIndices, ForwardVector, UpVector, BuildingFillColor, MeshBoundingBox );
						}
					}
					else
					{
						// Create vertices for the bottom
						const int32 FirstBottomVertexIndex = this->Vertices.Num();
						for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
						{
							const FVector2D Point = Building.BuildingPoints[ PointIndex ];

							FStreetMapVertex& NewVertex = *new( this->Vertices )FStreetMapVertex();
							NewVertex.Position = FVector3f( FVector2f(Point), 0.0f );
							NewVertex.TextureCoordinate = FVector2f( 0.0f, 0.0f );	// NOTE: We're not using texture coordinates for anything yet
							NewVertex.TangentX = FVector3f::ForwardVector;	 // NOTE: Tangents aren't important for these unlit buildings
							NewVertex.TangentZ = FVector3f::UpVector;
							NewVertex.Color = BuildingFillColor;

							MeshBoundingBox += NewVertex.Position;
						}

						// Create edges for the walls of the 3D buildings
						for( int32 LeftPointIndex = 0; LeftPointIndex < Building.BuildingPoints.Num(); ++LeftPointIndex )
						{
							const int32 RightPointIndex = ( LeftPointIndex + 1 ) % Building.BuildingPoints.Num();

							const int32 BottomLeftVertexIndex = FirstBottomVertexIndex + LeftPointIndex;
							const int32 BottomRightVertexIndex = FirstBottomVertexIndex + RightPointIndex;
							const int32 TopRightVertexIndex = FirstTopVertexIndex + RightPointIndex;
							const int32 TopLeftVertexIndex = FirstTopVertexIndex + LeftPointIndex;

							this->Indices.Add( BottomLeftVertexIndex );
							this->Indices.Add( TopLeftVertexIndex );
							this->Indices.Add( BottomRightVertexIndex );

							this->Indices.Add( BottomRightVertexIndex );
							this->Indices.Add( TopLeftVertexIndex );
							this->Indices.Add( TopRightVertexIndex );
						}
					}
				}
			}
			else
			{
				// @todo: Triangulation failed for some reason, possibly due to degenerate polygons.  We can
				//        probably improve the algorithm to avoid this happening.
			}

			// Building border
			if( bWantBuildingBorderOnGround )
			{
				for( int32 PointIndex = 0; PointIndex < Building.BuildingPoints.Num(); ++PointIndex )
				{
					AddThick2DLine(
						FVector2f(Building.BuildingPoints[ PointIndex ]),
						FVector2f(Building.BuildingPoints[ ( PointIndex + 1 ) % Building.BuildingPoints.Num() ]),
						BuildingBorderZ,
						BuildingBorderThickness,		// Thickness
						BuildingBorderColor,
						BuildingBorderColor,
						MeshBoundingBox );
				}
			}
		}

		CachedLocalBounds = FBox(MeshBoundingBox);
	}
}


#if WITH_EDITOR
void UStreetMapComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bNeedRefreshCustomizationModule = false;

	// Check to see if the "StreetMap" property changed.
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UStreetMapComponent, StreetMap))
		{
			bNeedRefreshCustomizationModule = true;
		}
		else if (IsCollisionProperty(PropertyName)) // For some unknown reason , GET_MEMBER_NAME_CHECKED(UStreetMapComponent, CollisionSettings) is not working ??? "TO CHECK LATER"
		{
			if (CollisionSettings.bGenerateCollision == true)
			{
				GenerateCollision();
			}
			else
			{
				ClearCollision();
			}
			bNeedRefreshCustomizationModule = true;
		}
	}

	if (bNeedRefreshCustomizationModule)
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Call the parent implementation of this function
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif	// WITH_EDITOR


void UStreetMapComponent::BuildMesh()
{
	// Wipes out our cached mesh data. Maybe unnecessary in case GenerateMesh is clearing cached mesh data and creating a new SceneProxy  !
	InvalidateMesh();

	GenerateMesh();

	if (HasValidMesh())
	{
		// We have a new bounding box
		UpdateBounds();
	}
	else
	{
		// No mesh was generated
	}

	GenerateCollision();

	// Mark our render state dirty so that CreateSceneProxy can refresh it on demand
	MarkRenderStateDirty();

	AssignDefaultMaterialIfNeeded();

	Modify();
}


void UStreetMapComponent::AssignDefaultMaterialIfNeeded()
{
	if (this->GetNumMaterials() == 0 || this->GetMaterial(0) == nullptr)
	{
		if (!HasValidMesh() || GetDefaultMaterial() == nullptr)
			return;

		this->SetMaterial(0, GetDefaultMaterial());
	}
}


void UStreetMapComponent::UpdateNavigationIfNeeded()
{
	if (bCanEverAffectNavigation || bNavigationRelevant)
	{
		FNavigationSystem::UpdateComponentData(*this);
	}
}

void UStreetMapComponent::InvalidateMesh()
{
	Vertices.Reset();
	Indices.Reset();
	CachedLocalBounds = FBoxSphereBounds(FBox(ForceInit));
	ClearCollision();
	// Mark our render state dirty so that CreateSceneProxy can refresh it on demand
	MarkRenderStateDirty();
	Modify();
}

FBoxSphereBounds UStreetMapComponent::CalcBounds( const FTransform& LocalToWorld ) const
{
	if( HasValidMesh() )
	{
		FBoxSphereBounds WorldSpaceBounds = CachedLocalBounds.TransformBy( LocalToWorld );
		WorldSpaceBounds.BoxExtent *= BoundsScale;
		WorldSpaceBounds.SphereRadius *= BoundsScale;
		return WorldSpaceBounds;
	}
	else
	{
		return FBoxSphereBounds( LocalToWorld.GetLocation(), FVector::ZeroVector, 0.0f );
	}
}


void UStreetMapComponent::AddThick2DLine( const FVector2f Start, const FVector2f End, const float Z, const float Thickness, const FColor& StartColor, const FColor& EndColor, FBox3f& MeshBoundingBox )
{
	const float HalfThickness = Thickness * 0.5f;

	const FVector2f LineDirection = ( End - Start ).GetSafeNormal();
	const FVector2f RightVector( -LineDirection.Y, LineDirection.X );

	const int32 BottomLeftVertexIndex = Vertices.Num();
	FStreetMapVertex& BottomLeftVertex = *new( Vertices )FStreetMapVertex();
	BottomLeftVertex.Position = FVector3f( Start - RightVector * HalfThickness, Z );
	BottomLeftVertex.TextureCoordinate = FVector2f( 0.0f, 0.0f );
	BottomLeftVertex.TangentX = FVector3f( LineDirection, 0.0f );
	BottomLeftVertex.TangentZ = FVector3f::UpVector;
	BottomLeftVertex.Color = StartColor;
	MeshBoundingBox += BottomLeftVertex.Position;

	const int32 BottomRightVertexIndex = Vertices.Num();
	FStreetMapVertex& BottomRightVertex = *new( Vertices )FStreetMapVertex();
	BottomRightVertex.Position = FVector3f( Start + RightVector * HalfThickness, Z );
	BottomRightVertex.TextureCoordinate = FVector2f( 1.0f, 0.0f );
	BottomRightVertex.TangentX = FVector3f( LineDirection, 0.0f );
	BottomRightVertex.TangentZ = FVector3f::UpVector;
	BottomRightVertex.Color = StartColor;
	MeshBoundingBox += BottomRightVertex.Position;

	const int32 TopRightVertexIndex = Vertices.Num();
	FStreetMapVertex& TopRightVertex = *new( Vertices )FStreetMapVertex();
	TopRightVertex.Position = FVector3f( End + RightVector * HalfThickness, Z );
	TopRightVertex.TextureCoordinate = FVector2f( 1.0f, 1.0f );
	TopRightVertex.TangentX = FVector3f( LineDirection, 0.0f );
	TopRightVertex.TangentZ = FVector3f::UpVector;
	TopRightVertex.Color = EndColor;
	MeshBoundingBox += TopRightVertex.Position;

	const int32 TopLeftVertexIndex = Vertices.Num();
	FStreetMapVertex& TopLeftVertex = *new( Vertices )FStreetMapVertex();
	TopLeftVertex.Position = FVector3f( End - RightVector * HalfThickness, Z );
	TopLeftVertex.TextureCoordinate = FVector2f( 0.0f, 1.0f );
	TopLeftVertex.TangentX = FVector3f( LineDirection, 0.0f );
	TopLeftVertex.TangentZ = FVector3f::UpVector;
	TopLeftVertex.Color = EndColor;
	MeshBoundingBox += TopLeftVertex.Position;

	Indices.Add( BottomLeftVertexIndex );
	Indices.Add( BottomRightVertexIndex );
	Indices.Add( TopRightVertexIndex );

	Indices.Add( BottomLeftVertexIndex );
	Indices.Add( TopRightVertexIndex );
	Indices.Add( TopLeftVertexIndex );
};


void UStreetMapComponent::AddTriangles( const TArray<FVector3f>& Points, const TArray<int32>& PointIndices, const FVector3f& ForwardVector, const FVector3f& UpVector, const FColor& Color, FBox3f& MeshBoundingBox )
{
	const int32 FirstVertexIndex = Vertices.Num();

	for( FVector3f Point : Points )
	{
		FStreetMapVertex& NewVertex = *new( Vertices )FStreetMapVertex();
		NewVertex.Position = Point;
		NewVertex.TextureCoordinate = FVector2f( 0.0f, 0.0f );	// NOTE: We're not using texture coordinates for anything yet
		NewVertex.TangentX = ForwardVector;
		NewVertex.TangentZ = UpVector;
		NewVertex.Color = Color;

		MeshBoundingBox += NewVertex.Position;
	}

	for( int32 PointIndex : PointIndices )
	{
		Indices.Add( FirstVertexIndex + PointIndex );
	}
};


FString UStreetMapComponent::GetStreetMapAssetName() const
{
	return StreetMap != nullptr ? StreetMap->GetName() : FString(TEXT("NONE"));
}

