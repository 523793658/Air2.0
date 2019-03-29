#pragma once
#include "CoreType.h"
#include "RHIResource.h""
#include "RHICommandList.h"
namespace Air
{
	inline void RHICreateTargetableShaderResource2D(uint32 sizeX, uint32 sizeY, uint8 format, uint32 numMips, uint32 flags, uint32 targetableTextureFlags, bool bForceSeparateTargetAndShaderResource, RHIResourceCreateInfo& createInfo, Texture2DRHIRef& outTargetableTexture, Texture2DRHIRef& outShaderResourceTexture, uint32 numSamplers = 1)
	{
		BOOST_ASSERT(!(flags & TexCreate_RenderTargetable));
		BOOST_ASSERT(!(flags & TexCreate_ResolveTargetable));
		BOOST_ASSERT(!(flags & TexCreate_ShaderResource));
		BOOST_ASSERT(!(flags & targetableTextureFlags));

		BOOST_ASSERT(targetableTextureFlags & (TexCreate_RenderTargetable | TexCreate_DepthStencilTargetable | TexCreate_UAV));
		if (numSamplers > 1)
		{
			bForceSeparateTargetAndShaderResource = RHISupportsSeparateMSAAAndResolveTextures(GMaxRHIShaderPlatform);
		}
		if (!bForceSeparateTargetAndShaderResource)
		{
			outTargetableTexture = outShaderResourceTexture = RHICreateTexture2D(sizeX, sizeY, format, numMips, numSamplers, flags | targetableTextureFlags | TexCreate_ShaderResource, createInfo);
		}
		else
		{
			uint32 resolveTargetableTextureFlags = TexCreate_ResolveTargetable;
			if (targetableTextureFlags & TexCreate_DepthStencilTargetable)
			{
				resolveTargetableTextureFlags |= TexCreate_DepthStencilResolveTarget;
			}
			outTargetableTexture = RHICreateTexture2D(sizeX, sizeY, format, numMips, numSamplers, resolveTargetableTextureFlags | flags, createInfo);
			outShaderResourceTexture = RHICreateTexture2D(sizeX, sizeY, format, numMips, 1, flags | resolveTargetableTextureFlags | TexCreate_ShaderResource, createInfo);
		}
	}

	inline void decodeRenderTargetMode(ESimpleRenderTargetMode mode, ERenderTargetLoadAction& colorLoadAction, ERenderTargetStoreAction& colorStoreAction, ERenderTargetLoadAction& depthLoadAction, ERenderTargetStoreAction& depthStoreAction, FExclusiveDepthStencil depthStencilUsage)
	{
		colorStoreAction = ERenderTargetStoreAction::EStore;
		depthStoreAction = ERenderTargetStoreAction::EStore;
		switch (mode)
		{
		case Air::ESimpleRenderTargetMode::EExistingColorAndDepth:
			colorLoadAction = ERenderTargetLoadAction::ELoad;
			depthLoadAction = ERenderTargetLoadAction::ELoad;
			break;
		case Air::ESimpleRenderTargetMode::EUninitializedColorAndDepth:
			colorLoadAction = ERenderTargetLoadAction::ENoAction;
			depthLoadAction = ERenderTargetLoadAction::ENoAction;
			break;
		case Air::ESimpleRenderTargetMode::EUninitializedColorExistingDepth:
			colorLoadAction = ERenderTargetLoadAction::ENoAction;
			depthLoadAction = ERenderTargetLoadAction::ELoad;
			break;
		case Air::ESimpleRenderTargetMode::EUninitializedColorClearDepth:
			colorLoadAction = ERenderTargetLoadAction::ENoAction;
			depthLoadAction = ERenderTargetLoadAction::EClear;
			break;
		case Air::ESimpleRenderTargetMode::EClearColorExistingDepth:
			colorLoadAction = ERenderTargetLoadAction::EClear;
			depthLoadAction = ERenderTargetLoadAction::ELoad;
			break;
		case Air::ESimpleRenderTargetMode::EClearColorAndDepth:
			colorLoadAction = ERenderTargetLoadAction::EClear;
			depthLoadAction = ERenderTargetLoadAction::EClear;
			break;
		case Air::ESimpleRenderTargetMode::EExistingContents_NoDepthStore:
			colorLoadAction = ERenderTargetLoadAction::ELoad;
			depthLoadAction = ERenderTargetLoadAction::ELoad;
			depthStoreAction = ERenderTargetStoreAction::ENoAction;
			break;
		case Air::ESimpleRenderTargetMode::EExistingColorAndClearDepth:
			colorLoadAction = ERenderTargetLoadAction::ELoad;
			depthLoadAction = ERenderTargetLoadAction::EClear;
			break;
		case Air::ESimpleRenderTargetMode::EExistingColorAndDepthAndClearStencil:
			colorLoadAction = ERenderTargetLoadAction::ELoad;
			depthLoadAction = ERenderTargetLoadAction::ELoad;
			break;
		default:
			break;
		}
		if (!depthStencilUsage.IsDepthWrite())
		{
			depthStoreAction = ERenderTargetStoreAction::ENoAction;
		}
	}

	inline void transitionSetRenderTargetsHelper(RHICommandList& RHICmdList, TextureRHIParamRef newRenderTarget, TextureRHIParamRef newDepthStencilTarget, FExclusiveDepthStencil depthStencilAccess)
	{
		int32 transitionIndex = 0;
		TextureRHIParamRef transitions[2];
		if (newRenderTarget)
		{
			transitions[transitionIndex] = newRenderTarget;
			++transitionIndex;
		}
		if (newDepthStencilTarget && depthStencilAccess.IsDepthWrite())
		{
			transitions[transitionIndex] = newDepthStencilTarget;
			++transitionIndex;
		}
		RHICmdList.transitionResources(EResourceTransitionAccess::EWritable, transitions, transitionIndex);
	}

	inline void setRenderTarget(RHICommandList& RHICmdList, TextureRHIParamRef newRenderTarget, TextureRHIParamRef newDepthStencilTarget, ESimpleRenderTargetMode mode, FExclusiveDepthStencil depthStencilAccess = FExclusiveDepthStencil::DepthWrite_StencilWrite, bool bWritableBarrier = false)
	{
		ERenderTargetLoadAction colorLoadAction, depthLoadAction;
		ERenderTargetStoreAction colorStoreAction, depthStoreAction;
		decodeRenderTargetMode(mode, colorLoadAction, colorStoreAction, depthLoadAction, depthStoreAction, depthStencilAccess);

		if (bWritableBarrier)
		{

		}
		RHIRenderTargetView colorView(newRenderTarget, 0, -1, colorLoadAction, colorStoreAction);
		RHISetRenderTargetsInfo info(1, &colorView, RHIDepthRenderTargetView(newDepthStencilTarget, depthLoadAction, depthStoreAction, depthStencilAccess));
		RHICmdList.setRenderTargetAndClear(info);
	}

	inline void setRenderTarget(RHICommandList& RHICmdList, TextureRHIParamRef newRenderTarget, TextureRHIParamRef newDepthStencilTarget, bool bWritableBarrier = false)
	{
		RHIRenderTargetView RTV(newRenderTarget);
		RHIDepthRenderTargetView depthRTV(newDepthStencilTarget);
		if (bWritableBarrier)
		{
			transitionSetRenderTargetsHelper(RHICmdList, newRenderTarget, newDepthStencilTarget, FExclusiveDepthStencil::DepthWrite_StencilWrite);
		}
		RHICmdList.setRenderTargets(1, &RTV, &depthRTV, 0, nullptr);
	}

	//inline void RHICreateTargetableShaderResource2DArray(
	//	uint32 width, 
	//	uint32 height,
	//	uint32 size,
	//	uint8 format,
	//	uint32 numMips,
	//	uint32 flags,
	//	uint32 targetableTextureFlags,
	//	RHIResourceCreateInfo& createInfo,
	//	Texture2DRHIRef
	//)

	inline uint32 getVertexCountForPrimitiveCount(uint32 numPrimitives, uint32 primitiveType)
	{
		uint32 vertexCount = 0;
		switch (primitiveType)
		{
		case PT_TriangleList: vertexCount = numPrimitives * 3; break;
		case PT_TriangleStrip: vertexCount = numPrimitives + 2; break;
		case PT_LineList: vertexCount = numPrimitives * 2; break;
		case PT_PointList: vertexCount = numPrimitives; break;
		case PT_1_ControlPointPatchList:
		case PT_2_ControlPointPatchList:
		case PT_3_ControlPointPatchList:
		case PT_4_ControlPointPatchList:
		case PT_5_ControlPointPatchList:
		case PT_6_ControlPointPatchList:
		case PT_7_ControlPointPatchList:
		case PT_8_ControlPointPatchList:
		case PT_9_ControlPointPatchList:
		case PT_10_ControlPointPatchList:
		case PT_11_ControlPointPatchList:
		case PT_12_ControlPointPatchList:
		case PT_13_ControlPointPatchList:
		case PT_14_ControlPointPatchList:
		case PT_15_ControlPointPatchList:
		case PT_16_ControlPointPatchList:
		case PT_17_ControlPointPatchList:
		case PT_18_ControlPointPatchList:
		case PT_19_ControlPointPatchList:
		case PT_20_ControlPointPatchList:
		case PT_21_ControlPointPatchList:
		case PT_22_ControlPointPatchList:
		case PT_23_ControlPointPatchList:
		case PT_24_ControlPointPatchList:
		case PT_25_ControlPointPatchList:
		case PT_26_ControlPointPatchList:
		case PT_27_ControlPointPatchList:
		case PT_28_ControlPointPatchList:
		case PT_29_ControlPointPatchList:
		case PT_30_ControlPointPatchList:
		case PT_31_ControlPointPatchList:
		case PT_32_ControlPointPatchList:
			vertexCount = (primitiveType - PT_1_ControlPointPatchList + 1) * numPrimitives;
			break;
		default:
			break;
		}
		return vertexCount;
	}

	inline void drawIndexedPrimitiveUP(RHICommandList& rhiCmdList,
		uint32 primitiveType,
		uint32 minVertexIndex,
		uint32 numVertices,
		uint32 numPrimitives,
		const void* indexData,
		uint32 indexDataStride,
		const void* vertexData,
		uint32 vertexDataStride)
	{
		void* vertexBuffer = nullptr;
		void * indexBuffer = nullptr;
		const uint32 numIndices = getVertexCountForPrimitiveCount(numPrimitives, primitiveType);
		rhiCmdList.beginDrawIndexedPrimitiveUP(primitiveType, numPrimitives, numVertices, vertexDataStride, vertexBuffer, minVertexIndex, numIndices, indexDataStride, indexBuffer);
		Memory::memcpy(vertexBuffer, vertexData, numVertices * vertexDataStride);
		Memory::memcpy(indexBuffer, indexData, numIndices * indexDataStride);
		rhiCmdList.endDrawIndexedPrimitiveUP();
	}

	inline void drawPrimitiveUP(RHICommandList& RHICmdList, uint32 primitiveType, uint32 numPrimitives, const void* vertexData, uint32 vertexDataStride)
	{
		void * buffer = nullptr;
		const uint32 vertexCount = getVertexCountForPrimitiveCount(numPrimitives, primitiveType);
		RHICmdList.beginDrawPrimitiveUP(primitiveType, numPrimitives, numPrimitives, vertexDataStride, buffer);
		Memory::memcpy(buffer, vertexData, vertexCount * vertexDataStride);
		RHICmdList.endDrawPrimitiveUP();
	}
}