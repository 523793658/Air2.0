#include "Slate/SceneViewport.h"
#include "GenericPlatform/GenericWindows.h"
#include "Misc/App.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "Slate/SlateRenderTargetRHI.h"
#include "Classes/Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Classes/Engine/GameInstance.h"
#include "Classes/GameFramework/PlayerController.h"
#include "AirEngine.h"
namespace Air
{

	extern EWindowMode::Type getWindowModeType(EWindowMode::Type windowMode);

	SceneViewport::SceneViewport(ViewportClient* inViewportClient, std::shared_ptr<SViewport> inViewportWidget)
		:Viewport(inViewportClient),
		mViewportWidget(inViewportWidget)
		, mCurrentReplyState(Reply::unhandled())
	{

	}

	void SceneViewport::resizeViewport(uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode)
	{
		if (newSizeX > 0 && newSizeY > 0)
		{
			mIsResizing = true;
			updateViewportRHI(false, newSizeX, newSizeY, newWindowMode, PF_Unknown);
			if (mViewportClient)
			{
				invalidate();
				if (mViewportClient->getWorld())
				{
					draw();
				}
			}
			mIsResizing = false;
		}
	}

	void SceneViewport::resizeFrame(int32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode)
	{
		if (App::isGame() && newSizeX > 0 && newSizeY > 0)
		{
			std::shared_ptr<SWindow> windowToResize = SlateApplication::get().getWindow(0);
			if (windowToResize)
			{
				newWindowMode = getWindowModeType(newWindowMode);
				const int2 oldWindowPos = windowToResize->getPositionInScreen();
				const float2 oldWindowSize = windowToResize->getSizeInScreen();
				const EWindowMode::Type oldWindowMode = mWindowMode;

				if (newWindowMode != oldWindowMode)
				{
					windowToResize->setWindowMode(newWindowMode);
				}

				float2 newWindowSize(newSizeX, newSizeY);
				if (newWindowMode == EWindowMode::Windowed)
				{
					if (oldWindowMode == EWindowMode::Windowed && newWindowSize == oldWindowSize)
					{

					}
				}

				if (newWindowSize != oldWindowSize || newWindowMode != oldWindowMode)
				{
					windowToResize->resize(newWindowSize);
				}

				float2 viewportSize = windowToResize->getWindowSizefromClientSize(float2(mWidth, mHeight));
				float2 newViewportSize = windowToResize->getViewportSize();
				if (newViewportSize != viewportSize || newWindowMode != oldWindowMode)
				{
					resizeViewport(newViewportSize.x, newViewportSize.y, newWindowMode);
				}


				float2 backBufferSize = windowToResize->isMirrorWindow() ? oldWindowSize : viewportSize;
				float2 newBackbufferSize = windowToResize->isMirrorWindow() ? newWindowSize : newViewportSize;

				if (newBackbufferSize != backBufferSize)
				{
					SlateApplicationBase::get().getRenderer()->updateFullscreenState(windowToResize, newBackbufferSize.x, newBackbufferSize.y);
				}
			}
		}
	}

	void SceneViewport::updateViewportRHI(bool bDestroyed, uint32 newSizeX, uint32 newSizeY, EWindowMode::Type newWindowMode, EPixelFormat preferredPixelFormat)
	{
		SCOPED_SUSPEND_RENDERING_THREAD(true);
		mWidth = newSizeX;
		mHeight = newSizeY;
		mWindowMode = newWindowMode;
		beginReleaseResource(this);
		if (!bDestroyed)
		{
			beginInitResource(this);
			if (!useSeparateRenderTarget())
			{
				std::shared_ptr<SlateRenderer> renderer = SlateApplication::get().getRenderer();
				void* viewportResource = renderer->getViewportResource(*SlateApplication::get().getWindow(0));
				if (viewportResource)
				{
					mViewportRHI = *((ViewportRHIRef*)viewportResource);
				}
			}
		}
		else
		{
			TArray<SlateRenderTargetRHI*>& mBufferedSlateHandles = mBufferedSlateHandles;

			SlateRenderTargetRHI*& mRenderThreadSlateTexture = mRenderThreadSlateTexture;
			ENQUEUE_RENDER_COMMAND(DeleteSlateRenderTarget)([&mBufferedSlateHandles, &mRenderThreadSlateTexture](RHICommandListImmediate& RHICmdList)
				{
					for (int32 i = 0; i < mBufferedSlateHandles.size(); ++i)
					{
						delete mBufferedSlateHandles[i];
						mBufferedSlateHandles[i] = nullptr;
						delete mRenderThreadSlateTexture;
						mRenderThreadSlateTexture = nullptr;
					}
				});
		}
	}

	SlateShaderResource* SceneViewport::getViewportRenderTargetTexture()
	{
		BOOST_ASSERT(isThreadSafeForSlateRendering());
		return (mBufferedSlateHandles.size() != 0) ? mBufferedSlateHandles[mCurrentBufferdTargetIndex] : nullptr;
	}

	Reply SceneViewport::onKeyDown(const Geometry & myGeometry, const KeyEvent& inKeyEvent)
	{
		mCurrentReplyState = Reply::unhandled();

		Key key = inKeyEvent.getKey();

		if (key.isValid())
		{
			mKeyStateMap.emplace(key, true);

			if (mViewportClient && getSizeXY() != int2::zero())
			{
				if (!mViewportClient->inputKey(this, inKeyEvent.getUserIndex(), key, inKeyEvent.isRepeat() ? IE_Repeat : IE_Pressed, 1.0f, key.isGamepadKey()))
				{
					mCurrentReplyState = Reply::unhandled();
				}
			}
		}
		else
		{
			mCurrentReplyState = Reply::unhandled();
		}
		return mCurrentReplyState;
	}

	Reply SceneViewport::onKeyUp(const Geometry & myGeometry, const KeyEvent& inKeyEvent)
	{
		mCurrentReplyState = Reply::unhandled();

		Key key = inKeyEvent.getKey();
		if (key.isValid())
		{
			mKeyStateMap.emplace(key, false);
			if (mViewportClient && getSizeXY() != int2::zero())
			{
				if (!mViewportClient->inputKey(this, inKeyEvent.getUserIndex(), key, IE_Released, 1.0f, key.isGamepadKey()))
				{
					mCurrentReplyState = Reply::unhandled();
				}
			}
		}
		else
		{
			mCurrentReplyState = Reply::unhandled();
		}
		return mCurrentReplyState;
	}

	Reply SceneViewport::onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		mCurrentReplyState = Reply::handled().preventThrottling();
		mKeyStateMap.emplace(mouseEvent.getEffectingButton(), true);
		updateModifierKeys(mouseEvent);
		updateCachedMousePos(myGeometry, mouseEvent);
		updateCachedGeometry(myGeometry);

		ScopedConditionalWorldSwitcher worldSwitcher(mViewportClient);
		if (mViewportClient && getSizeXY() != int2::Zero)
		{
			if (!hasFocus())
			{
				ModifierKeysState keyState = SlateApplication::get().getModifierKeys();
				applyModifierKeys(keyState);
			}

			const bool bTemporaryCapture = mViewportClient->captureMouseOnClick() == EMouseCaptureMode::CaptureDuringMouseDown || (mViewportClient->captureMouseOnClick() == EMouseCaptureMode::CaptureDuringRightMouseDown && mouseEvent.getEffectingButton() == EKeys::RightMouseButton);

			if (bTemporaryCapture)
			{
				if (!mViewportClient->inputKey(this, mouseEvent.getUserIndex(), mouseEvent.getEffectingButton(), IE_Pressed))
				{
					mCurrentReplyState = Reply::unhandled();
				}
			}
			if (SlateApplication::get().isActive() && (bTemporaryCapture))
			{
				mCurrentReplyState = acquireFocusAndCapture(int2(mouseEvent.getScreenSpacePosition()));
			}
		}
		mCurrentReplyState.preventThrottling();
		return mCurrentReplyState;
	}

	Reply SceneViewport::onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		mCurrentReplyState = Reply::handled();
		mKeyStateMap.emplace(mouseEvent.getEffectingButton(), false);
		updateModifierKeys(mouseEvent);
		updateCachedMousePos(myGeometry, mouseEvent);
		updateCachedGeometry(myGeometry);

		ScopedConditionalWorldSwitcher worldSwitcher(mViewportClient);

		bool bCursorVisible = true;
		bool bReleaseMouseCapture = true;
		if (mViewportClient && getSizeXY() != int2::Zero)
		{
			if (!mViewportClient->inputKey(this, mouseEvent.getUserIndex(), mouseEvent.getEffectingButton(), IE_Released))
			{
				mCurrentReplyState = Reply::unhandled();
			}
			bCursorVisible = mViewportClient->getCursor(this, getMouseX(), getMouseY()) != EMouseCursor::None;

			bReleaseMouseCapture = bCursorVisible || mViewportClient->captureMouseOnClick() == EMouseCaptureMode::CaptureDuringMouseDown || (mViewportClient->captureMouseOnClick() == EMouseCaptureMode::CaptureDuringRightMouseDown && mouseEvent.getEffectingButton() == EKeys::RightMouseButton);
		}
		if (bReleaseMouseCapture)
		{
			if (!mouseEvent.isMouseButtonDown(EKeys::RightMouseButton) && !mouseEvent.isMouseButtonDown(EKeys::LeftMouseButton))
			{
				if (bCursorHiddenDueToCapture)
				{
					bCursorHiddenDueToCapture = false;
					mCurrentReplyState.setMousePos(mMousePosBeforeHiddenDueToCapture);
					mMousePosBeforeHiddenDueToCapture = int2(-1, -1);
				}
				mCurrentReplyState.releaseMouseCapture();
				if (bCursorVisible && !mViewportClient->shouldAlwaysLockMouse())
				{
					mCurrentReplyState.releaseMouseLock();
				}
			}
		}
		return mCurrentReplyState;
	}

	Reply SceneViewport::onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		mCurrentReplyState = Reply::handled();
		updateCachedMousePos(myGeometry, mouseEvent);
		updateCachedGeometry(myGeometry);

		const bool bViewportHasCapture = !mViewportWidget.expired() && mViewportWidget.lock()->hasMouseCapture();
		if (mViewportClient && getSizeXY() != int2::Zero)
		{
			ScopedConditionalWorldSwitcher worldSwitcher(mViewportClient);
			if (bViewportHasCapture)
			{
				mViewportClient->capturedMouseMove(this, getMouseX(), getMouseY());
			}
			else
			{
				mViewportClient->mouseMove(this, getMouseX(), getMouseY());
			}
			if (bViewportHasCapture)
			{
				const float2 cursorDelta = mouseEvent.getCursorDelta();
				mMouseDelta.x += cursorDelta.x;
				++mNumMouseSamplesX;
				mMouseDelta.y -= cursorDelta.y;
				++mNumMouseSamplesY;
			}
			if (bCursorHiddenDueToCapture)
			{
				float2 revertedCursorPos(mMousePosBeforeHiddenDueToCapture);
				SlateApplication::get().setCursorPos(revertedCursorPos);
			}
		}
		return mCurrentReplyState;
	}


	void SceneViewport::onMouseLeave(const PointerEvent& mouseEvent)
	{
		if (mViewportClient)
		{
			mViewportClient->mouseLeave(this);
			mCachedMousePos = int2(-1, -1);
		}
	}

	void SceneViewport::onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		updateCachedMousePos(myGeometry, mouseEvent);
		mViewportClient->mouseEnter(this, getMouseX(), getMouseY());
	}

	bool SceneViewport::hasFocus() const
	{
		return SlateApplication::get().getUserFocusedWidget(0) == mViewportWidget.lock();
	}

	bool SceneViewport::hasMouseCapture() const
	{
		return false;
	}

	int32 SceneViewport::getMouseX() const
	{
		return mCachedMousePos.x;
	}

	int32 SceneViewport::getMouseY() const
	{
		return mCachedMousePos.y;
	}
	void SceneViewport::updateModifierKeys(const PointerEvent& inMouseEvent)
	{
		mKeyStateMap[EKeys::LeftAlt] = inMouseEvent.isLeftAltDown();
		mKeyStateMap[EKeys::RightAlt] = inMouseEvent.isRightAltDown();
		mKeyStateMap[EKeys::LeftCtrl] = inMouseEvent.isLeftControlDown();
		mKeyStateMap[EKeys::RightCtrl] = inMouseEvent.isRightControlDown();
		mKeyStateMap[EKeys::LeftShift] = inMouseEvent.isLeftShiftDown();
		mKeyStateMap[EKeys::RightShift] = inMouseEvent.isRightShiftDown();
		mKeyStateMap[EKeys::LeftCmd] = inMouseEvent.isLeftCommandDown();
		mKeyStateMap[EKeys::RightCmd] = inMouseEvent.isRightCommandDown();
	}

	void SceneViewport::updateCachedMousePos(const Geometry& inGeometry, const PointerEvent& inMouseEvent)
	{
		float2 localPixelMousePos = inGeometry.absoluteToLocal(inMouseEvent.getScreenSpacePosition());
		localPixelMousePos.x *= mCachedGeometry.mScale;
		localPixelMousePos.y *= mCachedGeometry.mScale;
		mCachedMousePos = localPixelMousePos;
	}
	void SceneViewport::updateCachedGeometry(const Geometry& inGeometry)
	{
		mCachedGeometry = inGeometry;
	}

	void SceneViewport::applyModifierKeys(const ModifierKeysState& inKeysState)
	{
		if (mViewportClient && getSizeXY() != int2::Zero)
		{
			ScopedConditionalWorldSwitcher worldSwitcher(mViewportClient);
			if (inKeysState.isLeftAltDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::LeftAlt, IE_Pressed);
			}
			if (inKeysState.isRightAltDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::RightAlt, IE_Pressed);
			}
			if (inKeysState.isLeftShiftDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::LeftShift, IE_Pressed);
			}
			if (inKeysState.isRightShiftDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::RightShift, IE_Pressed);
			}
			if (inKeysState.isLeftControlDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::LeftCtrl, IE_Pressed);
			}
			if (inKeysState.isRightControlDown())
			{
				mViewportClient->inputKey(this, 0, EKeys::RightCtrl, IE_Pressed);
			}
		}
	}

	Reply SceneViewport::acquireFocusAndCapture(int2 mousePosition)
	{
		bShouldCaptureMouseOnActivate = false;
		Reply replyState = Reply::handled().preventThrottling();
		std::shared_ptr<SViewport> viewportWidgetRef = mViewportWidget.lock();
		replyState.setUserFocus(viewportWidgetRef, EFocusCause::SetDirectly, true);
		World* world = mViewportClient->getWorld();
		if (world && world->isGameWorld() && world->getGameInstance() && (world->getGameInstance()->getFirstLocalPlayerController()))
		{
			replyState.captureMouse(viewportWidgetRef);
			if (mViewportClient->lockDuringCapture())
			{
				replyState.lockMouseToWidget(viewportWidgetRef);
			}
			APlayerController* pc = world->getGameInstance()->getFirstLocalPlayerController();
			const bool bShouldShowMouseCursor = pc && pc->shouldShowMouseCursor();
			if (mViewportClient->hideCursorDuringCapture())
			{
				bCursorHiddenDueToCapture = true;
				mMousePosBeforeHiddenDueToCapture = mousePosition;
			}
			if (bCursorHiddenDueToCapture || !bShouldShowMouseCursor)
			{
				replyState.useHighPrecisionMouseMovement(viewportWidgetRef);
			}
		}
		else
		{
			replyState.useHighPrecisionMouseMovement(viewportWidgetRef);
		}
		return replyState;
	}


	void SceneViewport::processAccumulatedPointerInput()
	{
		if (!mViewportClient)
		{
			return;
		}

		ScopedConditionalWorldSwitcher worldSwitcher(mViewportClient);
		const bool bViewportHasCapture = !mViewportWidget.expired() && mViewportWidget.lock()->hasMouseCapture();
		if (mNumMouseSamplesX > 0 || mNumMouseSamplesY > 0)
		{
			const float deltaTime = App::getDeltaTime();
			mViewportClient->inputAxis(this, 0, EKeys::MouseX, mMouseDelta.x, deltaTime, mNumMouseSamplesX);
			mViewportClient->inputAxis(this, 0, EKeys::MouseY, mMouseDelta.y, deltaTime, mNumMouseSamplesY);
		}
		if (bCursorHiddenDueToCapture)
		{
			switch (mViewportClient->captureMouseOnClick())
			{
			case EMouseCaptureMode::NoCapture:
			case EMouseCaptureMode::CaptureDuringMouseDown:
			case EMouseCaptureMode::CaptureDuringRightMouseDown:
				if (!bViewportHasCapture)
				{
					bool bshoudMouseBeVisibile = mViewportClient->getCursor(this, getMouseX(), getMouseY()) != EMouseCursor::None;
					World* world = mViewportClient->getWorld();
					if (world && world->isGameWorld() && world->getGameInstance())
					{
						APlayerController* pc = world->getGameInstance()->getFirstLocalPlayerController();
						bshoudMouseBeVisibile &= pc && pc->shouldShowMouseCursor();
					}
					if (bshoudMouseBeVisibile)
					{
						bCursorHiddenDueToCapture = false;
						mCurrentReplyState.setMousePos(mMousePosBeforeHiddenDueToCapture);
						mMousePosBeforeHiddenDueToCapture = int2(-1, -1);
					}
				}
				break;
			}
		}
		mMouseDelta = int2::Zero;
		mNumMouseSamplesX = 0;
		mNumMouseSamplesY = 0;
	}

	void SceneViewport::onFinishedPointerInput()
	{
		processAccumulatedPointerInput();
	}

}