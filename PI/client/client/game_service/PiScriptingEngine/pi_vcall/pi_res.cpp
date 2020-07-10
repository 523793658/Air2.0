// TODO: 与js的一些异步交互，后面可以重新组织代码 
#include "pi_res.h"


#include "pi_lib.h"
#include "game_tools/GameTools.h"
#include "pi_converter.h"
#include "game_service\PiScriptingEngine\tasks\ParseImageTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseMeshTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseTerrainMeshTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseSkeletonTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseUVAnimTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseVertexAnimTask.h"
#include "game_service\PiScriptingEngine\tasks\LoadTask.h"
#include "game_service\PiScriptingEngine\tasks\ParseAudio.h"
#include "game_service\PiScriptingEngine\tasks\ParseCLUTTask.h"
#include "game_service\PiScriptingEngine\tasks\ParsePhysicsMeshTask.h"


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

NAN_METHOD(ResBinding::LoadImg)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();
	PiBool isDecompress = info[2]->BooleanValue(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[3]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseImageTask* task = new ParseImageTask();
	task->SetParam(data, size, isDecompress, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::LoadMesh)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();
	PiBool createCollision = info[2]->BooleanValue(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[3]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseMeshTask* task = new ParseMeshTask();
	task->SetParam(data, size, createCollision, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::LoadTerrainMesh)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();
	Local<Object> param = info[2]->ToObject();
	int vw = param->Get(String::NewFromUtf8(isolate, "vw"))->Int32Value(context).FromJust();
	int vh = param->Get(String::NewFromUtf8(isolate, "vh"))->Int32Value(context).FromJust();
	int gs = param->Get(String::NewFromUtf8(isolate, "gs"))->Int32Value(context).FromJust();
	PiBool isEditor = param->Get(String::NewFromUtf8(isolate, "editable"))->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[3]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseTerrainMeshTask* task = new ParseTerrainMeshTask();
	task->SetParam(data, size, vw, vh, gs, isEditor, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::LoadCLUT)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[2]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseCLUTTask* task = new ParseCLUTTask();
	task->SetParam(data, size, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::LoadAudio)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);
	Local<Value> file(info[0]);
	std::wstring str = V8ToWString(file);
	int format = info[1]->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[2]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function>>(isolate, callbackFunction);

	ParseAudioTask* task = new ParseAudioTask();
	task->SetParam(str.c_str(), format, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::CreateSkeleton)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[2]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseSkeletonTask* task = new ParseSkeletonTask();
	task->SetParam(data, size, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::CreateVertexAnim)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[2]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseVertexAnimTask* task = new ParseVertexAnimTask();
	task->SetParam(data, size, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::CreateUVAnim)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	byte* data = (byte*)(info[0]->Int32Value(context).FromJust());
	uint size = info[1]->Int32Value(context).FromJust();

	Local<Function> callbackFunction = Local<Function>::Cast(info[2]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	ParseUVAnimTask* task = new ParseUVAnimTask();
	task->SetParam(data, size, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::AsyncLoad)
{
	Isolate* isolate = info.GetIsolate();

	HandleScope handle_scope(isolate);
	Local<Value> file(info[0]);

	Local<Function> callbackFunction = Local<Function>::Cast(info[1]);

	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

	std::wstring str = V8ToWString(file);

	// 异步调用

	LoadTask* task = new LoadTask();
	task->SetParam(str, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(ResBinding::LoadPhysicsMesh)
{
	Isolate* isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	Local<Context> context = isolate->GetCurrentContext();
	Local<v8::Array> meshes = Local<v8::Array>::Cast(info[0]);
	std::vector<PiMesh*> piMeshes;
	for (int i = 0; i < meshes->Length(); i++)
	{
		piMeshes.push_back(reinterpret_cast <PiMesh*>(meshes->Get(i)->Int32Value(context).FromJust()));
	}
	Local<Function> callbackFunction = Local<Function>::Cast(info[1]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);
	ParsePhysicsMeshTask* task = new ParsePhysicsMeshTask(piMeshes, callback);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

