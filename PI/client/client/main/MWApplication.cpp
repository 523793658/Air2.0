// 服务管理和各种服务
#include "game_service/Manager/PiGameServiceManager.h"

#include <Windows.h>
#include "game_service/Win32/WindowsService.h"
#include "pi_lib.h"
#include "game_service/Async/AsyncLoadResourceService.h"
#include "DinoUIEngineInclude.h"
#include "game_service/V8/V8Service.h"
#include "game_service/DinoUI/DinoUIGameService.h"
#include "game_service/Timer/TimerService.h"
#include "game_service/Settings/SettingsService.h"

#include "v8.h"
#include "libplatform/libplatform.h"
using namespace v8;

#include "game_service/PiScriptingEngine/PiScriptingEngineService.h"

#include "MWApplication.h"

// 构造函数
MWApplication::MWApplication()
{

}

// 析构函数
MWApplication::~MWApplication()
{

}

// 初始化
bool MWApplication::Initialize()
{
	mpServiceManager = PiGameServiceManager::GetInstance();
	mpServiceManager->Initialize();

	// ================================= 注册各种服务组件 =================================
	// 游戏的脚本引擎
	PiGameService *tpService = new PiScriptingEngineService();
	mpServiceManager->RegisterGameService(tpService);

	// 异步加载资源
	tpService = new AsyncLoadResourceService();
	mpServiceManager->RegisterGameService(tpService);

	// Windows
	tpService = new WindowsService();
	mpServiceManager->RegisterGameService(tpService);

	// Settings
	tpService = new SettingsService();
	mpServiceManager->RegisterGameService(tpService);

	// v8
	tpService = new V8Service();
	mpServiceManager->RegisterGameService(tpService);

	// Timer
	tpService = new TimerService();
	mpServiceManager->RegisterGameService(tpService);

	// DinoUI
	tpService = new DinoUIGameService();
	mpServiceManager->RegisterGameService(tpService);

	return true;
}

// 结束
void MWApplication::Finalize()
{
	mpServiceManager->Finalize();
}

// 更新
void MWApplication::Update(const float vfElapsedTime)
{
	mpServiceManager->Update(vfElapsedTime);
}

// Singleton
MWApplication * MWApplication::GetInstance()
{
	static MWApplication sApplication;
	return &sApplication;
}

// 强制退出程序
void MWApplication::QuitApplication(const int vnExitCode)
{
	WindowsService *tpWindowsService = (WindowsService *)mpServiceManager->GetServiceByName(L"WindowsService");
	tpWindowsService->Shutdown(vnExitCode);
}