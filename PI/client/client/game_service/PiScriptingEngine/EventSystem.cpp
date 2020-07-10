#include "EventSystem.h"

#include <Windows.h>
#include <utility>
#include <pi_lib.h>
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/V8/V8Service.h"
#include "game_service/PiScriptingEngine/backtrace.h"

const std::string kCommonKeyDown = "CommonKeyDown";
const std::string kMouseMoveEvent = "MouseMove";
const std::string kMouseWheelEvent = "MouseWheel";
const std::string kMouseDownEvent = "MouseDown";
const std::string kMouseUpEvent = "MouseUp";
const std::string kMouseDoubleClick = "doubleClick";
const std::string kKeyDown = "KeyDown";
const std::string kKeyUp = "KeyUp";
const std::string kClose = "Close";
const std::string kMinSize = "MinSize";
const std::string kMaxSize = "MaxSize";
const std::string kRestored = "Restored";
const std::string kActivateApp = "ActivateApp";

using v8::Context;
using v8::Isolate;
using v8::Value;
using v8::HandleScope;
using v8::Integer;
using v8::String;
using v8::Local;
using v8::Function;
using v8::Object;
using v8::NewStringType;

EventSystem::EventSystem()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	v8Service_ = (V8Service *)tpManager->GetServiceByName(L"V8");
}

EventSystem::~EventSystem()
{
	
}

void EventSystem::SetEventCallback(const std::string& eventName, const CallBackType& cb)
{
	auto cbIter = map_.find(eventName);
	if (cbIter != map_.end())
	{
		cbIter->second.Reset();
	}

	map_[eventName] = cb;
}

void EventSystem::Finalize()
{
	for (auto& cb : map_)
	{
		cb.second.Reset();
	}
}

bool EventSystem::OnLButtonUp(int x, int y)
{
	return OnMouseEvent(kMouseUpEvent, MouseType::LBUTTON, x, y);
}

bool EventSystem::OnLButtonDown(int x, int y)
{
	return OnMouseEvent(kMouseDownEvent, MouseType::LBUTTON, x, y);
}

bool EventSystem::OnButtonDown(int leftFlag, int rightFlag)
{
	return OnEventHelper(kCommonKeyDown, leftFlag, rightFlag);
}

bool EventSystem::OnMButtonUp(int x, int y)
{
	return OnMouseEvent(kMouseUpEvent, MouseType::MBUTTON, x, y);
}

bool EventSystem::OnMButtonDown(int x, int y)
{
	return OnMouseEvent(kMouseDownEvent, MouseType::MBUTTON, x, y);

}

bool EventSystem::OnRButtonUp(int x, int y)
{
	return OnMouseEvent(kMouseUpEvent, MouseType::RBUTTON, x, y);

}

bool EventSystem::OnRButtonDown(int x, int y)
{
	return OnMouseEvent(kMouseDownEvent, MouseType::RBUTTON, x, y);

}

bool EventSystem::OnLDoubleClicked(int x, int y)
{
	return OnEventHelper(kMouseDoubleClick, x, y);
}

bool EventSystem::OnClose()
{
	return OnEventHelper(kClose, 0, 0);
}

bool EventSystem::OnMinSize()
{
	return OnEventHelper(kMinSize, 0, 0);
}

bool EventSystem::OnMaxSize(int x, int y)
{
	return OnEventHelper(kMaxSize, x, y);
}

bool EventSystem::OnRestored(int x, int y)
{
	return OnEventHelper(kRestored, x, y);
}

bool EventSystem::OnActivaterApp(int arg)
{
	return OnEventHelper(kActivateApp, arg);
}

bool EventSystem::OnMouseMove(int x, int y)
{
	return OnEventHelper(kMouseMoveEvent, x, y);
}

bool EventSystem::OnKeyDown(int vkCode)
{
	return OnEventHelper(kKeyDown, vkCode);
}

bool EventSystem::OnKeyUp(int vkCode)
{
	return OnEventHelper(kKeyUp, vkCode);
}

bool EventSystem::IsKeyDown(int vkKey) 
{
	return (::GetAsyncKeyState(vkKey) & 0x8000) ? 1 : 0;
};

bool EventSystem::IsKeyUp(int vkKey)
{
	return (::GetAsyncKeyState(vkKey) & 0x8000) ? 0 : 1;
};

bool EventSystem::OnMouseWheel(int x, int y, int delta)
{
	auto cbIter = map_.find(kMouseWheelEvent);
	if (cbIter == map_.end())
	{
		return false;
	}

	Isolate* isolate = v8Service_->GetIsolate();
	Local<Context> context = v8Service_->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	param->Set(context,
			   String::NewFromUtf8(isolate, "x", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, x));
	param->Set(context,
			   String::NewFromUtf8(isolate, "y", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, y));
	param->Set(context,
			   String::NewFromUtf8(isolate, "delta", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, delta));

	Local<Value> argv[] = {
		param
	};

	Local<Function> local = Local<Function>::New(isolate, cbIter->second);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result)) {
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}

	return true;
}

bool EventSystem::OnMouseEvent(const std::string& eventName, MouseType type, int x, int y)
{
	auto cbIter = map_.find(eventName);
	if (cbIter == map_.end())
	{
		return false;
	}

	Isolate* isolate = v8Service_->GetIsolate();
	Local<Context> context = v8Service_->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	param->Set(context,
			   String::NewFromUtf8(isolate, "x", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, x));
	param->Set(context,
			   String::NewFromUtf8(isolate, "y", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, y));
	param->Set(context,
			   String::NewFromUtf8(isolate, "button", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, static_cast<int>(type)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "shiftKey", NewStringType::kNormal).ToLocalChecked(),
			   v8::Boolean::New(isolate, IsKeyDown(VK_SHIFT)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "ctrlKey", NewStringType::kNormal).ToLocalChecked(),
			   v8::Boolean::New(isolate, IsKeyDown(VK_CONTROL)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "altKey", NewStringType::kNormal).ToLocalChecked(),
			   v8::Boolean::New(isolate, IsKeyDown(VK_MENU)));

	Local<Value> argv[] = {
		param
	};

	Local<Function> local = Local<Function>::New(isolate, cbIter->second);

	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
	
	return true;
}

bool EventSystem::OnEventHelper(const std::string& eventName, int x, int y)
{
	auto cbIter = map_.find(eventName);
	if (cbIter == map_.end())
	{
		return false;
	}

	Isolate* isolate = v8Service_->GetIsolate();
	Local<Context> context = v8Service_->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	param->Set(context,
			   String::NewFromUtf8(isolate, "x", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, x));
	param->Set(context,
			   String::NewFromUtf8(isolate, "y", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, y));

	Local<Value> argv[] = {
		param
	};

	Local<Function> local = Local<Function>::New(isolate, cbIter->second);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
	return true;
}

bool EventSystem::OnEventHelper(const std::string& eventName, int arg)
{
	auto cbIter = map_.find(eventName);
	if (cbIter == map_.end())
	{
		return false;
	}

	Isolate* isolate = v8Service_->GetIsolate();
	Local<Context> context = v8Service_->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	param->Set(context,
		String::NewFromUtf8(isolate, "wParam", NewStringType::kNormal).ToLocalChecked(),
		Integer::New(isolate, arg));
	param->Set(context,
		String::NewFromUtf8(isolate, "shiftKey", NewStringType::kNormal).ToLocalChecked(),
		v8::Boolean::New(isolate, IsKeyDown(VK_SHIFT)));
	param->Set(context,
		String::NewFromUtf8(isolate, "ctrlKey", NewStringType::kNormal).ToLocalChecked(),
		v8::Boolean::New(isolate, IsKeyDown(VK_CONTROL)));
	param->Set(context,
		String::NewFromUtf8(isolate, "altKey", NewStringType::kNormal).ToLocalChecked(),
		v8::Boolean::New(isolate, IsKeyDown(VK_MENU)));

	Local<Value> argv[] = {
		param
	};

	Local<Function> local = Local<Function>::New(isolate, cbIter->second);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
	return true;
}

