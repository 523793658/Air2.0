#include "ParseMeshTask.h"

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
using v8::Array;

ParseMeshTask::ParseMeshTask()
: result_(nullptr)
{

}

ParseMeshTask::~ParseMeshTask()
{

}

void ParseMeshTask::SetParam(byte* data, uint size, PiBool createCollision, const CallBackType& cb)
{
	data_ = data;
	size_ = size;
	createCollision_ = createCollision;
	cb_ = cb;
}

bool ParseMeshTask::Run()
{
	meshResult* result = app_parse_load_mesh(data_, size_, createCollision_);

	result_ = result;

	return true;
}

void ParseMeshTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	// meshes
	int len = result_->count;
	Local<Array> meshArr = Array::New(isolate);
	for (int i = 0; i < len; i++)
	{
		meshArr->Set(i, Integer::New(isolate, reinterpret_cast<int32_t>(result_->meshes[i])));
	}

	param->Set(context,
			   String::NewFromUtf8(isolate, "meshes", NewStringType::kNormal).ToLocalChecked(),
			   meshArr);

	// collision
	Local<Array> collisionArr = Array::New(isolate);
	if (createCollision_)
	{
		for (int i = 0; i < len; i++)
		{
			collisionArr->Set(i, Integer::New(isolate, reinterpret_cast<int32_t>(result_->collision[i])));
		}

		param->Set(context,
			String::NewFromUtf8(isolate, "collision", NewStringType::kNormal).ToLocalChecked(),
			collisionArr);
	}
	else
	{
		param->Set(context,
			String::NewFromUtf8(isolate, "collision", NewStringType::kNormal).ToLocalChecked(),
			v8::Undefined(isolate));
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
	app_parse_mesh_result_free(result_);
}
