#include "ParseTerrainMeshTask.h"

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/PiScriptingEngine/backtrace.h"
#include "game_service/V8/V8Service.h"


using v8::Context;
using v8::Isolate;
using v8::Value;
using v8::HandleScope;
using v8::Integer;
using v8::String;
using v8::Number;
using v8::Local;
using v8::Function;
using v8::Object;
using v8::NewStringType;
using v8::Array;

ParseTerrainMeshTask::ParseTerrainMeshTask()
: result_(nullptr)
{

}

ParseTerrainMeshTask::~ParseTerrainMeshTask()
{

}

void ParseTerrainMeshTask::SetParam(byte* data, uint size, int vw, int vh, int gs, PiBool isEditor, const CallBackType& cb)
{
	data_ = data;
	size_ = size;
	vw_ = vw;
	vh_ = vh;
	gs_ = gs;
	isEditor_ = isEditor;
	cb_ = cb;
}

bool ParseTerrainMeshTask::Run()
{
	TerrainResult* result = app_parse_load_terrain_mesh(data_, size_, vw_, vh_, gs_, isEditor_);

	result_ = result;

	return true;
}

void ParseTerrainMeshTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	// posBuffer
	Local<Array> posBufferArr = Array::New(isolate);
	for (int i = 0; i < result_->posCount; i++)
	{
		posBufferArr->Set(i, Number::New(isolate, result_->posBuffer[i]));
	}

	// norBuffer
	Local<Array> norBufferArr = Array::New(isolate);
	for (int i = 0; i < result_->norCount; i++)
	{
		norBufferArr->Set(i, Number::New(isolate, result_->norBuffer[i]));
	}

	// colorBuffer
	Local<Array> colorBufferArr = Array::New(isolate);
	for (int i = 0; i < result_->colorCount; i++)
	{
		colorBufferArr->Set(i, Number::New(isolate, result_->colorBuffer[i]));
	}

	param->Set(context,
			   String::NewFromUtf8(isolate, "meshHandle", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, reinterpret_cast<int32_t>(result_->mesh)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "collision", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, reinterpret_cast<int32_t>(result_->collision)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "posBuffer", NewStringType::kNormal).ToLocalChecked(),
			   posBufferArr);
	param->Set(context,
			   String::NewFromUtf8(isolate, "normalBuffer", NewStringType::kNormal).ToLocalChecked(),
			   norBufferArr);
	param->Set(context,
			   String::NewFromUtf8(isolate, "colorBuffer", NewStringType::kNormal).ToLocalChecked(),
			   colorBufferArr);

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
	app_parse_terrain_result_free(result_);
}

