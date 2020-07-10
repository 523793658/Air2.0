#include "V8Service.h"

#include "v8-debug.h"

#include "game_tools/WinCommandLine.h"
#include "game_tools/GameTools.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Settings/SettingsService.h"
#include "V8DebugServer.h"


using namespace v8;
using GameTools::WinCommandLine;

// 分配一块内存，用0值初始化其内容
void * V8ArrayBufferAllocator::Allocate(size_t length)
{
	void *data = AllocateUninitialized(length);
	return data == NULL ? data : memset(data, 0, length);
}

// 分配一块内存，但不初始化内容
void * V8ArrayBufferAllocator::AllocateUninitialized(size_t length)
{
	return malloc(length);
}

// 释放内存
void V8ArrayBufferAllocator::Free(void* data, size_t)
{
	free(data);
}

// 构造函数
V8Service::V8Service()
{
	
}

// 析构函数
V8Service::~V8Service()
{

}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult V8Service::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"Settings");

	return SyncResult::SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult V8Service::Initialize()
{
	int argc = WinCommandLine::GetInstance()->argc();
	char** argv = WinCommandLine::GetInstance()->argv();

	// 初始化v8
	const char expose_as[] = "--expose_gc";
	V8::SetFlagsFromCommandLine(&argc, argv, true);
	V8::SetFlagsFromString(expose_as, strlen(expose_as)); //

#ifdef _V8_DEBUG_
	const char* debug_flag = "--expose_debug_as=v8debug";
	V8::SetFlagsFromString(debug_flag, strlen(debug_flag));
#endif // _V8_DEBUG_

	std::string appPath = GameTools::GetAppDir();

#ifdef _DEBUG
	std::string nativesPath = appPath + std::string("\\natives_datad.bin");
	std::string snapshotPath = appPath + std::string("\\snapshot_datad.bin");
#else
	std::string nativesPath = appPath + std::string("\\natives_data.bin");
	std::string snapshotPath = appPath + std::string("\\snapshot_data.bin");
#endif

	V8::InitializeICU();
	V8::InitializeExternalStartupData(nativesPath.c_str(), snapshotPath.c_str());
	mpPlatform = platform::CreateDefaultPlatform();
	V8::InitializePlatform(mpPlatform);
	V8::Initialize();

	// 创建一个Isolate并让它起作用
	mpBufferAllocator = new V8ArrayBufferAllocator();
	Isolate::CreateParams tCreateParameter;
	tCreateParameter.array_buffer_allocator = mpBufferAllocator;
	mpIsolate = Isolate::New(tCreateParameter);
	mpIsolateScope = new Isolate::Scope(mpIsolate);

#ifdef _V8_DEBUG_
	V8DebugServer::GetInstance()->Initialize(mpIsolate);
#endif

	HandleScope tHandleScope(mpIsolate);

	// 先创建一个Context
	v8::Local<v8::Context> tContext = Context::New(mpIsolate);
	mV8Context.Reset(mpIsolate, tContext);


	return AsyncResult::AsyncResult_Complete;
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult V8Service::Finalize()
{
	delete mpIsolateScope;

	mpIsolate->Dispose();
	V8::Dispose();
	V8::ShutdownPlatform();
	delete mpPlatform;

#ifdef _V8_DEBUG_
	V8DebugServer::GetInstance()->Finalize();
#endif

	return AsyncResult::AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult V8Service::Update(const float vfElaspedTime)
{
#ifdef _V8_DEBUG_
	V8DebugServer::GetInstance()->Update();
	return AsyncResult::AsyncResult_Pending;
#else
	return AsyncResult::AsyncResult_Complete;
#endif // _V8_DEBUG_
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring V8Service::GetDisplayName()
{
	return L"V8";
}

Isolate * V8Service::GetIsolate()
{
	return mpIsolate;
}

// 获取Context
Local<Context> V8Service::GetContext()
{
	return PersistentToLocal(mpIsolate, mV8Context);
}

// 添加Binding库
void V8Service::AddFunctionBinding(const char *vstrName, v8::Local<v8::FunctionTemplate> vFunctionTemplate)
{
	v8::Local<v8::Object> tFunctionObject = vFunctionTemplate->GetFunction();
	GetContext()->Global()->Set(v8::String::NewFromUtf8(mpIsolate, vstrName, v8::NewStringType::kNormal).ToLocalChecked(), tFunctionObject);
}