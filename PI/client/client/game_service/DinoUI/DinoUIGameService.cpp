#include "DinoUIEngineInclude.h"
#include "game_service\V8\V8Service.h"
using namespace v8;

#include "DinoUIPIRenderer\DinoUIPIRenderer.h"
#include "DinoUIV8ScriptingEngine\DinoUIV8ScriptingEngine.h"
#include "DinoUIV8ScriptingEngine\DinoUIV8Backtrace.h"
#include "DinoUIOpenALSoundSystem\DinoUIOpenALSoundSystem.h"

#include "DinoUIGameService.h"
#include "game_service/Manager/PiGameServiceManager.h"

#include "game_service/Win32/WindowsService.h"
#include "game_service/Settings/SettingsService.h"
#include "game_service/PiScriptingEngine/backtrace.h"

// 构造函数
DinoUIGameService::DinoUIGameService()
{
	mbInitialized = false;
	mpClipboardBuffer = nullptr;
	mpUIEngine = nullptr;
}

// 析构函数
DinoUIGameService::~DinoUIGameService()
{

}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult DinoUIGameService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");
	tpManager->AddDependency(this, L"WindowsService");

	mbInitialized = false;

	return SyncResult::SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult DinoUIGameService::Initialize()
{
	if (mpUIEngine == nullptr)
	{
		mpUIEngine = DinoUIEngine::GetUIEngine();
	}

	static bool tbV8FunctionAdded = false;
	if (tbV8FunctionAdded == false)
	{
		PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
		V8Service *tpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

		Isolate *tpIsolate = tpV8Service->GetIsolate();
		HandleScope tHandleScope(tpIsolate);

		Local<Context> tContext = tpV8Service->GetContext();
		Context::Scope tContextScope(tContext);

		Local<FunctionTemplate> tClassTemplate = FunctionTemplate::New(tpIsolate);
		tClassTemplate->SetClassName(v8::String::NewFromUtf8(tpIsolate, "DinoUIService", v8::NewStringType::kNormal).ToLocalChecked());
		tClassTemplate->Set(tpIsolate, "Initialize", FunctionTemplate::New(tpIsolate, DinoUIGameService::DoRealInitialize));

		tpV8Service->AddFunctionBinding("DinoUIService", tClassTemplate);

		tbV8FunctionAdded = true;
	}

	if (mbInitialized == false)
	{
		return AsyncResult_Pending;
	}
	else
	{
		return AsyncResult_Complete;
	}
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult DinoUIGameService::Finalize()
{
	mpUIEngine->Finalize();

	if (mpClipboardBuffer != nullptr)
	{
		delete[] mpClipboardBuffer;
	}

	return AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult DinoUIGameService::Update(const float vfElaspedTime)
{
	mpUIEngine->Update(vfElaspedTime);

	return AsyncResult_Pending;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring DinoUIGameService::GetDisplayName()
{
	return L"DinoUI";
}

// 对外部输出UI内部的输出信息
void DinoUIGameService::OutputUIInformation(DinoUICStr vstrInformation)
{
#ifdef DINOUI_WCHAR
	pi_log_wprint(LOG_WARNING, vstrInformation);
#else
	pi_log_print(LOG_WARNING, vstrInformation);
#endif
}

// 为了等待t3d渲染器完全初始化完成，真正的UI初始化搬到这里来做
void DinoUIGameService::DoRealInitialize(const FunctionCallbackInfo<Value> &vrArgs)
{
	// 把初始化UI的函数暴露给js层
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *tpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate *tpIsolate = tpV8Service->GetIsolate();
	HandleScope tHandleScope(tpIsolate);

	Local<Context> tContext = tpV8Service->GetContext();
	Context::Scope tContextScope(tContext);

	//Local<FunctionTemplate> tClassTemplate = FunctionTemplate::New(tpIsolate);
	//tClassTemplate->SetClassName(v8::String::NewFromUtf8(tpIsolate, "DinoUIService", v8::NewStringType::kNormal).ToLocalChecked());
	//tClassTemplate->Set(tpIsolate, "Initialize", FunctionTemplate::New(tpIsolate, DinoUIGameService::DoRealInitialize));

	//tpV8Service->AddFunctionBinding("DinoUIService", tClassTemplate);

	DinoUIEngine *tpUIEngine = DinoUIEngine::GetUIEngine();

	DinoUIGameService *tpDinoUIService = (DinoUIGameService *)PiGameServiceManager::GetInstance()->GetServiceByName(_DT("DinoUI"));

	// 注册信息输入代理
	tpUIEngine->SetOutputDelegate(tpDinoUIService, &DinoUIGameService::OutputUIInformation);

	// 注册替换系统硬鼠标的代理
	//tpUIEngine->SetSetMouseCursorDelegate(tpDinoUIService, &DinoUIGameService::SetHardMouseCursor);

	// 获取Windows的句柄并保存
	WindowsService *tpWindowsService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");
	tpDinoUIService->mhWnd = tpWindowsService->GetWindowHandle();

	// 注册复制到剪贴板代理
	tpUIEngine->SetCopyDelegate(tpDinoUIService, &DinoUIGameService::CopyToClipboard);

	// 注册从剪贴板获取内容的代理
	tpUIEngine->SetGetClipBoardDelegate(tpDinoUIService, &DinoUIGameService::GetClipBoardContent);

	vrArgs.GetReturnValue().SetUndefined();

	DinoUIEngine::DinoUIEngineCreateParameter tCreateParameter;

	// 显示尺寸
	tCreateParameter.mhWnd = tpWindowsService->GetWindowHandle();
	UINT tnWidth, tnHeight;
	tpWindowsService->GetWindowSize(&tnWidth, &tnHeight);
	tCreateParameter.mnScreenWidth = tnWidth;
	tCreateParameter.mnScreenHeight = tnHeight;

	// 工作目录
	// 本来想用SettingsService来读配置，不过Settings的写法扩展起来好麻烦
	// 而且编辑起来也好麻烦！所以还是用Xml吧 T.T
	SettingsService *tpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(_DT("Settings"));
	DinoUICStr tstrConfigFile = nullptr;
	if (tpSettingService->HasSwith(_DT("uc")) == TRUE)
	{
		tstrConfigFile = tpSettingService->GetSwithValue(_DT("uc"));
	}
	else
	{
		DinoUIEngine::GetUIEngine()->OutputInformation(_DT("请在命令行参数中指定UI的配置文件，如：--uc=client/p23/ui.cfg"));

		return;
	}

	// 从命令行参数读取远程调试的配置
	if (tpSettingService->HasSwith(_DT("urd")) == TRUE)
	{
		tCreateParameter.mRemoteDebug = true;
	}
	else
	{
		tCreateParameter.mRemoteDebug = false;
	}

	XmlDocument tConfigDocuemnt;
	if (tConfigDocuemnt.loadFromFile(tstrConfigFile) == false)
	{
		DinoUIEngine::GetUIEngine()->OutputInformation(_DT("UI无法读取路径配置，所以无法正常启动！"));

		return;
	}

	XmlNode *tpRootNode = tConfigDocuemnt.getFirstChild(NodeIterator());
	XmlNode *tpDirNode = tpRootNode->findChild(_DT("Dir"));

	tCreateParameter.mstrWorkingDirectory = tpDirNode->readAttributeAsString(_DT("Name"));
	tCreateParameter.mbLoadStartupScene = false;

	// 渲染器
	tCreateParameter.mpRenderer = new DinoUIPIRenderer();

	// 声音引擎
	tCreateParameter.mpSoundSystem = new DinoUIOpenALSoundSystem();

	// 脚本引擎
	DinoUIV8ScriptingEngine *tpScriptingEngine = new DinoUIV8ScriptingEngine();
	if (tpScriptingEngine->PreInitialize(tpV8Service->GetIsolate(), tpV8Service->GetContext()) == false)
	{
		return;
	}
	tCreateParameter.mpScript = tpScriptingEngine;

	tpScriptingEngine->SetBacktrace(&Backtrace::GetInstance());
	if (DinoUIEngine::GetUIEngine()->Initialize(&tCreateParameter) == DIR_Success)
	{
		PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
		DinoUIGameService *tpUIService = (DinoUIGameService *)tpManager->GetServiceByName(_DT("DinoUI"));
		tpUIService->mbInitialized = true;
	}
	else
	{
		// @@提示错误
	}
}

// 复制代理
void DinoUIGameService::CopyToClipboard(DinoUICStr vstrContent)
{
	int tnLength = (int)DinoUIStrLen(vstrContent);
	if (tnLength == 0)
	{
		return;
	}

	if (OpenClipboard(mhWnd) == FALSE || EmptyClipboard() == FALSE)
	{
		return;
	}

#ifdef _UNICODE
	int tnSizeInByte = tnLength * 2 + 2;
#else
	int tnSizeInByte = tnLength + 1;
#endif

	HANDLE thMemory = GlobalAlloc(GPTR, tnSizeInByte);
	TCHAR *tpMemory = (TCHAR *)GlobalLock(thMemory);
	memcpy(tpMemory, (char *)vstrContent, tnSizeInByte);
	GlobalUnlock(thMemory);

#ifdef _UNICODE
	SetClipboardData(CF_UNICODETEXT, thMemory);
#else
	SetClipboardData(CF_TEXT, thMemory);
#endif
	CloseClipboard();
}

// 粘贴代理
DinoUICStr DinoUIGameService::GetClipBoardContent(void)
{
	OpenClipboard(mhWnd);

#ifdef _UNICODE
	HANDLE thMemory = GetClipboardData(CF_UNICODETEXT);
#else
	HANDLE thMemory = GetClipboardData(CF_TEXT);
#endif
	if (thMemory == NULL)
	{
		CloseClipboard();

		return NULL;
	}

	int tSizeInByte = (int)GlobalSize(thMemory);

	// 缓冲区的尺寸
#ifdef _UNICODE
	int tnLength = tSizeInByte / 2 + 1;

#else
	int tnLength = tSizeInByte + 1;
#endif

	// 分配缓冲区
	if (mpClipboardBuffer != nullptr)
	{
		delete [] mpClipboardBuffer;
	}

	mpClipboardBuffer = new TCHAR[tnLength];

	TCHAR *tpMemory = (TCHAR *)GlobalLock(thMemory);
	memcpy(mpClipboardBuffer, tpMemory, tSizeInByte);

	GlobalUnlock(thMemory);
	CloseClipboard();

	return mpClipboardBuffer;
}