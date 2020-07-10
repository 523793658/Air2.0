#ifndef _PARSE_MESH_TASK_H_
#define _PARSE_MESH_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseMeshTask : public pi::Task
{
public:
	ParseMeshTask();
	virtual ~ParseMeshTask();

	void SetParam(byte* data, uint size, PiBool createCollision, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;
	PiBool createCollision_;

	meshResult* result_;
};

#endif