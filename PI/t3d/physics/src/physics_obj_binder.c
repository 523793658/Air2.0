#include "physics_obj_binder.h"


PiObjectBinder* PI_API pi_physics_obj_binder_create(PiSpatial* spatial, PiActor* actor)
{
	PiObjectBinder* binder = pi_new0(PiObjectBinder, 1);
	binder->actor = actor;
	binder->spatial = spatial;
	return binder;
}

void PI_API pi_physics_obj_binder_synch(PiObjectBinder* binder)
{
	pi_vec3_copy(&binder->actor->transform.translate, pi_spatial_get_world_translation(binder->spatial));
	pi_quat_copy(&binder->actor->transform.rotate, pi_spatial_get_world_rotation(binder->spatial));
	pi_physics_actor_set_transform(binder->actor, &binder->actor->transform);
}

void PI_API pi_physics_obj_binder_update(PiObjectBinder* binder)
{
	TransformData* transform = &binder->actor->transform;
	pi_physics_actor_get_world_transform(binder->actor, transform);
	pi_spatial_set_local_translation(binder->spatial, transform->translate.x, transform->translate.y, transform->translate.z);
	pi_spatial_set_local_rotation(binder->spatial, transform->rotate.w, transform->rotate.x, transform->rotate.y, transform->rotate.z);
	pi_spatial_update(binder->spatial);
}

void PI_API pi_physics_obj_binder_free(PiObjectBinder* binder)
{
	pi_free(binder);
}