#include "RenderResource.h"
#include "RenderingThread.h"
#include "Misc/ScopedEvent.h"
#include "RHI.h"
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
}