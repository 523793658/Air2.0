#include "pi_window.h"
#include "pi_converter.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Win32/WindowsService.h"
#include "game_service/PiScriptingEngine/EventSystem.h"
#include "game_service/Settings/SettingsService.h"

using v8::Isolate;
using v8::Local;
using v8::Persistent;
using v8::Context;
using v8::String;
using v8::NewStringType;
using v8::HandleScope;
using v8::Value;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::CopyablePersistentTraits;
using v8::Object;

NAN_METHOD(WindowBinding::AddEventListener)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	String::Utf8Value str(info[0]);
	Local<Function> callbackFunction = Local<Function>::Cast(info[1]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	EventSystem::GetInstance().SetEventCallback(*str, callback);
}
//设置窗口大小,width=arg[0], height=arg[1]
NAN_METHOD(WindowBinding::setWindowSize)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	uint width = info[0]->Int32Value(context).FromJust();
	uint height = info[1]->Int32Value(context).FromJust();
	WindowsService* wnService = (WindowsService* )PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->SetWindowSize(width, height);
}

//执行窗口闪烁
NAN_METHOD(WindowBinding::flashWindow)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	uint count = info[0]->Uint32Value(context).FromJust();
	DWORD time = info[1]->Uint32Value(context).FromJust();
	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->FlashWindowProc(count, time);
}

//设置窗口标题
NAN_METHOD(WindowBinding::setWindowTitle)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	Local<Value> str(info[0]);
	std::wstring title = V8ToWString(str);

	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->SetWindowHeader(title.c_str());
}
//获取窗口大小
int WindowBinding::getWindowSize()
{
	//WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	return 0;
}

//改变全屏状态 fullScreen = arg[0]
NAN_METHOD(WindowBinding::toggleFullScreen)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	bool fullScreen = info[0]->BooleanValue(context).FromJust();
	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->ToggleFullScreen(fullScreen);
}

//执行最大化
NAN_METHOD(WindowBinding::setWindowsShow)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	uint arg = info[0]->Int32Value(context).FromJust();
	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->SetWindowsShow(arg);
}

//显示隐藏窗口 show = arg[0]
NAN_METHOD(WindowBinding::showMainWindow)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	bool show = info[0]->BooleanValue(context).FromJust();
	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->ShowMainWindow(show);
}

NAN_METHOD(WindowBinding::immEnable)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	bool enable = info[0]->BooleanValue(context).FromJust();
	WindowsService* wnService = (WindowsService*)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");
	wnService->enableImm(enable);
}

//销毁窗口
NAN_METHOD(WindowBinding::destroyWindow)
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	WindowsService* wndService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");
	wndService->destroy();
}

//通过Key值获取命令行参数
NAN_METHOD(WindowBinding::getCommandByKey)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	Local<Value> key(info[0]);
	std::wstring str = V8ToWString(key);

	Local<Object> param = Object::New(isolate);

	SettingsService *tpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	if (tpSettingService->HasSwith(str.c_str()) == TRUE)
	{
		std::wstring swithValue = tpSettingService->GetSwithValue(str.c_str());

		info.GetReturnValue().Set(
			v8::String::NewFromTwoByte(info.GetIsolate(), (uint16_t*)(swithValue.c_str()), v8::String::kNormalString,
			swithValue.length()));
	}
}

//打开网页
NAN_METHOD(WindowBinding::openUrl)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	Local<Value> url(info[0]);
	std::wstring urlStr = V8ToWString(url);
	ShellExecute(NULL, L"open", urlStr.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//获取启动程序的微秒数
NAN_METHOD(WindowBinding::getMachineTime)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	WindowsService* server = (WindowsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"WindowsService");

	info.GetReturnValue().Set(v8::Number::New(isolate, server->GetMachineTime()));
}

