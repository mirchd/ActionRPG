﻿// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "RealtimeMeshCore.h"
#include "RealtimeMeshGuard.h"
#include "Core/RealtimeMeshKeys.h"
#include "Core/RealtimeMeshDataStream.h"
#include "Async/Async.h"

struct FRealtimeMeshSimpleGeometry;
struct FRealtimeMeshCollisionConfiguration;
struct FRealtimeMeshCollisionInfo;
enum class ERealtimeMeshCollisionUpdateResult : uint8;

namespace RealtimeMesh
{
	// ReSharper disable CppExpressionWithoutSideEffects
	struct FRealtimeMeshBounds
	{
	private:
		TOptional<FBoxSphereBounds3f> UserSetBounds;
		TOptional<FBoxSphereBounds3f> CalculatedBounds;

	public:
		bool HasUserSetBounds() const { return UserSetBounds.IsSet(); }
		void SetUserSetBounds(const FBoxSphereBounds3f& InBounds) { UserSetBounds = InBounds; }
		void ClearUserSetBounds() { UserSetBounds.Reset(); }

		bool HasComputedBounds() const { return CalculatedBounds.IsSet(); }
		void SetComputedBounds(const FBoxSphereBounds3f& InBounds) { CalculatedBounds = InBounds; }
		void ClearCachedValue() { CalculatedBounds.Reset(); }

		bool HasBounds() const { return UserSetBounds.IsSet() || CalculatedBounds.IsSet(); }
		const FBoxSphereBounds3f& GetBounds() const { check(HasBounds()); return UserSetBounds.IsSet()? UserSetBounds.GetValue() : CalculatedBounds.GetValue(); }

		TOptional<FBoxSphereBounds3f> Get() const { return UserSetBounds.IsSet() ? UserSetBounds : CalculatedBounds; }
		
		void Reset()
		{
			UserSetBounds.Reset();
			CalculatedBounds.Reset();
		}

		friend FArchive& operator<<(FArchive& Ar, FRealtimeMeshBounds& Bounds)
		{
			if (Ar.CustomVer(FRealtimeMeshVersion::GUID) >= FRealtimeMeshVersion::DataRestructure)
			{
				Ar << Bounds.UserSetBounds;
				Ar << Bounds.CalculatedBounds;
			}
			else
			{
				FBoxSphereBounds3f TempBounds;
				Ar << TempBounds;
				Bounds.CalculatedBounds = TempBounds;
			}
			return Ar;
		}

	};
	// ReSharper restore CppExpressionWithoutSideEffects



	enum class ERealtimeMeshChangeType
	{
		Unknown,
		Added,
		Updated,
		Removed
	};

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FRealtimeMeshStreamChangedEvent, const FRealtimeMeshSectionGroupKey&, const FRealtimeMeshStreamKey&, ERealtimeMeshChangeType);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FRealtimeMeshStreamPropertyChangedEvent, const FRealtimeMeshSectionGroupKey&, const FRealtimeMeshStreamKey&);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FRealtimeMeshSectionChangedEvent, const FRealtimeMeshSectionKey&, ERealtimeMeshChangeType);
	DECLARE_MULTICAST_DELEGATE_OneParam(FRealtimeMeshSectionPropertyChangedEvent, const FRealtimeMeshSectionKey&);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FRealtimeMeshSectionGroupChangedEvent, const FRealtimeMeshSectionGroupKey&, ERealtimeMeshChangeType);
	DECLARE_MULTICAST_DELEGATE_OneParam(FRealtimeMeshSectionGroupPropertyChangedEvent, const FRealtimeMeshSectionGroupKey&);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FRealtimeMeshLODChangedEvent, const FRealtimeMeshLODKey&, ERealtimeMeshChangeType);
	DECLARE_MULTICAST_DELEGATE_OneParam(FRealtimeMeshLODPropertyChangedEvent, const FRealtimeMeshLODKey&);

	DECLARE_MULTICAST_DELEGATE(FRealtimeMeshPropertyChangedEvent);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FRealtimeMeshRenderDataChangedEvent, bool, int32);


	DECLARE_DELEGATE(FRealtimeMeshRequestEndOfFrameUpdateDelegate);
	DECLARE_DELEGATE_ThreeParams(FRealtimeMeshCollisionUpdateDelegate, const TSharedRef<TPromise<ERealtimeMeshCollisionUpdateResult>>&,
	                             const TSharedRef<FRealtimeMeshCollisionInfo>&, bool);

	DECLARE_MULTICAST_DELEGATE(FRealtimeMeshSimpleEvent);

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSharedResources : public TSharedFromThis<FRealtimeMeshSharedResources>
	{
		mutable FRealtimeMeshGuard Guard;
		FName MeshName;

		TWeakObjectPtr<URealtimeMesh> OwningMesh;
		FRealtimeMeshWeakPtr Owner;
		FRealtimeMeshProxyWeakPtr Proxy;

		FRealtimeMeshSimpleEvent OnRenderProxyRequiresUpdateEvent;
		FRealtimeMeshSimpleEvent OnBoundsChangedEvent;

	public:
		virtual ~FRealtimeMeshSharedResources() = default;

		FRealtimeMeshSharedResources()
		{
		}

		template <typename SharedResourcesType>
		const SharedResourcesType& As() const
		{
			return *static_cast<SharedResourcesType*>(this);
		}

		template <typename SharedResourcesType>
		SharedResourcesType& As()
		{
			return *static_cast<SharedResourcesType*>(this);
		}

		virtual void SetOwnerMesh(URealtimeMesh* InOwningMesh, const FRealtimeMeshRef& InOwner);
		virtual void SetProxy(const FRealtimeMeshProxyRef& InProxy) { Proxy = InProxy; }

		FRealtimeMeshGuard& GetGuard() const { return Guard; }
		FName GetMeshName() const { return MeshName; }
		void SetMeshName(FName InName) { MeshName = InName; }

		URealtimeMesh* GetOwningMesh() const { return OwningMesh.Get(); }
		FRealtimeMeshPtr GetOwner() const { return Owner.Pin(); }
		FRealtimeMeshProxyPtr GetProxy() const { return Proxy.Pin(); }

		ERHIFeatureLevel::Type GetFeatureLevel() const;

		virtual bool WantsStreamOnGPU(const FRealtimeMeshStreamKey& StreamKey) const
		{ 
			static const TSet WantedStreams =
			{
				FRealtimeMeshStreams::Position,
				FRealtimeMeshStreams::Tangents,
				FRealtimeMeshStreams::TexCoords,
				FRealtimeMeshStreams::Color,
				FRealtimeMeshStreams::Triangles
			};
			return WantedStreams.Contains(StreamKey);
		}


		FRealtimeMeshSimpleEvent& OnRenderProxyRequiresUpdate() { return OnRenderProxyRequiresUpdateEvent; }
		FRealtimeMeshSimpleEvent& OnBoundsChanged() { return OnBoundsChangedEvent; }

		
		/*
		FRealtimeMeshSectionChangedEvent& OnSectionChanged() { return SectionChangedEvent; }
		void BroadcastSectionChanged(const FRealtimeMeshSectionKey& SectionKey, ERealtimeMeshChangeType Type) const
		{
			SectionChangedEvent.Broadcast(SectionKey, Type);
		}

		void BroadcastSectionChanged(const TSet<FRealtimeMeshSectionKey>& SectionKeys, ERealtimeMeshChangeType Type) const
		{
			for (const auto& SectionKey : SectionKeys) { SectionChangedEvent.Broadcast(SectionKey, Type); }
		}

		FRealtimeMeshSectionPropertyChangedEvent& OnSectionConfigChanged() { return SectionConfigChangedEvent; }

		void BroadcastSectionConfigChanged(const FRealtimeMeshSectionKey& SectionKey) const
		{
			SectionConfigChangedEvent.Broadcast(SectionKey);
			SectionChangedEvent.Broadcast(SectionKey, ERealtimeMeshChangeType::Updated);
		}

		FRealtimeMeshSectionPropertyChangedEvent& OnSectionStreamRangeChanged() { return SectionStreamRangeChangedEvent; }

		void BroadcastSectionStreamRangeChanged(const FRealtimeMeshSectionKey& SectionKey) const
		{
			SectionStreamRangeChangedEvent.Broadcast(SectionKey);
			SectionChangedEvent.Broadcast(SectionKey, ERealtimeMeshChangeType::Updated);
		}

		FRealtimeMeshSectionPropertyChangedEvent& OnSectionBoundsChanged() { return SectionBoundsChangedEvent; }

		void BroadcastSectionBoundsChanged(const FRealtimeMeshSectionKey& SectionKey) const
		{
			SectionBoundsChangedEvent.Broadcast(SectionKey);
			SectionChangedEvent.Broadcast(SectionKey, ERealtimeMeshChangeType::Updated);
		}


		FRealtimeMeshStreamChangedEvent& OnStreamChanged() { return StreamChangedEvent; }

		void BroadcastStreamChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamKey& StreamKey, ERealtimeMeshChangeType Type) const
		{
			StreamChangedEvent.Broadcast(SectionGroupKey, StreamKey, Type);
		}

		void BroadcastStreamChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const TSet<FRealtimeMeshStreamKey>& StreamKeys, ERealtimeMeshChangeType Type) const
		{
			for (const auto& StreamKey : StreamKeys) { StreamChangedEvent.Broadcast(SectionGroupKey, StreamKey, Type); }
		}

		FRealtimeMeshStreamPropertyChangedEvent& OnStreamRangeChanged() { return StreamRangeChangedEvent; }

		void BroadcastStreamRangeChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamKey& StreamKey) const
		{
			StreamRangeChangedEvent.Broadcast(SectionGroupKey, StreamKey);
			StreamChangedEvent.Broadcast(SectionGroupKey, StreamKey, ERealtimeMeshChangeType::Updated);
		}


		FRealtimeMeshSectionGroupChangedEvent& OnSectionGroupChanged() { return SectionGroupChangedEvent; }

		void BroadcastSectionGroupChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, ERealtimeMeshChangeType Type) const
		{
			SectionGroupChangedEvent.Broadcast(SectionGroupKey, Type);
		}

		void BroadcastSectionGroupChanged(const TSet<FRealtimeMeshSectionGroupKey>& SectionGroupKeys, ERealtimeMeshChangeType Type) const
		{
			for (const auto& SectionGroup : SectionGroupKeys) SectionGroupChangedEvent.Broadcast(SectionGroup, Type);
		}

		FRealtimeMeshSectionGroupPropertyChangedEvent& OnSectionGroupInUseRangeChanged() { return SectionGroupInUseRangeChangedEvent; }

		void BroadcastSectionGroupInUseRangeChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey) const
		{
			SectionGroupInUseRangeChangedEvent.Broadcast(SectionGroupKey);
			SectionGroupChangedEvent.Broadcast(SectionGroupKey, ERealtimeMeshChangeType::Updated);
		}

		FRealtimeMeshSectionGroupPropertyChangedEvent& OnSectionGroupBoundsChanged() { return SectionGroupBoundsChangedEvent; }

		void BroadcastSectionGroupBoundsChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey) const
		{
			SectionGroupBoundsChangedEvent.Broadcast(SectionGroupKey);
			SectionGroupChangedEvent.Broadcast(SectionGroupKey, ERealtimeMeshChangeType::Updated);
		}


		FRealtimeMeshLODChangedEvent& OnLODChanged() { return LODChangedEvent; }
		void BroadcastLODChanged(const FRealtimeMeshLODKey& LODKey, ERealtimeMeshChangeType Type) const { LODChangedEvent.Broadcast(LODKey, Type); }

		FRealtimeMeshLODPropertyChangedEvent& OnLODConfigChanged() { return LODConfigChangedEvent; }

		void BroadcastLODConfigChanged(const FRealtimeMeshLODKey& LODKey) const
		{
			LODConfigChangedEvent.Broadcast(LODKey);
			LODChangedEvent.Broadcast(LODKey, ERealtimeMeshChangeType::Updated);
		}

		FRealtimeMeshLODPropertyChangedEvent& OnLODBoundsChanged() { return LODBoundsChangedEvent; }

		void BroadcastLODBoundsChanged(const FRealtimeMeshLODKey& LODKey) const
		{
			LODBoundsChangedEvent.Broadcast(LODKey);
			LODChangedEvent.Broadcast(LODKey, ERealtimeMeshChangeType::Updated);
		}


		FRealtimeMeshRenderDataChangedEvent& OnMeshRenderDataChanged() { return MeshRenderDataChangedEvent; }
		void BroadcastMeshRenderDataChanged(bool bShouldRecreateProxies, int32 CommandsVersion) const
		{
			if (IsInGameThread())
			{
				MeshRenderDataChangedEvent.Broadcast(bShouldRecreateProxies, CommandsVersion);
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [ThisWeak = this->AsWeak(), bShouldRecreateProxies, CommandsVersion]()
				{
					if (const auto Pinned = ThisWeak.Pin())
					{
						Pinned->MeshRenderDataChangedEvent.Broadcast(bShouldRecreateProxies, CommandsVersion);
					}
				});
			}
		}

		FRealtimeMeshPropertyChangedEvent& OnMeshConfigChanged() { return MeshConfigChangedEvent; }
		void BroadcastMeshConfigChanged() const
		{
			if (IsInGameThread())
			{
				MeshConfigChangedEvent.Broadcast();
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [ThisWeak = this->AsWeak()]()
				{
					if (const auto Pinned = ThisWeak.Pin())
					{
						Pinned->MeshConfigChangedEvent.Broadcast();
					}
				});
			}
		}

		FRealtimeMeshPropertyChangedEvent& OnMeshBoundsChanged() { return MeshBoundsChangedEvent; }
		void BroadcastMeshBoundsChanged() const
		{
			if (IsInGameThread())
			{
				MeshBoundsChangedEvent.Broadcast();
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [ThisWeak = this->AsWeak()]()
				{
					if (const auto Pinned = ThisWeak.Pin())
					{
						Pinned->MeshBoundsChangedEvent.Broadcast();
					}
				});
			}
		}

		FRealtimeMeshRequestEndOfFrameUpdateDelegate& GetEndOfFrameRequestHandler() { return EndOfFrameRequestHandler; }
		FRealtimeMeshCollisionUpdateDelegate& GetCollisionUpdateHandler() { return CollisionUpdateHandler; }*/

	public:
		virtual FRealtimeMeshUpdateStateRef CreateUpdateState() const;
		
		virtual FRealtimeMeshVertexFactoryRef CreateVertexFactory() const;
		virtual FRealtimeMeshSectionProxyRef CreateSectionProxy(const FRealtimeMeshSectionKey& InKey) const;
		virtual FRealtimeMeshSectionGroupProxyRef CreateSectionGroupProxy(const FRealtimeMeshSectionGroupKey& InKey) const;
		virtual FRealtimeMeshLODProxyRef CreateLODProxy(const FRealtimeMeshLODKey& InKey) const;
		virtual FRealtimeMeshProxyRef CreateRealtimeMeshProxy() const;

		virtual FRealtimeMeshSectionRef CreateSection(const FRealtimeMeshSectionKey& InKey) const;
		virtual FRealtimeMeshSectionGroupRef CreateSectionGroup(const FRealtimeMeshSectionGroupKey& InKey) const;
		virtual FRealtimeMeshLODRef CreateLOD(const FRealtimeMeshLODKey& InKey) const;

		virtual FRealtimeMeshRef CreateRealtimeMesh() const;

		virtual FRealtimeMeshSharedResourcesRef CreateSharedResources() const;
	};
}
