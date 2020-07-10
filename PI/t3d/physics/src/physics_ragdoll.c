#include "physics_ragdoll.h"
#include "physics_wrap.h"
#include "skanim.h"




PiCollection* PI_API pi_physics_collection_load(void* data, uint size)
{
	PiCollection* collection = pi_new0(PiCollection, 1);
	collection->actors = pi_vector_new();
	physics_collection_load(collection, data, size);
	return collection;
}


void PI_API pi_physics_collection_clear(PiCollection* collection)
{
	uint i, size;
	size = pi_vector_size(collection->actors);
	for (i = 0; i < size; i++)
	{
		pi_free(pi_vector_get(collection->actors, i));
	}
	physics_collection_clear(collection);
}


void PI_API pi_physics_collection_free(PiCollection* collection)
{
	physics_collection_free(collection);
	pi_vector_free(collection->actors);
	pi_free(collection);
}

PiAggregate* PI_API pi_physics_aggregate_new(uint max_actors, PiBool self_collision)
{
	PiAggregate* agg = pi_new0(PiAggregate, 1);
	
	agg->max_actors = max_actors;
	agg->self_collision = self_collision;
	agg->actors = pi_new0(PiActor*, max_actors);
	physics_aggregate_init(agg);
	return agg;
}

void PI_API pi_physics_aggregate_add_actor(PiAggregate* agg, PiActor* actor)
{
	agg->actors[agg->num_actors] = actor;
	physics_aggregate_add(agg, actor);
	agg->num_actors++;
}

void PI_API pi_physics_aggregate_free(PiAggregate* agg)
{
	physics_aggregate_release(agg);
	pi_free(agg);
}

uint32 PI_API pi_physics_aggregate_num_actors(PiAggregate* agg)
{
	return agg->num_actors;
}

PiActor* PI_API pi_physics_aggregate_get_actors(PiAggregate* agg, uint32 index)
{
	return agg->actors[index];
}

Ragdoll* PI_API pi_physics_ragdoll_new()
{
	Ragdoll* ragdoll = pi_new0(Ragdoll, 1);
	ragdoll->joins = pi_vector_new();
	ragdoll->spaital = pi_spatial_node_create();
	return ragdoll;
}

void PI_API pi_physics_ragdoll_free(Ragdoll* ragdoll)
{
	uint i, size;
	if (ragdoll->name)
	{
		pi_free(ragdoll->name);
	}
	pi_spatial_destroy(ragdoll->spaital);
	pi_free(ragdoll->bone_actors);
	pi_physics_aggregate_free(ragdoll->aggregate);
	pi_physics_collection_clear(ragdoll->collection);
	pi_physics_collection_free(ragdoll->collection);
	pi_vector_free(ragdoll->joins);
	pi_free(ragdoll);
}

Ragdoll* PI_API pi_physics_ragdoll_load(RagdollData* data)
{
	Ragdoll* ragdoll = pi_physics_ragdoll_new();

	PiCollection* collection = pi_physics_collection_load(data->base_data, data->data_size);
	uint i, num, j;
	PiActor* actor;
	ragdoll->collection = collection;
	ragdoll->aggregate = pi_physics_aggregate_new(pi_vector_size(collection->actors), FALSE);
	num = pi_vector_size(collection->actors);

	for (i = 0; i < num; i++)
	{
		actor = pi_vector_get(collection->actors, i);
		pi_physics_aggregate_add_actor(ragdoll->aggregate, actor);
	}
	ragdoll->bone_actors = pi_new(BoneActor, data->map_size);
	ragdoll->bone_actors_num = data->map_size;
	for (i = 0; i < data->map_size; i++)
	{
		ragdoll->bone_actors[i].boneIndex = data->map[i].boneId;
		for (j = 0; j < ragdoll->aggregate->num_actors; j++)
		{
			if (ragdoll->aggregate->actors[j]->id == data->map[i].actorId)
			{
				ragdoll->bone_actors[i].actor = ragdoll->aggregate->actors[j];
				ragdoll->bone_actors[i].forceBone = data->map[i].force_bone;
			}
		}
	}
	return ragdoll;
}


PiSpatial* PI_API pi_physics_ragdoll_get_spatial(Ragdoll* ragdoll)
{
	return ragdoll->spaital;
}

void PI_API pi_physics_ragdoll_init_bone(Ragdoll* ragdoll, PiController* c)
{
	uint i;
	PiMatrix4 m;
	TransformData t;
	for (i = 0; i < ragdoll->bone_actors_num; i++)
	{
		BoneActor* boneActor = &ragdoll->bone_actors[i];
		pi_skanim_get_bone_matrix(c, boneActor->boneIndex, &m);
		pi_mat4_mul(&m, pi_spatial_get_world_transform(ragdoll->spaital), &m);
		pi_mat4_decompose(&t.translate, &t.scale, &t.rotate, &m);
		pi_physics_actor_set_transform(boneActor->actor, &t);
	}
}

void PI_API pi_physics_ragdoll_bind_bone_pose(Ragdoll* ragdoll, BoneTransform* boneTransform)
{
	uint i;
	for (i = 0; i < ragdoll->bone_actors_num; i++)
	{
		BoneActor* boneActor = &ragdoll->bone_actors[i];
		pi_physics_actor_get_world_transform(boneActor->actor, &boneTransform->transforms[boneActor->boneIndex]);
		boneTransform->is_computed[boneActor->boneIndex] = TRUE;
	}
}

void PI_API pi_physics_ragdoll_set_type(Ragdoll* ragdoll, RagdollType type)
{
	uint count = pi_physics_aggregate_num_actors(ragdoll->aggregate);

	uint i;
	PiBool is_kinematic = type == RT_Kinematic;
	for (i = 0; i < count; i++)
	{
		PiActor* actor = pi_physics_aggregate_get_actors(ragdoll->aggregate, i);
		pi_physics_actor_set_kinematic(actor, is_kinematic);
	}
}

RagdollData* PI_API pi_physics_ragdoll_data_load(void* map_data, uint32 map_data_size)
{
	PiBytes* buffer = pi_bytes_new();
	int size, i;
	RagdollData* ragdoll_data = pi_new(RagdollData, 1);
	pi_bytes_load(buffer, map_data, map_data_size, FALSE);
	pi_bytes_read_int(buffer, &size);

	ragdoll_data->map = pi_new(ActorIdBoneId, size);
	ragdoll_data->map_size = size;
	for (i = 0; i < size; i++)
	{
		wchar* name;
		int64_t actorId;
		int boneId;
		pi_bytes_read_wstr(buffer, &name);
		pi_bytes_read_int64(buffer, &actorId);
		pi_bytes_read_int(buffer, &boneId);
		ragdoll_data->map[i].actorId = actorId;
		ragdoll_data->map[i].boneId = boneId;
		ragdoll_data->map[i].force_bone = TRUE; // pi_wstr_start_with(name, L"Bip001 Spine") || pi_wstr_start_with(name, L"Bip001 Pelvis");
	}
	void* repx_data;
	ragdoll_data->data_size = pi_bytes_read_binary(buffer, &repx_data);
	ragdoll_data->base_data = pi_malloc(ragdoll_data->data_size);
	pi_memcpy_inline(ragdoll_data->base_data, repx_data, ragdoll_data->data_size);
	return ragdoll_data;
}

void PI_API pi_physics_ragdoll_set_linear_velocity(Ragdoll* ragdoll, PiVector3* v)
{
	uint count = pi_physics_aggregate_num_actors(ragdoll->aggregate);
	uint i;
	for (i = 0; i < count; i++)
	{
		PiActor* actor = pi_physics_aggregate_get_actors(ragdoll->aggregate, i);
		pi_physics_actor_set_linear_velocity(actor, v->x, v->y, v->z);
	}
}

void PI_API pi_physics_ragdoll_set_force(Ragdoll* ragdoll, float force_x, float force_y, float force_z, int modeType)
{
	uint i;
	
	for (i = 0; i < ragdoll->bone_actors_num; i++)
	{
		if (ragdoll->bone_actors[i].forceBone){
			pi_physics_actor_set_force(ragdoll->bone_actors[i].actor, force_x, force_y, force_z, modeType);
		}
		pi_physics_actor_wakeup(ragdoll->bone_actors[i].actor);
	}
}


void PI_API pi_physics_ragdoll_set_angular_velocity(Ragdoll* ragdoll, PiVector3* v)
{
	uint count = pi_physics_aggregate_num_actors(ragdoll->aggregate);
	uint i;
	for (i = 0; i < count; i++)
	{
		PiActor* actor = pi_physics_aggregate_get_actors(ragdoll->aggregate, i);
		pi_physics_actor_set_angular_velocity(actor, v);
	}
}

void PI_API pi_physics_ragdoll_set_sleep(Ragdoll* ragdoll)
{
	uint count = pi_physics_aggregate_num_actors(ragdoll->aggregate);
	uint i;
	for (i = 0; i < count; i++)
	{
		PiActor* actor = pi_physics_aggregate_get_actors(ragdoll->aggregate, i);
		pi_physics_actor_set_sleep(actor);
	}
}

void PI_API pi_physics_ragdoll_set_linear_damp(Ragdoll* ragdoll, float damp)
{
	uint i;
	for (i = 0; i < ragdoll->bone_actors_num; i++)
	{
		pi_physics_actor_set_linear_damp(ragdoll->bone_actors[i].actor, damp);
	}
}

void PI_API pi_physics_ragdoll_set_angular_damp(Ragdoll* ragdoll, float damp)
{
	uint i;
	for (i = 0; i < ragdoll->bone_actors_num; i++)
	{
		pi_physics_actor_set_angular_damp(ragdoll->bone_actors[i].actor, damp);
	}
}
