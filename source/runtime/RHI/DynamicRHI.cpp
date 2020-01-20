#include "DynamicRHI.h"
#include "RHICommandList.h"
#include "RHI.h"
namespace Air
{
	DynamicRHI* GDynamicRHI = nullptr;

	void RHIInit(bool bHasEditorToken)
	{
		if (!GDynamicRHI)
		{
			bool canEverRender = true;
			if (canEverRender)
			{
				GDynamicRHI = platformCreateDynamicRHI();
				if (GDynamicRHI)
				{
					GDynamicRHI->init();
					GRHICommandList.getImmediateCommandList().setContext(GDynamicRHI->getDefualtContext());

					GRHICommandList.getImmediateAsyncComputeCommandList().setComputeContext(GDynamicRHI->getDefualtAsynicComputeContext());
				}
			}
		}
#if PLATFORM_WINDOWS
		//RHIDetect
#endif
	}

	static struct LockTracker
	{
		struct LockParams
		{
			void* RHIBuffer;
			void* Buffer;
			uint32 bufferSize;
			uint32 offset;
			EResourceLockMode lockMode;

			FORCEINLINE_DEBUGGABLE LockParams(void* inRHIBuffer, void* inBuffer, uint32 inOffset, uint32 inBufferSize, EResourceLockMode inLockMode)
				:RHIBuffer(inRHIBuffer)
				, Buffer(inBuffer)
				, bufferSize(inBufferSize)
				, offset(inOffset)
				, lockMode(inLockMode)
			{}

		};

		TArray<LockParams, TInlineAllocator<16>> mOutstandingLocks;
		uint32 mTotalMemoryOutstanding;

		LockTracker()
		{
			mTotalMemoryOutstanding = 0;
		}

		FORCEINLINE_DEBUGGABLE void lock(void* RHIBuffer, void *buffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
		{
#if DO_CHECK
			for (auto& params : mOutstandingLocks)
			{
				BOOST_ASSERT(params.RHIBuffer != RHIBuffer);
			}
#endif
			mOutstandingLocks.add(LockParams(RHIBuffer, buffer, offset, sizeRHI, lockMode));
			mTotalMemoryOutstanding += sizeRHI;
		}

		FORCEINLINE_DEBUGGABLE LockParams unlock(void* RHIBuffer)
		{
			for (int32 index = 0; index < mOutstandingLocks.size(); index++)
			{
				if (mOutstandingLocks[index].RHIBuffer == RHIBuffer)
				{
					LockParams result = mOutstandingLocks[index];
					mOutstandingLocks.removeAtSwap(index, 1, false);
					return result;
				}
			}
			BOOST_ASSERT(false);
			return LockParams(nullptr, nullptr, 0, 0, RLM_WriteOnly);
		}
	} GLockTracker;

	void DynamicRHI::enableIdealGPUCaptureOptions(bool enable)
	{
	}
	Texture2DRHIRef DynamicRHI::RHICreateTexture2D_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamples, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateTexture2D(width, height, format, numMips, numSamples, flags, createInfo);
	}

	TextureCubeRHIRef DynamicRHI::RHICreateTextureCube_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateTextureCube(size, format, numMips, flags, createInfo);
	}

	ShaderResourceViewRHIRef DynamicRHI::RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHITexture* texture, const RHITextureSRVCreateInfo& createInfo) 
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateShaderResourceView(texture, createInfo);
	}
	StructuredBufferRHIRef DynamicRHI::RHICreateStructuredBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateStructuredBuffer(stride, size, inUsage, createInfo);
	}

	ShaderResourceViewRHIRef DynamicRHI::RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateShaderResourceView(vertexBuffer, stride, format);
	}

	ShaderResourceViewRHIRef DynamicRHI::RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIStructuredBuffer* structuredBuffer)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateShaderResourceView(structuredBuffer);
	}

	UnorderedAccessViewRHIRef DynamicRHI::RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer, uint8 format)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateUnorderedAccessView(indexBuffer, format);
	}

	UnorderedAccessViewRHIRef DynamicRHI::RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHITexture* texture, uint32 mipLevel)
	{
		ScopedRHIThreadStaller stallRHIThread(RHICmdList);
		return GDynamicRHI->RHICreateUnorderedAccessView(texture, mipLevel);
	}

	UnorderedAccessViewRHIRef DynamicRHI::RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint8 format)
	{
		ScopedRHIThreadStaller stallThread(RHICmdList);
		return GDynamicRHI->RHICreateUnorderedAccessView(vertexBuffer, format);
	}

	UnorderedAccessViewRHIRef DynamicRHI::RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer)
	{
		ScopedRHIThreadStaller stallThread(RHICmdList);
		return GDynamicRHI->RHICreateUnorderedAccessView(structuredBuffer, bUseUAVCounter, bAppendBuffer);
	}


	void* DynamicRHI::lockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail, bool bNeedsDefaultRHIFlush)
	{
		if (bNeedsDefaultRHIFlush)
		{
			rhiCmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			return GDynamicRHI->RHILockTexture2D(texture, mipIndex, lockMode, destStride, bLockWithinMiptail);
		}
		ScopedRHIThreadStaller stallRHIThread(rhiCmdList);
		return GDynamicRHI->RHILockTexture2D(texture, mipIndex, lockMode, destStride, bLockWithinMiptail);
	}

	void DynamicRHI::unlockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, RHITexture2D* texture, uint32 mipIndex, bool bLockWithMiptail, bool bFlushRHIThread /* = true */)
	{
		if (bFlushRHIThread)
		{
			rhiCmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIUnlockTexture2D(texture, mipIndex, bLockWithMiptail);
			return;
		}
		ScopedRHIThreadStaller stallRHIThread(rhiCmdList);
		GDynamicRHI->RHIUnlockTexture2D(texture, mipIndex, bLockWithMiptail);
	}

	VertexBufferRHIRef DynamicRHI::createAndLockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outPtr)
	{
		VertexBufferRHIRef vertexBuffer = createVertexBuffer_RenderThread(RHICmdList, size, inUsage, createInfo);
		outPtr = lockVertexBuffer_RenderThread(RHICmdList, vertexBuffer, 0, size, RLM_WriteOnly);
		return vertexBuffer;
	}

	VertexBufferRHIRef DynamicRHI::createVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return GDynamicRHI->RHICreateVertexBuffer(size, inUsage, createInfo);
	}

	IndexBufferRHIRef DynamicRHI::createAndLockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer)
	{
		IndexBufferRHIRef indexBuffer =  createIndexBuffer_RenderThread(RHICmdList, stride, size, inUsage, createInfo);
		outDataBuffer = lockIndexBuffer_RenderThread(RHICmdList, indexBuffer, 0, size, RLM_WriteOnly);
		return indexBuffer;
	}

	IndexBufferRHIRef DynamicRHI::createIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return GDynamicRHI->RHICreateIndexBuffer(stride, size, inUsage, createInfo);
	}

	void* DynamicRHI::lockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
	{
		BOOST_ASSERT(isInRenderingThread());
		void* result;
		if (lockMode != RLM_WriteOnly || RHICmdList.bypass() || !GRHIThread)
		{
			RHICmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			result = GDynamicRHI->RHILockIndexBuffer(indexBuffer, offset, sizeRHI, lockMode);
		}
		else
		{
			result = Memory::malloc(sizeRHI, 16);
		}
		BOOST_ASSERT(result);
		GLockTracker.lock(indexBuffer, result, offset, sizeRHI, lockMode);
		return result;
	}

	struct RHICommandUpdateIndexBuffer : RHICommand<RHICommandUpdateIndexBuffer>
	{
		RHIIndexBuffer* mIndexBuffer;
		void* mBuffer;
		uint32 mBufferSize;
		uint32 mOffset;
		FORCEINLINE_DEBUGGABLE RHICommandUpdateIndexBuffer(RHIIndexBuffer* inIndexBuffer, void* inBuffer, uint32 inOffset, uint32 inBufferSize)
			:mIndexBuffer(inIndexBuffer)
			,mBuffer(inBuffer)
			,mBufferSize(inBufferSize)
			,mOffset(inOffset)
		{}

		void execute(RHICommandListBase& cmdList)
		{
			void* data = GDynamicRHI->RHILockIndexBuffer(mIndexBuffer, mOffset, mBufferSize, RLM_WriteOnly);
			Memory::memcpy(data, mBuffer, mBufferSize);
			Memory::free(mBuffer);
			GDynamicRHI->RHIUnlockIndexBuffer(mIndexBuffer);
		}
	};

	void DynamicRHI::unlockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer)
	{
		BOOST_ASSERT(isInRenderingThread());
		LockTracker::LockParams params = GLockTracker.unlock(indexBuffer);
		if (params.lockMode != RLM_WriteOnly || RHICmdList.bypass() || !GRHIThread)
		{
			RHICmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIUnlockIndexBuffer(indexBuffer);
			GLockTracker.mTotalMemoryOutstanding = 0;
		}
		else
		{
			new (RHICmdList.allocCommand<RHICommandUpdateIndexBuffer>())RHICommandUpdateIndexBuffer(indexBuffer, params.Buffer, params.offset, params.bufferSize);
			RHICmdList.RHIThreadFence(true);
			if (GLockTracker.mTotalMemoryOutstanding > 256 * 1024)
			{
				RHICmdList.immediateFlush(EImmediateFlushType::DispatchToRHIThread);
				GLockTracker.mTotalMemoryOutstanding = 0;
			}
		}
	}

	VertexDeclarationRHIRef DynamicRHI::createVertexDeclaration_RenderThread(class RHICommandListImmediate& RHICmdList, const VertexDeclarationElementList& elements)
	{
		return GDynamicRHI->RHICreateVertexDeclaration(elements);
	}

	

	void* DynamicRHI::lockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode)
	{
		BOOST_ASSERT(isInRenderingThread());
		void* result;
		if (lockMode != RLM_WriteOnly || RHICmdList.bypass() || !GRHIThread)
		{
			RHICmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			result = GDynamicRHI->RHILockVertexBuffer(vertexBuffer, offset, sizeRHI, lockMode);
		}
		else
		{
			result = Memory::malloc(sizeRHI, 16);
		}
		BOOST_ASSERT(result);
		GLockTracker.lock(vertexBuffer, result, offset, sizeRHI, lockMode);
		return result;
	}


	struct RHICommandUpdateVertexBuffer : public RHICommand<RHICommandUpdateVertexBuffer>
	{
		RHIVertexBuffer* mVertexBuffer;
		void* mBuffer;
		uint32 mBufferSize;
		uint32 offset;

		FORCEINLINE_DEBUGGABLE RHICommandUpdateVertexBuffer(RHIVertexBuffer* inVertexBuffer, void* inBuffer, uint32 inBufferSize, uint32 inOffset)
			:mVertexBuffer(inVertexBuffer),
			mBuffer(inBuffer),
			mBufferSize(inBufferSize),
			offset(inOffset)
		{

		}

		void execute(RHICommandListBase& cmdList)
		{
			void* data = GDynamicRHI->RHILockVertexBuffer(mVertexBuffer, offset, mBufferSize, RLM_WriteOnly);
			Memory::memcpy(data, mBuffer, mBufferSize);
			Memory::free(mBuffer);
			GDynamicRHI->RHIUnlockVertexBuffer(mVertexBuffer);
		}
	};

	void DynamicRHI::unlockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer)
	{
		BOOST_ASSERT(isInRenderingThread());
		LockTracker::LockParams params = GLockTracker.unlock(vertexBuffer);
		if (params.lockMode != RLM_WriteOnly || RHICmdList.bypass() || !GRHIThread)
		{
			RHICmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
			GDynamicRHI->RHIUnlockVertexBuffer(vertexBuffer);
			GLockTracker.mTotalMemoryOutstanding = 0;
		}
		else
		{
			new (RHICmdList.allocCommand<RHICommandUpdateVertexBuffer>())RHICommandUpdateVertexBuffer(vertexBuffer, params.Buffer, params.offset, params.bufferSize);
			RHICmdList.RHIThreadFence(true);
			if (GLockTracker.mTotalMemoryOutstanding > 256 * 1024)
			{
				RHICmdList.immediateFlush(EImmediateFlushType::DispatchToRHIThread);
				GLockTracker.mTotalMemoryOutstanding = 0; 
			}
		}
	}
}