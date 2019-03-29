#include "SWindow.h"
#include "Application/SlateApplicationBase.h"
#include "Rendering/RenderingCommon.h"
#include "Math/TransformCalculus2D.h"
namespace Air
{
	namespace SWindowDefs
	{
		static const float DefaultTitleBarSize = 24.0f;
		static const int32 CornerRadius = 6;
	}


	SWindow::SWindow(SWindowAttribute const & attr)
		:mViewportSize(float2::zero()),
		mHasEverBeenShown(false)
	{
		mWindowsDesc.SizeDesiredOnScreen = attr.mSize;
		mWindowsDesc.ExpectedMaxWidth = attr.mMaxSize.x;
		mWindowsDesc.ExpectedMaxHeight = attr.mMaxSize.y;
		mWindowsDesc.AcceptsInput = false;
		mWindowsDesc.ActivateWhenFirstShown = true;
		mWindowsDesc.AppearsInTaskBar = true;
		mWindowsDesc.CornerRadius = 0;
		mWindowsDesc.DesiredPositionOnScree = attr.mScreenPosition;
		mWindowsDesc.FocusWhenFirstShown = true;
		mWindowsDesc.HasCloseButton = true;
		mWindowsDesc.HasOSWindowBorder = true;
		mWindowsDesc.HasSizingFrame = false;
		mWindowsDesc.IsModalWindow = false;
		mWindowsDesc.mType = attr.mType;
		mWindowsDesc.SizeWillChangeOften = false;
		mWindowsDesc.SupportsMaximize = true;
		mWindowsDesc.SupportsMinimize = true;
		mWindowsDesc.Title = attr.mTitle;
		mWindowsDesc.TransparencySupport = EWindowTransparency::None;
		mWindowsDesc.IsTopmostWindow = false;
	}


	void SWindow::showWindow()
	{
		if (!mHasEverBeenShown)
		{
			if (mNativeWindow)
			{
				SlateApplicationBase::get().getRenderer()->createViewport(std::dynamic_pointer_cast<SWindow>(this->shared_from_this()));
			}
			initialMaximize();
			initialMinimize();
		}
		mHasEverBeenShown = true;
		if (mNativeWindow)
		{
			mNativeWindow->show();
			if (mIsTopmostWindow)
			{
				mNativeWindow->bringToFront();
			}
		}
	}

	void SWindow::initialMaximize()
	{
		if (mNativeWindow && mInitiallyMaximized)
		{
			mNativeWindow->maximize();
		}
	}

	void SWindow::initialMinimize()
	{
		if (mNativeWindow && mInitiallyMinimized)
		{
			mNativeWindow->minimize();
		}
	}

	void SWindow::setWindowMode(EWindowMode::Type windowMode)
	{
		EWindowMode::Type currentWindowMode = mNativeWindow->getWindowMode();

		if (currentWindowMode != windowMode)
		{
			bool bFullscreen = windowMode != EWindowMode::Windowed;
			bool bWasFullScree = currentWindowMode != EWindowMode::Windowed;

			if (bFullscreen)
			{
				mPreFullscreenPosition = mScreenPosition;
			}

			mIsDrawingEnable = false;

			mNativeWindow->setWindowMode(windowMode);

			const float2 vp = isMirrorWindow() ? getSizeInScreen() : getViewportSize();

			SlateApplicationBase::get().getRenderer()->updateFullscreenState(std::dynamic_pointer_cast<SWindow>(this->shared_from_this()), vp.x, vp.y);

			mIsDrawingEnable = true;
		}
	}

	std::shared_ptr<SWindow> SWindow::getParentWindow() const
	{
		return mParentWindowPtr.lock();
	}

	std::shared_ptr<GenericWindow> SWindow::getNativeWindow()
	{
		return mNativeWindow;
	}

	bool SWindow::isVisible() const
	{
		return (!!mNativeWindow) && mNativeWindow->isVisible();
	}
	bool SWindow::isWindowMinimized() const
	{
		if (mNativeWindow)
		{
			return mNativeWindow->isWindowMinimized();
		}
	}

	uint2 SWindow::getInitialDesiredSizeInScreen() const
	{
		return mWindowsDesc.SizeDesiredOnScreen;
	}

	void SWindow::setNativeWindow(std::shared_ptr<GenericWindow> inNativeWindow)
	{
		mNativeWindow = inNativeWindow;
	}

	void SWindow::setCachedScreenPosition(int2 NewPosition)
	{
		mScreenPosition = NewPosition;
	}

	void SWindow::setCachedSize(uint2 newSize)
	{
		if (mNativeWindow)
		{
			mNativeWindow->adjustCachedSize(newSize);
		}
		mSize = newSize;
	}

	int2 SWindow::getPositionInScreen() const
	{
		return mScreenPosition;
	}

	float2 SWindow::getClientSizeInScreen() const
	{
		return float2(0, 0);
	}


	std::shared_ptr<const SWidget> SWindow::getContent() const
	{
		return mContent;
	}

	void SWindow::setContent(std::shared_ptr<SWidget> inContent)
	{
		mContent = inContent;
	}

	void SWindow::resize(float2 newSize)
	{
		newSize = getWindowSizefromClientSize(newSize);
		if (mSize != newSize)
		{
			if (mNativeWindow)
			{
				mNativeWindow->reshapeWindow(mScreenPosition.x, mScreenPosition.y, newSize.x, newSize.y);
			}
			else
			{
				mInitialDesiredSize = newSize;
			}
		}
		setCachedSize(newSize);
	}

	bool SWindow::isRegularWindow() const
	{
		return !mIsPopupWindow && mType != EWindowType::ToolTip && mType != EWindowType::CursorDecorator;
	}

	float2 SWindow::getWindowSizefromClientSize(float2 inClientSize)
	{
		if (isRegularWindow() && !hasOSWindowBorder())
		{
			const Margin BorderSize = getWindowBorderSize();
			inClientSize.x += BorderSize.mLeft + BorderSize.mRight;
			inClientSize.y += BorderSize.mBottom + BorderSize.mTop;
			if (mCreateTitleBar)
			{
				inClientSize.y += SWindowDefs::DefaultTitleBarSize;
			}
		}
		return inClientSize;
	}

	Margin SWindow::getWindowBorderSize(bool bIncTitleBar /* = false */) const
	{
		if (mNativeWindow && mNativeWindow->isMaximized())
		{
			const float desktopPixelsToSlateUnits = 1.0f / (SlateApplicationBase::get().getApplicationScale() * mNativeWindow->getDPIScaleFactor());
			Margin borderSize(mNativeWindow->getWindowBorderSize() * desktopPixelsToSlateUnits);
			if (bIncTitleBar)
			{
				borderSize.mTop += mNativeWindow->getWindowTitleBarSize() * desktopPixelsToSlateUnits;
			}
			return borderSize;
		}
	}

	Reply SWindow::onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		if (!mViewport.expired())
		{
			std::shared_ptr<ISlateViewport> ptr = mViewport.lock();
			return ptr->onKeyDown(myGeometry, inKeyEvent);
		}
		return Reply::unhandled();
	}
	Reply SWindow::onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		if (!mViewport.expired())
		{
			std::shared_ptr<ISlateViewport> ptr = mViewport.lock();
			return ptr->onMouseButtonDown(myGeometry, mouseEvent);
		}
		return Reply::unhandled();
	}

	Reply SWindow::onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		if (!mViewport.expired())
		{
			std::shared_ptr<ISlateViewport> ptr = mViewport.lock();
			return ptr->onMouseButtonUp(myGeometry, mouseEvent);
		}
		return Reply::unhandled();
	}

	Reply SWindow::onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		if (!mViewport.expired())
		{
			std::shared_ptr<ISlateViewport> ptr = mViewport.lock();
			return ptr->onMouseMove(myGeometry, mouseEvent);
		}
		return Reply::unhandled();
	}

	Reply SWindow::onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		if (!mViewport.expired())
		{
			std::shared_ptr<ISlateViewport> ptr = mViewport.lock();
			return ptr->onKeyUp(myGeometry, inKeyEvent);
		}
		return Reply::unhandled();
	}
	SlateLayoutTransform SWindow::getLocalToScreenTransform() const
	{
		return SlateLayoutTransform(SlateApplicationBase::get().getApplicationScale() * mNativeWindow->getDPIScaleFactor(), mScreenPosition);
	}

	Geometry SWindow::getWindowGeometryInScreen() const
	{
		SlateLayoutTransform localToScreen = getLocalToScreenTransform();
		return Geometry::makeRoot(transformVector(inverse(localToScreen), mSize), localToScreen);
	}
}