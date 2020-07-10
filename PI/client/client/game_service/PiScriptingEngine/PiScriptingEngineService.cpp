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


// 构造函数
PiScriptingEngineService::PiScriptingEngineService()
{
	bindings = nullptr;
	mpV8Service = nullptr;
}

// 析构函数
PiScriptingEngineService::~PiScriptingEngineService()
{

}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult PiScriptingEngineService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");
	tpManager->AddDependency(this, L"AsyncLoadResource");
	tpManager->AddDependency(this, L"Settings");

	bindings = new PiBindings();

	return SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
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

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
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

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult PiScriptingEngineService::Update(const float vfElaspedTime)
{
	startMainJS();
	return AsyncResult_Complete;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring PiScriptingEngineService::GetDisplayName()
{
	return L"PiScriptingEngine";
}

// 执行js文件
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
	//加载并运行主JS代码
	MountFileData main_file = GameTools::ReadMountFile(w_path);

	if (main_file.errorType == 0 && main_file.fileBuf != NULL)
	{
		executeJS(js_path, main_file.fileBuf);
	}

	pi_free(w_path);
}