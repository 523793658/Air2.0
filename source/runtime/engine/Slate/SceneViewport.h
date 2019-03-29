#pragma once
#include "EngineMininal.h"
#include "AirClient.h"
#include "Rendering/RenderingCommon.h"
#include "Textures/SlateShaderResource.h"
#include "Widgets/SViewport.h"
namespace Air
{
	class ENGINE_API SceneViewport : public ViewportFrame, public Viewport, public ISlateViewport, public IViewportRenderTargetProvider
	{
	public:
		SceneViewport(ViewportClient* inViewportClient, std::shared_ptr<SViewport> inViewportWidget);

		virtual Viewport* getViewport() override
		{
			return this;
		}

		virtual void resizeFrame(int32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode) override;

		virtual void resizeViewport(uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode);

		void resizeViewport(uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, int32, int32)
		{
			resizeViewport(newSizeX, newSizeY, newWindowMode);
		}

		virtual void* getWindow() override { return nullptr; }

		virtual void updateViewportRHI(bool bDestroyed, uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, EPixelFormat preferredPixelFormat) override;

		bool useSeparateRenderTarget() const
		{
			return mUseSeparateRenderTarget || mForceSeparateRenderTarget;
		}
		virtual SlateShaderResource* getViewportRenderTargetTexture() override;

		virtual Reply onKeyDown(const Geometry & myGeometry, const KeyEvent& inKeyEvent) override;

		virtual Reply onKeyUp(const Geometry & myGeometry, const KeyEvent& inKeyEvent) override;

		virtual Reply onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual void onMouseLeave(const PointerEvent& mouseEvent) override;

		virtual void onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		bool hasFocus() const;

		virtual bool hasMouseCapture() const;

		virtual int32 getMouseX() const override;

		virtual int32 getMouseY() const override;

		virtual void onFinishedPointerInput() override;
	private:
		void updateModifierKeys(const PointerEvent& inMouseEvent);

		void updateCachedMousePos(const Geometry& inGeometry, const PointerEvent& inMouseEvent);
		void updateCachedGeometry(const Geometry& inGeometry);

		void applyModifierKeys(const ModifierKeysState& inKeysState);
		
		Reply acquireFocusAndCapture(int2 mousePosition);

		void processAccumulatedPointerInput();
	private:
		Reply mCurrentReplyState;
		TMap<Key, bool> mKeyStateMap;

		std::weak_ptr<SViewport> mViewportWidget;

		TArray<class SlateRenderTargetRHI*> mBufferedSlateHandles;
		TArray<Texture2DRHIRef> mBufferedRenderTargetsRHI;
		TArray<Texture2DRHIRef> mBufferedShaderResourceTextureRHI;

		Texture2DRHIRef mRenderTargetTextureRenderThreadRHI;
		class SlateRenderTargetRHI* mRenderThreadSlateTexture;

		bool mIsResizing;


		bool mUseSeparateRenderTarget{ false };
		bool mForceSeparateRenderTarget{ false };
		bool bCursorHiddenDueToCapture{ false };
		bool bShouldCaptureMouseOnActivate{ false };
		int2 mMousePosBeforeHiddenDueToCapture;

		int2 mCachedMousePos;
		Geometry mCachedGeometry;

		int32 mCurrentBufferdTargetIndex;
		int32 mNumBufferedFrames;
		int32 mNextBufferedTargetIndex;

		int32 mNumMouseSamplesX;
		int32 mNumMouseSamplesY;
		int2 mMouseDelta;
	};
}