// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_COMMON_API_OBJECT_LIFE_MONITOR_H_
#define ATOM_COMMON_API_OBJECT_LIFE_MONITOR_H_

#include "v8.h"
#include "game_service/PiScriptingEngine/pi_util.h"
#include "base/weak_ptr_factory.hpp"

class ObjectLifeMonitor
{
protected:
	ObjectLifeMonitor(v8::Isolate* isolate, v8::Local<v8::Object> target);
	virtual ~ObjectLifeMonitor();

	virtual void RunDestructor() = 0;

private:
	static void OnObjectGC(const v8::WeakCallbackInfo<ObjectLifeMonitor>& data);
	static void Free(const v8::WeakCallbackInfo<ObjectLifeMonitor>& data);

	v8::Global<v8::Context> context_;
	v8::Global<v8::Object> target_;
};

#endif  // ATOM_COMMON_API_OBJECT_LIFE_MONITOR_H_
