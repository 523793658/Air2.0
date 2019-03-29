#pragma once
#include "SlateCore.h"
#include "Widgets/SCompoundWidget.h"
#include "GenericPlatform/GenericWindows.h"
#include "Math/Math.h"
#include "Layout/Margin.h"

namespace Air
{
	class ISlateViewport;
	struct SWindowAttribute 
	{
		EWindowType mType;
		int2 mSize;
		wstring mTitle;
		int2 mScreenPosition;
		int2 mMaxSize;
		bool mFocusWhenFirstShown;
		bool mUseOSWindowBorder;
	};


	class SLATE_CORE_API SWindow : public SCompoundWidget
	{
	public:
		SWindow(SWindowAttribute const & attr);

		void setViewport(std::shared_ptr<ISlateViewport> viewport)
		{
			mViewport = viewport;
		}

		std::shared_ptr<ISlateViewport> getViewport()
		{
			return mViewport.lock();
		}

		void showWindow();

		void setWindowMode(EWindowMode::Type windowMode);

		EWindowMode::Type getWindowMode() const { return mNativeWindow->getWindowMode(); }

		bool isMirrorWindow()
		{
			return mIsMirrorWindow;
		}

		float2 getSizeInScreen() const
		{
			return mSize;
		}

		int2 getPositionInScreen() const;

		float2 getClientSizeInScreen() const;

		void resize(float2 newSize);


		inline float2 getViewportSize() const
		{
			return (mViewportSize.x > 1) ? mViewportSize : mSize;
		}

		float2 getWindowSizefromClientSize(float2 inClientSize);

		std::shared_ptr<SWindow> getParentWindow() const;

		std::shared_ptr<GenericWindow> getNativeWindow();

		EWindowType getType() const
		{
			return mWindowsDesc.mType;
		}

		bool hasOSWindowBorder() const
		{
			return mWindowsDesc.HasOSWindowBorder;
		}

		uint2 getInitialDesiredSizeInScreen() const;

		EWindowTransparency getTransparencySupport() const
		{
			return mWindowsDesc.TransparencySupport;
		}

		const GenericWindowDefinition& getWindowsDesc() const
		{
			return mWindowsDesc;
		}

		void setNativeWindow(std::shared_ptr<GenericWindow> inNativeWindow);

		void setCachedScreenPosition(int2 NewPosition);

		void setCachedSize(uint2 newSize);

		void initialMaximize();

		void initialMinimize();

		bool isRegularWindow() const;

		Margin getWindowBorderSize(bool bIncTitleBar = false) const;

		std::shared_ptr<const SWidget> getContent() const;

		void setContent(std::shared_ptr<SWidget> inContent);

		inline bool isViewportSizeDrivenByWindow() const
		{
			return mViewportSize.x == 0;
		}

		bool isVisible() const;

		bool isWindowMinimized() const;

		virtual Reply onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent) override;

		virtual Reply onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent) override;

		virtual Reply onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		virtual Reply onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent) override;

		Geometry getWindowGeometryInScreen() const;

		SlateLayoutTransform getLocalToScreenTransform() const;

	protected:

		EWindowType mType;

		std::shared_ptr<GenericWindow> mNativeWindow;

		float2 mScreenPosition;

		float2 mPreFullscreenPosition;

		float2 mSize;

		float2 mViewportSize;

		float2 mInitialDesiredSize;

		bool mIsDrawingEnable;

		bool mIsMirrorWindow;

		bool mCreateTitleBar;

		bool mHasEverBeenShown : 1;
		bool mInitiallyMaximized : 1;
		bool mInitiallyMinimized : 1;

		bool mIsTopmostWindow : 1;

		bool mIsPopupWindow : 1;



		std::weak_ptr<ISlateViewport> mViewport;

		std::weak_ptr<SWindow> mParentWindowPtr;

		std::shared_ptr<SWidget> mContent;

		GenericWindowDefinition mWindowsDesc;

	};

	struct ScopedSwitchWorldHack
	{
		SLATE_CORE_API ScopedSwitchWorldHack(std::shared_ptr<SWindow> inWindow)
			:mWindows(inWindow)
			,mWorldId(-1)
		{
			if (mWindows)
			{
			}
		}

	private:
		std::shared_ptr<SWindow> mWindows;
		int32 mWorldId;
	};
}