#include "physics_wrap.h"
#include "physx_engine.h"
#include "PxAggregate.h"


extern "C" {

	extern PhysicsEngine *g_physics_engine;


	void PI_API physics_aggregate_init(PiAggregate* agg)
	{
		PhysXEngine* engine = static_cast<PhysXEngine*>(g_physics_engine->impl);
 		agg->impl = engine->getPhysics().createAggregate(agg->max_actors, agg->self_collision);
		
	}

	void PI_API physics_aggregate_add(PiAggregate* agg, PiActor* actor)
	{
		PxAggregate* impl = static_cast<PxAggregate*>(agg->impl);
		PxActor* ac = static_cast<PxActor*>(actor->impl);
		impl->addActor(*ac);
	}

	void PI_API physics_aggregate_release(PiAggregate* agg)
	{
		PxAggregate* impl = static_cast<PxAggregate*>(agg->impl);
		impl->release();
	}

}
