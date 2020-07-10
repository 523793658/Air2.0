// �������͸��ַ���
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

// ���캯��
MWApplication::MWApplication()
{

}

// ��������
MWApplication::~MWApplication()
{

}

// ��ʼ��
bool MWApplication::Initialize()
{
	mpServiceManager = PiGameServiceManager::GetInstance();
	mpServiceManager->Initialize();

	// ================================= ע����ַ������ =================================
	// ��Ϸ�Ľű�����
	PiGameService *tpService = new PiScriptingEngineService();
	mpServiceManager->RegisterGameService(tpService);

	// �첽������Դ
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

// ����
void MWApplication::Finalize()
{
	mpServiceManager->Finalize();
}

// ����
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

// ǿ���˳�����
void MWApplication::QuitApplication(const int vnExitCode)
{
	WindowsService *tpWindowsService = (WindowsService *)mpServiceManager->GetServiceByName(L"WindowsService");
	tpWindowsService->Shutdown(vnExitCode);
}