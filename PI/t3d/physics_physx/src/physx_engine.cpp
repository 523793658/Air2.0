#include "physx_engine.h"
#include "physics_wrap.h"
#include "PxFoundationVersion.h"
#include "PhysXSDKVersion.h"
#include "PxPvdTransport.h"
#include "PxExtensionsAPI.h"
#include "physx_resource_callback.h"
#include "physx_render_resource_manager.h"
#include "physx_scene.h"
extern "C" {

	extern PhysicsEngine *g_physics_engine;


	void PI_API physics_engine_init(PhysicsEngine* engine, InitialType type, char* url)
	{
		PhysXEngine* impl = new PhysXEngine();
		impl->initialise(type, url);
		engine->impl = impl;
	}

	void PI_API physics_engine_simulate(PhysicsEngine* engine, float tpf)
	{
		PhysXEngine* impl = static_cast<PhysXEngine*>(engine->impl);
		impl->simulate(tpf);
	}

	void PI_API physics_engine_release(PhysicsEngine* engine)
	{
		PhysXEngine* impl = static_cast<PhysXEngine*>(engine->impl);
		delete impl;
	}

}

PhysXEngine* PhysXEngine::getInstance()
{
	PhysXEngine* impl = static_cast<PhysXEngine*>(g_physics_engine->impl);
	return impl;
}

PhysXEngine::PhysXEngine() : mFoundation(nullptr), mPvd(nullptr), mPhysics(nullptr), mPvdUrl("127.0.0.1")
{

}

PhysXEngine::~PhysXEngine()
{
	if (mInitialType == PT_DEBUG)
	{
		mApexRenderDebug->release();
		mPvd->disconnect();
		mPvd->release();
	}
	mModuleLegacy->release();
	mModuleDestructible->release();
	mApexSDK->release();
	delete mResourceCallback;
	delete mRenderResourceManager;
	PxDefaultCpuDispatcher* dis = static_cast<PxDefaultCpuDispatcher*>(mDispatcher);
	dis->release();
	mCooking->release();
	mSerializationRegistry->release();
	PxCloseExtensions();
	mPhysics->release();
	mFoundation->release();

}

void PhysXEngine::attachDebugScene(PhysXScene* scene)
{
	mPvdSceneClient = scene->mPxScene->getScenePvdClient();
	mPvdSceneClient->setScenePvdFlags(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | PxPvdSceneFlag::eTRANSMIT_CONTACTS);

}

void PhysXEngine::detachDebugScene()
{

}

void PhysXEngine::initPhysX()
{
	mFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, mAllocator, mErrorCallback);
	if (mInitialType == PT_DEBUG)
	{
		mPvd = PxCreatePvd(*mFoundation);
	}
	PxTolerancesScale scale;
	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, true, mPvd);
	PxInitExtensions(*mPhysics, mPvd);
	mSerializationRegistry = PxSerialization::createSerializationRegistry(*mPhysics);
	PxCookingParams cookingParams(scale);
	cookingParams.buildGPUData = false;
	mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, cookingParams);
	mDispatcher = PxDefaultCpuDispatcherCreate(2);


	if (mInitialType == PT_DEBUG)
	{
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(mPvdUrl.c_str(), 5425, 10);
		mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	}
}

void PhysXEngine::initApex()
{
	ApexSDKDesc apexDesc;
	apexDesc.physXSDK = mPhysics;
	apexDesc.cooking = mCooking;
	apexDesc.pvd = mPvd;
	mRenderResourceManager = new PiRenderResourceManager();
	apexDesc.renderResourceManager = mRenderResourceManager;
	mResourceCallback = new PiResourceCallback();
	apexDesc.resourceCallback = mResourceCallback;
	apexDesc.foundation = mFoundation;

	ApexCreateError error;
	mApexSDK = CreateApexSDK(apexDesc, &error);
	mResourceCallback->setApexSDK(mApexSDK);
	mModuleDestructible = static_cast<ModuleDestructible*>(mApexSDK->createModule("Destructible"));
	NvParameterized::Interface* params = mModuleDestructible->getDefaultModuleDesc();
	mModuleDestructible->init(*params);
	mModuleLegacy = mApexSDK->createModule("Legacy");
	if (mInitialType == PT_DEBUG)
	{
		mApexRenderDebug = mApexSDK->createApexRenderDebug(mRenderDebugInterface);
	}

}

void PhysXEngine::initialise(InitialType type, char* url)
{
#if defined(_DEBUG) || defined(DEBUG)
	mInitialType = type;
	if (url)
	{
		mPvdUrl = std::string(url);
	}
#endif

	initPhysX();
	initApex();
}

void PhysXEngine::simulate(float tpf)
{

}


PxCooking&	PhysXEngine::getCooking()
{
	return *mCooking;
}
PxSerializationRegistry& PhysXEngine::getSerializationRegistry()
{
	return *mSerializationRegistry;
}
PxPhysics&	PhysXEngine::getPhysics()
{
	return *mPhysics;
}


PxCpuDispatcher& PhysXEngine::getCpuDispatcher()
{
	return *mDispatcher;
}

ApexSDK& PhysXEngine::getApex()
{
	return *mApexSDK;
}

InitialType PhysXEngine::getInitialType()
{
	return mInitialType;
}

RenderDebugInterface* PhysXEngine::getRenderDebugInterface()
{
	return mApexRenderDebug;
}



