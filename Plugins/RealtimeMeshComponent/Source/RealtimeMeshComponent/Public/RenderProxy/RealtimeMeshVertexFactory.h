﻿// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "RealtimeMeshGPUBuffer.h"
#include "ShaderParameters.h"
#include "Components.h"
#include "VertexFactory.h"
#include "Core/RealtimeMeshStreamRange.h"


class FMaterial;
class FSceneView;
struct FMeshBatchElement;


namespace RealtimeMesh
{
	class FRealtimeMeshNullColorVertexBuffer : public FVertexBuffer
	{
	public:
		FRealtimeMeshNullColorVertexBuffer() = default;
		virtual ~FRealtimeMeshNullColorVertexBuffer() override = default;
#if RMC_ENGINE_ABOVE_5_3
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
		virtual void InitRHI() override;
#endif
		virtual void ReleaseRHI() override;

		FShaderResourceViewRHIRef VertexBufferSRV;
	};
	class FRealtimeMeshNullTangentVertexBuffer : public FVertexBuffer
	{
	public:
		FRealtimeMeshNullTangentVertexBuffer() = default;
		virtual ~FRealtimeMeshNullTangentVertexBuffer() override = default;
#if RMC_ENGINE_ABOVE_5_3
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
		virtual void InitRHI() override;
#endif
		virtual void ReleaseRHI() override;

		FShaderResourceViewRHIRef VertexBufferSRV;
	};
	class FRealtimeMeshNullTexCoordVertexBuffer : public FVertexBuffer
	{
	public:
		FRealtimeMeshNullTexCoordVertexBuffer() = default;
		virtual ~FRealtimeMeshNullTexCoordVertexBuffer() override = default;
#if RMC_ENGINE_ABOVE_5_3
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
		virtual void InitRHI() override;
#endif
		virtual void ReleaseRHI() override;

		FShaderResourceViewRHIRef VertexBufferSRV;
	};

	/** The global null color vertex buffer, which is set with a stride of 0 on meshes without a color component. */
#if RMC_ENGINE_ABOVE_5_3
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer, FRenderResource::EInitPhase::Pre> GRealtimeMeshNullColorVertexBuffer;
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer, FRenderResource::EInitPhase::Pre> GRealtimeMeshNullTangentVertexBuffer;
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer, FRenderResource::EInitPhase::Pre> GRealtimeMeshNullTexCoordVertexBuffer;
#else
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer> GRealtimeMeshNullColorVertexBuffer;
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer> GRealtimeMeshNullTangentVertexBuffer;
	extern REALTIMEMESHCOMPONENT_API TGlobalResource<FNullColorVertexBuffer> GRealtimeMeshNullTexCoordVertexBuffer;
#endif

	
	extern REALTIMEMESHCOMPONENT_API TUniformBufferRef<FLocalVertexFactoryUniformShaderParameters> CreateRealtimeMeshVFUniformBuffer(
		const class FRealtimeMeshLocalVertexFactory* VertexFactory, uint32 LODLightmapDataIndex);


	class REALTIMEMESHCOMPONENT_API FRealtimeMeshVertexFactory : public FVertexFactory
	{
	public:
		FRealtimeMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
			: FVertexFactory(InFeatureLevel)
		{
		}

		virtual EPrimitiveType GetPrimitiveType() const { return PT_TriangleList; }

		
		virtual FIndexBuffer& GetIndexBuffer(bool& bDepthOnly, bool& bMatrixInverted, struct FRealtimeMeshResourceReferenceList& ActiveResources) const = 0;

		virtual FRealtimeMeshStreamRange GetValidRange() const = 0;
		virtual bool IsValidStreamRange(const FRealtimeMeshStreamRange& StreamRange) const = 0;

		virtual void Initialize(FRHICommandListBase& RHICmdList, const TMap<FRealtimeMeshStreamKey, TSharedPtr<FRealtimeMeshGPUBuffer>>& Buffers) = 0;

		virtual FRHIUniformBuffer* GetUniformBuffer() const = 0;

		virtual bool GatherVertexBufferResources(struct FRealtimeMeshResourceReferenceList& ActiveResources) const = 0;
		
	protected:
		static TSharedPtr<FRealtimeMeshGPUBuffer> FindBuffer(const FRealtimeMeshStreamProxyMap& Buffers, ERealtimeMeshStreamType StreamType, FName BufferName)
		{
			const FRealtimeMeshStreamKey Key(StreamType, BufferName);
			const TSharedPtr<FRealtimeMeshGPUBuffer>* FoundBuffer = Buffers.Find(Key);
			return FoundBuffer ? *FoundBuffer : TSharedPtr<FRealtimeMeshGPUBuffer>();
		}

		static void BindVertexBufferSRV(bool& bIsValid, FRHIShaderResourceView*& OutStreamSRV, const FRealtimeMeshStreamProxyMap& Buffers, FName BufferName,
		                                bool bIsOptional = false)
		{
			const TSharedPtr<FRealtimeMeshGPUBuffer> FoundBuffer = FindBuffer(Buffers, ERealtimeMeshStreamType::Vertex, BufferName);

			if (!FoundBuffer.IsValid())
			{
				// If the buffer isn't optional, invalidate the result
				bIsValid &= bIsOptional;
				return;
			}

			const TSharedPtr<FRealtimeMeshVertexBuffer> VertexBuffer = StaticCastSharedPtr<FRealtimeMeshVertexBuffer>(FoundBuffer);
			OutStreamSRV = VertexBuffer->ShaderResourceViewRHI;
		}

		static void BindVertexBuffer(bool& bIsValid, FInt32Range& ValidRange, TSet<TWeakPtr<FRealtimeMeshVertexBuffer>>& InUseBuffers, FVertexStreamComponent& OutStreamComponent,
		                             const FRealtimeMeshStreamProxyMap& Buffers,
		                             FName BufferName, EVertexStreamUsage Usage, bool bIsOptional = false, uint8 ElementIndex = 0,
		                             bool bAllowZeroStride = false)
		{
			const TSharedPtr<FRealtimeMeshGPUBuffer> FoundBuffer = FindBuffer(Buffers, ERealtimeMeshStreamType::Vertex, BufferName);

			if (!FoundBuffer.IsValid())
			{
				// If the buffer isn't optional, invalidate the result
				bIsValid &= bIsOptional;
				if (!bIsOptional)
				{
					ValidRange = FInt32Range(0, 0);
				}
				return;
			}

			const TSharedPtr<FRealtimeMeshVertexBuffer> VertexBuffer = StaticCastSharedPtr<FRealtimeMeshVertexBuffer>(FoundBuffer);

			const uint16 ElementOffset = ElementIndex * VertexBuffer->GetElementStride();

			const bool bIsElementValid = static_cast<uint32>(ElementOffset + VertexBuffer->GetElementStride()) <= VertexBuffer->GetStride();
			bIsValid &= bIsOptional || bIsElementValid;

			if (bIsElementValid)
			{
				InUseBuffers.Add(VertexBuffer);

				const bool bIsZeroStride = bAllowZeroStride && VertexBuffer->Num() == 1;
				const int32 Stride = bIsZeroStride ? 0 : VertexBuffer->GetStride();

				OutStreamComponent = FVertexStreamComponent(VertexBuffer.Get(), ElementOffset, Stride, VertexBuffer->GetVertexType(), Usage);

				// Update the valid range
				// In the case of a zero stride buffer, where 1 element applies to the entire range, we don't need to intersect the buffers
				if (!bIsZeroStride)
				{
					ValidRange = FInt32Range::Intersection(ValidRange, FInt32Range(0, VertexBuffer->Num()));
				}
			}
		}

#if RMC_ENGINE_ABOVE_5_3		
		static void BindTexCoordsBuffer(bool& bIsValid, FInt32Range& ValidRange, TSet<TWeakPtr<FRealtimeMeshVertexBuffer>>& InUseBuffers, TArray<FVertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2>>& OutStreamComponents,
				uint8& OutNumTexCoords, const FRealtimeMeshStreamProxyMap& Buffers, FName BufferName, EVertexStreamUsage Usage, bool bIsOptional = false, bool bAllowZeroStride = false)
#else
		static void BindTexCoordsBuffer(bool& bIsValid, FInt32Range& ValidRange, TSet<TWeakPtr<FRealtimeMeshVertexBuffer>>& InUseBuffers, TArray<FVertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2>>& OutStreamComponents,
				int32& OutNumTexCoords, const FRealtimeMeshStreamProxyMap& Buffers, FName BufferName, EVertexStreamUsage Usage, bool bIsOptional = false, bool bAllowZeroStride = false)
#endif
		{
			const TSharedPtr<FRealtimeMeshGPUBuffer> FoundBuffer = FindBuffer(Buffers, ERealtimeMeshStreamType::Vertex, BufferName);

			if (!FoundBuffer.IsValid())
			{
				// If the buffer isn't optional, invalidate the result
				bIsValid &= bIsOptional;
				if (!bIsOptional)
				{
					ValidRange = FInt32Range(0, 0);
				}
				return;
			}

			const TSharedPtr<FRealtimeMeshVertexBuffer> VertexBuffer = StaticCastSharedPtr<FRealtimeMeshVertexBuffer>(FoundBuffer);

			OutNumTexCoords = VertexBuffer->NumElements();

			InUseBuffers.Add(VertexBuffer);
			const bool bIsZeroStride = bAllowZeroStride && VertexBuffer->Num() == 1;

			

			for (int32 Index = 0; Index < VertexBuffer->NumElements();)
			{
				// if we have 2 or more remaining elements we can bind in groups of two
				const int32 RemainingElements = VertexBuffer->NumElements() - Index;
				const int32 ElementOffset = Index * VertexBuffer->GetElementStride();
				const EVertexElementType VertexType = VertexBuffer->GetVertexType();
				const auto DoubleElementType = FRealtimeMeshElementType(VertexBuffer->GetBufferLayout().GetElementType().GetDatumType(),
					VertexBuffer->GetBufferLayout().GetElementType().GetNumDatums() * 2);
				const EVertexElementType DoubleVertexType = FRealtimeMeshBufferLayoutUtilities::GetElementTypeDetails(DoubleElementType).GetVertexType();
				
				if (RemainingElements >= 2 && DoubleVertexType != VET_None)
				{
					OutStreamComponents.Emplace(VertexBuffer.Get(), ElementOffset, VertexBuffer->GetStride(), DoubleVertexType, Usage);
					Index += 2;
				}
				else
				{
					OutStreamComponents.Emplace(VertexBuffer.Get(), ElementOffset, VertexBuffer->GetStride(), VertexType, Usage);
					Index += 1;
				}
			}
			
			// Update the valid range
			// In the case of a zero stride buffer, where 1 element applies to the entire range, we don't need to intersect the buffers
			if (!bIsZeroStride)
			{
				ValidRange = FInt32Range::Intersection(ValidRange, FInt32Range(0, VertexBuffer->Num()));
			}
		}

		static void BindIndexBuffer(bool& bIsValid, FInt32Range& ValidRange, TWeakPtr<FRealtimeMeshIndexBuffer>& OutIndexBuffer, const FRealtimeMeshStreamProxyMap& Buffers,
		                            FName BufferName, bool bIsOptional = false)
		{
			const TSharedPtr<FRealtimeMeshGPUBuffer> FoundBuffer = FindBuffer(Buffers, ERealtimeMeshStreamType::Index, BufferName);

			if (!FoundBuffer.IsValid() || !FoundBuffer->IsResourceInitialized() || !StaticCastSharedPtr<FRealtimeMeshIndexBuffer>(FoundBuffer)->IndexBufferRHI)
			{
				// If the buffer isn't optional, invalidate the result
				if (!bIsOptional)
				{
					bIsValid = false;
					ValidRange = FInt32Range(0, 0);
				}
				return;
			}

			const TSharedPtr<FRealtimeMeshIndexBuffer> IndexBuffer = StaticCastSharedPtr<FRealtimeMeshIndexBuffer>(FoundBuffer);
			OutIndexBuffer = IndexBuffer;

			// Update the valid range
			ValidRange = FInt32Range::Intersection(ValidRange, FInt32Range(0, IndexBuffer->Num()));
		}
	};


	/**
	 * A basic vertex factory which closely resembles the functionality of LocalVertexFactory to show how to make custom vertex factories for the RMC
	 */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshLocalVertexFactory : public FRealtimeMeshVertexFactory
	{
		DECLARE_VERTEX_FACTORY_TYPE(FRealtimeMeshLocalVertexFactory);

	public:
		FRealtimeMeshLocalVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
			: FRealtimeMeshVertexFactory(InFeatureLevel)
			  , IndexBuffer(nullptr)
			  , DepthOnlyIndexBuffer(nullptr)
			  , ReversedIndexBuffer(nullptr)
			  , ReversedDepthOnlyIndexBuffer(nullptr)
			  , ValidRange(FRealtimeMeshStreamRange::Empty())
			  , ColorStreamIndex(INDEX_NONE)
		{
		}

		struct FDataType : public FStaticMeshDataType
		{
			FVertexStreamComponent PreSkinPositionComponent;
			FRHIShaderResourceView* PreSkinPositionComponentSRV = nullptr;
#if WITH_EDITORONLY_DATA
			const class UStaticMesh* StaticMesh = nullptr;
			bool bIsCoarseProxy = false;
#endif
		};

		/**
		 * Should we cache the material's shadertype on this platform with this vertex factory? 
		 */
		static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);

		static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

		static void ValidateCompiledResult(const FVertexFactoryType* Type, EShaderPlatform Platform, const FShaderParameterMap& ParameterMap, TArray<FString>& OutErrors);

		static void GetPSOPrecacheVertexFetchElements(EVertexInputStreamType VertexInputStreamType, FVertexDeclarationElementList& Elements);
		
#if RMC_ENGINE_ABOVE_5_2
		static void GetVertexElements(ERHIFeatureLevel::Type FeatureLevel, EVertexInputStreamType InputStreamType, bool bSupportsManualVertexFetch, FDataType& Data, FVertexDeclarationElementList& Elements);
#endif
		
		/**
		* Copy the data from another vertex factory
		* @param Other - factory to copy from
		*/
		void Copy(const FRealtimeMeshLocalVertexFactory& Other);

		virtual EPrimitiveType GetPrimitiveType() const override { return PT_TriangleList; }

		virtual FIndexBuffer& GetIndexBuffer(bool& bDepthOnly, bool& bMatrixInverted, struct FRealtimeMeshResourceReferenceList& ActiveResources) const override;

		virtual FRealtimeMeshStreamRange GetValidRange() const override { return ValidRange; }
		virtual bool IsValidStreamRange(const FRealtimeMeshStreamRange& StreamRange) const override;

		virtual void Initialize(FRHICommandListBase& RHICmdList, const TMap<FRealtimeMeshStreamKey, TSharedPtr<FRealtimeMeshGPUBuffer>>& Buffers) override;

		virtual bool GatherVertexBufferResources(struct FRealtimeMeshResourceReferenceList& ActiveResources) const override;

		// FRenderResource interface.
#if RMC_ENGINE_ABOVE_5_3
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
		virtual void InitRHI() override;
#endif
		
		virtual void ReleaseRHI() override;


		void SetColorOverrideStream(FRHICommandList& RHICmdList, const FVertexBuffer* ColorVertexBuffer) const;

		void GetColorOverrideStream(const FVertexBuffer* ColorVertexBuffer, FVertexInputStreamArray& VertexStreams) const;

		FORCEINLINE FRHIShaderResourceView* GetPositionsSRV() const
		{
			return Data.PositionComponentSRV;
		}

		FORCEINLINE FRHIShaderResourceView* GetPreSkinPositionSRV() const
		{
			return Data.PreSkinPositionComponentSRV ? Data.PreSkinPositionComponentSRV : GNullColorVertexBuffer.VertexBufferSRV.GetReference();
		}

		FORCEINLINE FRHIShaderResourceView* GetTangentsSRV() const
		{
			return Data.TangentsSRV;
		}

		FORCEINLINE FRHIShaderResourceView* GetTextureCoordinatesSRV() const
		{
			return Data.TextureCoordinatesSRV;
		}

		FORCEINLINE FRHIShaderResourceView* GetColorComponentsSRV() const
		{
			return Data.ColorComponentsSRV;
		}

		FORCEINLINE uint32 GetColorIndexMask() const
		{
			return Data.ColorIndexMask;
		}

		FORCEINLINE int GetLightMapCoordinateIndex() const
		{
			return Data.LightMapCoordinateIndex;
		}

		FORCEINLINE int GetNumTexcoords() const
		{
			return Data.NumTexCoords;
		}

		virtual FRHIUniformBuffer* GetUniformBuffer() const override
		{
			return UniformBuffer.GetReference();
		}

	protected:
		const FDataType& GetData() const { return Data; }


#if RMC_ENGINE_ABOVE_5_2
		static
#endif
		void GetVertexElements(
			ERHIFeatureLevel::Type FeatureLevel, 
			EVertexInputStreamType InputStreamType, 
			bool bSupportsManualVertexFetch,
			FDataType& Data, 
			FVertexDeclarationElementList& Elements,
#if RMC_ENGINE_ABOVE_5_2
			FVertexStreamList& InOutStreams, 
#endif
			int32& OutColorStreamIndex);
		
		FDataType Data;
		TUniformBufferRef<FLocalVertexFactoryUniformShaderParameters> UniformBuffer;

		TSet<TWeakPtr<FRealtimeMeshVertexBuffer>> InUseVertexBuffers;

		TWeakPtr<FRealtimeMeshIndexBuffer> IndexBuffer;
		TWeakPtr<FRealtimeMeshIndexBuffer> DepthOnlyIndexBuffer;
		TWeakPtr<FRealtimeMeshIndexBuffer> ReversedIndexBuffer;
		TWeakPtr<FRealtimeMeshIndexBuffer> ReversedDepthOnlyIndexBuffer;

		FRealtimeMeshStreamRange ValidRange;

		int32 ColorStreamIndex;
	};


	/** Shader parameter class used by FRealtimeMeshVertexFactory only - no derived classes. */
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
	{
		DECLARE_TYPE_LAYOUT(FRealtimeMeshVertexFactoryShaderParameters, NonVirtual);

	public:
		void Bind(const FShaderParameterMap& ParameterMap);

		void GetElementShaderBindings(const FSceneInterface* Scene, const FSceneView* View, const FMeshMaterialShader* Shader,
		                              const EVertexInputStreamType InputStreamType, ERHIFeatureLevel::Type FeatureLevel, const FVertexFactory* VertexFactory,
		                              const FMeshBatchElement& BatchElement, FMeshDrawSingleShaderBindings& ShaderBindings, FVertexInputStreamArray& VertexStreams) const;

		// SpeedTree LOD parameter
		LAYOUT_FIELD(FShaderParameter, LODParameter);

		// True if LODParameter is bound, which puts us on the slow path in GetElementShaderBindings
		LAYOUT_FIELD(bool, bAnySpeedTreeParamIsBound);
	};
}
