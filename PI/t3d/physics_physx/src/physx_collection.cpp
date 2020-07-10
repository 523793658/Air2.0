#include "PxDefaultStreams.h"

#include "physics_wrap.h"
#include "physx_engine.h"
#include "PxTypeInfo.h"
#include "PxD6Joint.h"
#include "PxRigidDynamic.h"
//#include "profileapi.h"
using namespace physx;



extern "C" {

	extern PhysicsEngine *g_physics_engine;


	void PI_API physics_collection_load(PiCollection* collection, uint8_t* data, uint size)
	{
		pi_time_elapse();

 		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);
		PxDefaultMemoryInputData inputData(data, size);
		PxCollection* impl = PxSerialization::createCollectionFromXml(inputData, engine->getCooking(), engine->getSerializationRegistry());
		collection->impl = impl;
		uint32 objNb = impl->getNbObjects();
		for (uint32 i = 0; i < objNb; i++)
		{
			PxBase* object = &impl->getObject(i);
			switch (object->getConcreteType())
			{
			case PxConcreteType::eRIGID_DYNAMIC:
			{
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(object);
				PxTransform t = PxTransform(PxQuat(-PxPi * 1.0f / 2.0f, PxVec3(1.0, 0.0, 0.0))) * actor->getGlobalPose();
				actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
				actor->setGlobalPose(t);
				PiActor* ac = pi_new0(PiActor, 1);
				ac->id = impl->getId(*object);
				ac->impl = actor;
				pi_vector_push(collection->actors, ac);
			}
			break;
			case PxJointConcreteType::eD6:
			{
// 				PxD6Joint* joint = static_cast<PxD6Joint*>(object);
// 				PxJointLimitCone c = joint->getSwingLimit();
// 				joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
// 				joint->setProjectionAngularTolerance(0.1);
				//joint->setProjectionLinearTolerance(0.1);
				/*joint->setSwingLimit(PxJointLimitCone(c.yAngle / 2.0f, c.zAngle / 2.0f, c.contactDistance));*/
			}
			break;
			default:
				break;
			}
		}
	}


	//只释放collection本身，collection内部包含的obj不会被释放
	void PI_API physics_collection_free(PiCollection* collection)
	{
		PxCollection* impl = static_cast<PxCollection*>(collection->impl);
		impl->release();
	}

	void PI_API physics_collection_clear(PiCollection* collection)
	{
		PxCollection* impl = static_cast<PxCollection*>(collection->impl);
		uint32 objNb = impl->getNbObjects();
		for (uint32 i = 0; i < objNb; i++)
		{
			PxBase& obj = impl->getObject(i);
			obj.release();
		}
	}

}