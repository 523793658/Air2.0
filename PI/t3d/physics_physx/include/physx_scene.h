#ifndef _PhysX_Scene_H_
#define _PhysX_Scene_H_
#include "PxScene.h"
#include "Scene.h"


using namespace nvidia;
struct PhysXScene
{
	PxScene* mPxScene;
	Scene* mApexScene;
};





#endif