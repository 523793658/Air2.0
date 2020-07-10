#ifndef _PARSE_TERRAIN_MESH_TASK_H_
#define _PARSE_TERRAIN_MESH_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseTerrainMeshTask : public pi::Task
{
public:
	ParseTerrainMeshTask();
	virtual ~ParseTerrainMeshTask();

	void SetParam(byte* data, uint size, int vw, int vh, int gs, PiBool isEditor, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;
	int vw_;
	int vh_;
	int gs_;
	PiBool isEditor_;

	TerrainResult* result_;
};

#endif