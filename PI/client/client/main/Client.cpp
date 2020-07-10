// ���������
#include "stdafx.h"
#include "Client.h"
#include <windows.h>
#include "game_service/Win32/WindowsService.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "MWApplication.h"
//#include <ShellScalingAPI.h>
//#include <vld.h> // ��Ҫ����ڴ�й¶��ʱ���ٴ����ע�ͣ���������²�Ҫ


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	MWApplication *tpApplication = MWApplication::GetInstance();
	tpApplication->Initialize();

	//TCHAR tstrOutput[255] = { 0 };
	//PROCESS_DPI_AWARENESS tValue = PROCESS_PER_MONITOR_DPI_AWARE;
	//SetProcessDpiAwareness(tValue);
	//SetProcessDPIAware
	pi_log_print(LOG_INFO, "system log: start mowen game");
	// ����ֻ���ֹ��Ķ�Windows����һЩ��Ҫ�ĳ�ʼ��
	WindowsService *tpWindowsService = (WindowsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	tpWindowsService->CreateGameWindow(hInstance, L"MWClient", 1, 1);
	tpWindowsService->EnterMessageLoop();
	pi_log_print(LOG_INFO, "system log: start mowen finalize ");
	tpApplication->Finalize();
	pi_log_print(LOG_INFO, "system log: end mowen exe");
	pi_log_close();
	return 0;
}