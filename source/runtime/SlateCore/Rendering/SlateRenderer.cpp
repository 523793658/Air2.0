#include "Rendering/SlateRenderer.h"
#include "HAL/PlatformProperties.h"
namespace Air
{
	bool isThreadSafeForSlateRendering()
	{
		return ((GSlateLoadingThreadId != 0) || isInGameThread());
	}

	bool SlateRenderer::isViewportFullscreen(const SWindow& window) const
	{
		BOOST_ASSERT(isThreadSafeForSlateRendering());
		bool bFullscree = false;
		if (PlatformProperties::supportsWindowedMode())
		{
			bFullscree = window.getWindowMode() == EWindowMode::Fullscreen;
		}
		else
		{
			bFullscree = true;
		}
		return bFullscree;
	}
}