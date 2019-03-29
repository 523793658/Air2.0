#pragma once
#include "SlateCore.h"
#include "Rendering/SlateRenderer.h"
#include "GenericPlatform/GenericApplication.h"
namespace Air
{

	class SWindow;
	class GenericApplication;

	class SLATE_CORE_API SlateApplicationBase
	{
		friend class SWidget;
	public:
		virtual std::shared_ptr<SWindow> addWindow(std::shared_ptr<SWindow> inSlateWindow, const bool bShowImmediately = true) = 0;

		static SlateApplicationBase& get()
		{
			return *mCurrentBaseApplication;
		}

		std::shared_ptr<SlateRenderer> getRenderer()
		{
			return mRenderer;
		}

		virtual bool initializeRenderer(std::shared_ptr<SlateRenderer> inRenderer, bool bQuietMode) = 0;

		virtual float getApplicationScale() const = 0;

		std::shared_ptr<class GenericApplication> getPlatformApplication()
		{
			return mPlatformApplication;
		}

		virtual bool doesWidgetHaveMouseCapture(const std::shared_ptr<const SWidget> widget) const = 0;

		virtual float2 getCursorPos() const = 0;

		virtual float2 getLastCursorPos() const = 0;

		virtual float2 getCursorSize() const = 0;

		virtual bool isActive() const = 0;

		virtual std::shared_ptr<SWidget> getUserFocusedWidget(uint32 userIndex) const = 0;

		virtual const double getCurrentTime() const = 0;
	public:
		const static uint32 CursorPointerIndex;
		const static uint32 CursorUseIndex;
	protected:
		std::shared_ptr<SlateRenderer> mRenderer;


	protected:
		static std::shared_ptr<SlateApplicationBase> mCurrentBaseApplication;

		static std::shared_ptr<GenericApplication> mPlatformApplication;
	};
}