#include "ParsePhysicsMeshTask.h"

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

ParsePhysicsMeshTask::ParsePhysicsMeshTask(std::vector<PiMesh*> meshes, const CallBackType& cb) : mMeshes(meshes), cb_(cb)
{

}

ParsePhysicsMeshTask::~ParsePhysicsMeshTask()
{

}


bool ParsePhysicsMeshTask::Run()
{
	
	for (auto p : mMeshes)
	{
		mPhysicsMeshes.push_back(pi_physics_mesh_create_triangle_mesh(p));
	}
	return true;
}

void ParsePhysicsMeshTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Array> physicsMeshArray = Array::New(isolate, mPhysicsMeshes.size());
	for (int i = 0; i < mPhysicsMeshes.size(); i++)
	{
		physicsMeshArray->Set(i, Integer::New(isolate, reinterpret_cast<int32_t>(mPhysicsMeshes[i])));
	}
	Local<Value> argv[] = {
		physicsMeshArray
	};
	Local<Function> local = Local<Function>::New(isolate, cb_);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}
}
