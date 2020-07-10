#include "Resource.h"
#include <Windows.h>
#include <Windowsx.h>
#include <CommCtrl.h> // 用于跟踪和隐藏系统光标

#include "WindowsService.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "main/MWApplication.h"
#include "game_service/PiScriptingEngine/EventSystem.h"

#include "DinoUIEngineInclude.h"
#include "game_service/Settings/SettingsService.h"

#include "rendersystem.h"

#include <ShellScalingAPI.h>
//#include <VersionHelpers.h>

// 这里保存一个UI的指针方便访问，虽然很不好看
DinoUIEngine *gpUIEngine;

BOOL gbMouseTracking = FALSE;
//隐藏系统鼠标
BOOL hideWindowsCursor = FALSE;

HCURSOR ghCursor = 0;

TCHAR gstrCurrentDirectory[255];

std::wstring gstrCursorPath = L"/piassets/ui/cursors/";
std::wstring gstrCursorPathTemp = L"/data/piassets/ui/cursors/";

extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; }


// 更换硬鼠标的代理
void __stdcall SetHardMouseCursor(DinoUICStr vstrCursorName)
{
	// 如果原来有光标资源，那么释放掉吧
	if (ghCursor != 0)
	{
		DestroyCursor(ghCursor);
		ghCursor = 0;
	}

	DinoUIChar tstrCursorFile[255] = { 0 };
	DinoUIStrPrintf(tstrCursorFile, 255, _DT("%s%s%s%s"), 
		gstrCurrentDirectory, gstrCursorPath.c_str(), vstrCursorName, _DT(".cur"));
	ghCursor = (HCURSOR)LoadImage(NULL, tstrCursorFile, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
	if (ghCursor == 0)
	{
		DinoUIStrPrintf(tstrCursorFile, 255, _DT("%s%s%s%s"),
			gstrCurrentDirectory, gstrCursorPath.c_str(), vstrCursorName, _DT(".ani"));
		ghCursor = (HCURSOR)LoadImage(NULL, tstrCursorFile, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
	}
	//这里要改成vfs读取，临时代码为了适应打包和非打包版本
	if (ghCursor == 0)
	{
		DinoUIStrPrintf(tstrCursorFile, 255, _DT("%s%s%s%s"),
			gstrCurrentDirectory, gstrCursorPathTemp.c_str(), vstrCursorName, _DT(".cur"));
		ghCursor = (HCURSOR)LoadImage(NULL, tstrCursorFile, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
	}
	if (ghCursor == 0)
	{
		DinoUIStrPrintf(tstrCursorFile, 255, _DT("%s%s%s%s"),
			gstrCurrentDirectory, gstrCursorPathTemp.c_str(), vstrCursorName, _DT(".ani"));
		ghCursor = (HCURSOR)LoadImage(NULL, tstrCursorFile, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
	}

	if (ghCursor == 0)
	{
		// 输出这次换的鼠标是什么，便于查找鼠标光标消失的问题
		DinoUIStrPrintf(tstrCursorFile, 255, _DT("[换鼠标光标失败！]：%s"), vstrCursorName);
		DinoUIEngine::GetUIEngine()->OutputInformation(tstrCursorFile);
	}
	else
	{
		SetCursor(ghCursor);
	}
}

MoveTitle::MoveTitle()
{
	mLButtonDownFlag = false;
}

MoveTitle::~MoveTitle()
{
	
}

void MoveTitle::update()
{
	if (mLButtonDownFlag)
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			::GetCursorPos(&mCurMousePos);
			const int nDeltaX = mCurMousePos.x - mLastMousePos.x;
			const int nDeltaY = mCurMousePos.y - mLastMousePos.y;
			if (nDeltaX < -1 || nDeltaX > 1 || nDeltaY < -1 || nDeltaY > 1)
			{
				mCurWindowRect.left += nDeltaX;
				mCurWindowRect.top += nDeltaY;
				mLastMousePos = mCurMousePos;
				//执行移动操作的时候不要更改窗口size。  
				PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
				WindowsService *tpWinService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");
				::SetWindowPos(tpWinService->GetWindowHandle(), HWND_NOTOPMOST, mCurWindowRect.left, mCurWindowRect.top, mCurWindowRect.right, mCurWindowRect.bottom, SWP_NOSIZE);
			}
		}
		else
		{
			mLButtonDownFlag = false;
		}
	}
}

void MoveTitle::startMove()
{
	mLButtonDownFlag = true;
	::GetCursorPos(&mLastMousePos);
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	WindowsService *tpWinService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");
	::GetWindowRect(tpWinService->GetWindowHandle(), &mCurWindowRect);
}

void MoveTitle::endMove()
{
	mLButtonDownFlag = false;
}

// 构造函数
WindowsService::WindowsService()
{
	gpUIEngine = DinoUIEngine::GetUIEngine();
	mnTimer = new GameTimer();
	GetCurrentDirectory(255, gstrCurrentDirectory);
	gpUIEngine->SetSetMouseCursorDelegate(SetHardMouseCursor);

	// 然后马上调用一次，设置成普通光标
	SetHardMouseCursor(_DT("normalCursor"));
}

// 析构函数
WindowsService::~WindowsService()
{
	if (mnTimer != nullptr)
	{
		delete mnTimer;
		mnTimer = nullptr;
	}
}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系

SyncResult WindowsService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"Settings");
	return SyncResult::SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult WindowsService::Initialize()
{
	SettingsService* mpSettingService = (SettingsService* )PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	hideWindowsCursor = mpSettingService->getWinSetting().hideCursor;

	timeBeginPeriod(1);
	return AsyncResult_Complete;
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult WindowsService::Finalize()
{
	timeEndPeriod(1);
	return AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult WindowsService::Update(const float vfElaspedTime)
{
	return AsyncResult_Complete;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring WindowsService::GetDisplayName()
{
	return L"WindowsService";
}

// 获取游戏程序的窗口句柄
HWND WindowsService::GetWindowHandle()
{
	return mhWnd;
}

// 创建游戏程序窗口
bool WindowsService::CreateGameWindow(HINSTANCE vhInstance, const wchar_t *vstrTitle, const int vnWidth, const int vnHeight)
{
	mhInstance = vhInstance;
	mnResolutionWidth = vnWidth;
	mnResolutionHeight = vnHeight;

	// 注册窗口类
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WindowsService::WindowsMessageProcess;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = vhInstance;
	wcex.hIcon = LoadIcon(vhInstance, MAKEINTRESOURCE(IDI_CLIENT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = L"MWClient";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	RECT tWindowRect;
	tWindowRect.left = tWindowRect.top = 0;
	tWindowRect.right = vnWidth;
	tWindowRect.bottom = vnHeight;

	AdjustWindowRect(&tWindowRect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, FALSE);

	mhWnd = CreateWindow(L"MWClient", vstrTitle, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
		0, 0, tWindowRect.right - tWindowRect.left, tWindowRect.bottom - tWindowRect.top, 0, 0, vhInstance, 0);

	if (!mhWnd)
	{
		return false;
	}
	
	CenterWindow();
	
	UpdateWindow(mhWnd);

	//初始化ime
	imeInstance.Init(mhWnd);
	imeInstance.Disable();

	//创建时不显示window
	//ShowWindow(mhWnd, SW_SHOW);

	return true;
}

// 进入消息循环
void WindowsService::EnterMessageLoop()
{
	MWApplication *tpApplication = MWApplication::GetInstance();

	MSG tMessage;
	ZeroMemory(&tMessage, sizeof(MSG));
	mnTimer->Reset();

	// 消息循环
	while (tMessage.message != WM_QUIT)
	{
		// 有消息过来就处理windows消息
		if (PeekMessage(&tMessage, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&tMessage);
			DispatchMessage(&tMessage);
		}
		
		mnTimer->Tick();
		tpApplication->Update(mnTimer->DeltaTime());
		
		//执行按住title移动
		MoveTitle::GetInstance().update();

		Sleep(1);
	}
}

// 特殊处理策划需求：输入文字时，点击其它地方，都会取消文本框的输入焦点
DinoUIControl *gpFocusControl = nullptr;

// 处理Windows消息
// @@ 还有IME消息，看看游戏有没有这方面的需要
LRESULT CALLBACK WindowsService::WindowsMessageProcess(HWND vhWnd, UINT vnMessage, WPARAM wParam, LPARAM lParam)
{
	switch (vnMessage)
	{
		case WM_CLOSE:
		{
			EventSystem::GetInstance().OnClose();
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);

			break;
		}
		case WM_ACTIVATEAPP:
		{
			EventSystem::GetInstance().OnActivaterApp(wParam);

			if (wParam == 0) 
			{
				gpUIEngine->OnApplicationDeactivated();
			}
			else 
			{
				// 获得焦点 通过获取按键是否按下来注入几个组合键的KeyDown
				if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
				{
					gpUIEngine->InjectKeyDown(16);
				}
				if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
				{
					gpUIEngine->InjectKeyDown(17);
				}
				if (::GetAsyncKeyState(VK_MENU) & 0x8000)
				{
					gpUIEngine->InjectKeyDown(18);
				}
				gpUIEngine->OnApplicationActivated();
			}
		

			break;
		}
		case WM_ERASEBKGND:
		{
			pi_rendersystem_swapbuffer();
			break;
		}
		case WM_SIZE:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			switch (wParam)
			{
				case SIZE_MINIMIZED:
					EventSystem::GetInstance().OnMinSize();
					break;
				case SIZE_MAXIMIZED:
					EventSystem::GetInstance().OnMaxSize(tnX, tnY);
					break;
				case SIZE_RESTORED:
					EventSystem::GetInstance().OnRestored(tnX, tnY);
					break;
				default:
					break;
			}

			break;
		}
		case WM_KEYDOWN:
		{
			if (gpUIEngine->InjectKeyDown(wParam) == false)
			{
				// @@ 把这个消息传给游戏逻辑
			}
			EventSystem::GetInstance().OnKeyDown(wParam);

			break;
		}
		case WM_SYSKEYDOWN:
		{
			// 先注入Alt
			gpUIEngine->InjectKeyDown(18);

			// 再注入按键码
			if (gpUIEngine->InjectKeyDown(wParam) == false)
			{
				// @@ 把这个消息传给游戏逻辑
			}
			EventSystem::GetInstance().OnKeyDown(wParam);

			break;
		}
		case WM_KEYUP:
		{
			if (gpUIEngine->InjectKeyUp(wParam) == false)
			{
				// @@ 把这个消息传给游戏逻辑
			}
			EventSystem::GetInstance().OnKeyUp(wParam);
			break;
		}
		case WM_SYSKEYUP:
		{
			// 再注入按键码
			if (gpUIEngine->InjectKeyUp(wParam) == false)
			{
				// @@ 把这个消息传给游戏逻辑
			}

			// 先处理完事件再把Alt清了
			gpUIEngine->InjectKeyUp(18);

			EventSystem::GetInstance().OnKeyUp(wParam);
			
			break;
		}
		case WM_LBUTTONDOWN:
		{
			// @@ 这里只是简单的取消文本框的输入焦点，做的仓促，待验证对不对
			if (gpFocusControl != nullptr && gpFocusControl->GetType() == ControlType_RichTextBox)
			{
				gpUIEngine->SetFocus(nullptr);
				gpFocusControl = nullptr;
			}

			SetCapture(vhWnd);
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Left, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnLButtonDown(tnX, tnY);
			}
			EventSystem::GetInstance().OnButtonDown(1, 0);
			
			break;
		}
		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonUp(DinoUIEngine::DMB_Left, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnLButtonUp(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MBUTTONDOWN:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Middle, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnMButtonDown(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MBUTTONUP:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonUp(DinoUIEngine::DMB_Middle, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnMButtonUp(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MOUSEMOVE:
		{
			int tnX = GET_X_LPARAM(lParam), tnY = GET_Y_LPARAM(lParam);

			// 这里改由游戏逻辑层自己限制了，不再在这里限制
			RECT tClientRectangle;
			GetClientRect(vhWnd, &tClientRectangle);
			int tnWidth = tClientRectangle.right - tClientRectangle.left;
			int tnHeight = tClientRectangle.bottom - tClientRectangle.top;

			//针对移动消息，由于协同性的关系，所以都执行
			int tnUIX = tnX < 0 ? 0 : (tnX > tnWidth ? tnWidth : tnX);
			int tnUIY = tnY < 0 ? 0 : (tnY > tnHeight ? tnHeight : tnY);
			// UI还是要限制，否则会把界面拖到窗口外
			gpUIEngine->InjectMousePosition(tnUIX, tnUIY);
			EventSystem::GetInstance().OnMouseMove(tnX, tnY);

			//tnX = tnX < 0 ? 0 : (tnX > width ? width : tnX);
			//tnY = tnY < 0 ? 0 : (tnY > height ? height : tnY);

			// 用来跟踪鼠标，隐藏系统光标
			//if (hideWindowsCursor && !gbMouseTracking)
			//{
			//	TRACKMOUSEEVENT tme;
			//	tme.cbSize = sizeof(TRACKMOUSEEVENT);
			//	tme.dwFlags = TME_LEAVE;
			//	tme.hwndTrack = vhWnd;
			//	if (::_TrackMouseEvent(&tme))
			//	{
			//		gbMouseTracking = TRUE;

			//		::ShowCursor(TRUE);
			//	}
			//}

			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Left, tnX, tnY);
			EventSystem::GetInstance().OnLDoubleClicked(tnX, tnY);
			break;
		}
		case WM_MOUSELEAVE:
		{
			//gbMouseTracking = FALSE;

			//::ShowCursor(TRUE);

			break;
		}
		case WM_MOUSEWHEEL:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			short tnDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (gpUIEngine->InjectMouseWheel(tnX, tnY, tnDelta) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnMouseWheel(tnX, tnY, tnDelta);
			}
			
			break;
		}
		case WM_CHAR:
		{
			// 这个消息应该游戏逻辑用不到
			gpUIEngine->InjectChar(wParam);

			break;
		}
		case WM_RBUTTONDOWN:
		{
			SetCapture(vhWnd);
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Right, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnRButtonDown(tnX, tnY);
			}
			EventSystem::GetInstance().OnButtonDown(0, 1);
			
			break;
		}
		case WM_RBUTTONUP:
		{
			ReleaseCapture();
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonUp(DinoUIEngine::DMB_Right, tnX, tnY) == false)
			{
				// 把这个消息传给游戏逻辑
				EventSystem::GetInstance().OnRButtonUp(tnX, tnY);
			}
			
			break;
		}
		case WM_SETCURSOR: // 设置鼠标光标
		{
			SetCursor(ghCursor);

			break;
		}
		case WM_NCRBUTTONDOWN:
		{
			gpUIEngine->HideMenu();
			//拦截鼠标右击标题栏，不要通知给默认的消息处理函数  
			break;
		}
		case WM_NCLBUTTONDOWN:
		{
			gpUIEngine->HideMenu();
			//该消息需要交给DefWindowProc去处理
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
		case WM_SYSCOMMAND:
		{
			switch (wParam & 0xfff0)
			{
			case SC_KEYMENU:
			case SC_MOUSEMENU:
				//拦截鼠标左击标题栏左侧的图标，不要通知给默认的消息处理函数  
				break;
			case SC_MOVE:
				//拦截拖动title操作
				MoveTitle::GetInstance().startMove();
				break;
			default:
				//拦截部分消息后，相应的消息还是要交给DefWindowProc去处理
				return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
			}
			break;
		}
		case WM_NCLBUTTONUP:
		{
			//在窗口的非客户区（主要是标题栏），鼠标抬起了，表示不再拖动窗口   
			MoveTitle::GetInstance().endMove();
			//需要交给DefWindowProc去处理。
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
		default:
		{
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
	}

	return 0;
}

// 设置窗口的标题
void WindowsService::SetWindowHeader(const wchar_t *vstrHeader)
{
	SetWindowText(mhWnd, vstrHeader);
}

// 闪烁窗口
bool WindowsService::FlashWindowProc(UINT nCount, DWORD time)
{
	FLASHWINFO info =
	{
		sizeof(FLASHWINFO),
		mhWnd,
		FLASHW_ALL | FLASHW_TIMERNOFG,
		nCount,
		time
	};
	return FlashWindowEx(&info);
}

//窗口居中
void WindowsService::CenterWindow()
{
	DWORD iStyle = GetWindowLong(mhWnd, GWL_STYLE);
	RECT tWindowRect;
	tWindowRect.left = tWindowRect.top = 0;
	tWindowRect.right = mnResolutionWidth;
	tWindowRect.bottom = mnResolutionHeight;
	AdjustWindowRect(&tWindowRect, iStyle, FALSE);
	
	//工作区域大小
	RECT  workAreaRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
	UINT winWidth = tWindowRect.right - tWindowRect.left;
	UINT winHeight = tWindowRect.bottom - tWindowRect.top;
	UINT workAreaWidth = workAreaRect.right - workAreaRect.left;
	UINT workAreaHeight = workAreaRect.bottom - workAreaRect.top;
	//窗口大小不能超过工作区大小
	winWidth = winWidth < workAreaWidth ? winWidth : workAreaWidth;
	winHeight = winHeight < workAreaHeight ? winHeight : workAreaHeight;
	UINT x = workAreaRect.left + (workAreaWidth - winWidth) / 2;
	UINT y = workAreaRect.top + (workAreaHeight - winHeight) / 2;
	SetWindowPos(mhWnd, HWND_TOP, x, y, winWidth, winHeight, SWP_NOZORDER);
	
	
}

// 设置窗口的尺寸
void WindowsService::SetWindowSize(UINT vnWidth, UINT vnHeight)
{
	mnResolutionWidth = vnWidth;
	mnResolutionHeight = vnHeight;
	CenterWindow();
}

// 获取窗口的尺寸
void WindowsService::GetWindowSize(UINT *vpWidth, UINT *vpHeight)
{
	*vpWidth = mnResolutionWidth;
	*vpHeight = mnResolutionHeight;
}
// 开关窗口全屏
void WindowsService::ToggleFullScreen(BOOL fullScreen)
{
	if (fullScreen)
	{
		DWORD iStyle = GetWindowLong(mhWnd, GWL_STYLE);
		SetWindowLong(mhWnd, GWL_STYLE, iStyle & ~WS_CAPTION );
		UINT screenWidth = GetSystemMetrics(SM_CXSCREEN);
		UINT screenHeight = GetSystemMetrics(SM_CYSCREEN);

		SetWindowPos(mhWnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
	}
	else{
		DWORD iStyle = GetWindowLong(mhWnd, GWL_STYLE);
		SetWindowLong(mhWnd, GWL_STYLE, iStyle | WS_CAPTION );
		CenterWindow();
	}
	
}

void WindowsService::SetWindowsShow(UINT type)
{
	ShowWindow(mhWnd, type);
}

// 显示隐藏窗口
void WindowsService::ShowMainWindow(BOOL show)
{
	if (show)
	{
		ShowWindow(mhWnd, SW_SHOW);
	}
	else{
		ShowWindow(mhWnd, SW_HIDE);
	}

}

//是否启用输入法
void WindowsService::enableImm(BOOL enableFlag)
{
	if (enableFlag)
	{
		imeInstance.Enable();
	}
	else{
		imeInstance.Disable();
	}
}

// 强制关闭程序
void WindowsService::Shutdown(int vnExitCode)
{
	pi_log_print(LOG_INFO, "system log: quit message mowen exe");
	PostQuitMessage(vnExitCode);
}

void WindowsService::destroy()
{
	pi_log_print(LOG_INFO, "system log: destroy windows mowen exe");
	DestroyWindow(mhWnd);
}

int64 WindowsService::GetMachineTime()
{
	return mnTimer->GetMachineTime();
}