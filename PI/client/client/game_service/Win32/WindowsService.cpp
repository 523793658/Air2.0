#include "Resource.h"
#include <Windows.h>
#include <Windowsx.h>
#include <CommCtrl.h> // ���ڸ��ٺ�����ϵͳ���

#include "WindowsService.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "main/MWApplication.h"
#include "game_service/PiScriptingEngine/EventSystem.h"

#include "DinoUIEngineInclude.h"
#include "game_service/Settings/SettingsService.h"

#include "rendersystem.h"

#include <ShellScalingAPI.h>
//#include <VersionHelpers.h>

// ���ﱣ��һ��UI��ָ�뷽����ʣ���Ȼ�ܲ��ÿ�
DinoUIEngine *gpUIEngine;

BOOL gbMouseTracking = FALSE;
//����ϵͳ���
BOOL hideWindowsCursor = FALSE;

HCURSOR ghCursor = 0;

TCHAR gstrCurrentDirectory[255];

std::wstring gstrCursorPath = L"/piassets/ui/cursors/";
std::wstring gstrCursorPathTemp = L"/data/piassets/ui/cursors/";

extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; }


// ����Ӳ���Ĵ���
void __stdcall SetHardMouseCursor(DinoUICStr vstrCursorName)
{
	// ���ԭ���й����Դ����ô�ͷŵ���
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
	//����Ҫ�ĳ�vfs��ȡ����ʱ����Ϊ����Ӧ����ͷǴ���汾
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
		// �����λ��������ʲô�����ڲ����������ʧ������
		DinoUIStrPrintf(tstrCursorFile, 255, _DT("[�������ʧ�ܣ�]��%s"), vstrCursorName);
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
				//ִ���ƶ�������ʱ��Ҫ���Ĵ���size��  
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

// ���캯��
WindowsService::WindowsService()
{
	gpUIEngine = DinoUIEngine::GetUIEngine();
	mnTimer = new GameTimer();
	GetCurrentDirectory(255, gstrCurrentDirectory);
	gpUIEngine->SetSetMouseCursorDelegate(SetHardMouseCursor);

	// Ȼ�����ϵ���һ�Σ����ó���ͨ���
	SetHardMouseCursor(_DT("normalCursor"));
}

// ��������
WindowsService::~WindowsService()
{
	if (mnTimer != nullptr)
	{
		delete mnTimer;
		mnTimer = nullptr;
	}
}

// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ

SyncResult WindowsService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"Settings");
	return SyncResult::SyncResult_Success;
}

// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult WindowsService::Initialize()
{
	SettingsService* mpSettingService = (SettingsService* )PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	hideWindowsCursor = mpSettingService->getWinSetting().hideCursor;

	timeBeginPeriod(1);
	return AsyncResult_Complete;
}

// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult WindowsService::Finalize()
{
	timeEndPeriod(1);
	return AsyncResult_Complete;
}

// ֡���£��������Pending���������յ����»ص�
// �������Complete�����Ժ��ٸ������Service���������Failure�����������
AsyncResult WindowsService::Update(const float vfElaspedTime)
{
	return AsyncResult_Complete;
}

// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
std::wstring WindowsService::GetDisplayName()
{
	return L"WindowsService";
}

// ��ȡ��Ϸ����Ĵ��ھ��
HWND WindowsService::GetWindowHandle()
{
	return mhWnd;
}

// ������Ϸ���򴰿�
bool WindowsService::CreateGameWindow(HINSTANCE vhInstance, const wchar_t *vstrTitle, const int vnWidth, const int vnHeight)
{
	mhInstance = vhInstance;
	mnResolutionWidth = vnWidth;
	mnResolutionHeight = vnHeight;

	// ע�ᴰ����
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

	//��ʼ��ime
	imeInstance.Init(mhWnd);
	imeInstance.Disable();

	//����ʱ����ʾwindow
	//ShowWindow(mhWnd, SW_SHOW);

	return true;
}

// ������Ϣѭ��
void WindowsService::EnterMessageLoop()
{
	MWApplication *tpApplication = MWApplication::GetInstance();

	MSG tMessage;
	ZeroMemory(&tMessage, sizeof(MSG));
	mnTimer->Reset();

	// ��Ϣѭ��
	while (tMessage.message != WM_QUIT)
	{
		// ����Ϣ�����ʹ���windows��Ϣ
		if (PeekMessage(&tMessage, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&tMessage);
			DispatchMessage(&tMessage);
		}
		
		mnTimer->Tick();
		tpApplication->Update(mnTimer->DeltaTime());
		
		//ִ�а�סtitle�ƶ�
		MoveTitle::GetInstance().update();

		Sleep(1);
	}
}

// ���⴦��߻�������������ʱ����������ط�������ȡ���ı�������뽹��
DinoUIControl *gpFocusControl = nullptr;

// ����Windows��Ϣ
// @@ ����IME��Ϣ��������Ϸ��û���ⷽ�����Ҫ
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
				// ��ý��� ͨ����ȡ�����Ƿ�����ע�뼸����ϼ���KeyDown
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
				// @@ �������Ϣ������Ϸ�߼�
			}
			EventSystem::GetInstance().OnKeyDown(wParam);

			break;
		}
		case WM_SYSKEYDOWN:
		{
			// ��ע��Alt
			gpUIEngine->InjectKeyDown(18);

			// ��ע�밴����
			if (gpUIEngine->InjectKeyDown(wParam) == false)
			{
				// @@ �������Ϣ������Ϸ�߼�
			}
			EventSystem::GetInstance().OnKeyDown(wParam);

			break;
		}
		case WM_KEYUP:
		{
			if (gpUIEngine->InjectKeyUp(wParam) == false)
			{
				// @@ �������Ϣ������Ϸ�߼�
			}
			EventSystem::GetInstance().OnKeyUp(wParam);
			break;
		}
		case WM_SYSKEYUP:
		{
			// ��ע�밴����
			if (gpUIEngine->InjectKeyUp(wParam) == false)
			{
				// @@ �������Ϣ������Ϸ�߼�
			}

			// �ȴ������¼��ٰ�Alt����
			gpUIEngine->InjectKeyUp(18);

			EventSystem::GetInstance().OnKeyUp(wParam);
			
			break;
		}
		case WM_LBUTTONDOWN:
		{
			// @@ ����ֻ�Ǽ򵥵�ȡ���ı�������뽹�㣬���Ĳִ٣�����֤�Բ���
			if (gpFocusControl != nullptr && gpFocusControl->GetType() == ControlType_RichTextBox)
			{
				gpUIEngine->SetFocus(nullptr);
				gpFocusControl = nullptr;
			}

			SetCapture(vhWnd);
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Left, tnX, tnY) == false)
			{
				// �������Ϣ������Ϸ�߼�
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
				// �������Ϣ������Ϸ�߼�
				EventSystem::GetInstance().OnLButtonUp(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MBUTTONDOWN:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Middle, tnX, tnY) == false)
			{
				// �������Ϣ������Ϸ�߼�
				EventSystem::GetInstance().OnMButtonDown(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MBUTTONUP:
		{
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonUp(DinoUIEngine::DMB_Middle, tnX, tnY) == false)
			{
				// �������Ϣ������Ϸ�߼�
				EventSystem::GetInstance().OnMButtonUp(tnX, tnY);
				
			}
			
			break;
		}
		case WM_MOUSEMOVE:
		{
			int tnX = GET_X_LPARAM(lParam), tnY = GET_Y_LPARAM(lParam);

			// ���������Ϸ�߼����Լ������ˣ���������������
			RECT tClientRectangle;
			GetClientRect(vhWnd, &tClientRectangle);
			int tnWidth = tClientRectangle.right - tClientRectangle.left;
			int tnHeight = tClientRectangle.bottom - tClientRectangle.top;

			//����ƶ���Ϣ������Эͬ�ԵĹ�ϵ�����Զ�ִ��
			int tnUIX = tnX < 0 ? 0 : (tnX > tnWidth ? tnWidth : tnX);
			int tnUIY = tnY < 0 ? 0 : (tnY > tnHeight ? tnHeight : tnY);
			// UI����Ҫ���ƣ������ѽ����ϵ�������
			gpUIEngine->InjectMousePosition(tnUIX, tnUIY);
			EventSystem::GetInstance().OnMouseMove(tnX, tnY);

			//tnX = tnX < 0 ? 0 : (tnX > width ? width : tnX);
			//tnY = tnY < 0 ? 0 : (tnY > height ? height : tnY);

			// ����������꣬����ϵͳ���
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
				// �������Ϣ������Ϸ�߼�
				EventSystem::GetInstance().OnMouseWheel(tnX, tnY, tnDelta);
			}
			
			break;
		}
		case WM_CHAR:
		{
			// �����ϢӦ����Ϸ�߼��ò���
			gpUIEngine->InjectChar(wParam);

			break;
		}
		case WM_RBUTTONDOWN:
		{
			SetCapture(vhWnd);
			WORD tnX = LOWORD(lParam), tnY = HIWORD(lParam);
			if (gpUIEngine->InjectMouseButtonDown(DinoUIEngine::DMB_Right, tnX, tnY) == false)
			{
				// �������Ϣ������Ϸ�߼�
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
				// �������Ϣ������Ϸ�߼�
				EventSystem::GetInstance().OnRButtonUp(tnX, tnY);
			}
			
			break;
		}
		case WM_SETCURSOR: // ���������
		{
			SetCursor(ghCursor);

			break;
		}
		case WM_NCRBUTTONDOWN:
		{
			gpUIEngine->HideMenu();
			//��������һ�����������Ҫ֪ͨ��Ĭ�ϵ���Ϣ������  
			break;
		}
		case WM_NCLBUTTONDOWN:
		{
			gpUIEngine->HideMenu();
			//����Ϣ��Ҫ����DefWindowProcȥ����
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
		case WM_SYSCOMMAND:
		{
			switch (wParam & 0xfff0)
			{
			case SC_KEYMENU:
			case SC_MOUSEMENU:
				//��������������������ͼ�꣬��Ҫ֪ͨ��Ĭ�ϵ���Ϣ������  
				break;
			case SC_MOVE:
				//�����϶�title����
				MoveTitle::GetInstance().startMove();
				break;
			default:
				//���ز�����Ϣ����Ӧ����Ϣ����Ҫ����DefWindowProcȥ����
				return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
			}
			break;
		}
		case WM_NCLBUTTONUP:
		{
			//�ڴ��ڵķǿͻ�������Ҫ�Ǳ������������̧���ˣ���ʾ�����϶�����   
			MoveTitle::GetInstance().endMove();
			//��Ҫ����DefWindowProcȥ����
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
		default:
		{
			return DefWindowProc(vhWnd, vnMessage, wParam, lParam);
		}
	}

	return 0;
}

// ���ô��ڵı���
void WindowsService::SetWindowHeader(const wchar_t *vstrHeader)
{
	SetWindowText(mhWnd, vstrHeader);
}

// ��˸����
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

//���ھ���
void WindowsService::CenterWindow()
{
	DWORD iStyle = GetWindowLong(mhWnd, GWL_STYLE);
	RECT tWindowRect;
	tWindowRect.left = tWindowRect.top = 0;
	tWindowRect.right = mnResolutionWidth;
	tWindowRect.bottom = mnResolutionHeight;
	AdjustWindowRect(&tWindowRect, iStyle, FALSE);
	
	//���������С
	RECT  workAreaRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
	UINT winWidth = tWindowRect.right - tWindowRect.left;
	UINT winHeight = tWindowRect.bottom - tWindowRect.top;
	UINT workAreaWidth = workAreaRect.right - workAreaRect.left;
	UINT workAreaHeight = workAreaRect.bottom - workAreaRect.top;
	//���ڴ�С���ܳ�����������С
	winWidth = winWidth < workAreaWidth ? winWidth : workAreaWidth;
	winHeight = winHeight < workAreaHeight ? winHeight : workAreaHeight;
	UINT x = workAreaRect.left + (workAreaWidth - winWidth) / 2;
	UINT y = workAreaRect.top + (workAreaHeight - winHeight) / 2;
	SetWindowPos(mhWnd, HWND_TOP, x, y, winWidth, winHeight, SWP_NOZORDER);
	
	
}

// ���ô��ڵĳߴ�
void WindowsService::SetWindowSize(UINT vnWidth, UINT vnHeight)
{
	mnResolutionWidth = vnWidth;
	mnResolutionHeight = vnHeight;
	CenterWindow();
}

// ��ȡ���ڵĳߴ�
void WindowsService::GetWindowSize(UINT *vpWidth, UINT *vpHeight)
{
	*vpWidth = mnResolutionWidth;
	*vpHeight = mnResolutionHeight;
}
// ���ش���ȫ��
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

// ��ʾ���ش���
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

//�Ƿ��������뷨
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

// ǿ�ƹرճ���
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