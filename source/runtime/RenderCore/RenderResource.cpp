#include "RenderResource.h"
#include "RenderingThread.h"
#include "Misc/ScopedEvent.h"
#include "RHI.h"
#include "Misc/App.h"
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
		if (!mInitialized)
		{
			mResourceLink = TLinkedList<RenderResource*>(this);
			mResourceLink.linkHead(getResourceList());
			if (GIsRHIInitialized)
			{
				initDynamicRHI();
				initRHI();
			}
			mInitialized = true;
		}
	}

	void RenderResource::releaseResource()
	{
		BOOST_ASSERT(isInRenderingThread());
		if (mInitialized)
		{
			if (GIsRHIInitialized)
			{
				releaseRHI();
				releaseDynamicRHI();
			}
			mResourceLink.unLink();
			mInitialized = false;
		}
	}

	void beginReleaseResource(RenderResource* resource)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			releaseCommand,
			RenderResource*, resource, resource,
			{
				resource->releaseResource();
			});
	}

	void beginInitResource(RenderResource* resource)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			initCommand,
			RenderResource*, resource, resource,
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
					return ENamedThreads::RenderThread_Local;
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
				GraphTask<InitResourceRenderThreadTask>::createTask().constructAndDispatchWhenReady(*this, e);
			}
		}
	}

	void RenderResource::updateRHI()
	{
		BOOST_ASSERT(isInRenderingThread());
		if (mInitialized && GIsRHIInitialized)
		{
			releaseRHI();
			releaseDynamicRHI();
			initDynamicRHI();
			initRHI();
		}
	}

	void releaseResourceAndFlush(RenderResource* resource)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			releaseCommand,
			RenderResource*, resource, resource,
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
}