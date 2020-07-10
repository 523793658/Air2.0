#ifndef _PHYSX_ENGINE_H
#define _PHYSX_ENGINE_H
#include "physics_engine.h"
#include "PxPhysics.h"
#include "PxFiltering.h"
#include "PxFoundation.h"
#include "PxSerialization.h"
#include "PxDefaultAllocator.h"
#include "PxDefaultErrorCallback.h"
#include "PxPvd.h"
#include "Apex.h"
#include "ModuleDestructible.h"
#include "PxPvdSceneClient.h"

#include "physx_render_resource_manager.h"
#include "physx_resource_callback.h"
using namespace nvidia;
using namespace nvidia::apex;
using namespace physx;

struct PhysXScene;




class PhysXEngine
{
public:
	PhysXEngine();
	~PhysXEngine();

	static PhysXEngine* getInstance();

	void initialise(InitialType type, char* url);

	void simulate(float tpf);

	void attachDebugScene(PhysXScene* scene);
	void detachDebugScene();

	InitialType getInitialType();
private:
	void initPhysX();
	void initApex();

public:
	PxCooking&					getCooking();
	PxSerializationRegistry&	getSerializationRegistry();
	PxPhysics&					getPhysics();
	PxCpuDispatcher&			getCpuDispatcher();
	ApexSDK&					getApex();
	RenderDebugInterface*		getRenderDebugInterface();

private:
	PxDefaultErrorCallback		mErrorCallback;
	PxDefaultAllocator			mAllocator;
	PxFoundation*				mFoundation{ NULL };
	PxPvd*						mPvd{ NULL };
	PxPhysics*					mPhysics{ NULL };
	PxSerializationRegistry*	mSerializationRegistry{ NULL };
	PxCooking*					mCooking{ NULL };
	PxCpuDispatcher*			mDispatcher{ NULL };
	InitialType					mInitialType{ PT_RELEASE };
	std::string					mPvdUrl;
	ApexSDK*					mApexSDK{ NULL };
	PiRenderResourceManager*	mRenderResourceManager{ NULL };
	PiResourceCallback*			mResourceCallback{ NULL };
	ModuleDestructible*			mModuleDestructible{ NULL };
	Module*						mModuleLegacy{ NULL };
	RenderDebugInterface*		mApexRenderDebug{ NULL };
	RENDER_DEBUG::RenderDebugInterface* mRenderDebugInterface{ NULL };
	PxPvdSceneClient*			mPvdSceneClient{ NULL };

};
#endif
