#include "RenderResource.h"
#include "RenderingThread.h"
#include "Misc/ScopedEvent.h"
#include "RHI.h"
#include "Misc/App.h"
#include "Containers/IndirectArray.h"
#include "RHICommandList.h"
namespace Air
{
	float GEnableMipLevelFading = 0.0f;

	TGlobalResource<NullColorVertexBuffer> GNullColorVertexBuffer;

	void NullColorVertexBuffer::initRHI()
	{
		RHIResourceCreateInfo createInfo;
		void* LockedData = NULL;
		mVertexBufferRHI = RHICreateAndLockVertexBuffer(sizeof(uint32), BUF_Static | BUF_ZeroStride | BUF_ShaderResource, createInfo, LockedData);
		uint32* vertices = (uint32*)LockedData;
		vertices[0] = Color(255, 255, 255, 255).DWColor();
		RHIUnlockVertexBuffer(mVertexBufferRHI);
		mVertexBufferSRV = RHICreateShaderResourceView(mVertexBufferRHI, sizeof(Color), PF_R8G8B8A8);
	}

	TLinkedList<RenderResource*>*& RenderResource::getResourceList()
	{
		static TLinkedList<RenderResource*>* mFirstResourceLink = nullptr;
		return mFirstResourceLink;
	}

	void RenderResource::initResource()
	{
		if (!bInitialized)
		{
			mResourceLink = TLinkedList<RenderResource*>(this);
			mResourceLink.linkHead(getResourceList());
			if (GIsRHIInitialized)
			{
				initDynamicRHI();
				initRHI();
			}
			bInitialized = true;
		}
	}

	RenderResource::~RenderResource()
	{
		if (bInitialized && !GIsCriticalError)
		{
			//AIR_LOG(LogRenderCore, Fatal, TEXT("A RenderResource was D"))
		}
	}

	void RenderResource::releaseResource()
	{
		BOOST_ASSERT(isInRenderingThread());
		if (bInitialized)
		{
			if (GIsRHIInitialized)
			{
				releaseRHI();
				releaseDynamicRHI();
			}
			mResourceLink.unLink();
			bInitialized = false;
		}
	}

	void beginReleaseResource(RenderResource* resource)
	{
		ENQUEUE_RENDER_COMMAND(
			releaseCommand)([resource](RHICommandListImmediate& RHICmdList)
			{
				resource->releaseResource();
			});
	}

	void beginInitResource(RenderResource* resource)
	{
		ENQUEUE_RENDER_COMMAND(
			initCommand)([resource](RHICommandListImmediate& RHICmdList)
			{
				resource->initResource();
			});
	}

	void RenderResource::initResourceFromPossiblyParallelRendering()
	{
		if (isInRenderingThread())
		{
			initResource();
		}
		else
		{
			BOOST_ASSERT(isInParallelRenderingThread());
			class InitResourceRenderThreadTask
			{
				RenderResource& mResource;
				ScopedEvent& mEvent;
			public:
				InitResourceRenderThreadTask(RenderResource& inResource, ScopedEvent& inEvent)
					:mResource(inResource)
					,mEvent(inEvent)
				{}
				static FORCEINLINE ENamedThreads::Type getDesiredThread()
				{
					return ENamedThreads::getRenderThread_Local();
				}
				static FORCEINLINE ESubsequentsMode::Type getSubsequentsMode()
				{
					return ESubsequentsMode::FireAndForget;
				}

				void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
				{
					mResource.initResource();
					mEvent.trigger();
				}
			};
			{
				ScopedEvent e;
				TGraphTask<InitResourceRenderThreadTask>::createTask().constructAndDispatchWhenReady(*this, e);
			}
		}
	}

	void RenderResource::updateRHI()
	{
		BOOST_ASSERT(isInRenderingThread());
		if (bInitialized && GIsRHIInitialized)
		{
			releaseRHI();
			releaseDynamicRHI();
			initDynamicRHI();
			initRHI();
		}
	}

	void releaseResourceAndFlush(RenderResource* resource)
	{
		ENQUEUE_RENDER_COMMAND(
			releaseCommand)([resource](RHICommandListImmediate&  RHICmdList)
			{
				resource->releaseResource();
			});
		flushRenderingCommands();
	}

	float GMipLevelFadingAgeThreshold = 0.5f;

	void MipBaseFade::setNewMipCount(float actualMipCount, float targetMipCount, double lastRenderTime, EMipFadeSettings fadeSetting)
	{
		BOOST_ASSERT(actualMipCount >= 0 && targetMipCount <= actualMipCount);
		float timeSinceLastRendered = float(App::getCurrentTime() - lastRenderTime);
		if (mTotalMipCount == 0 || timeSinceLastRendered >= GMipLevelFadingAgeThreshold || GEnableMipLevelFading < 0.0f)
		{
			mTotalMipCount = actualMipCount;
			mMipCountDelta = 0.0f;
			mMipCountFadingRate = 0.0f;
			mStartTime = GRenderingRealtimeClock.getCurrentDeltaTime();
			mBiasOffset = 0.0f;
			return;
		}

		float currentTargetMipCount = mTotalMipCount - mBiasOffset + mMipCountDelta;

		if (Math::isNearlyEqual(mTotalMipCount, actualMipCount) && Math::isNearlyEqual(targetMipCount, currentTargetMipCount))
		{
			return;
		}

		float currentInterpolatedMipCount = mTotalMipCount - calcMipBias();

		currentInterpolatedMipCount = Math::clamp<float>(currentInterpolatedMipCount, 0, actualMipCount);

		mStartTime = GRenderingRealtimeClock.getCurrentTime();
		mTotalMipCount = actualMipCount;
		mMipCountDelta = targetMipCount - currentInterpolatedMipCount;

		if (Math::abs(mMipCountDelta) < 0.01)
		{
			mMipCountDelta = 0.0f;
			mBiasOffset = 0.0f;
			mMipCountFadingRate = 0.0f;
		}
		else
		{
			mBiasOffset = mTotalMipCount - currentInterpolatedMipCount;
			if (mMipCountDelta > 0.0f)
			{
				mMipCountFadingRate = 1.0f / (mMipCountDelta);
			}
			else
			{
				mMipCountFadingRate = -1.f / (mMipCountDelta);
			}
		}
	}

	void TextureReference::beginInit_RenderThread()
	{
		bInitialized_GameThread = true;
		beginInitResource(this);
	}

	void TextureReference::beginRelease_GameThread()
	{
		beginReleaseResource(this);
		bInitialized_GameThread = false;
	}

	void TextureReference::initRHI()
	{
		mTextureReferenceRHI = RHICreateTextureReference(&mLastRenderTimeRHI);
	}

	void TextureReference::releaseRHI()
	{
		mTextureReferenceRHI.safeRelease();
	}

	class DynamicVertexBuffer : public VertexBuffer
	{
	public:
		enum { ALIGNMENT = (1 << 16) };

		uint8* mMappedBuffer;

		uint32 mBufferSize;

		uint32 mAllocatedByteCount;

		explicit DynamicVertexBuffer(uint32 inMinBufferSize)
			:mMappedBuffer(nullptr)
			,mBufferSize(Math::max<uint32>(align(inMinBufferSize, ALIGNMENT), ALIGNMENT))
			,mAllocatedByteCount(0)
		{}

		void lock()
		{
			BOOST_ASSERT(mMappedBuffer == nullptr);
			BOOST_ASSERT(mAllocatedByteCount == 0);
			BOOST_ASSERT(isValidRef(mVertexBufferRHI));
			mMappedBuffer = (uint8*)RHILockVertexBuffer(mVertexBufferRHI, 0, mBufferSize, RLM_WriteOnly);
		}

		void unlock()
		{
			BOOST_ASSERT(mMappedBuffer != nullptr);
			BOOST_ASSERT(isValidRef(mVertexBufferRHI));
			RHIUnlockVertexBuffer(mVertexBufferRHI);
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}

		virtual void initRHI() override
		{
			BOOST_ASSERT(!isValidRef(mVertexBufferRHI));
			RHIResourceCreateInfo createInfo;
			mVertexBufferRHI = RHICreateVertexBuffer(mBufferSize, BUF_Volatile, createInfo);
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}

		virtual void releaseRHI() override
		{
			VertexBuffer::releaseRHI();
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}

		virtual wstring getFriendlyName() const override
		{
			return TEXT("DynamicVertexBuffer");
		}
	};

	struct DynamicVertexBufferPool
	{
		TindirectArray<DynamicVertexBuffer> mVertexBuffers;
		DynamicVertexBuffer* mCurrentVertexBuffer;

		DynamicVertexBufferPool()
			:mCurrentVertexBuffer(nullptr)
		{}

		~DynamicVertexBufferPool()
		{
			int32 numVertexBuffers = mVertexBuffers.size();
			for (int32 bufferIndex = 0; bufferIndex < numVertexBuffers; ++bufferIndex)
			{
				mVertexBuffers[bufferIndex].releaseResource();
			}
		}


	};

	GlobalDynamicVertexBuffer::GlobalDynamicVertexBuffer()
		:mTotalAllocatedSincelastCommit(0)
	{
		mPool = new DynamicVertexBufferPool();
	}

	GlobalDynamicVertexBuffer::~GlobalDynamicVertexBuffer()
	{
		delete mPool;
		mPool = nullptr;
	}

	
	void GlobalDynamicVertexBuffer::commit()
	{
		for (int32 bufferIndex = 0, numBuffers = mPool->mVertexBuffers.size(); bufferIndex < numBuffers; bufferIndex++)
		{
			DynamicVertexBuffer& vertexBuffer = mPool->mVertexBuffers[bufferIndex];
			if (vertexBuffer.mMappedBuffer != nullptr)
			{
				vertexBuffer.unlock();
			}
		}
		mPool->mCurrentVertexBuffer = nullptr;
		mTotalAllocatedSincelastCommit = 0;
	}

	class DynamicIndexBuffer : public IndexBuffer
	{
	public:
		enum { ALIGNMENT = (1 << 16) };
		uint8* mMappedBuffer;
		uint32 mBufferSize;
		uint32 mAllocatedByteCount;
		uint32 mStride;

		explicit DynamicIndexBuffer(uint32 inMinBufferSize, uint32 inStride)
			:mMappedBuffer(nullptr)
			,mBufferSize(Math::max<uint32>(align(inMinBufferSize, ALIGNMENT), ALIGNMENT))
			,mAllocatedByteCount(0)
			, mStride(inStride)
		{

		}

		void lock()
		{
			BOOST_ASSERT(mMappedBuffer == nullptr);
			BOOST_ASSERT(mAllocatedByteCount == 0);
			BOOST_ASSERT(isValidRef(mIndexBufferRHI));
			mMappedBuffer = (uint8*)RHILockIndexBuffer(mIndexBufferRHI, 0, mBufferSize, RLM_WriteOnly);
		}

		void unlock()
		{
			BOOST_ASSERT(mMappedBuffer != nullptr);
			BOOST_ASSERT(isValidRef(mIndexBufferRHI));
			RHIUnlockIndexBuffer(mIndexBufferRHI);
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}
		virtual void initRHI() override
		{
			BOOST_ASSERT(!isValidRef(mIndexBufferRHI));
			RHIResourceCreateInfo createInfo;
			mIndexBufferRHI = RHICreateIndexBuffer(mStride, mBufferSize, BUF_Volatile, createInfo);
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}

		virtual void releaseRHI() override
		{
			IndexBuffer::releaseRHI();
			mMappedBuffer = nullptr;
			mAllocatedByteCount = 0;
		}

		virtual wstring getFriendlyName() const override
		{
			return TEXT("DynamicIndexBuffer");
		}
	};

	struct DynamicIndexBufferPool
	{
		TindirectArray<DynamicIndexBuffer> mIndexBuffers;
		DynamicIndexBuffer* mCurrentIndexBuffer;
		uint32 mBufferStride;

		explicit DynamicIndexBufferPool(uint32 inBufferStride)
			:mCurrentIndexBuffer(nullptr)
			,mBufferStride(inBufferStride)
		{}

		~DynamicIndexBufferPool()
		{
			int32 numIndexBuffer = mIndexBuffers.size();
			for (int32 bufferIndex = 0; bufferIndex < numIndexBuffer; bufferIndex++)
			{
				mIndexBuffers[bufferIndex].releaseResource();

			}
		}
	};

	GlobalDynamicIndexBuffer::GlobalDynamicIndexBuffer()
	{
		mPools[0] = new DynamicIndexBufferPool(sizeof(uint16));
		mPools[1] = new DynamicIndexBufferPool(sizeof(uint32));
	}

	GlobalDynamicIndexBuffer::~GlobalDynamicIndexBuffer()
	{
		for (int32 i = 0; i < 2; i++)
		{
			delete mPools[i];
			mPools[i] = nullptr;
		}
	}

	void GlobalDynamicIndexBuffer::commit()
	{
		for (int32 i = 0; i < 2; i++)
		{
			DynamicIndexBufferPool* pool = mPools[i];
			for (int32 bufferIndex = 0, numBuffer = pool->mIndexBuffers.size(); bufferIndex < numBuffer; ++bufferIndex)
			{
				DynamicIndexBuffer& indexBuffer = pool->mIndexBuffers[bufferIndex];
				if (indexBuffer.mMappedBuffer != nullptr)
				{
					indexBuffer.unlock();
				}
			}
			pool->mCurrentIndexBuffer = nullptr;
		}
	}

}