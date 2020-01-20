#pragma once
#include "RHIResource.h"
#include "VulkanMemory.h"
#include "VulkanDynamicRHI.h"
#include "RHICommandList.h"
#include "VulkanResources.h"
#include "HAL/CriticalSection.h"
namespace Air
{
	class VulkanViewport;
	class VulkanSwapChain;
	class VulkanCmdBuffer;

	class VulkanBackBuffer : public VulkanTexture2D
	{
	public:
		VulkanBackBuffer(VulkanDevice& device, VulkanViewport* inViewport, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 AirFlags);
		virtual ~VulkanBackBuffer();

		virtual void OnTransitionResource(VulkanCommandListContext& context, EResourceTransitionAccess transitionType) override final;

		void onGetBackBufferImage(RHICommandListImmediate& RHICmdList);

		void onAdvanceBackBufferFrame(RHICommandListImmediate& RHICmdList);

		void releaseViewport();

	private:
		void acquireBackBufferImage(VulkanCommandListContext& context);

		void releaseAcquiredImage();

	private:
		VulkanViewport* mViewport;
	};


	class VulkanViewport : public RHIViewport, public DeviceChild
	{
	public:
		enum { NUM_BUFFERS = 3 };

		VulkanViewport(VulkanDynamicRHI* inRHI, VulkanDevice* inDevice, void* inWindowHandle, uint32 inSizeX, uint32 inSizeY, bool bInIsFullscreen, EPixelFormat inPreferredPixelFormat);

		~VulkanViewport();

		Texture2DRHIRef getBackBuffer(RHICommandListImmediate& RHICmdList);

		void advanceBackBufferFrame(RHICommandListImmediate& RHICmdList);

		void waitForFrameEventCompletion();

		void issueFrameEvent();

		void resize(uint32 inSizeX, uint32 inSizeY, bool bIsFullscreen, EPixelFormat preferredPixelFormat);

		inline int2 getSizeXY() const
		{
			return int2(mSizeX, mSizeY);
		}

	protected:
		TStaticArray<VkImage, NUM_BUFFERS> mBackBufferImages;

		TStaticArray<Semaphore*, NUM_BUFFERS> mRenderingDoneSemaphores;

		TStaticArray<VulkanTextureView, NUM_BUFFERS> mTextureViews;

		TRefCountPtr<VulkanBackBuffer> mRHIBackBuffer;

		TRefCountPtr<VulkanTexture2D> mRenderingBackBuffer;

		CriticalSection mRecreatingSwapchain;
		VulkanDynamicRHI* mRHI;

		uint32 mSizeX;
		uint32 mSizeY;

		bool bIsFullscreen;
		EPixelFormat mPixelFormat;
		int32 mAcquiredImageIndex;
		VulkanSwapChain* mSwapChain;
		void* mWindowHandle;
		uint32 mPresentCount;
		int8 mLockToVsync;

		Semaphore* mAcquiredSemaphore;

		CustomPresentRHIRef mCustomPresent;

		VulkanCmdBuffer* mLastFrameCommandBuffer = nullptr;
		uint64 mLastFrameFenceCounter = 0;

		void createSwapchain();

		void acquireImageIndex();

		bool tryAcquireImageIndex();

		void recreateSwapchain(void* newNativeWindow, bool bForce = false);

		friend class VulkanDynamicRHI;
		friend class VulkanCommandListContext;
		friend struct RHICommandAcquireBackBuffer;
		friend class VulkanBackBuffer;



	};


	template<>
	struct TVulkanResourceTraits<RHIViewport>
	{
		typedef VulkanViewport TConcreteType;
	};
	
}