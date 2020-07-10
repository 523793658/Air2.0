
#include "physics_wrap.h"
#include "physx_scene.h"
#include "physx_engine.h"



extern "C"
{

	extern PhysicsEngine *g_physics_engine;

	void PI_API physics_scene_create(PiPhysicsScene* scene, PiVector3* gravity, void* filter)
	{
		PxSimulationFilterShader filterShader;
		if (filter == NULL)
		{
			filterShader =static_cast<PxSimulationFilterShader>(physics_get_defualt_filter_shader());
		}
		else
		{
			filterShader = static_cast<PxSimulationFilterShader>(filter);
		}
		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);
		PhysXScene* physXScene = pi_new0(PhysXScene, 1);
		PxSceneDesc pxSceneDesc(engine->getPhysics().getTolerancesScale());
		pxSceneDesc.gravity = PxVec3(gravity->x, gravity->y, gravity->z);
		pxSceneDesc.cpuDispatcher = &engine->getCpuDispatcher();
		pxSceneDesc.filterShader = filterShader;
		pxSceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION | PxSceneFlag::eENABLE_STABILIZATION;
		physXScene->mPxScene = engine->getPhysics().createScene(pxSceneDesc);

		SceneDesc sceneDesc;
		sceneDesc.scene = physXScene->mPxScene;
		physXScene->mApexScene = engine->getApex().createScene(sceneDesc);
		physXScene->mApexScene->allocViewMatrix(ViewMatrixType::LOOK_AT_RH);
		physXScene->mApexScene->allocProjMatrix(ProjMatrixType::USER_CUSTOMIZED);
		scene->impl = physXScene;


		if (engine->getInitialType() == PT_DEBUG)
		{
			PxPvdSceneClient *pvdScene = physXScene->mPxScene->getScenePvdClient();
			pvdScene->setScenePvdFlags(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | PxPvdSceneFlag::eTRANSMIT_CONTACTS);
		}
	}

	void PI_API physics_scene_free(PiPhysicsScene* scene)
	{
		PhysXScene* physXScene = static_cast<PhysXScene*>(scene->impl);
		physXScene->mApexScene->release();
		physXScene->mPxScene->release();
		pi_free(physXScene);
	}

	void PI_API physics_scene_add_actor(PiPhysicsScene* scene, PiActor* actor)
	{
		PhysXScene* pxScene = static_cast<PhysXScene*>(scene->impl);
		PxActor* pxActor = static_cast<PxActor*>(actor->impl);
		pxScene->mPxScene->addActor(*pxActor);
	}

	void PI_API physics_scene_remove_actor(PiPhysicsScene* scene, PiActor* actor)
	{
		PhysXScene* pxScene = static_cast<PhysXScene*>(scene->impl);
		PxActor* pxActor = static_cast<PxActor*>(actor->impl);
		pxScene->mPxScene->removeActor(*pxActor);
	}

	void PI_API physics_scene_add_aggregate(PiPhysicsScene* scene, PiAggregate* agg)
	{
		PhysXScene* pxScene = static_cast<PhysXScene*>(scene->impl);
		PxAggregate* pxAgg = static_cast<PxAggregate*>(agg->impl);
		pxScene->mPxScene->addAggregate(*pxAgg);
	}

	void PI_API physics_scene_remove_aggregate(PiPhysicsScene* scene, PiAggregate* agg)
	{
		PhysXScene* sc = static_cast<PhysXScene*>(scene->impl);
		PxAggregate* pxAgg = static_cast<PxAggregate*>(agg->impl);
		sc->mPxScene->removeAggregate(*pxAgg);
	}

	void PI_API physics_scene_simulate(PiPhysicsScene* scene, float tpf)
	{
		PhysXScene* physXScene = static_cast<PhysXScene*>(scene->impl);

		physXScene->mApexScene->simulate(tpf);
	}

	void PI_API physics_scene_fetch_results(PiPhysicsScene* scene, PiBool block, uint32* error_state)
	{
		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);

		PhysXScene* physXScene = static_cast<PhysXScene*>(scene->impl);

		physXScene->mApexScene->fetchResults(block, error_state);

		if (engine->getInitialType() == PT_DEBUG)
		{
			const physx::PxRenderBuffer& renderBuffer = physXScene->mPxScene->getRenderBuffer();
			engine->getRenderDebugInterface()->addDebugRenderable(renderBuffer);
		}
	}


}
