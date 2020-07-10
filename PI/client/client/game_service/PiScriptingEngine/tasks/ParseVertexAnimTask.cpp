#include "ParseVertexAnimTask.h"

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

ParseVertexAnimTask::ParseVertexAnimTask()
: result_(nullptr)
{

}

ParseVertexAnimTask::~ParseVertexAnimTask()
{

}

void ParseVertexAnimTask::SetParam(byte* data, uint size, const CallBackType& cb)
{
	data_ = data;
	size_ = size;
	cb_ = cb;
}

bool ParseVertexAnimTask::Run()
{
	PiVertexAnim* result = app_parse_load_vertex_anim(data_, size_);

	result_ = result;

	return true;
}

void ParseVertexAnimTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Value> argv[] = {
		Integer::New(isolate, reinterpret_cast<int32_t>(result_))
	};

	Local<Function> local = Local<Function>::New(isolate, cb_);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
}

