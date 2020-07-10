#include "remote_object_freer.h"
#include "game_service/PiScriptingEngine/backtrace.h"


// static
void RemoteObjectFreer::BindTo(
	v8::Isolate* isolate, v8::Local<v8::Object> target, 
	v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > callback)
{
	new RemoteObjectFreer(isolate, target, callback);
}

void RemoteObjectFreer::Release()
{
	delete this;
}

RemoteObjectFreer::RemoteObjectFreer(
	v8::Isolate* isolate, v8::Local<v8::Object> target,
	v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > callback)
	: ObjectLifeMonitor(isolate, target),
	isolate_(isolate),
	callback_(callback)
{
		
}

RemoteObjectFreer::~RemoteObjectFreer()
{
}

void RemoteObjectFreer::RunDestructor()
{
	v8::HandleScope handle_scope(isolate_);
	v8::Local<v8::Context> context = isolate_->GetCurrentContext();
	v8::Context::Scope scope(context);
	v8::Local<v8::Function> local = v8::Local<v8::Function>::New(isolate_, callback_);
	v8::TryCatch try_catch(isolate_);
	v8::Local<v8::Value> result;
	if (!local->Call(context, context->Global(), 0, nullptr).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate_, &try_catch);
	}
}
