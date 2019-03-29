#pragma once
#include "SlateRHIRendererConfig.h"
#include "Rendering/SlateRenderer.h"
#include "RenderResource.h"
#include "RHIResource.h"
#include "Widgets/SWindow.h"
#include "RHICommandList.h"
#include <map>
namespace Air
{
	class SlateRHIRenderingPolicy;


	const uint32 NumDrawBuffers = 3;

	class SlateRHIRenderer : public SlateRenderer
	{
	private:
		struct ViewportInfo : RenderResource
		{
			Matrix mProjectionMatrix;
			ViewportRHIRef mViewportRHI;
			Texture2DRHIRef	mDepthStencil;
			void* mOSWindow;
			uint32 width;
			uint32 height;
			uint32 mDesiredWidth;
			uint32 mDesiredHeight;
			EPixelFormat mPixelFormat;
			bool mFullscreen;
		};



		virtual void createViewport(const std::shared_ptr<SWindow> inWindow) override;

		virtual void updateFullscreenState(const std::shared_ptr<SWindow> inWindow, uint32 overrideResX = 0, uint32 overrideResY = 0) override;

		virtual bool initialize() override;

		virtual void* getViewportResource(const SWindow& window) override;

		Matrix createProjectionMatrix(uint32 width, uint32 height);

		virtual void drawWindows() override;

		virtual void drawWindows(SlateDrawBuffer& windowDrawBuffer) override;

		virtual SlateDrawBuffer& getDrawBuffer() override;

		virtual void flushCommands() const override;

		void conditionalResizeViewport(ViewportInfo* viewportInfo, uint32 width, uint32 height, bool bFullscreen);

		void drawWindow_RenderThread(RHICommandListImmediate& RHICmdList, SlateRHIRenderer::ViewportInfo& viewportInfo, SlateWindowElementList& windowElementList, bool lockToVsync, bool bClear);

	private:
		void drawWindows_Private(SlateDrawBuffer& windowDrawBuffer);

		uint8 mFreeBufferIndex{ 0 };
	private:
		std::map<const SWindow*, ViewportInfo*> mWindowToViewportInfo;

		std::shared_ptr<SlateRHIRenderingPolicy> mRenderingPolicy;

		SlateDrawBuffer mDrawBuffers[NumDrawBuffers];

		std::shared_ptr<SlateElementBatcher> mElementBatcher;

		bool mRequiresStencilTest{ false };
	};


	struct SlateEndDrawingWindowsCommand : public RHICommand<SlateEndDrawingWindowsCommand>
	{
		SlateRHIRenderingPolicy& mPolicy;
		SlateDrawBuffer* mDrawBuffer;

		SlateEndDrawingWindowsCommand(SlateRHIRenderingPolicy& inPolicy, SlateDrawBuffer* inDrawBuffer);

		void execute(RHICommandListBase& cmdList);

		static void endDrwingWindows(RHICommandListImmediate& RHICmdList, SlateDrawBuffer* drawBuffer, SlateRHIRenderingPolicy& policy);
	};
}