#ifndef _ParsePhysicsMeshTask_H_
#define _ParsePhysicsMeshTask_H_


#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"
#include <physics_mesh.h>


class ParsePhysicsMeshTask : public pi::Task
{
public:
	ParsePhysicsMeshTask(std::vector<PiMesh*> meshes, const CallBackType& cb);
	virtual ~ParsePhysicsMeshTask();
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;
	std::vector<PiMesh*> mMeshes;
	std::vector<PhysicsTriangleMesh*> mPhysicsMeshes;
};



#endif