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

	inline void transitionSetRenderTargetsHelper(RHICommandList& RHICmdList, RHITexture* newRenderTarget, RHITexture* newDepthStencilTarget, FExclusiveDepthStencil depthStencilAccess)
	{
		int32 transitionIndex = 0;
		RHITexture* transitions[2];
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

	inline void setRenderTarget(RHICommandList& RHICmdList, RHITexture* newRenderTarget, RHITexture* newDepthStencilTarget, ESimpleRenderTargetMode mode, FExclusiveDepthStencil depthStencilAccess = FExclusiveDepthStencil::DepthWrite_StencilWrite, bool bWritableBarrier = false)
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

	inline void setRenderTarget(RHICommandList& RHICmdList, RHITexture* newRenderTarget, RHITexture* newDepthStencilTarget, bool bWritableBarrier = false)
	{
		RHIRenderTargetView RTV(newRenderTarget);
		RHIDepthRenderTargetView depthRTV(newDepthStencilTarget);
		if (bWritableBarrier)
		{
			transitionSetRenderTargetsHelper(RHICmdList, newRenderTarget, newDepthStencilTarget, FExclusiveDepthStencil::DepthWrite_StencilWrite);
		}
		RHICmdList.setRenderTargets(1, &RTV, &depthRTV, 0, nullptr);
	}

	inline void setRenderTarget(RHICommandList& RHICmdList, RHITexture* newRenderTarget, int32 mipIndex, int32 arraySliceIndex, RHITexture* newDepthStencilTarget, bool bWritableBarrier = false)
	{
		RHIRenderTargetView rtv(newRenderTarget, mipIndex, arraySliceIndex);
		RHIDepthRenderTargetView DepthRTV(newDepthStencilTarget);
		if (bWritableBarrier)
		{
			transitionSetRenderTargetsHelper(RHICmdList, newRenderTarget, newDepthStencilTarget, FExclusiveDepthStencil::DepthWrite_StencilWrite);
		}
		RHICmdList.setRenderTargets(1, &rtv, &DepthRTV, 0, nullptr);
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

	inline void RHICreateTargetableShaderResourceCube(
		uint32 linearSize,
		uint8 format,
		uint32 numMips,
		uint32 flags,
		uint32 targetableTextureFlags,
		bool bForceSeparateTargetAndShaderResource,
		RHIResourceCreateInfo& createInfo,
		TextureCubeRHIRef& outTargetableTexture,
		TextureCubeRHIRef& outShaderResourceTexture
	)
	{
		BOOST_ASSERT(!(flags & TexCreate_RenderTargetable));
		BOOST_ASSERT(!(flags & TexCreate_ResolveTargetable));
		BOOST_ASSERT(!(flags & TexCreate_ShaderResource));

		BOOST_ASSERT(!(flags & targetableTextureFlags));

		BOOST_ASSERT(targetableTextureFlags & (TexCreate_RenderTargetable | TexCreate_DepthStencilTargetable));

		bForceSeparateTargetAndShaderResource &= (GMaxRHIFeatureLevel > ERHIFeatureLevel::ES2);
		if (!bForceSeparateTargetAndShaderResource)
		{
			outTargetableTexture = outShaderResourceTexture = RHICreateTextureCube(linearSize, format, numMips, flags | targetableTextureFlags | TexCreate_ShaderResource, createInfo);
		}
		else
		{
			outTargetableTexture = RHICreateTextureCube(linearSize, format, numMips, flags | targetableTextureFlags, createInfo);
			outShaderResourceTexture = RHICreateTextureCube(linearSize, format, numMips, flags | TexCreate_ShaderResource | TexCreate_ShaderResource, createInfo);
		}
	}

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

	struct ReadBuffer
	{
		VertexBufferRHIRef mBuffer;
		ShaderResourceViewRHIRef mSRV;
		uint32 mNumBytes;

		ReadBuffer() :mNumBytes(0) {}

		void initialize(uint32 bytesPerElement, uint32 numElements, EPixelFormat format, uint32 additionalUsage = 0)
		{
			BOOST_ASSERT(GSupportsResourceView);
			mNumBytes = bytesPerElement * numElements;
			RHIResourceCreateInfo createInfo;
			mBuffer = RHICreateVertexBuffer(mNumBytes, BUF_ShaderResource | additionalUsage, createInfo);
			mSRV = RHICreateShaderResourceView(mBuffer, bytesPerElement, format);
		}

		void release()
		{
			mNumBytes = 0;
			mBuffer.safeRelease();
			mSRV.safeRelease();
		}
	};

	struct RWBufferStructured
	{
		StructuredBufferRHIRef mBuffer;
		UnorderedAccessViewRHIRef mUAV;
		ShaderResourceViewRHIRef mSRV;
		uint32 mNumBytes;

		RWBufferStructured() :mNumBytes(0) {
		}

		~RWBufferStructured()
		{
			release();
		}

		void initialize(uint32 bytesPerElement, uint32 numElements, uint32 additionalUsage = 0, const TCHAR * inDebugName = nullptr, bool bUseUavCounter = false, bool bAppendBuffer = false)
		{
			BOOST_ASSERT(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5 || GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1);

			BOOST_ASSERT(!((additionalUsage & BUF_FastVRAM) && !inDebugName));

			mNumBytes = bytesPerElement * numElements;

			RHIResourceCreateInfo createInfo;

			createInfo.mDebugName = inDebugName;


			mBuffer = RHICreateStructuredBuffer(bytesPerElement, mNumBytes, BUF_UnorderedAccess | BUF_ShaderResource | additionalUsage, createInfo);
			mUAV = RHICreateUnorderedAccessView(mBuffer, bUseUavCounter, bAppendBuffer);
			mSRV = RHICreateShaderResourceView(mBuffer);

		}

		void release()
		{
			int32 bufferRefCount = mBuffer ? mBuffer->GetRefCount() : -1;

			if (bufferRefCount == 1)
			{
				discardTransientResource();
			}

			mNumBytes = 0;
			mBuffer.safeRelease();
			mUAV.safeRelease();
			mSRV.safeRelease();
		}

		void acquireTransientResource()
		{
			RHIAcquireTransientResource(mBuffer);
		}

		void discardTransientResource()
		{
			RHIDiscardTransientResource(mBuffer);
		}

	};

	struct DynamicReadBuffer : public ReadBuffer
	{
		uint8* mMappedBuffer;

		DynamicReadBuffer()
			: mMappedBuffer(nullptr)
		{}

		virtual ~DynamicReadBuffer()
		{
			release();
		}

		virtual void initialize(uint32 bytesPerElement, uint32 numElements, EPixelFormat format, uint32 additionalUsage = 0)
		{
			BOOST_ASSERT(additionalUsage & (BUF_Dynamic | BUF_Volatile | BUF_Static) && (additionalUsage & (BUF_Dynamic | BUF_Volatile)) ^ (BUF_Dynamic | BUF_Volatile));

			ReadBuffer::initialize(bytesPerElement, numElements, format, additionalUsage);
		}

		void lock()
		{
			BOOST_ASSERT(mMappedBuffer == nullptr);
			BOOST_ASSERT(isValidRef(mBuffer));
			mMappedBuffer = (uint8*)RHILockVertexBuffer(mBuffer, 0, mNumBytes, RLM_ReadOnly);
		}

		void unlock()
		{
			BOOST_ASSERT(mMappedBuffer);
			BOOST_ASSERT(isValidRef(mBuffer));
			RHIUnlockVertexBuffer(mBuffer);
			mMappedBuffer = nullptr;
		}
	};

	inline void transitionRenderPassTargets(RHICommandList& RHICmdList, const RHIRenderPassInfo& RPInfo)
	{
		RHITexture* transitions[MaxSimultaneousRenderTargets + 1];
		int32 transitionIndex = 0;
		uint32 numColorRenderTargets = RPInfo.getNumColorRenderTargets();
		for (uint32 index = 0; index < numColorRenderTargets; index++)
		{
			const RHIRenderPassInfo::ColorEntry& colorRenderTarget = RPInfo.mColorRenderTargets[index];
			if (colorRenderTarget.mRenderTarget != nullptr)
			{
				transitions[transitionIndex] = colorRenderTarget.mRenderTarget;
				transitionIndex++;
			}
		}

		const RHIRenderPassInfo::DepthStencilEntry& depthStencilTarget = RPInfo.mDepthStencilRenderTarget;
		if (depthStencilTarget.mDepthStencilTarget != nullptr && RPInfo.mDepthStencilRenderTarget.mExculusiveDepthStencil.IsDepthWrite())
		{
			transitions[transitionIndex] = depthStencilTarget.mDepthStencilTarget;
			transitionIndex++;
		}

		RHICmdList.transitionResources(EResourceTransitionAccess::EWritable, transitions, transitionIndex);
	}

	struct RWBuffer
	{
		VertexBufferRHIRef mBuffer;
		UnorderedAccessViewRHIRef mUAV;
		ShaderResourceViewRHIRef mSRV;
		uint32 mNumBytes;

		RWBuffer()
			:mNumBytes(0)
		{}

		~RWBuffer()
		{
			release();
		}

		void initialize(uint32 bytesPerElement, uint32 numElements, EPixelFormat format, uint32 additionalUsage = 0, const TCHAR* inDebugName = nullptr, ResourceArrayInterface* inResourceArray = nullptr)
		{
			BOOST_ASSERT(GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5
				|| isVulkanPlatform(GMaxRHIShaderPlatform)
				|| isMetalPlatform(GMaxRHIShaderPlatform)
				|| (GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1 && GSupportsResourceView));

			BOOST_ASSERT(!((additionalUsage & BUF_FastVRAM) && !inDebugName));
			RHIResourceCreateInfo createInfo;
			createInfo.mResourceArray = inResourceArray;
			createInfo.mDebugName = inDebugName;
			mBuffer = RHICreateVertexBuffer(mNumBytes, BUF_UnorderedAccess | BUF_ShaderResource | additionalUsage, createInfo);
			mUAV = RHICreateUnorderedAccessView(mBuffer, format);
			mSRV = RHICreateShaderResourceView(mBuffer, bytesPerElement, format);
		}

		void acquireTransientResource()
		{
			RHIAcquireTransientResource(mBuffer);
		}

		void discardTransientResource()
		{
			RHIDiscardTransientResource(mBuffer);
		}
		void release()
		{
			int32 bufferRefCount = mBuffer ? mBuffer->GetRefCount() : -1;
			if (bufferRefCount == 1)
			{
				discardTransientResource();
			}

			mNumBytes = 0;

			mBuffer.safeRelease();
			mUAV.safeRelease();
			mSRV.safeRelease();
		}
	};

	
}