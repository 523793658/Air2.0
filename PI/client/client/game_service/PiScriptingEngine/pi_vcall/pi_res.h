#ifndef SCRIPT_ENGINE_PI_RES_H_
#define SCRIPT_ENGINE_PI_RES_H_

#include "v8.h"
#include "nan/nan.h"


class ResBinding 
{
public:
	static NAN_METHOD(LoadImg);
	static NAN_METHOD(LoadMesh);
	static NAN_METHOD(LoadTerrainMesh);
	static NAN_METHOD(CreateSkeleton);
	static NAN_METHOD(CreateVertexAnim);
	static NAN_METHOD(CreateUVAnim);

	static NAN_METHOD(AsyncLoad);

	static NAN_METHOD(LoadAudio);

	static NAN_METHOD(LoadCLUT);

	static NAN_METHOD(LoadPhysicsMesh);
};


#endif