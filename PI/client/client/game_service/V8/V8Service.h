#ifndef _V8Service_H_
#define _V8Service_H_

#include "v8.h"
#include "libplatform/libplatform.h"

#include "game_service/BaseClass/PiGameService.h"

template <class TypeName>
inline v8::Local<TypeName> PersistentToLocal(
	v8::Isolate* isolate,
	const v8::Persistent<TypeName>& persistent) {
	if (persistent.IsWeak()) {
		return WeakPersistentToLocal(isolate, persistent);
	}
	else {
		return StrongPersistentToLocal(persistent);
	}
}

template <class TypeName>
inline v8::Local<TypeName> StrongPersistentToLocal(
	const v8::Persistent<TypeName>& persistent) {
	return *reinterpret_cast<v8::Local<TypeName>*>(
		const_cast<v8::Persistent<TypeName>*>(&persistent));
}

template <class TypeName>
inline v8::Local<TypeName> WeakPersistentToLocal(
	v8::Isolate* isolate,
	const v8::Persistent<TypeName>& persistent) {
	return v8::Local<TypeName>::New(isolate, persistent);
}

// 给V8用的内存分配类
class V8ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:

	// 分配一块内存，用0值初始化其内容
	virtual void * Allocate(size_t length);

	// 分配一块内存，但不初始化内容
	virtual void* AllocateUninitialized(size_t length);

	// 释放内存
	virtual void Free(void* data, size_t);
};

class V8Service : public PiGameService
{
private:

	// 能不能这里保留一个Isolate？整个项目一个Isolate够不够？
	V8ArrayBufferAllocator *mpBufferAllocator;
	v8::Isolate *mpIsolate;
	v8::Platform *mpPlatform;

	v8::Isolate::Scope *mpIsolateScope;
	v8::Persistent<v8::Context> mV8Context;

public:

	// 构造函数
	V8Service();

	// 析构函数
	virtual ~V8Service();

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize();

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize();

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize();

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime);

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName();

	// ============================== 对外使用的函数 ==============================
	// 获取Isolate
	v8::Isolate * GetIsolate();

	// 获取Context
	v8::Local<v8::Context> GetContext();

	// 添加Binding库
	void AddFunctionBinding(const char *vstrName, v8::Local<v8::FunctionTemplate> vFunctionTemplate);
};

#endif