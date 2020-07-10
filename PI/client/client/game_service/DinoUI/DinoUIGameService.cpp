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

// ���캯��
DinoUIGameService::DinoUIGameService()
{
	mbInitialized = false;
	mpClipboardBuffer = nullptr;
	mpUIEngine = nullptr;
}

// ��������
DinoUIGameService::~DinoUIGameService()
{

}

// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
SyncResult DinoUIGameService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");
	tpManager->AddDependency(this, L"WindowsService");

	mbInitialized = false;

	return SyncResult::SyncResult_Success;
}

// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
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

// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult DinoUIGameService::Finalize()
{
	mpUIEngine->Finalize();

	if (mpClipboardBuffer != nullptr)
	{
		delete[] mpClipboardBuffer;
	}

	return AsyncResult_Complete;
}

// ֡���£��������Pending���������յ����»ص�
// �������Complete�����Ժ��ٸ������Service���������Failure�����������
AsyncResult DinoUIGameService::Update(const float vfElaspedTime)
{
	mpUIEngine->Update(vfElaspedTime);

	return AsyncResult_Pending;
}

// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
std::wstring DinoUIGameService::GetDisplayName()
{
	return L"DinoUI";
}

// ���ⲿ���UI�ڲ��������Ϣ
void DinoUIGameService::OutputUIInformation(DinoUICStr vstrInformation)
{
#ifdef DINOUI_WCHAR
	pi_log_wprint(LOG_WARNING, vstrInformation);
#else
	pi_log_print(LOG_WARNING, vstrInformation);
#endif
}

// Ϊ�˵ȴ�t3d��Ⱦ����ȫ��ʼ����ɣ�������UI��ʼ���ᵽ��������
void DinoUIGameService::DoRealInitialize(const FunctionCallbackInfo<Value> &vrArgs)
{
	// �ѳ�ʼ��UI�ĺ�����¶��js��
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

	// ע����Ϣ�������
	tpUIEngine->SetOutputDelegate(tpDinoUIService, &DinoUIGameService::OutputUIInformation);

	// ע���滻ϵͳӲ���Ĵ���
	//tpUIEngine->SetSetMouseCursorDelegate(tpDinoUIService, &DinoUIGameService::SetHardMouseCursor);

	// ��ȡWindows�ľ��������
	WindowsService *tpWindowsService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");
	tpDinoUIService->mhWnd = tpWindowsService->GetWindowHandle();

	// ע�Ḵ�Ƶ����������
	tpUIEngine->SetCopyDelegate(tpDinoUIService, &DinoUIGameService::CopyToClipboard);

	// ע��Ӽ������ȡ���ݵĴ���
	tpUIEngine->SetGetClipBoardDelegate(tpDinoUIService, &DinoUIGameService::GetClipBoardContent);

	vrArgs.GetReturnValue().SetUndefined();

	DinoUIEngine::DinoUIEngineCreateParameter tCreateParameter;

	// ��ʾ�ߴ�
	tCreateParameter.mhWnd = tpWindowsService->GetWindowHandle();
	UINT tnWidth, tnHeight;
	tpWindowsService->GetWindowSize(&tnWidth, &tnHeight);
	tCreateParameter.mnScreenWidth = tnWidth;
	tCreateParameter.mnScreenHeight = tnHeight;

	// ����Ŀ¼
	// ��������SettingsService�������ã�����Settings��д����չ�������鷳
	// ���ұ༭����Ҳ���鷳�����Ի�����Xml�� T.T
	SettingsService *tpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(_DT("Settings"));
	DinoUICStr tstrConfigFile = nullptr;
	if (tpSettingService->HasSwith(_DT("uc")) == TRUE)
	{
		tstrConfigFile = tpSettingService->GetSwithValue(_DT("uc"));
	}
	else
	{
		DinoUIEngine::GetUIEngine()->OutputInformation(_DT("���������в�����ָ��UI�������ļ����磺--uc=client/p23/ui.cfg"));

		return;
	}

	// �������в�����ȡԶ�̵��Ե�����
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
		DinoUIEngine::GetUIEngine()->OutputInformation(_DT("UI�޷���ȡ·�����ã������޷�����������"));

		return;
	}

	XmlNode *tpRootNode = tConfigDocuemnt.getFirstChild(NodeIterator());
	XmlNode *tpDirNode = tpRootNode->findChild(_DT("Dir"));

	tCreateParameter.mstrWorkingDirectory = tpDirNode->readAttributeAsString(_DT("Name"));
	tCreateParameter.mbLoadStartupScene = false;

	// ��Ⱦ��
	tCreateParameter.mpRenderer = new DinoUIPIRenderer();

	// ��������
	tCreateParameter.mpSoundSystem = new DinoUIOpenALSoundSystem();

	// �ű�����
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
		// @@��ʾ����
	}
}

// ���ƴ���
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

// ճ������
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

	// �������ĳߴ�
#ifdef _UNICODE
	int tnLength = tSizeInByte / 2 + 1;

#else
	int tnLength = tSizeInByte + 1;
#endif

	// ���仺����
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