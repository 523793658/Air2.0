#include "physics_manager.h"
static void _update_binder(PiPhysicsManager* manager)
{
	uint i, size = pi_vector_size(manager->obj_binder_list_dynamic);
	PiObjectBinder* binder;
	PiBool sleeping;
	for (i = 0; i < size; i++)
	{
		binder = pi_vector_get(manager->obj_binder_list_dynamic, i);
		sleeping = pi_physics_actor_is_sleep(binder->actor);
		if (sleeping && binder->sleeping)
		{
			binder->sleeping = sleeping;
			continue;
		}
		pi_physics_obj_binder_update(binder);
		binder->sleeping = sleeping;
	}
}

static void _update_impl(PiPhysicsManager* manager, float dt)
{
	uint i, size;
	PiPhysicsScene* scene;
	PiObjectBinder* binder;
	uint32 error_state;
	PiBool sleeping;
	size = pi_vector_size(manager->scene_list);
	for (i = 0; i < size; i++)
	{
		scene = pi_vector_get(manager->scene_list, i);
		pi_physics_scene_simulate(scene, dt);
		pi_physics_scene_fetch_results(scene, TRUE, &error_state);
	}
	_update_binder(manager);
}

static void _update_impl2(PiPhysicsManager* manager, float dt)
{
	uint i, size;
	PiPhysicsScene* scene;
	uint32 error_state;
	size = pi_vector_size(manager->scene_list);

	for (i = 0; i < size; i++)
	{
		scene = pi_vector_get(manager->scene_list, i);
		pi_physics_scene_simulate(scene, dt);
		pi_physics_scene_fetch_results(scene, TRUE, &error_state);
	}
	
}





PiPhysicsManager* PI_API pi_physics_manager_create(PhysicsManagerUpdateStrategy strategy)
{
	PiPhysicsManager* manager = pi_new0(PiPhysicsManager, 1);
	manager->obj_binder_list_dynamic = pi_vector_new();
	manager->obj_binder_list_static = pi_vector_new();
	manager->scene_list = pi_vector_new();
	manager->strategy = strategy;
	manager->fixed_timestep = 0.02f;
	return manager;
}

void PI_API pi_physics_manager_add_scene(PiPhysicsManager* manager, PiPhysicsScene* scene)
{
	pi_vector_push(manager->scene_list, scene);
}

void PI_API pi_physics_manager_add_obj_binder(PiPhysicsManager* manager, PiObjectBinder* binder)
{
	if (binder->actor->type == AT_STATIC)
	{
		pi_vector_push(manager->obj_binder_list_static, binder);
	}
	else
	{
		pi_vector_push(manager->obj_binder_list_dynamic, binder);
	}
}

void PI_API pi_physics_manager_remove_obj_binder(PiPhysicsManager* manager, PiObjectBinder* binder)
{
	uint i, size;
	if (binder->actor->type == AT_STATIC)
	{
		size = pi_vector_size(manager->obj_binder_list_static);
		for (i = 0; i < size; i++)
		{
			PiObjectBinder* b = pi_vector_get(manager->obj_binder_list_static, i);
			if (b == binder)
			{
				pi_vector_remove_unorder(manager->obj_binder_list_static, i);
				return;
			}
		}
	}
	else
	{
		size = pi_vector_size(manager->obj_binder_list_dynamic);
		for (i = 0; i < size; i++)
		{
			PiObjectBinder* b = pi_vector_get(manager->obj_binder_list_dynamic, i);
			if (b == binder)
			{
				pi_vector_remove_unorder(manager->obj_binder_list_dynamic, i);
				return;
			}
		}
	}
}

void PI_API pi_physics_manager_update(PiPhysicsManager* manager, float tpf)
{
	float dt;
	switch (manager->strategy)
	{
	case PMUS_CLAMP:
	{
		dt = pi_clamp_float(tpf, 0.001, 0.33);
		_update_impl(manager, dt);
	}
		break;
	case PMUS_FIXTIME:
	{
		manager->time_remainder += tpf;
		if (manager->time_remainder <= 0.0f)
		{
			return;
		}
		manager->time_remainder -= manager->fixed_timestep;
		if (manager->time_remainder > 0.0f)
		{
			manager->time_remainder = 0.0f;
		}
		dt = manager->fixed_timestep;
		_update_impl(manager, dt);
		break;
	}
	case PMUS_TIME_DIVISION:
	{

		manager->time_remainder += tpf;
		while (manager->time_remainder > manager->fixed_timestep)
		{
			_update_impl2(manager, manager->fixed_timestep);
			manager->time_remainder -= manager->fixed_timestep;

		}
		if (manager->time_remainder > 0)
		{
			_update_impl2(manager, manager->time_remainder);
			manager->time_remainder = 0;
		}
		_update_binder(manager);
		break;
	}
	default:
		break;
	}


}


void PI_API pi_physics_manager_free(PiPhysicsManager* manager)
{
	pi_vector_free(manager->scene_list);
	pi_vector_free(manager->obj_binder_list_static);
	pi_vector_free(manager->obj_binder_list_dynamic);
	pi_free(manager);
}

void PI_API pi_physics_manager_remove_scene(PiPhysicsManager* manager, PiPhysicsScene* scene)
{
	uint size, i;
	size = pi_vector_size(manager->scene_list);
	for (i = 0; i < size; i++)
	{
		PiPhysicsScene* s = pi_vector_get(manager->scene_list, i);
		if (s == scene)
		{
			pi_vector_remove_unorder(manager->scene_list, i);
			return;
		}
	}
}