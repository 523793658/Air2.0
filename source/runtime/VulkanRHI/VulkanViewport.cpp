#include "VulkanRHIPrivate.h"

#include "VulkanDynamicRHI.h"
#include "VulkanViewport.h"
#include "VulkanResources.h"
#include "RenderingThread.h"
namespace Air
{
	void VulkanDynamicRHI::RHIResizeViewport(RHIViewport* viewportRHI, uint32 sizeX, uint32 sizeY, bool isFullscreen)
	{
		BOOST_ASSERT(isInGameThread());
		VulkanViewport* viewport = resourceCast(viewportRHI);
		if (viewport->getSizeXY() != int2(sizeX, sizeY))
		{
			flushRenderingCommands();
			ENQUEUE_RENDER_COMMAND(ResizeViewport)(
				[viewport, sizeX, sizeY, isFullscreen](RHICommandListImmediate& RHICmdList)
				{
					viewport->resize(sizeX, sizeY, isFullscreen, PF_Unknown);
				});
			flushRenderingCommands();
		}
		
	}

	void VulkanDynamicRHI::RHIResizeViewport(RHIViewport* viewportRHI, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat)
	{
		BOOST_ASSERT(isInGameThread());
		VulkanViewport* viewport = resourceCast(viewportRHI);
		if (preferredPixelFormat == PF_Unknown)
		{
			preferredPixelFormat = EPixelFormat::PF_A2B10G10R10;
		}

		if (viewport->getSizeXY() != int2(sizeX, sizeY))
		{
			flushRenderingCommands();
			ENQUEUE_RENDER_COMMAND(ResizeViewport)(
				[viewport, sizeX, sizeY, isFullscreen, preferredPixelFormat](RHICommandListImmediate& RHICmdList)
				{
					viewport->resize(sizeX, sizeY, isFullscreen, preferredPixelFormat);
				});
			flushRenderingCommands();
		}
	}

	ViewportRHIRef VulkanDynamicRHI::RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool bIsFullscreen, EPixelFormat preferredPixelFormat)
	{
		BOOST_ASSERT(isInGameThread());
		if (preferredPixelFormat == PF_Unknown)
		{
			preferredPixelFormat = PF_A2B10G10R10;
		}
		return new VulkanViewport(this, mDevice, windowHandle, sizeX, sizeY, bIsFullscreen, preferredPixelFormat);
	}
}