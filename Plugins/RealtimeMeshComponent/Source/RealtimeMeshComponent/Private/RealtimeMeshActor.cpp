// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#include "RealtimeMeshActor.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSubsystem.h"
#include "Engine/CollisionProfile.h"
#include "Mesh/RealtimeMeshBlueprintMeshBuilder.h"
#if RMC_ENGINE_ABOVE_5_2
#include "Engine/Level.h"
#endif

#define LOCTEXT_NAMESPACE "ARealtimeMeshActor"

URealtimeMeshStream* ARealtimeMeshActor::MakeStream(const FRealtimeMeshStreamKey& StreamKey, ERealtimeMeshSimpleStreamType StreamType, int32 NumElements)
{
	auto Stream = NewObject<URealtimeMeshStream>(this);
	check(IsValid(Stream));
	Stream->Initialize(StreamKey, StreamType, NumElements);
	return Stream;
}

URealtimeMeshStreamSet* ARealtimeMeshActor::MakeStreamSet()
{
	auto StreamSet = NewObject<URealtimeMeshStreamSet>(this);
	check(IsValid(StreamSet));
	return StreamSet;
}

URealtimeMeshLocalBuilder* ARealtimeMeshActor::MakeMeshBuilder(ERealtimeMeshSimpleStreamConfig WantedTangents, ERealtimeMeshSimpleStreamConfig WantedTexCoords,
	bool bWants32BitIndices, ERealtimeMeshSimpleStreamConfig WantedPolyGroupType, bool bWantsColors, int32 WantedTexCoordChannels, bool bKeepExistingData)
{
	auto Builder = NewObject<URealtimeMeshLocalBuilder>(this);
	check(IsValid(Builder));
	Builder->Initialize(WantedTangents, WantedTexCoords, bWants32BitIndices, WantedPolyGroupType, bWantsColors, WantedTexCoordChannels, bKeepExistingData);
	return Builder;
}

void ARealtimeMeshActor::BeginPlay()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bReplicates = true;
		SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
		SetReplicates(true);
	}

#if RMC_ENGINE_ABOVE_5_3
	if (RealtimeMeshComponent && RealtimeMeshComponent->BodyInstance.bSimulatePhysics)
	{
		SetPhysicsReplicationMode(EPhysicsReplicationMode::Resimulation);
	}
#endif
	
	Super::BeginPlay();
}

ARealtimeMeshActor::ARealtimeMeshActor()
{
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("RealtimeMeshComponent"));
	RealtimeMeshComponent->SetMobility(EComponentMobility::Movable);
	RealtimeMeshComponent->SetGenerateOverlapEvents(false);
	RealtimeMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	//RealtimeMeshComponent->CollisionType = ECollisionTraceFlag::CTF_UseDefault;

	SetRootComponent(RealtimeMeshComponent);

#if RMC_ENGINE_BELOW_5_1
	RegisterWithGenerationManager();
#endif
}


ARealtimeMeshActor::~ARealtimeMeshActor()
{
	// Ensure we're unregistered on destruction
	UnregisterWithGenerationManager();
}


void ARealtimeMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bGeneratedMeshRebuildPending = true;
}

void ARealtimeMeshActor::PostLoad()
{
	Super::PostLoad();

#if RMC_ENGINE_ABOVE_5_1
	RegisterWithGenerationManager();
#endif
}

void ARealtimeMeshActor::PostActorCreated()
{
	Super::PostActorCreated();
#if RMC_ENGINE_ABOVE_5_1
	RegisterWithGenerationManager();
#endif
}

void ARealtimeMeshActor::Destroyed()
{
	UnregisterWithGenerationManager();
	Super::Destroyed();
}


void ARealtimeMeshActor::PreRegisterAllComponents()
{
	Super::PreRegisterAllComponents();

	// Handle UWorld::AddToWorld() to catch ULevel visibility toggles
	if (GetLevel() && GetLevel()->bIsAssociatingLevel)
	{
		RegisterWithGenerationManager();
	}
}

void ARealtimeMeshActor::PostUnregisterAllComponents()
{
	// Handle UWorld::RemoveFromWorld() to catch ULevel visibility toggles
	if (GetLevel() && GetLevel()->bIsDisassociatingLevel)
	{
		UnregisterWithGenerationManager();
	}

	Super::PostUnregisterAllComponents();
}


#if WITH_EDITOR

void ARealtimeMeshActor::PostEditUndo()
{
	Super::PostEditUndo();

	// There is no direct signal that an Actor is being created or destroyed due to undo/redo.
	// Currently (5.1) the checks below will tell us if the undo/redo has destroyed the
	// Actor, and we assume otherwise it was created

	if (IsActorBeingDestroyed() || !IsValid(this)) // equivalent to AActor::IsPendingKillPending()
	{
		UnregisterWithGenerationManager();
	}
	else
	{
		RegisterWithGenerationManager();
	}
}

#endif


void ARealtimeMeshActor::RegisterWithGenerationManager()
{
	// Ignore generation on CDO, or if we duplicated to PIE from editor
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	if (bIsRegisteredWithGenerationManager == false)
	{
		// this could fail if the subsystem is not initialized yet, or if it is shutting down
		if (URealtimeMeshSubsystem* Subsystem = URealtimeMeshSubsystem::GetInstance(GetWorld()))
		{
			bIsRegisteredWithGenerationManager = Subsystem->RegisterGeneratedMeshActor(this);
		}
	}
}


void ARealtimeMeshActor::UnregisterWithGenerationManager()
{
	if (bIsRegisteredWithGenerationManager)
	{
		if (UWorld* World = GetWorld())
		{
			if (URealtimeMeshSubsystem* Subsystem = URealtimeMeshSubsystem::GetInstance(World))
			{
				Subsystem->UnregisterGeneratedMeshActor(this);
			}
		}
		bIsRegisteredWithGenerationManager = false;
		bGeneratedMeshRebuildPending = false;
	}
}


void ARealtimeMeshActor::ExecuteRebuildGeneratedMeshIfPending()
{
	if (!bDeferGeneration || bFrozen ||
		!bGeneratedMeshRebuildPending ||
		!IsValid(RealtimeMeshComponent))
	{
		return;
	}

	if (bResetOnRebuild)
	{
		RealtimeMeshComponent->SetRealtimeMesh(nullptr);
	}

	FEditorScriptExecutionGuard Guard;

	OnGenerateMesh();

	bGeneratedMeshRebuildPending = false;
}


#undef LOCTEXT_NAMESPACE





