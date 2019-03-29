#include "WindowsWindow.h"
#include <dwmapi.h>
#include "HAL\Platform.h"
#include "HAL\AirMemory.h"
#include "boost\algorithm\string.hpp"
#include <Ole2.h>
#include "Containers/StringUtil.h"
#include "CoreGlobals.h"
#include <ShlObj.h>
#include "boost/assert.hpp"
namespace Air
{
	const TCHAR WindowsWindow::AppWindowClass[] = TEXT("AirWindow");

	void WindowsWindow::initialize(WindowsApplication* const app, const std::shared_ptr<GenericWindowDefinition>& inDefinition, HINSTANCE inHinstance, const std::shared_ptr<WindowsWindow>& inParent, const bool bShowImmediately)
	{
		mDefinition = inDefinition;
		mOwningApplication = app;

		uint32 windowExStyle = 0;
		uint32 windowStyle = 0;
		mRegionWidth = mRegionHeight = INDEX_NONE;
		const int32 XInitialRect = mDefinition->DesiredPositionOnScree.x;
		const int32 YInitialRect = mDefinition->DesiredPositionOnScree.y;

		const uint32 WidthInitial = mDefinition->SizeDesiredOnScreen.x;
		const uint32 HeightInitial = mDefinition->SizeDesiredOnScreen.y;
		int32 WindowX = XInitialRect;
		int32 WindowY = YInitialRect;
		int32 windowWidth = WidthInitial;
		int32 windowHeight = HeightInitial;

		if (!mDefinition->HasOSWindowBorder)
		{
			windowExStyle = WS_EX_WINDOWEDGE;
			if (mDefinition->TransparencySupport == EWindowTransparency::PerWindow)
			{
				windowExStyle |= WS_EX_LAYERED;
			}

			windowStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			if (mDefinition->AppearsInTaskBar)
			{
				windowExStyle |= WS_EX_APPWINDOW;
			}
			else
			{
				windowExStyle |= WS_EX_TOOLWINDOW;
			}

			if (mDefinition->IsTopmostWindow)
			{
				windowExStyle |= WS_EX_TOPMOST;
			}
			if (!mDefinition->AcceptsInput)
			{
				windowExStyle |= WS_EX_TRANSPARENT;
			}
		}
		else
		{
			windowExStyle = WS_EX_APPWINDOW;
			windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
			if (mDefinition->IsRegularWindow)
			{
				if (mDefinition->SupportsMaximize)
				{
					windowStyle |= WS_MAXIMIZEBOX;
				}
				if (mDefinition->SupportsMinimize)
				{
					windowStyle |= WS_MINIMIZEBOX;
				}
				if (mDefinition->HasSizingFrame)
				{
					windowStyle |= WS_THICKFRAME;
				}
				else
				{
					windowStyle |= WS_BORDER;
				}
			}
			else
			{
				windowStyle |= WS_POPUP | WS_BORDER;
			}
			RECT borderRect = { 0, 0, 0, 0 };
			::AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);

			WindowX += borderRect.left;
			WindowY += borderRect.top;
			windowWidth += borderRect.right - borderRect.left;
			windowHeight += borderRect.bottom - borderRect.top;
		}
		//mHWnd = CreateWindowW(AppWindowClass, mDefinition->Title.c_str(), windowStyle, WindowX, WindowY, windowWidth, windowHeight, NULL, NULL, inHinstance, NULL);
		mHWnd = CreateWindowEx(windowExStyle, AppWindowClass, mDefinition->Title.c_str(), windowStyle, WindowX, WindowY, windowWidth, windowHeight, (inParent ? static_cast<HWND>(inParent->mHWnd) : NULL), NULL, inHinstance, NULL);
		uint32 Error2 = GetLastError();
#if WINVER >= 0x0601
		if (RegisterTouchWindow(mHWnd, 0) == false)
		{
			uint32 Error = GetLastError();
			
		}
#endif
		mVirtualWidth = WidthInitial;
		mVirtualHeight = HeightInitial;
		
		//reshapeWindow(XInitialRect, YInitialRect, WidthInitial, HeightInitial);
		if (mHWnd == NULL)
		{
			return;
		}
		if (mDefinition->TransparencySupport == EWindowTransparency::PerWindow)
		{
			setOpacity(mDefinition->Opacity);
		}

		if (!mDefinition->HasOSWindowBorder)
		{
			const DWMNCRENDERINGPOLICY renderingPolicy = DWMNCRP_DISABLED;
			DwmSetWindowAttribute(mHWnd, DWMWA_NCRENDERING_POLICY, &renderingPolicy, sizeof(renderingPolicy));

			const BOOL bEnableNCPaint = false;
			DwmSetWindowAttribute(mHWnd, DWMWA_ALLOW_NCPAINT, &bEnableNCPaint, sizeof(bEnableNCPaint));

		}
		if (mDefinition->IsRegularWindow && !mDefinition->HasOSWindowBorder)
		{
			windowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
			if (mDefinition->SupportsMaximize)
			{
				windowStyle |= WS_MAXIMIZEBOX;
			}
			if (mDefinition->SupportsMinimize)
			{
				windowStyle |= WS_MINIMIZEBOX;
			}
			if (mDefinition->HasSizingFrame)
			{
				windowStyle |= WS_THICKFRAME;
			}
			SetWindowLong(mHWnd, GWL_STYLE, windowStyle);
			uint32 setWindowPostionFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

			if (!mDefinition->ActivateWhenFirstShown)
			{
				setWindowPostionFlags |= SWP_NOACTIVATE;
			}
			::SetWindowPos(mHWnd, nullptr, 0, 0, 0, 0, setWindowPostionFlags);
			adjustWindowRegion(windowWidth, windowHeight);
		}
		if (mDefinition->IsRegularWindow)
		{
			RegisterDragDrop(mHWnd, static_cast<LPDROPTARGET>(this));
		}
	}

	bool WindowsWindow::isVisible() const
	{
		return mIsVisible;
	}

	HWND WindowsWindow::getHWnd() const
	{
		return mHWnd;
	}

	void WindowsWindow::onParentWindowMinimized()
	{
		::GetWindowPlacement(mHWnd, &mPreParentMinimizedWindowPlacement);
	}

	void WindowsWindow::onParentWindowRestored()
	{
		::SetWindowPlacement(mHWnd, &mPreParentMinimizedWindowPlacement);
	}

	void WindowsWindow::reshapeWindow(int32 x, int32 y, int32 width, int32 height)
	{
		WINDOWINFO windowInfo;
		Memory::memzero(windowInfo);
		windowInfo.cbSize = sizeof(windowInfo);
		::GetWindowInfo(mHWnd, &windowInfo);
		mAspectRatio = (float)width / (float)height;
		if (mDefinition->HasOSWindowBorder)
		{
			RECT borderRect = { 0, 0, 0, 0 };
			::AdjustWindowRectEx(&borderRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);

			x += borderRect.left;
			y += borderRect.top;
			width += borderRect.right - borderRect.left;
			height += borderRect.bottom - borderRect.top;
		}
		int32 WindowX = x;
		int32 WiddowY = y;
		const bool bVirtualSizeChange = width != mVirtualWidth || height != mVirtualHeight;
		mVirtualHeight = height;
		mVirtualWidth = width;

		if (mDefinition->SizeWillChangeOften)
		{
			const RECT oldWindowRect = windowInfo.rcWindow;
			const int32 oldWidth = oldWindowRect.right - oldWindowRect.left;
			const int32 oldHeight = oldWindowRect.bottom - oldWindowRect.top;
			const int32 minRetainedWidth = mDefinition->ExpectedMaxWidth != INDEX_NONE ? mDefinition->ExpectedMaxWidth : oldWidth;
			const int32 minRecainedHeight = mDefinition->ExpectedMaxHeight != INDEX_NONE ? mDefinition->ExpectedMaxHeight : oldHeight;

			width = std::max<int32>(width, std::min<int32>(oldWidth, minRetainedWidth));
			height = std::max<int32>(height, std::min<int32>(oldHeight, minRecainedHeight));
		}
		if (isMaximized())
		{
			restore();
		}

		::SetWindowPos(mHWnd, nullptr, WindowX, WiddowY, width, height, SWP_NOZORDER | SWP_NOACTIVATE | ((mWindowMode == EWindowMode::Fullscreen) ? SWP_NOSENDCHANGING : 0));
		if (mDefinition->SizeWillChangeOften &&bVirtualSizeChange)
		{
			adjustWindowRegion(mVirtualWidth, mVirtualHeight);
		}
	}

	void WindowsWindow::restore()
	{
		::ShowWindow(mHWnd, SW_RESTORE);
	}

	bool WindowsWindow::isMaximized() const
	{
		bool bIsMaximized = !!::IsZoomed(mHWnd);
		return bIsMaximized;
	}

	void WindowsWindow::setOpacity(const float inOpacity)
	{
		SetLayeredWindowAttributes(mHWnd, 0, std::trunc(inOpacity * 255.0f), LWA_ALPHA);
	}

	std::shared_ptr<WindowsWindow> WindowsWindow::make()
	{
		return MakeSharedPtr<WindowsWindow>();
	}

	HRESULT STDCALL WindowsWindow::QueryInterface(REFIID iid, void** ppvObject)
	{
		if (IID_IDropTarget == iid || IID_IUnknown == iid)
		{
			AddRef();
			*ppvObject = (IDropTarget*)(this);
			return S_OK;
		}
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}
	}
	ULONG STDCALL	WindowsWindow::AddRef(void)
	{
		PlatformAtomics::interlockedIncrement(&mOLEReferenceCount);
		return mOLEReferenceCount;
	}
	ULONG STDCALL	WindowsWindow::Release(void)
	{
		PlatformAtomics::interLockedDecrement(&mOLEReferenceCount);
		return mOLEReferenceCount;
	}


	DragDropOLEData decipherOLEData(IDataObject* DataObjectPointer)
	{
		DragDropOLEData oleData;
		struct OLEResourceGuard
		{
			STGMEDIUM& mStorageMedius;
			LPVOID mDataPointer;
			OLEResourceGuard(STGMEDIUM& inStorage)
				:mStorageMedius(inStorage),
				mDataPointer(GlobalLock(inStorage.hGlobal))
			{

			}
			~OLEResourceGuard()
			{
				GlobalUnlock(mStorageMedius.hGlobal);
				ReleaseStgMedium(&mStorageMedius);
			}
		};

		FORMATETC formatEtc_Ansii = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		const bool bHaveAnsiText = (DataObjectPointer->QueryGetData(&formatEtc_Ansii) != S_OK) ? true : false;
		FORMATETC formatEtc_Unicode = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		const bool bHaveUnicodeText = (DataObjectPointer->QueryGetData(&formatEtc_Unicode) == S_OK) ? true : false;

		FORMATETC formateEtc_File = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		const bool bHaveFiles = (DataObjectPointer->QueryGetData(&formateEtc_File) == S_OK) ? true : false;

		STGMEDIUM storageMedius;
		if (bHaveUnicodeText && S_OK == DataObjectPointer->GetData(&formatEtc_Unicode, &storageMedius))
		{
			OLEResourceGuard resourceGuard(storageMedius);
			oleData.mType |= DragDropOLEData::Text;
			oleData.mOperationText = static_cast<TCHAR*>(resourceGuard.mDataPointer);
			oleData.isUnicode = true;
		}
		if (bHaveAnsiText && S_OK == DataObjectPointer->GetData(&formatEtc_Ansii, &storageMedius))
		{

			OLEResourceGuard resourceGuard(storageMedius);
			oleData.isUnicode = false;
			oleData.mType |= DragDropOLEData::Text;
			oleData.mOperationTextAsic = 
				static_cast<ANSICHAR*>(resourceGuard.mDataPointer);
		}

		if (bHaveFiles && S_OK == DataObjectPointer->GetData(&formateEtc_File, &storageMedius))
		{
			oleData.mType |= DragDropOLEData::Files;
			OLEResourceGuard resourceGurad(storageMedius);
			const DROPFILES* dropFiles = static_cast<DROPFILES*>(resourceGurad.mDataPointer);
			LPVOID fileListStart = (BYTE*)resourceGurad.mDataPointer + dropFiles->pFiles;
			if (dropFiles->fWide)
			{
				TCHAR* pos = static_cast<TCHAR*>(fileListStart);
				while (pos[0] != 0)
				{
					const wstring listElment = wstring(pos);
					oleData.mOperationFilenames.push_back(listElment);
					pos += listElment.length() + 1;
				}
			}
			else
			{
				ANSICHAR* pos = static_cast<ANSICHAR*>(fileListStart);
				while (pos[0] != 0)
				{
					const string listElment = string(pos);
					wstring elment = StringUtil::covert(listElment);
					oleData.mOperationFilenames.push_back(elment);
					pos += listElment.length() + 1;
				}
			}
		}
		return oleData;
	}


	HRESULT STDCALL WindowsWindow::DragEnter(__RPC__in_opt IDataObject *DataObjectPointer, ::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect)
	{
		const DragDropOLEData& oleData = decipherOLEData(DataObjectPointer);
		if (isInGameThread())
		{
			return mOwningApplication->onOLEDragEnter(mHWnd, oleData, KeyState, CursorPosition, CursorEffect);
		}
		else
		{
			return 0;
		}
	}


	HRESULT STDCALL WindowsWindow::DragOver(::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect)
	{
		return 0;
	}
	HRESULT STDCALL WindowsWindow::DragLeave(void)
	{
		return 0;
	}
	HRESULT STDCALL WindowsWindow::Drop(__RPC__in_opt IDataObject *DataObjectPointer, ::DWORD KeyState, POINTL CursorPosition, __RPC__inout::DWORD *CursorEffect)
	{
		return 0;
	}

	HRGN WindowsWindow::makeWindowRegionObject(bool bIncludeBorderWhenMaximized) const
	{
		HRGN region;
		if (mRegionWidth != INDEX_NONE && mRegionHeight != INDEX_NONE)
		{
			const bool bIsBorderlessGameWindow = mDefinition->mType == EWindowType::GameWindow && !mDefinition->HasOSWindowBorder;
			if (isMaximized())
			{
				if (bIsBorderlessGameWindow)
				{
					WINDOWINFO windowInfo;
					Memory::memzero(windowInfo);
					windowInfo.cbSize = sizeof(windowInfo);
					::GetWindowInfo(mHWnd, &windowInfo);
					const int32 windowBorderSize = bIncludeBorderWhenMaximized ? windowInfo.cxWindowBorders : 0;
					region = CreateRectRgn(windowBorderSize, windowBorderSize, mRegionWidth + windowBorderSize, mRegionHeight + windowBorderSize);
				}
				else
				{
					const int32 windowBorderSize = bIncludeBorderWhenMaximized ? getWindowBorderSize() : 0;
					region = CreateRectRgn(windowBorderSize, windowBorderSize, mRegionWidth - windowBorderSize, mRegionHeight - windowBorderSize);
				}
			}
			else
			{
				const bool bUseCornerRadius = mWindowMode == EWindowMode::Windowed && !bIsBorderlessGameWindow && mDefinition->CornerRadius > 0;
				if (bUseCornerRadius)
				{
					region = CreateRoundRectRgn(0, 0, mRegionWidth + 1, mRegionHeight + 1, mDefinition->CornerRadius, mDefinition->CornerRadius);
				}
				else
				{
					region = CreateRectRgn(0, 0, mRegionWidth, mRegionHeight);
				}
			}
		}
		else
		{
			RECT rcWnd;
			GetWindowRect(mHWnd, &rcWnd);
			region = CreateRectRgn(0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top);
		}
		return region;
	}

	void WindowsWindow::adjustWindowRegion(int32 width, int32 height)
	{
		mRegionWidth = width;
		mRegionHeight = height;
		HRGN region = makeWindowRegionObject(true);

		BOOST_VERIFY(SetWindowRgn(mHWnd, region, false) == S_OK, "");
	}
	

	int32 WindowsWindow::getWindowBorderSize() const
	{
		if (mDefinition->mType == EWindowType::GameWindow && !mDefinition->HasOSWindowBorder)
		{
			return 0;
		}
		WINDOWINFO windowInfo;
		Memory::memzero(windowInfo);
		windowInfo.cbSize = sizeof(windowInfo);
		::GetWindowInfo(mHWnd, &windowInfo);
		return windowInfo.cxWindowBorders;
	}

	void WindowsWindow::maximize()
	{
		::ShowWindow(mHWnd, SW_MAXIMIZE);
	}

	void WindowsWindow::minimize()
	{
		::ShowWindow(mHWnd, SW_MINIMIZE);
	}

	void WindowsWindow::bringToFront(bool bForce)
	{
		if (mDefinition->IsRegularWindow)
		{
			if (::IsIconic(mHWnd))
			{
				::ShowWindow(mHWnd, SW_RESTORE);
			}
			else
			{
				::SetActiveWindow(mHWnd);
			}
		}
		else
		{
			HWND hwndInsertAfter = HWND_TOP;
			uint32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER;
			if (!bForce)
			{
				flags |= SWP_NOACTIVATE;
			}
			if (mDefinition->IsTopmostWindow)
			{
				hwndInsertAfter = HWND_TOPMOST;
			}
			::SetWindowPos(mHWnd, hwndInsertAfter, 0, 0, 0, 0, flags);
		}
	}

	bool WindowsWindow::isWindowMinimized() const
	{
		return !!::IsIconic(mHWnd);
	}

	void WindowsWindow::show()
	{
		if (!mIsVisible)
		{
			mIsVisible = true;
			const bool bShouldActivate = mDefinition->AcceptsInput && mDefinition->ActivateWhenFirstShown;

			::ShowWindow(mHWnd, bShouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE);
		}
	}

	int32 WindowsWindow::getWindowTitleBarSize() const
	{
		return GetSystemMetrics(SM_CYCAPTION);
	}
}