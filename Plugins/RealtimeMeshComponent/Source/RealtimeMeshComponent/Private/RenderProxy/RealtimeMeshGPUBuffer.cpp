// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#include "RenderProxy/RealtimeMeshGPUBuffer.h"
#include "Data/RealtimeMeshUpdateBuilder.h"

namespace RealtimeMesh
{
	void FRealtimeMeshSectionGroupStreamUpdateData::CreateBufferAsyncIfPossible(FRealtimeMeshUpdateContext& UpdateContext)
	{
		if (GRHISupportsAsyncTextureCreation)
		{
			auto& RHICmdList = UpdateContext.GetRHICmdList();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
			
#else
			FRHIResourceCreateInfo CreateInfo(TEXT("RealtimeMeshBuffer-Temp"), &Stream);
			CreateInfo.bWithoutNativeResource = Stream.Num() == 0 || Stream.GetStride() == 0;
#endif
				
			if (GetStreamKey().IsVertexStream())
			{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
				if (Stream.Num() == 0 || Stream.GetStride() == 0)
				{
					const FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::CreateNull(TEXT("RealtimeMeshBuffer-Temp"));

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
				else
				{
					FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::Create(TEXT("RealtimeMeshBuffer-Temp"), Stream.GetResourceDataSize(), Stream.GetStride(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource)
						.SetInitialState(ERHIAccess::SRVMask)
						.SetInitActionResourceArray(&Stream);

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
#else

#if RMC_ENGINE_ABOVE_5_5
				Buffer = RHICmdList.CreateBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource,
					Stream.GetStride(), ERHIAccess::SRVMask, CreateInfo);
#else
				Buffer = RHICmdList->CreateBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource,
					Stream.GetStride(), ERHIAccess::SRVMask, CreateInfo);
#endif

#endif
			}
			else
			{
				check(GetStreamKey().IsIndexStream());

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
				if (Stream.Num() == 0 || Stream.GetStride() == 0)
				{
					const FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::CreateNull(TEXT("RealtimeMeshBuffer-Temp"));

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
				else
				{
					FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::Create(TEXT("RealtimeMeshBuffer-Temp"), Stream.GetResourceDataSize(), Stream.GetElementStride(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource)
						.SetInitialState(ERHIAccess::SRVMask)
						.SetInitActionResourceArray(&Stream);

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
#else

#if RMC_ENGINE_ABOVE_5_5
				Buffer = RHICmdList.CreateBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource,
					Stream.GetElementStride(), ERHIAccess::SRVMask, CreateInfo);
#else
				Buffer = RHICmdList->CreateBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource,
					Stream.GetElementStride(), ERHIAccess::SRVMask, CreateInfo);
#endif

#endif
			}				
		}
	}

	void FRealtimeMeshSectionGroupStreamUpdateData::FinalizeInitialization(FRHICommandListBase& RHICmdList)
	{
		if (!Buffer.IsValid())
		{
			check(Stream.GetResourceDataSize());
				
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6

#else
			FRHIResourceCreateInfo CreateInfo(TEXT("RealtimeMeshBuffer-Temp"), &Stream);
			CreateInfo.bWithoutNativeResource = Stream.Num() == 0 || Stream.GetStride() == 0;
#endif

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
			if (GetStreamKey().IsVertexStream())
			{
				if (Stream.Num() == 0 || Stream.GetStride() == 0)
				{
					const FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::CreateNull(TEXT("RealtimeMeshBuffer-Temp"));

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
				else
				{
					FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::Create(TEXT("RealtimeMeshBuffer-Temp"), Stream.GetResourceDataSize(), Stream.GetStride(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource)
						.SetInitialState(ERHIAccess::SRVMask)
						.SetInitActionResourceArray(&Stream);

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
			}
			else
			{
				check(GetStreamKey().IsIndexStream());

				if (Stream.Num() == 0 || Stream.GetStride() == 0)
				{
					const FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::CreateNull(TEXT("RealtimeMeshBuffer-Temp"));

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
				else
				{
					FRHIBufferCreateDesc CreateDesc =
						FRHIBufferCreateDesc::Create(TEXT("RealtimeMeshBuffer-Temp"), Stream.GetResourceDataSize(), Stream.GetElementStride(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource)
						.SetInitialState(ERHIAccess::SRVMask)
						.SetInitActionResourceArray(&Stream);

					Buffer = RHICmdList.CreateBuffer(CreateDesc);
				}
			}
#else

#if RMC_ENGINE_ABOVE_5_3
			if (GetStreamKey().IsVertexStream())
			{
				Buffer = RHICmdList.CreateVertexBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource, CreateInfo);
			}
			else
			{
				check(GetStreamKey().IsIndexStream());
				Buffer =  RHICmdList.CreateIndexBuffer(Stream.GetElementStride(), Stream.GetResourceDataSize(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource, CreateInfo);
			}
#else
				if (GetStreamKey().IsVertexStream())
				{
					Buffer = RHICreateVertexBuffer(Stream.GetResourceDataSize(), UsageFlags | BUF_VertexBuffer | BUF_ShaderResource, CreateInfo);
				}
				else
				{
					check(GetStreamKey().IsIndexStream());
					Buffer = RHICreateIndexBuffer(Stream.GetElementStride(), Stream.GetResourceDataSize(), UsageFlags | BUF_IndexBuffer | BUF_ShaderResource, CreateInfo);
				}
#endif

#endif
		}
	}
}
