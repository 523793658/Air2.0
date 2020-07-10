#ifndef ATOM_COMMON_API_REMOTE_OBJECT_FREER_H_
#define ATOM_COMMON_API_REMOTE_OBJECT_FREER_H_

#include "object_life_monitor.h"



class RemoteObjectFreer : public ObjectLifeMonitor
{
public:
	static void BindTo(
		v8::Isolate* isolate, v8::Local<v8::Object> target, 
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > callback);
	void Release();

protected:
	RemoteObjectFreer(
		v8::Isolate* isolate, v8::Local<v8::Object> target,
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > callback);
	~RemoteObjectFreer() override;

	void RunDestructor() override;

private:
	v8::Isolate* isolate_;
	v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > callback_;
	//DISALLOW_COPY_AND_ASSIGN(RemoteObjectFreer);
};


#endif  // ATOM_COMMON_API_REMOTE_OBJECT_FREER_H_