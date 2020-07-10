#include "physics_wrap.h"
#include "PxRigidActor.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "PxMat44.h"
#include "PxSimpleFactory.h"
#include "pi_matrix4.h"
#include "physx_engine.h"
#include "PxMaterial.h"
using namespace physx;
using namespace nvidia;

extern "C" {

	extern PhysicsEngine *g_physics_engine;


	void PI_API physics_actor_get_mat(PiActor* actor, PiMatrix4* mat)
	{
		PxRigidActor* pxActor = static_cast<PxRigidActor*>(actor->impl);

		pi_mat4_copy(mat, (PiMatrix4*)PxMat44(pxActor->getGlobalPose()).getTranspose().front());
	}


	void PI_API physics_actor_apply_transform(PiActor* actor, TransformData* transform)
	{
		PxRigidActor* pxActor = static_cast<PxRigidActor*>(actor->impl);
		pxActor->setGlobalPose(PxTransform(PxVec3(transform->translate.x, transform->translate.y, transform->translate.z), PxQuat(transform->rotate.x, transform->rotate.y, transform->rotate.z, transform->rotate.w)));
	}

	void PI_API physics_actor_set_kinematic_target(PiActor* actor, TransformData* destination)
	{
		PxRigidDynamic* pxActor = static_cast<PxRigidDynamic*>(actor->impl);
		pxActor->setKinematicTarget(PxTransform(PxVec3(destination->translate.x, destination->translate.y, destination->translate.z), PxQuat(destination->rotate.x, destination->rotate.y, destination->rotate.z, destination->rotate.w)));
	}

	void PI_API physics_actor_get_transform(PiActor* actor, TransformData* transform)
	{
		PxRigidActor* pxActor = static_cast<PxRigidActor*>(actor->impl);
		PxTransform&& t = pxActor->getGlobalPose();
		pi_transform_set(transform, t.p.x, t.p.y, t.p.z, t.q.x, t.q.y, t.q.z, t.q.w);
	}

	void PI_API physics_actor_set_kinematic(PiActor* actor, PiBool is_kinematic)
	{
		PxRigidDynamic* pxActor = static_cast<PxRigidDynamic*>(actor->impl);
		pxActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, is_kinematic);
		if (!is_kinematic)
		{
			pxActor->wakeUp();
		}
	}


	void PI_API physics_actor_init(PiActor* actor)
	{
		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);
		PxGeometry* g = static_cast<PxGeometry*>(actor->geometry->impl);
		PxMaterial* material = static_cast<PxMaterial*>(actor->material->impl);
		if (actor->type == AT_DYNAMIC)
		{
			actor->impl = PxCreateDynamic(engine->getPhysics(), PxTransform(PxVec3(0.0, 2.5, 0.0)), *g, *material, actor->density, PxTransform(PxVec3(actor->shapeOffset.translate.x, actor->shapeOffset.translate.y, actor->shapeOffset.translate.z), PxQuat(actor->shapeOffset.rotate.x, actor->shapeOffset.rotate.y, actor->shapeOffset.rotate.z, actor->shapeOffset.rotate.w)));
		}
		else
		{
			actor->impl = PxCreateStatic(engine->getPhysics(), PxTransform(PxIdentity), *g, *material, PxTransform(PxVec3(actor->shapeOffset.translate.x, actor->shapeOffset.translate.y, actor->shapeOffset.translate.z), PxQuat(actor->shapeOffset.rotate.x, actor->shapeOffset.rotate.y, actor->shapeOffset.rotate.z, actor->shapeOffset.rotate.w)));
		}
	}

	void PI_API physics_material_create(PiPhyscisMaterial* material, float staticFriction, float dynamicFriction, float restitution)
	{
		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);
		PxMaterial* impl = engine->getPhysics().createMaterial(staticFriction, dynamicFriction, restitution);
		material->impl = impl;
	}

	void PI_API physics_material_free(PiPhyscisMaterial* material)
	{
		PxMaterial* impl = static_cast<PxMaterial*>(material->impl);
		impl->release();
	}
	void PI_API physics_actor_set_linear_velocity(PiActor* actor, float vx, float vy, float vz)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->setLinearVelocity(PxVec3(vx, vy, vz));
	}

	void PI_API physics_actor_set_force(PiActor* actor, float vx, float vy, float vz, int modeType)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->addForce(PxVec3(vx, vy, vz), (PxForceMode::Enum)modeType, true);
	}

	void PI_API physics_actor_wakeup(PiActor * actor)
	{
		PxRigidDynamic* impl = static_cast <PxRigidDynamic*>(actor->impl);
		impl->wakeUp();
	}

	void PI_API physics_actor_set_angular_velocity(PiActor* actor, PiVector3* v)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->setAngularVelocity(PxVec3(v->x, v->y, v->z));
	}

	void PI_API physics_actor_set_sleep(PiActor* actor)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->putToSleep();
	}

	PiBool PI_API physics_actor_is_sleep(PiActor* actor)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		return impl->isSleeping();
	}

	void PI_API physics_actor_set_linear_damp(PiActor* actor, float linearDamp)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->setLinearDamping(linearDamp);
	}

	void PI_API physics_actor_set_angular_damp(PiActor* actor, float damp)
	{
		PxRigidDynamic* impl = static_cast<PxRigidDynamic*>(actor->impl);
		impl->setAngularDamping(damp);
	}

	void PI_API physics_actor_release(PiActor* actor)
	{
		PxRigidActor* impl = static_cast<PxRigidActor*>(actor->impl);
		impl->release();
	}
}