#include "PiScriptingEngineService.h"
#include "game_service/Settings/SettingsService.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/V8/V8Service.h"
#include "game_tools/GameTools.h"
#include "pi_bindings.h"
#include "pi_lib.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_vcall.h"
#include "EventSystem.h"

using namespace v8;


// ���캯��
PiScriptingEngineService::PiScriptingEngineService()
{
	bindings = nullptr;
	mpV8Service = nullptr;
}

// ��������
PiScriptingEngineService::~PiScriptingEngineService()
{

}

// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
SyncResult PiScriptingEngineService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");
	tpManager->AddDependency(this, L"AsyncLoadResource");
	tpManager->AddDependency(this, L"Settings");

	bindings = new PiBindings();

	return SyncResult_Success;
}

// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult PiScriptingEngineService::Initialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");


	Isolate *tpIsolate = mpV8Service->GetIsolate();
	HandleScope tHandleScope(tpIsolate);

	Local<Context> tContext = mpV8Service->GetContext();
	Context::Scope tContextScope(tContext);

	Local<FunctionTemplate> tClassTemplate = bindings->CreateGlobalPiObj();

	mpV8Service->AddFunctionBinding("pi", tClassTemplate);

	return AsyncResult_Complete;
}

// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult PiScriptingEngineService::Finalize()
{
	if (bindings != nullptr)
	{
		delete bindings;
		bindings = nullptr;
	}

	EventSystem::GetInstance().Finalize();

	return AsyncResult_Complete;
}

// ֡���£��������Pending���������յ����»ص�
// �������Complete�����Ժ��ٸ������Service���������Failure�����������
AsyncResult PiScriptingEngineService::Update(const float vfElaspedTime)
{
	startMainJS();
	return AsyncResult_Complete;
}

// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
std::wstring PiScriptingEngineService::GetDisplayName()
{
	return L"PiScriptingEngine";
}

// ִ��js�ļ�
void PiScriptingEngineService::executeJS(const char* path, const char* data) {
	wchar_t* buf = nullptr;
	buf = pi_str_to_wstr(data, PI_CP_UTF8);

	Isolate* isolate = mpV8Service->GetIsolate();
	Context::Scope scope(mpV8Service->GetContext());
	HandleScope handle_scope(isolate);

	Local<String> name = String::NewFromUtf8(
		isolate,
		path,
		NewStringType::kNormal).ToLocalChecked();

	Local<Value> result = VCallBinding::ExecuteString(
		isolate,
		String::NewFromTwoByte(isolate, (uint16_t*)buf),
		name);

	pi_free(buf);
}

void PiScriptingEngineService::startMainJS() {
	SettingsService *tpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	const char* js_path = tpSettingService->getAppSetting().startPath.c_str();
	wchar* w_path = nullptr;
	w_path = pi_str_to_wstr(js_path, PI_CP_UTF8);
	//���ز�������JS����
	MountFileData main_file = GameTools::ReadMountFile(w_path);

	if (main_file.errorType == 0 && main_file.fileBuf != NULL)
	{
		executeJS(js_path, main_file.fileBuf);
	}

	pi_free(w_path);
}