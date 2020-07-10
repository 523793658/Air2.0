// Copyright www.jzyx.com reserved 
// 2015-05-06

#include "pi_bindings.h"
#include "game_service/PiScriptingEngine/pi_util.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_res.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_vcall.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_window.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_backtrace.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_heap.h"
#include "game_service/PiScriptingEngine/pi_vcall/pi_cpu.h"

#include "game_service/PiScriptingEngine/v8-profiler/heap_snapshot.h"
#include "game_service/PiScriptingEngine/v8-profiler/cpu_profile.h"

using v8::Isolate;
using v8::Local;
using v8::Context;
using v8::String;
using v8::ObjectTemplate;
using v8::FunctionTemplate;
using v8::NewStringType;
using v8::Object;
using v8::Array;

PiBindings::PiBindings() 
{

}

PiBindings::~PiBindings() 
{
}

Local<FunctionTemplate> PiBindings::CreateGlobalPiObj()
{
	Isolate* isolate = Isolate::GetCurrent();

	Local<FunctionTemplate> pi_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> vcall_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> v8_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> res_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> window_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> backtrace_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> heap_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> cpu_obj = FunctionTemplate::New(isolate);
	Local<FunctionTemplate> audio_obj = FunctionTemplate::New(isolate);


	pi_obj->Set(String::NewFromUtf8(isolate, "vcall", NewStringType::kNormal).ToLocalChecked(),
				vcall_obj);

	pi_obj->Set(String::NewFromUtf8(isolate, "v8", NewStringType::kNormal).ToLocalChecked(),
				v8_obj);

	pi_obj->Set(String::NewFromUtf8(isolate, "res", NewStringType::kNormal).ToLocalChecked(),
				res_obj);
	pi_obj->Set(String::NewFromUtf8(isolate, "window", NewStringType::kNormal).ToLocalChecked(),
		window_obj);

	pi_obj->Set(String::NewFromUtf8(isolate, "backtrace", NewStringType::kNormal).ToLocalChecked(),
				backtrace_obj);

	pi_obj->Set(String::NewFromUtf8(isolate, "heap", NewStringType::kNormal).ToLocalChecked(),
				heap_obj);

	pi_obj->Set(String::NewFromUtf8(isolate, "cpu", NewStringType::kNormal).ToLocalChecked(),
				cpu_obj);

	BindToPi(pi_obj);
	BindToVCall(vcall_obj);
	BindToV8(v8_obj);
	BindToRes(res_obj);
	BindToWindow(window_obj);
	BindToBacktrace(backtrace_obj);
	BindToHeap(heap_obj);
	BindToCpu(cpu_obj);
	
	return pi_obj;
}

void PiBindings::BindToPi(const Local<FunctionTemplate>& pi_obj)
{
	// pi.log(), pi.gc() 
	Nan::SetMethod(pi_obj, "log", &VCallBinding::Log);
	Nan::SetMethod(pi_obj, "gc", &VCallBinding::VCallGC);

	// pi.vfsMount("$root/a/b", "FILE", 1, "F:/x/y/z")
	Nan::SetMethod(pi_obj, "vfsMount", &VCallBinding::SetVfsMountPath);

	//  pi.getCommandline()
	Nan::SetMethod(pi_obj, "getCommandline", &VCallBinding::GetCommandLineString);

	Nan::SetMethod(pi_obj, "getWinHandle", &VCallBinding::GetWinHandle);

	Nan::SetMethod(pi_obj, "httpRequest", &VCallBinding::HttpRequest);
}

void PiBindings::BindToVCall(const Local<FunctionTemplate>& vcall_obj)
{

	Nan::SetMethod(vcall_obj, "register", &VCallBinding::Register);
	Nan::SetMethod(vcall_obj, "parseCmdList", &VCallBinding::ParseCmdList);
	Nan::SetMethod(vcall_obj, "getBufferAddress", &VCallBinding::GetBufferAddress);
	Nan::SetMethod(vcall_obj, "getHandle", &VCallBinding::GetHandle);
	Nan::SetMethod(vcall_obj, "getVar", &VCallBinding::HandleToVar);
	Nan::SetMethod(vcall_obj, "load", &VCallBinding::Load);
	Nan::SetMethod(vcall_obj, "executeStr", &VCallBinding::ExecuteStr);
	Nan::SetMethod(vcall_obj, "cls", &VCallBinding::ClearConsole);
	Nan::SetMethod(vcall_obj, "type", &VCallBinding::PrintParamType);
	Nan::SetMethod(vcall_obj, "loadScriptFromStr", &VCallBinding::LoadScriptFromStr);


	Nan::SetMethod(vcall_obj, "fillCmdList", &VCallBinding::FillCmdList);
	Nan::SetMethod(vcall_obj, "writeBuffer", &VCallBinding::WriteBuffer);
	Nan::SetMethod(vcall_obj, "readBuffer", &VCallBinding::ReadBuffer);
	Nan::SetMethod(vcall_obj, "invoke", &VCallBinding::Invoke);
	Nan::SetMethod(vcall_obj, "readJson", &VCallBinding::ReadJson);
	Nan::SetMethod(vcall_obj, "buffer2object", &VCallBinding::UpdateObjFromArrayBuffer);
	Nan::SetMethod(vcall_obj, "object2buffer", &VCallBinding::UpdateArrayBufferFromObject);
	Nan::SetMethod(vcall_obj, "createObjectFromBuffer", &VCallBinding::CreateObjFromArrayBuffer);

	Nan::SetMethod(vcall_obj, "openEXE", &VCallBinding::openEXE);
}

void PiBindings::BindToV8(const Local<FunctionTemplate>& v8_obj)
{
	Nan::SetMethod(v8_obj, "createObjectWithName", &VCallBinding::CreateObjectWithName);
	Nan::SetMethod(v8_obj, "getHiddenValue", &VCallBinding::GetHiddenValue);
	Nan::SetMethod(v8_obj, "setHiddenValue", &VCallBinding::SetHiddenValue);
	Nan::SetMethod(v8_obj, "getObjectHash", &VCallBinding::GetObjectHash);
	Nan::SetMethod(v8_obj, "setWeakCallback", &VCallBinding::SetDestructor);
	Nan::SetMethod(v8_obj, "takeHeapSnapshot", &VCallBinding::TakeHeapSnapshot);
	Nan::SetMethod(v8_obj, "linkWeakRef", &VCallBinding::LinkWeakRef);
	Nan::SetMethod(v8_obj, "getWeakRef", &VCallBinding::GetWeakRef);
	Nan::SetMethod(v8_obj, "createWeakRef", &VCallBinding::CreateWeakRef);
}

void PiBindings::BindToRes(const v8::Local<v8::FunctionTemplate>& res_obj)
{
	Nan::SetMethod(res_obj, "loadImg", &ResBinding::LoadImg);
	Nan::SetMethod(res_obj, "loadMesh", &ResBinding::LoadMesh);
	Nan::SetMethod(res_obj, "loadTerrainMesh", &ResBinding::LoadTerrainMesh);
	Nan::SetMethod(res_obj, "createSkeleton", &ResBinding::CreateSkeleton);
	Nan::SetMethod(res_obj, "createVertexAnim", &ResBinding::CreateVertexAnim);
	Nan::SetMethod(res_obj, "createUVAnim", &ResBinding::CreateUVAnim);
	Nan::SetMethod(res_obj, "asyncLoad", &ResBinding::AsyncLoad);
	Nan::SetMethod(res_obj, "loadAudio", &ResBinding::LoadAudio);
	Nan::SetMethod(res_obj, "loadCLUT", &ResBinding::LoadCLUT);
	Nan::SetMethod(res_obj, "loadPhysicsMesh", &ResBinding::LoadPhysicsMesh);
}

void PiBindings::BindToWindow(const v8::Local<v8::FunctionTemplate>& window_obj)
{
	Nan::SetMethod(window_obj, "addEventListener", &WindowBinding::AddEventListener);
	Nan::SetMethod(window_obj, "destroyWindow", &WindowBinding::destroyWindow);
	Nan::SetMethod(window_obj, "setWindowSize", &WindowBinding::setWindowSize);
	Nan::SetMethod(window_obj, "toggleFullScreen", &WindowBinding::toggleFullScreen);
	Nan::SetMethod(window_obj, "setWindowsShow", &WindowBinding::setWindowsShow);
	Nan::SetMethod(window_obj, "showMainWindow", &WindowBinding::showMainWindow);
	Nan::SetMethod(window_obj, "immEnable", &WindowBinding::immEnable);
	Nan::SetMethod(window_obj, "getCommandByKey", &WindowBinding::getCommandByKey);
	Nan::SetMethod(window_obj, "setWindowTitle", &WindowBinding::setWindowTitle);
	Nan::SetMethod(window_obj, "flashWindow", &WindowBinding::flashWindow);
	Nan::SetMethod(window_obj, "openUrl", &WindowBinding::openUrl);
	Nan::SetMethod(window_obj, "getMachineTime", &WindowBinding::getMachineTime);
}

void PiBindings::BindToBacktrace(const v8::Local<v8::FunctionTemplate>& backtrace_obj)
{
	Nan::SetMethod(backtrace_obj, "filter", &BacktraceBinding::Filter);
	Nan::SetMethod(backtrace_obj, "addPlugin", &BacktraceBinding::AddPlugin);
}

void PiBindings::BindToHeap(const v8::Local<v8::FunctionTemplate>& heap_obj)
{
	Nan::HandleScope scope;
	Local<Object> snapshots = Nan::New<Object>();

	Nan::SetMethod(heap_obj, "takeSnapshot", &HeapBinding::TakeSnapshot);
	Nan::SetMethod(heap_obj, "startTrackingHeapObjects", &HeapBinding::StartTrackingHeapObjects);
	Nan::SetMethod(heap_obj, "stopTrackingHeapObjects", &HeapBinding::StopTrackingHeapObjects);
	Nan::SetMethod(heap_obj, "getHeapStats", &HeapBinding::GetHeapStats);
	Nan::SetMethod(heap_obj, "getObjectByHeapObjectId", &HeapBinding::GetObjectByHeapObjectId);
	Nan::SetMethod(heap_obj, "getHeapObjectId", &HeapBinding::GetHeapObjectId);
	Nan::SetMethod(heap_obj, "getHeapSpaceStatistics", &HeapBinding::GetHeapSpaceStatistics);
	Nan::SetMethod(heap_obj, "getHeapStatistics", &HeapBinding::GetHeapStatistics);

 	nodex::Snapshot::snapshots.Reset(snapshots);
}

void PiBindings::BindToCpu(const v8::Local<v8::FunctionTemplate>& cpu_obj)
{
	Nan::HandleScope scope;
	Local<Array> profiles = Nan::New<Array>();

	Nan::SetMethod(cpu_obj, "startProfiling", &CpuBinding::StartProfiling);
	Nan::SetMethod(cpu_obj, "stopProfiling", &CpuBinding::StopProfiling);
	Nan::SetMethod(cpu_obj, "setSamplingInterval", &CpuBinding::SetSamplingInterval);
 	nodex::Profile::profiles.Reset(profiles);
}

void PiBindings::BindToAudio(const v8::Local<v8::FunctionTemplate>& audio_obj)
{

}
