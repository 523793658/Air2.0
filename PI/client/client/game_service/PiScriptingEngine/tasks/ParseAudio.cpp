#include "ParseAudio.h"

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/PiScriptingEngine/backtrace.h"
#include "game_service/V8/V8Service.h"
#include "V8Task.h"

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


ParseAudioTask::ParseAudioTask()
{

}

ParseAudioTask::~ParseAudioTask()
{
	if (this->path)
	{
		pi_free(this->path);
	}
}

void ParseAudioTask::SetParam(const wchar* path, int type, const CallBackType& cb)
{
	this->path = pi_wstr_dup(path);
	this->format = type;
	this->cb_ = cb;
}

bool ParseAudioTask::Run()
{
	AudioResult* result = app_load_audio(this->path, (AudioFormat)format);
	this->result_ = result;
	return true;
}

void ParseAudioTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	if (result_->errorType == 0)
	{
		param->Set(context,
			String::NewFromUtf8(isolate, "data", NewStringType::kNormal).ToLocalChecked(),
			Integer::New(isolate, reinterpret_cast<int32_t>(result_->data)));
	}
	else
	{
		param->Set(context,
			String::NewFromUtf8(isolate, "error", NewStringType::kNormal).ToLocalChecked(),
			Integer::New(isolate, static_cast<int32_t>(result_->errorType)));
		param->Set(context,
			String::NewFromUtf8(isolate, "reason", NewStringType::kNormal).ToLocalChecked(),
			String::NewFromTwoByte(isolate, reinterpret_cast<uint16*>(result_->errorInfo)));
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
	app_load_audio_result_free(result_);
}