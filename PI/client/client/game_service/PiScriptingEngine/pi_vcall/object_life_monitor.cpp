// Copyright (c) 2013 GitHub, Inc.
// Copyright (c) 2012 Intel Corp. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "object_life_monitor.h"
#include "game_tools/GameTools.h"
#include "game_service/PiScriptingEngine/tasks/ResCBTask.h"

ObjectLifeMonitor::ObjectLifeMonitor(v8::Isolate* isolate,
									 v8::Local<v8::Object> target)
									 : context_(isolate, isolate->GetCurrentContext()),
									 target_(isolate, target)
{
	target_.SetWeak(this, OnObjectGC, v8::WeakCallbackType::kParameter);
}

ObjectLifeMonitor::~ObjectLifeMonitor()
{
	if (target_.IsEmpty())
		return;
	target_.ClearWeak();
	target_.Reset();
}

// static
void ObjectLifeMonitor::OnObjectGC(
	const v8::WeakCallbackInfo<ObjectLifeMonitor>& data)
{
	ObjectLifeMonitor* self = data.GetParameter();
	self->RunDestructor();
	self->target_.Reset();
	data.SetSecondPassCallback(Free);
	//delete self;
	ResCBTask* task = new ResCBTask();
	task->SetResPoint(self);
	GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

// static
void ObjectLifeMonitor::Free(
	const v8::WeakCallbackInfo<ObjectLifeMonitor>& data)
{
	//delete data.GetParameter();
}

