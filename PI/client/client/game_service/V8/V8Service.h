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

// ��V8�õ��ڴ������
class V8ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:

	// ����һ���ڴ棬��0ֵ��ʼ��������
	virtual void * Allocate(size_t length);

	// ����һ���ڴ棬������ʼ������
	virtual void* AllocateUninitialized(size_t length);

	// �ͷ��ڴ�
	virtual void Free(void* data, size_t);
};

class V8Service : public PiGameService
{
private:

	// �ܲ������ﱣ��һ��Isolate��������Ŀһ��Isolate��������
	V8ArrayBufferAllocator *mpBufferAllocator;
	v8::Isolate *mpIsolate;
	v8::Platform *mpPlatform;

	v8::Isolate::Scope *mpIsolateScope;
	v8::Persistent<v8::Context> mV8Context;

public:

	// ���캯��
	V8Service();

	// ��������
	virtual ~V8Service();

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize();

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize();

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize();

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime);

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName();

	// ============================== ����ʹ�õĺ��� ==============================
	// ��ȡIsolate
	v8::Isolate * GetIsolate();

	// ��ȡContext
	v8::Local<v8::Context> GetContext();

	// ���Binding��
	void AddFunctionBinding(const char *vstrName, v8::Local<v8::FunctionTemplate> vFunctionTemplate);
};

#endif