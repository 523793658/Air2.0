#include "LoadTask.h"
#include "game_tools/GameTools.h"

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/PiScriptingEngine/backtrace.h"
#include "game_service/V8/V8Service.h"

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

LoadTask::LoadTask()
{

}

LoadTask::~LoadTask()
{

}


bool LoadTask::Run()
{
	MountFileData data = GameTools::ReadMountFile(path_.c_str());

	file_buf_ = data.fileBuf;
	file_size_ = data.fileSize;
	error_type_ = data.errorType;
	error_message_ = data.errorInfo;

	return true;
}

void LoadTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	if (error_type_ == 0)
	{
		param->Set(context,
				   String::NewFromUtf8(isolate, "data", NewStringType::kNormal).ToLocalChecked(),
				   Integer::New(isolate, reinterpret_cast<int32_t>(file_buf_)));
		param->Set(context,
				   String::NewFromUtf8(isolate, "size", NewStringType::kNormal).ToLocalChecked(),
				   Integer::New(isolate, static_cast<int32_t>(file_size_)));
	}
	else
	{
		param->Set(context,
				   String::NewFromUtf8(isolate, "error", NewStringType::kNormal).ToLocalChecked(),
				   Integer::New(isolate, static_cast<int32_t>(error_type_)));
		param->Set(context,
				   String::NewFromUtf8(isolate, "reason", NewStringType::kNormal).ToLocalChecked(),
				   String::NewFromTwoByte(isolate, reinterpret_cast<uint16*>(error_message_)));
	}

	Local<Value> argv[] = {
		param
	};

	Local<Function> local = Local<Function>::New(isolate, cb_);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
}

void LoadTask::SetParam(const std::wstring& path, const CallBackType& cb)
{
	path_ = path;
	cb_ = cb;
}

