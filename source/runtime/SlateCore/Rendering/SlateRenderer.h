#pragma once
#include "SlateCore.h"
#include "Widgets/SWindow.h"
#include "Rendering/SlateDrawBuffer.h"
namespace Air
{
	class SLATE_CORE_API SlateRenderer
	{
	public:
		virtual void createViewport(const std::shared_ptr<SWindow> inWindow) = 0;

		virtual void updateFullscreenState(const std::shared_ptr<SWindow> inWindow, uint32 overrideResX = 0, uint32 overrideResY = 0) = 0;

		virtual bool initialize() = 0;

		virtual void* getViewportResource(const SWindow& window)
		{
			return nullptr;
		}

		virtual bool isViewportFullscreen(const SWindow& window) const;

		virtual void drawWindows() {}

		virtual void drawWindows(SlateDrawBuffer& windowDrawBuffer) = 0;

		virtual SlateDrawBuffer& getDrawBuffer() = 0;

		virtual void flushCommands() const = 0;
	};

	bool SLATE_CORE_API isThreadSafeForSlateRendering();
}