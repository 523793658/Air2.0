#pragma once
#include "GenericPlatform/GenericWindows.h"
#include "EngineMininal.h"
#include "RenderTarget.h"
#include "RenderResource.h"
#include "RHICommandList.h"
#include "RHIResource.h"
namespace Air
{
	class ViewportClient;

	class Viewport : public RenderTarget, protected RenderResource
	{
	public:
		Viewport(ViewportClient* inViewportClient);

		ENGINE_API void draw(bool bShouldPresent = true);

		ViewportClient* getClient() const { return mViewportClient; }

		void enqueueBeginRenderFrame();

		const ViewportRHIRef& getViewportRHI() const { return mViewportRHI; }

		ENGINE_API virtual void beginRenderFrame(RHICommandListImmediate& RHICmdList);

		virtual void setRequiresVsync(bool bshouldVsync) {}

		void updateRenderTargetSurfaceRHIToCurrentBackBuffer();

		ENGINE_API virtual void updateViewportRHI(bool bDestroyed, uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, EPixelFormat preferredPixelFormat);

		virtual bool isFullscreen() const { return mWindowMode == EWindowMode::Fullscreen || mWindowMode == EWindowMode::WindowedFullscreen; }

		virtual void* getWindow() = 0;

		void invalidate();

		void deferInvalidateHitProxy();

		void invalidateHitProxy();

		void invalidateDisplay();

		virtual int2 getSizeXY() const override { return int2(mWidth, mHeight); }

		IntRect calculateViewExtents(float aspectRatio, const IntRect& viewRect);

		virtual float getDesiredAspectRatio() const;

		virtual bool hasMouseCapture() const { return true; }

		virtual bool hasFocus() const { return true; }

		virtual int32 getMouseX() const = 0;

		virtual int32 getMouseY() const = 0;
	protected:
		ViewportClient* mViewportClient{ nullptr };

		ViewportRHIRef mViewportRHI;

		ENGINE_API static  bool bIsGameRenderingEnable;

		uint32 mHitProxiesCached;

		uint32 mWidth{ 0 };
		uint32 mHeight{ 0 };
	protected:

		EWindowMode::Type mWindowMode;

	};

}