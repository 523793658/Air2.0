#include "WindowsApplication.h"
#include "WindowsPlatformMisc.h"
#include "HAL/AirMemory.h"
#include "WindowsWindow.h"
#include <Ole2.h>
#include "CoreGlobals.h"

#include "Containers/Map.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "windows/WindowsCursor.h"
#include <windowsx.h>
namespace Air
{
	WindowsApplication* mWindowsApplication = nullptr;
	const int2 WindowsApplication::MinimizedWindowPosition(-32000, -32000);

	static std::shared_ptr<WindowsWindow> findWindowByHWND(const TArray<std::shared_ptr<WindowsWindow>>& windowsToSearch, HWND handleToFind)
	{
		for (int32 windowIndex = 0; windowIndex < windowsToSearch.size(); ++windowIndex)
		{
			std::shared_ptr<WindowsWindow> window = windowsToSearch[windowIndex];
			if (window->getHWnd() == handleToFind)
			{
				return window;
			}
		}
		return std::shared_ptr<WindowsWindow>();
	}

	WindowsApplication::~WindowsApplication()
	{

	}

	LRESULT CALLBACK WindowsApplication::appWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
	{
		return mWindowsApplication->processMessage(hwnd, msg, wParam, lParam);
	}

	int32 WindowsApplication::processMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
	{
		std::shared_ptr<WindowsWindow> currentNativeEventWindowPtr = findWindowByHWND(mWindows, hwnd);

		if (mWindows.size() && currentNativeEventWindowPtr)
		{
			static const TMap<uint32, wstring> WindowsMessagestrings = []()
			{
				TMap<uint32, wstring> result;
#define ADD_WINDOWS_MESSAGE_STRING(WMCode) result.emplace(WMCode, TEXT(#WMCode))
				ADD_WINDOWS_MESSAGE_STRING(WM_INPUTLANGCHANGEREQUEST);
				ADD_WINDOWS_MESSAGE_STRING(WM_INPUTLANGCHANGE);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_SETCONTEXT);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_NOTIFY);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_REQUEST);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_STARTCOMPOSITION);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_COMPOSITION);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_ENDCOMPOSITION);
				ADD_WINDOWS_MESSAGE_STRING(WM_IME_CHAR);
#undef ADD_WINDOWS_MESSAGE_STRING
				return result;
			}();

			static const TMap<uint32, wstring> IMNStrings = []()
			{
				TMap<uint32, wstring> result;
#define ADD_IMN_STRING(IMNCode) result.emplace(IMNCode, TEXT(#IMNCode))
				ADD_IMN_STRING(IMN_CLOSESTATUSWINDOW);
				ADD_IMN_STRING(IMN_OPENSTATUSWINDOW);
				ADD_IMN_STRING(IMN_CHANGECANDIDATE);
				ADD_IMN_STRING(IMN_CLOSECANDIDATE);
				ADD_IMN_STRING(IMN_OPENCANDIDATE);
				ADD_IMN_STRING(IMN_SETCONVERSIONMODE);
				ADD_IMN_STRING(IMN_SETSENTENCEMODE);
				ADD_IMN_STRING(IMN_SETOPENSTATUS);
				ADD_IMN_STRING(IMN_SETCANDIDATEPOS);
				ADD_IMN_STRING(IMN_SETCOMPOSITIONFONT);
				ADD_IMN_STRING(IMN_SETCOMPOSITIONWINDOW);
				ADD_IMN_STRING(IMN_SETSTATUSWINDOWPOS);
				ADD_IMN_STRING(IMN_GUIDELINE);
				ADD_IMN_STRING(IMN_PRIVATE);
#undef ADD_IMN_STRING
				return result;
			}();

			static const TMap<uint32, wstring> IMRStrings = []()
			{
				TMap<uint32, wstring> result;
#define ADD_IMR_STRING(IMRCode) result.emplace(IMRCode, TEXT(#IMRCode))
				ADD_IMR_STRING(IMR_CANDIDATEWINDOW);
				ADD_IMR_STRING(IMR_COMPOSITIONFONT);
				ADD_IMR_STRING(IMR_COMPOSITIONWINDOW);
				ADD_IMR_STRING(IMR_CONFIRMRECONVERTSTRING);
				ADD_IMR_STRING(IMR_DOCUMENTFEED);
				ADD_IMR_STRING(IMR_QUERYCHARPOSITION);
				ADD_IMR_STRING(IMR_RECONVERTSTRING);
#undef ADD_IMR_STRING
				return result;
			}();

			bool bMessageExternallyHandled = false;
			int32 externalMessageHandlerResult = 0;
			for (IWindowsMessageHandler* handler : mMessageHandlers)
			{
				int32 handlerResult = 0;
				if (handler->processMessage(hwnd, msg, wParam, lParam, handlerResult))
				{
					if (!bMessageExternallyHandled)
					{
						bMessageExternallyHandled = true;
						externalMessageHandlerResult = handlerResult;
					}
				}
			}

			switch (msg)
			{
			case WM_INPUTLANGCHANGEREQUEST:
			case WM_INPUTLANGCHANGE:
			case WM_IME_SETCONTEXT:
			case WM_IME_STARTCOMPOSITION:
			case WM_IME_COMPOSITION:
			case WM_IME_ENDCOMPOSITION:
			case WM_IME_CHAR:
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			case WM_IME_NOTIFY:
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			case WM_IME_REQUEST:
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			case WM_CHAR:
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			case WM_SYSCHAR:
			{
				if ((HIWORD(lParam) & 0x2000) != 0 && wParam == VK_SPACE)
				{
					break;
				}
				else
				{
					return 0;
				}
			}
			break;
			case WM_SYSKEYDOWN:
			{
				if (wParam != VK_F4 && wParam != VK_SPACE)
				{
					deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				}
			}
			break;
			case WM_KEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYUP:
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_XBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			case WM_NCMOUSEMOVE:
			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
#if WINVER >= 0x0601
			case WM_TOUCH:
#endif
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			}
			break;
			case WM_SETCURSOR:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				if (!currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					return 0;
				}
			}
			break;
			case WM_INPUT:
			{
				uint32 size = 0;
				::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
				std::unique_ptr<uint8[]> rawData = makeUniquePtr<uint8[]>(size);
				if (::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawData.get(), &size, sizeof(RAWINPUTHEADER)) == size)
				{
					const RAWINPUT* const raw = (const RAWINPUT* const)rawData.get();
					if (raw->header.dwType == RIM_TYPEMOUSE)
					{
						const bool isAboluteInput = (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE;
						if (isAboluteInput)
						{
							deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam, 0, 0, MOUSE_MOVE_ABSOLUTE);
							return 1;
						}
						const int xPosRelative = raw->data.mouse.lLastX;
						const int yPosRelative = raw->data.mouse.lLastY;
						deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam, xPosRelative, yPosRelative, MOUSE_MOVE_RELATIVE);
						return 1;
					}
				}
			}
			break;
			case WM_NCCALCSIZE:
			{
				if (wParam && !currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					if (currentNativeEventWindowPtr->getDefinition().mType == EWindowType::GameWindow && currentNativeEventWindowPtr->isMaximized())
					{
						WINDOWINFO windowInfo;
						Memory::memzero(windowInfo);
						windowInfo.cbSize = sizeof(windowInfo);
						::GetWindowInfo(hwnd, &windowInfo);
						LPNCCALCSIZE_PARAMS resizingRects = (LPNCCALCSIZE_PARAMS)lParam;
						resizingRects->rgrc[0].left += windowInfo.cxWindowBorders;
						resizingRects->rgrc[0].top += windowInfo.cxWindowBorders;
						resizingRects->rgrc[0].right -= windowInfo.cxWindowBorders;
						resizingRects->rgrc[0].bottom -= windowInfo.cxWindowBorders;

						resizingRects->rgrc[1].left = resizingRects->rgrc[0].left;
						resizingRects->rgrc[1].top = resizingRects->rgrc[0].top;
						resizingRects->rgrc[1].right = resizingRects->rgrc[0].right;
						resizingRects->rgrc[1].bottom = resizingRects->rgrc[0].bottom;

						resizingRects->lppos->x += windowInfo.cxWindowBorders;
						resizingRects->lppos->y += windowInfo.cxWindowBorders;
						resizingRects->lppos->cx -= 2 * windowInfo.cxWindowBorders;
						resizingRects->lppos->cy -= 2 * windowInfo.cxWindowBorders;

						return WVR_VALIDRECTS;
					}
					return 0;
				}
			}
			break;
			case WM_SHOWWINDOW:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
			}
			break;
			case WM_SIZE:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				const bool bWasMaximized = (wParam == SIZE_MAXIMIZED);
				const bool bWasRestored = (wParam == SIZE_RESTORED);
				if (bWasMaximized || bWasRestored)
				{
					mMessageHandler->onWindowAction(currentNativeEventWindowPtr, bWasMaximized ? EWindowAction::Maximize : EWindowAction::Resotre);
				}
				return 0;
			}
			break;

			case WM_SIZING:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam, 0, 0);
				if (currentNativeEventWindowPtr->getDefinition().ShouldPreserveAspectRatio)
				{
					WINDOWINFO windowInfo;
					Memory::memzero(windowInfo);
					windowInfo.cbSize = sizeof(windowInfo);
					::GetWindowInfo(hwnd, &windowInfo);

					RECT testRect;
					testRect.left = testRect.right = testRect.top = testRect.bottom = 0;
					AdjustWindowRectEx(&testRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
					RECT* rect = (RECT*)lParam;
					rect->left -= testRect.left;
					rect->right -= testRect.right;
					rect->top -= testRect.top;
					rect->bottom -= testRect.bottom;

					const float aspectRatio = currentNativeEventWindowPtr->getAspectRatio();
					int32 newWidth = rect->right - rect->left;
					int32 newHight = rect->bottom - rect->top;

					switch (wParam)
					{
					case WMSZ_LEFT:
					case WMSZ_RIGHT:
					{
						int32 adjustedHeight = newWidth / aspectRatio;
						rect->top -= (adjustedHeight - newHight) / 2;
						rect->bottom += (adjustedHeight - newHight) / 2;
						break;
					}
					case WMSZ_TOP:
					case WMSZ_BOTTOM:
					{
						int32 adjustedWidth = newHight * aspectRatio;
						rect->left -= (adjustedWidth - newWidth) / 2;
						rect->right += (adjustedWidth - newWidth) / 2;
						break;
					}
					case WMSZ_TOPLEFT:
					{
						int32 adjustedHeight = newWidth / aspectRatio;
						rect->top -= adjustedHeight - newHight;
						break;
					}
					case WMSZ_TOPRIGHT:
					{
						int32 adjustedHeight = newWidth / aspectRatio;
						rect->top -= adjustedHeight - newHight;
						break;
					}
					case WMSZ_BOTTOMLEFT:
					{
						int32 adjustedHeight = newWidth / aspectRatio;
						rect->bottom += adjustedHeight - newHight;
						break;
					}
					case WMSZ_BOTTOMRIGHT:
					{

						int32 adjustedHeight = newWidth / aspectRatio;
						rect->bottom += adjustedHeight - newHight;
						break;
					}
					}
					AdjustWindowRectEx(rect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
					return true;
				}
			}
			break;
			case WM_ENTERSIZEMOVE:
			{
				bInModalSizeLoop = true;
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
			}
			break;
			case WM_EXITSIZEMOVE:
			{
				bInModalSizeLoop = false;
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
			}
			break;

			case WM_MOVE:
			{
				const int32 newX = (int)(short)(LOWORD(lParam));
				const int32 newY = (int)(short)(HIWORD(lParam));
				int2 newPosition(newX, newY);
				if (WindowsApplication::MinimizedWindowPosition != newPosition)
				{
					mMessageHandler->onMovedWindow(currentNativeEventWindowPtr, newX, newY);
					return 0;
				}
			}
			break;
			case WM_NCHITTEST:
			{

			}
			break;
			case WM_MOUSEACTIVATE:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				break;
			}
			case WM_ACTIVATE:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				break;
			}
			case WM_ACTIVATEAPP:
			{
				bInModalSizeLoop = false;
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
			}
			break;
			case WM_SETTINGCHANGE:
			{
				if ((lParam != NULL) && (wcscmp(TEXT("ConvertibleSlateMode"), (TCHAR*)lParam) == 0))
				{
					deferMessage(currentNativeEventWindowPtr, hwnd, NULL, wParam, lParam);
				}
			}
			break;
			case WM_PAINT:
			{
				if (bInModalSizeLoop && isInGameThread())
				{
					mMessageHandler->onOSPaint(currentNativeEventWindowPtr);
				}
			}
			break;
			case WM_ERASEBKGND:
			{
				return 1;
			}
			break;
			case WM_NCACTIVATE:
			{
				if (!currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					return true;
				}
			}
			break;
			case WM_NCPAINT:
				if (!currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					return 0;
				}
				break;
			case WM_DESTROY:
			{
				mWindows.remove(currentNativeEventWindowPtr);
				return 0;
			}
			break;
			case WM_CLOSE:
			{
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				return 0;
			}
			break;
			case WM_SYSCOMMAND:
			{
				switch (wParam & 0xfff0)
				{
				case SC_RESTORE:
					if (IsIconic(hwnd))
					{
						::ShowWindow(hwnd, SW_RESTORE);
						return 0;
					}
					else
					{
						if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::Resotre))
						{
							return 1;
						}
					}
					break;
				case SC_MAXIMIZE:
				{
					if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::Maximize))
					{
						return 1;
					}
				}
				break;
			default:
				if (!(mMessageHandler->shouldProcessUserInputMessages(currentNativeEventWindowPtr) && isInputMessage(msg)))
				{
					return 0;
				}
				break;
				}
			}
			break;
			case WM_GETMINMAXINFO:
			{
				MINMAXINFO* minmaxInfo = (MINMAXINFO*)lParam;
				break;
			}
			case WM_NCLBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
			{
				switch (wParam)
				{
				case HTMINBUTTON:
				{
					if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::ClickedNonClientArea))
					{
						return 1;
					}
				}
				break;
				case HTMAXBUTTON:
				{
					if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::ClickedNonClientArea))
					{
						return 1;
					}
				}
				break;
				case HTCLOSE:
				{
					if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::ClickedNonClientArea))
					{
						return 1;
					}
				}
				break;
				case HTCAPTION:
				{
					if (!mMessageHandler->onWindowAction(currentNativeEventWindowPtr, EWindowAction::ClickedNonClientArea))
					{
						return 1;
					}
				}
				break;
				}
			}
			break;
			case WM_DISPLAYCHANGE:
			{
			}
			break;
			case WM_DPICHANGED:
				deferMessage(currentNativeEventWindowPtr, hwnd, msg, wParam, lParam);
				break;
			case WM_GETDLGCODE:
			{
				return DLGC_WANTALLKEYS;
			}
			break;
			case WM_CREATE:
				return 0;
			case WM_DEVICECHANGE:
			{
				
			}
			break;

			default:
			{
				if (bMessageExternallyHandled)
				{
					return externalMessageHandlerResult;
				}
			}
				break;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	
	void WindowsApplication::deferMessage(std::shared_ptr<WindowsWindow>& nativeWindow, HWND inHwnd, uint32 inMessage, WPARAM inWParam, LPARAM inLParam, int32 mouseX /* = 0 */, int32 mouseY /* = 0 */, uint32 rawInputFlags /* = 0 */)
	{
		if (GPumpingMessagesOutsideOfMainLoop && bAllowToDeferMessageProcessing)
		{
			mDeferredMessages.add(DeferredWindowsMessage(nativeWindow, inHwnd, inMessage, inWParam, inLParam, mouseX, mouseY, rawInputFlags));
		}
		else
		{
			processDeferredMessage(DeferredWindowsMessage(nativeWindow, inHwnd, inMessage, inWParam, inLParam, mouseX, mouseY, rawInputFlags));
		}
	}

	int32 WindowsApplication::processDeferredMessage(const DeferredWindowsMessage& deferredMessage)
	{
		if (mWindows.size() && !deferredMessage.mNativeWindow.expired())
		{
			HWND hwnd = deferredMessage.mHWND;
			uint32 msg = deferredMessage.mMessage;
			WPARAM wParam = deferredMessage.mWParam;
			LPARAM lParam = deferredMessage.mLParam;
			std::shared_ptr<WindowsWindow> currentNativeEventWindowPtr = deferredMessage.mNativeWindow.lock();
			if (!mMessageHandler->shouldProcessUserInputMessages(currentNativeEventWindowPtr) && isInputMessage(msg))
			{
				if (isKeyboardInputMessage(msg))
				{
					updateAllModifierKeyStates();
				}
				return 0;
			}
			switch (msg)
			{
			case WM_INPUTLANGCHANGEREQUEST:
			case WM_INPUTLANGCHANGE:
			case WM_IME_SETCONTEXT:
			case WM_IME_NOTIFY:
			case WM_IME_REQUEST:
			case WM_IME_STARTCOMPOSITION:
			case WM_IME_COMPOSITION:
			case WM_IME_ENDCOMPOSITION:
			case WM_IME_CHAR:
			{
				break;
			}
			case WM_CHAR:
			{
				const TCHAR character = wParam;
				const bool bIsRepeat = (lParam & 0x40000000) != 0;
				mMessageHandler->onKeyChar(character, bIsRepeat);
				return 0;
			}
			break;

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				const int32 win32Key = wParam;
				int32 actualKey = win32Key;
				bool bIsRepeat = (lParam & 0x4000000) != 0;
				switch (win32Key)
				{
				case VK_MENU:
				{
					if ((lParam & 0x1000000) == 0)
					{
						actualKey = VK_LMENU;
						bIsRepeat = mModifierKeyState[EModifierKey::LeftAlt];
						mModifierKeyState[EModifierKey::LeftAlt] = true;
					}
					else
					{
						actualKey = VK_RMENU;
						bIsRepeat = mModifierKeyState[EModifierKey::RightAlt];
						mModifierKeyState[EModifierKey::RightAlt] = true;
					}
				}
				break;
				case VK_CONTROL:
					if ((lParam & 0x1000000) == 0)
					{
						actualKey = VK_LCONTROL;
						bIsRepeat = mModifierKeyState[EModifierKey::LeftControl];
						mModifierKeyState[EModifierKey::LeftControl] = true;
					}
					else
					{
						actualKey = VK_RCONTROL;
						bIsRepeat = mModifierKeyState[EModifierKey::RightControl];
						mModifierKeyState[EModifierKey::RightControl] = true;
					}
					break;
				case VK_SHIFT:
					actualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
					if (actualKey == VK_LSHIFT)
					{
						bIsRepeat = mModifierKeyState[EModifierKey::LeftShift];
						mModifierKeyState[EModifierKey::LeftShift] = true;
					}
					else
					{
						bIsRepeat = mModifierKeyState[EModifierKey::RightShift];
						mModifierKeyState[EModifierKey::LeftShift] = true;
					}
					break;
				case VK_CAPITAL:
					mModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
					break;
				default:
					break;
				}

				uint32 charCode = ::MapVirtualKey(win32Key, MAPVK_VK_TO_CHAR);

				const bool result = mMessageHandler->onKeyDown(actualKey, charCode, bIsRepeat);
				if (result || msg != WM_SYSKEYDOWN)
				{
					return 0;
				}
			}
			break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				int32 win32Key = wParam;
				int32 actualKey = win32Key;
				bool bModifierKeyReleased = false;
				switch (win32Key)
				{
				case VK_MENU:
					if ((lParam & 0x1000000) == 0)
					{
						actualKey = VK_LMENU;
						mModifierKeyState[EModifierKey::LeftAlt] = false;
					}
					else
					{
						actualKey = VK_RMENU;
						mModifierKeyState[EModifierKey::RightAlt] = false;
					}
					break;
				case VK_CONTROL:
					if ((lParam & 0x1000000) == 0)
					{
						actualKey = VK_LCONTROL;
						mModifierKeyState[EModifierKey::LeftControl] = false;
					}
					else
					{
						actualKey = VK_RCONTROL;
						mModifierKeyState[EModifierKey::RightAlt] = false;
					}
					break;
				case VK_SHIFT:
					actualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
					if (actualKey == VK_LSHIFT)
					{
						mModifierKeyState[EModifierKey::LeftShift] = false;
					}
					else
					{
						mModifierKeyState[EModifierKey::RightShift] = false;
					}
					break;
				case VK_CAPITAL:
					mModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
					break;
				default:
					break;
				}

				uint32 charCode = ::MapVirtualKey(win32Key, MAPVK_VK_TO_CHAR);
				const bool bIsRepeat = false;
				const bool result = mMessageHandler->onKeyUp(actualKey, charCode, bIsRepeat);
				if (result || msg != WM_SYSKEYUP)
				{
					return 0;
				}
			}
			break;
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_XBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			case WM_XBUTTONUP:
			{
				POINT cursorPoint;
				cursorPoint.x = GET_X_LPARAM(lParam);
				cursorPoint.y = GET_Y_LPARAM(lParam);
				ClientToScreen(hwnd, &cursorPoint);
				const float2 cursorPos(cursorPoint.x, cursorPoint.y);
				EMouseButtons::Type mouseButton = EMouseButtons::Invalid;
				bool bDoubleClick = false;
				bool bMouseUp = false;
				switch (msg)
				{
				case WM_LBUTTONDBLCLK:
					bDoubleClick = true;
					mouseButton = EMouseButtons::Left;
					break;
				case WM_LBUTTONUP:
					bMouseUp = true;
					mouseButton = EMouseButtons::Left;
					break;
				case WM_LBUTTONDOWN:
					mouseButton = EMouseButtons::Left;
					break;
				case WM_MBUTTONDBLCLK:
					bDoubleClick = true;
					mouseButton = EMouseButtons::Middle;
					break;
				case WM_MBUTTONUP:
					bMouseUp = true;
					mouseButton = EMouseButtons::Middle;
					break;
				case WM_MBUTTONDOWN:
					mouseButton = EMouseButtons::Middle;
					break;
				case WM_RBUTTONDBLCLK:
					bDoubleClick = true;
					mouseButton = EMouseButtons::Right;
					break;
				case WM_RBUTTONUP:
					bMouseUp = true;
					mouseButton = EMouseButtons::Right;
					break;
				case WM_RBUTTONDOWN:
					mouseButton = EMouseButtons::Right;
					break;
				case WM_XBUTTONDBLCLK:
					bDoubleClick = true;
					mouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
					break;
				case WM_XBUTTONUP:
					bMouseUp = true;
					mouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
					break;
				case WM_XBUTTONDOWN:
					mouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
					break;
				default:
					assert(0);
				}
				if (bMouseUp)
				{
					return mMessageHandler->onMouseUp(mouseButton, cursorPos) ? 0 : 1;
				}
				else if (bDoubleClick)
				{
					mMessageHandler->onMouseDoubleClick(currentNativeEventWindowPtr, mouseButton, cursorPos);
				}
				else
				{
					mMessageHandler->onMouseDown(currentNativeEventWindowPtr, mouseButton, cursorPos);
				}
				return 0;
			}
			break;
			case WM_INPUT:
			{
				if (deferredMessage.mRawInputFlags == MOUSE_MOVE_RELATIVE)
				{
					mMessageHandler->onRawMouseMove(deferredMessage.mX, deferredMessage.mY);
				}
				else
				{
					mMessageHandler->onMouseMove();
				}
				return 0;
			}
			break;

			case WM_NCMOUSEMOVE:
			case WM_MOUSEMOVE:
			{
				BOOL result = FALSE;
				if (!bUsingHighPrecisionMouseInput)
				{
					result = mMessageHandler->onMouseMove();
				}
				return result ? 0 : 1;
			}
			break;
			case WM_MOUSEWHEEL:
			{
				const float spinFactor = 1 / 120.0f;
				const SHORT wheeldelta = GET_WHEEL_DELTA_WPARAM(wParam);
				POINT cursorPoint;
				cursorPoint.x = GET_X_LPARAM(lParam);
				cursorPoint.y = GET_Y_LPARAM(lParam);

				const float2 cursorPos(cursorPoint.x, cursorPoint.y);
				const BOOL result = mMessageHandler->onMouseWheel(static_cast<float>(wheeldelta) * spinFactor, cursorPos);

				return result ? 0 : 1;

			}

			case WM_SETCURSOR:
			{
				return mMessageHandler->onCursorSet() ? 0 : 1;
			}
			break;
			case WM_MOUSEACTIVATE:
			{
				bForceActivateByMouse = !(LOWORD(lParam) & HTCLIENT);
				return 0;
			}
			break;

			case WM_ACTIVATE:
			{
				EWindowActivation::Type activationType;
				if (LOWORD(wParam) & WA_ACTIVE)
				{
					activationType = bForceActivateByMouse ? EWindowActivation::ActivateByMouse : EWindowActivation::Activate;
				}
				else if (LOWORD(wParam) & WA_CLICKACTIVE)
				{
					activationType = EWindowActivation::ActivateByMouse;
				}
				else
				{
					activationType = EWindowActivation::Deactivate;
				}
				bForceActivateByMouse = false;
				updateAllModifierKeyStates();
				if(currentNativeEventWindowPtr)
				{
					BOOL result = FALSE;
					result = mMessageHandler->OnWindowActivationChanged(currentNativeEventWindowPtr, activationType);
					return result ? 0 : 1;

				}
				return 1;
			}
			break;
			case WM_ACTIVATEAPP:
				updateAllModifierKeyStates();
				mMessageHandler->onApplicationActivationChanged(!!wParam);
				break;
			case WM_SETTINGCHANGE:
				if ((lParam != 0) && (CString::strcmp((LPCTSTR)lParam, TEXT("ConvertibleSlateMode")) == 0))
				{
					
				}
				break;
			case WM_NCACTIVATE:
				if (currentNativeEventWindowPtr && !currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					return true;
				}
				break;
			case WM_NCPAINT:
				if (currentNativeEventWindowPtr&& currentNativeEventWindowPtr->getDefinition().HasOSWindowBorder)
				{
					return 0;
				}
				break;
			case WM_CLOSE:
			{
				if (currentNativeEventWindowPtr)
				{
					mMessageHandler->onWindowClose(currentNativeEventWindowPtr);
				}
				return 0;
			}
			break;
			case WM_SHOWWINDOW:
			{
				if (currentNativeEventWindowPtr)
				{
					switch (lParam)
					{
					case SW_PARENTCLOSING:
						currentNativeEventWindowPtr->onParentWindowMinimized();
						break;
					case SW_PARENTOPENING:
						currentNativeEventWindowPtr->onParentWindowRestored();
						break;

					default:
						break;
					}
				}
			}
			break;

			case WM_SIZE:
			{
				if (currentNativeEventWindowPtr)
				{
					const int32 NewWidth = (int)(short)(LOWORD(lParam));
					const int32 newHeight = (int)(short)(HIWORD(lParam));

					const GenericWindowDefinition& definition = currentNativeEventWindowPtr->getDefinition();
					if (definition.IsRegularWindow && !definition.HasOSWindowBorder)
					{
						currentNativeEventWindowPtr->adjustWindowRegion(NewWidth, newHeight);
					}
					const bool bWasMinimized = (wParam == SIZE_MINIMIZED);
					const bool bIsFullscreen = (currentNativeEventWindowPtr->getWindowMode() == EWindowMode::Type::Fullscreen);

					if (!bIsFullscreen)
					{
						const bool result = mMessageHandler->onSizeChanged(currentNativeEventWindowPtr, NewWidth, newHeight, bWasMinimized);
					}
				}
			}
			break;
			case WM_SIZING:
			{
				if (currentNativeEventWindowPtr)
				{
					mMessageHandler->onResizingWindow(currentNativeEventWindowPtr);
				}
			}
			break;
			case WM_ENTERSIZEMOVE:
			{
				if (currentNativeEventWindowPtr)
				{
					mMessageHandler->beginReshapingWindow(currentNativeEventWindowPtr);
				}
			}
			break;
			case WM_EXITSIZEMOVE:
			{
				if (currentNativeEventWindowPtr)
				{
					mMessageHandler->finishedReshapingWindow(currentNativeEventWindowPtr);
				}
			}
			break;

			case WM_DPICHANGED:
			{
				if (currentNativeEventWindowPtr)
				{
					currentNativeEventWindowPtr->setDPIScaleFactor(LOWORD(wParam) / 96.0f);
					LPRECT newRect = (LPRECT)lParam;
					SetWindowPos(hwnd, nullptr, newRect->left, newRect->top, newRect->right - newRect->left, newRect->bottom - newRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
			break;
			}
		}
		return 0;
	}

	void WindowsApplication::setHighPrecisionMouseMode(const bool enable, const std::shared_ptr<GenericWindow>& inWindow)
	{
		HWND hwnd = NULL;
		DWORD flags = RIDEV_REMOVE;
		bUsingHighPrecisionMouseInput = enable;
		if (enable)
		{
			flags = 0;
			if (inWindow)
			{
				hwnd = (HWND)inWindow->getOSWindowHandle();
			}
		}
		RAWINPUTDEVICE rawInputDevice;
		const uint16 standardMouse = 0x02;
		rawInputDevice.usUsagePage = 0.01;
		rawInputDevice.usUsage = standardMouse;
		rawInputDevice.dwFlags = flags;
		rawInputDevice.hwndTarget = hwnd;
		::RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE));
	}

	WindowsApplication* WindowsApplication::createWindowsApplication(const HINSTANCE instatnceHandle, const HICON iconHandle)
	{
		mWindowsApplication = new WindowsApplication(instatnceHandle, iconHandle);
		return mWindowsApplication;
	}

	WindowsApplication::WindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle)
		:GenericApplication(MakeSharedPtr<WindowsCursor>())
		,mInstanceHandle(instanceHandle)
	{
		Memory::memzero(mModifierKeyState, EModifierKey::Count);
		::DisableProcessWindowsGhosting();
		//WindowsPlatformMisc::
		const bool bClassRegistered = registerClass(instanceHandle, iconHandle);
		OleInitialize(NULL);


	}

	bool WindowsApplication::registerClass(const HINSTANCE instanceHandle, const HICON iconHandle)
	{
		WNDCLASS wc;
		Memory::memzero(&wc, sizeof(wc));
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = appWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = instanceHandle;
		wc.hIcon = iconHandle;
		wc.hCursor = LoadCursor(NULL, IDC_HELP);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WindowsWindow::AppWindowClass;

		if (!::RegisterClass(&wc))
		{
			DWORD e = GetLastError();
			return false;
		}
		return true;
	}

	void WindowsApplication::initializeWindow(const std::shared_ptr<GenericWindow>& inWindow, const std::shared_ptr<GenericWindowDefinition>& inDesc, const std::shared_ptr<GenericWindow>& inParent, const bool bShowImmediately)
	{
		const std::shared_ptr<WindowsWindow> window = std::static_pointer_cast<WindowsWindow>(inWindow);
		const std::shared_ptr<WindowsWindow> parentWindow = std::static_pointer_cast<WindowsWindow>(inParent);
		mWindows.push_back(window);
		window->initialize(this, inDesc, mInstanceHandle, parentWindow, bShowImmediately);
	}

	void WindowsApplication::addMessageHandler(IWindowsMessageHandler& inMessageHandler)
	{
		mWindowsApplication->mMessageHandlers.addUnique(&inMessageHandler);
	}

	std::shared_ptr<GenericWindow> WindowsApplication::makeWindow()
	{
		return WindowsWindow::make();
	}


	HRESULT WindowsApplication::onOLEDragEnter(const HWND hWnd, const DragDropOLEData& oleData, ::DWORD KeyState, POINTL cursorPositon, ::DWORD* cursorEffect)
	{
		return 0;
	}
	HRESULT WindowsApplication::onOLEDragOver(const HWND HWnd, ::DWORD KeyState, POINTL CursorPosition, ::DWORD *CursorEffect)
	{
		return 0;
	}

	/** Invoked by a window when an OLE Drag and Drop exits the window. */
	HRESULT WindowsApplication::onOLEDragOut(const HWND HWnd)
	{
		return 0;

	}

	/** Invoked by a window when an OLE Drag and Drop is dropped onto the window. */
	HRESULT WindowsApplication::onOLEDrop(const HWND HWnd, const DragDropOLEData& OLEData, ::DWORD KeyState, POINTL CursorPosition, ::DWORD *CursorEffect)
	{
		return 0;

	}

	ModifierKeysState WindowsApplication::getModifierKeys() const
	{
		return ModifierKeysState(mModifierKeyState[EModifierKey::LeftShift], mModifierKeyState[EModifierKey::RightShift], mModifierKeyState[EModifierKey::LeftControl], mModifierKeyState[EModifierKey::RightControl], mModifierKeyState[EModifierKey::LeftAlt], mModifierKeyState[EModifierKey::RightAlt], false, false, mModifierKeyState[EModifierKey::CapsLock]);
	}

	bool WindowsApplication::isKeyboardInputMessage(uint32 msg)
	{
		switch (msg)
		{
		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_SYSCOMMAND:
			return true;
		}
		return false;
	}

	bool WindowsApplication::isMouseInputMessage(uint32 msg)
	{
		switch (msg)
		{
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE:
		case WM_NCMBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCXBUTTONDBLCLK:
		case WM_NCXBUTTONDOWN:
		case WM_NCXBUTTONUP:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONUP:
			return true;
		}
		return false;
	}

	bool WindowsApplication::isInputMessage(uint32 msg)
	{
		if (isKeyboardInputMessage(msg) || isMouseInputMessage(msg))
		{
			return true;
		}
		switch (msg)
		{
		case WM_INPUT:
		case WM_INPUT_DEVICE_CHANGE:
			return true;
		}
		return false;
	}

	void WindowsApplication::updateAllModifierKeyStates()
	{
		mModifierKeyState[EModifierKey::LeftShift] = (::GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::RightShift] = (::GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::LeftControl] = (::GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::RightControl] = (::GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::LeftAlt] = (::GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::RightAlt] = (::GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
		mModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
	}
	void WindowsApplication::setMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& inMessageHandler)
	{
		GenericApplication::setMessageHandler(inMessageHandler);
	}

	bool WindowsApplication::isCursorDirectlyOverSlateWindow() const
	{
		POINT cursorPos;
		BOOL bGotPoint = ::GetCursorPos(&cursorPos);
		if (bGotPoint)
		{
			HWND hWnd = ::WindowFromPoint(cursorPos);
			std::shared_ptr<WindowsWindow> slateWindowUnderCursor = findWindowByHWND(mWindows, hWnd);
			return !!slateWindowUnderCursor;
		}
		return false;
	}

	void WindowsApplication::setCapture(const std::shared_ptr<GenericWindow>& inWindow)
	{
		if (inWindow)
		{
			::SetCapture((HWND)inWindow->getOSWindowHandle());
		}
		else
		{
			::ReleaseCapture();
		}
	}

	void* WindowsApplication::getCapture() const
	{
		return ::GetCapture();
	}

	void WindowsApplication::pollGameDeviceState(const float timeDelta)
	{
	}
}