#pragma once
#include "GenericPlatform/GenericWindows.h"
#include "WindowsApplication.h"
#include <Ole2.h>
namespace Air
{
	class CORE_API WindowsWindow : public GenericWindow, public IDropTarget
	{
	public:
		static const TCHAR AppWindowClass[];

		virtual void reshapeWindow(int32 x, int32 y, int32 width, int32 height) override;

		virtual void setOpacity(const float inOpacity) override;

		virtual bool isMaximized() const override;

		virtual void restore() override;

		static std::shared_ptr<WindowsWindow> make();

		virtual int32 getWindowBorderSize() const override;

		virtual void maximize() override;

		virtual void minimize() override;

		virtual void show() override;

		virtual void bringToFront(bool bForce = false) override;

		virtual void* getOSWindowHandle() const override { return mHWnd; }

		virtual bool isVisible() const override;

		virtual bool isWindowMinimized() const override;

		HWND getHWnd() const;

		void onParentWindowMinimized();

		void onParentWindowRestored();

		float getAspectRatio()const { return mAspectRatio; }
	public:
		void initialize(WindowsApplication* const app, const std::shared_ptr<GenericWindowDefinition>& inDefinition, HINSTANCE inHinstance, const std::shared_ptr<WindowsWindow>& inParent, const bool bShowImmediately);

		void adjustWindowRegion(int32 width, int32 height);

		HRGN makeWindowRegionObject(bool bIncludeBorderWhenMaximized) const;

		virtual float getDPIScaleFactor() const override
		{
			return mDPIScaleFactor;
		}

		void setDPIScaleFactor(float value)
		{
			mDPIScaleFactor = value;
		}

		virtual int32 getWindowTitleBarSize() const override;
	public:
		HRESULT STDCALL QueryInterface(REFIID iid, void** ppvObject) override;
		ULONG STDCALL	AddRef(void) override;
		ULONG STDCALL	Release(void) override;

		virtual HRESULT STDCALL DragEnter(__RPC__in_opt IDataObject *DataObjectPointer, ::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect) override;
		virtual HRESULT STDCALL DragOver(::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect) override;
		virtual HRESULT STDCALL DragLeave(void) override;
		virtual HRESULT STDCALL Drop(__RPC__in_opt IDataObject *DataObjectPointer, ::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect) override;

	private:
		WindowsApplication* mOwningApplication;

		int32 mRegionWidth;
		int32 mRegionHeight;

		float mDPIScaleFactor{ 1.0f };

		float mAspectRatio;

		bool mIsVisible{ false };

		HWND mHWnd;

		uint32 mVirtualWidth;
		uint32 mVirtualHeight;

		EWindowMode::Type mWindowMode;

		int32 mOLEReferenceCount;
		WINDOWPLACEMENT mPreParentMinimizedWindowPlacement;
	};
}