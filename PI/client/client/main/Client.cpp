// 主程序入口
#include "stdafx.h"
#include "Client.h"
#include <windows.h>
#include "game_service/Win32/WindowsService.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "MWApplication.h"
//#include <ShellScalingAPI.h>
//#include <vld.h> // 需要检查内存泄露的时候再打开这个注释，其它情况下不要


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
	// 这里只能手工的对Windows进行一些必要的初始化
	WindowsService *tpWindowsService = (WindowsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	tpWindowsService->CreateGameWindow(hInstance, L"MWClient", 1, 1);
	tpWindowsService->EnterMessageLoop();
	pi_log_print(LOG_INFO, "system log: start mowen finalize ");
	tpApplication->Finalize();
	pi_log_print(LOG_INFO, "system log: end mowen exe");
	pi_log_close();
	return 0;
}