﻿// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshLatentUpdateTestActor.generated.h"

class URealtimeMeshSimple;


/*
 * This is a test where the mesh structure is created on mesh generation, and the actual data is generated and applied later in BeginPlay
 */
UCLASS()
class REALTIMEMESHTESTS_API ARealtimeMeshLatentUpdateTestActor : public ARealtimeMeshActor
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TObjectPtr<URealtimeMeshSimple> RealtimeMesh;
	
	UPROPERTY()
	FRealtimeMeshSectionGroupKey GroupA;
	
	UPROPERTY()
	FRealtimeMeshSectionGroupKey GroupB;

	
	// Sets default values for this actor's properties
	ARealtimeMeshLatentUpdateTestActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
};