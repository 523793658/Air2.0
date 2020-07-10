#ifndef _Physics_Ragdoll_H_
#define _Physics_Ragdoll_H_
#include "pi_lib.h"
#include "controller.h"
#include "physics_actor.h"
#include "pi_skeleton.h"
#include "physics_mesh.h"

typedef struct
{
	int boneId;
	int64_t actorId;
	PiBool* force_bone;
}ActorIdBoneId;

typedef struct  
{
	void* base_data;
	uint data_size;
	ActorIdBoneId* map;
	uint map_size;
}RagdollData;

typedef enum
{
	RT_Dynamic,
	RT_Kinematic
}RagdollType;

typedef struct  
{
	void *impl;
	PiVector* actors;
}PiCollection;

typedef struct  
{
	void *impl;
	uint max_actors;
	uint num_actors;
	PiBool self_collision;
	PiActor** actors;
}PiAggregate;

typedef struct
{
	PiBool forceBone;
	int boneIndex;
	PiActor* actor;
}BoneActor;

typedef struct
{
	char* name;
	BoneActor* bone_actors;
	uint bone_actors_num;
	PiVector* joins;
	PiSpatial* spaital;
	PiAggregate* aggregate;
	PiCollection* collection;
}Ragdoll;

PI_BEGIN_DECLS

Ragdoll* PI_API pi_physics_ragdoll_new();

Ragdoll* PI_API pi_physics_ragdoll_load(RagdollData* data);

void PI_API pi_physics_ragdoll_free(Ragdoll* ragdoll);

void PI_API pi_physics_ragdoll_sleep();

void PI_API pi_physics_ragdoll_weak_up();

void PI_API pi_physics_ragdoll_init_bone(Ragdoll* ragdoll, PiController* c);

void PI_API pi_physics_ragdoll_bind_bone_pose(Ragdoll* ragdoll, BoneTransform* boneTransform);

PiSpatial* PI_API pi_physics_ragdoll_get_spatial(Ragdoll* ragdoll);

void PI_API pi_physics_ragdoll_set_type(Ragdoll* ragdoll, RagdollType type);

RagdollData* PI_API pi_physics_ragdoll_data_load(void* map_data, uint32 map_data_size);

void PI_API pi_physics_ragdoll_set_linear_velocity(Ragdoll* ragdoll, PiVector3* v);

void PI_API pi_physics_ragdoll_set_force(Ragdoll* ragdoll, float force_x, float force_y, float force_z, int modeType);

void PI_API pi_physics_ragdoll_set_angular_velocity(Ragdoll* ragdoll, PiVector3* v);

void PI_API pi_physics_ragdoll_set_sleep(Ragdoll* ragdoll);

void PI_API pi_physics_ragdoll_set_linear_damp(Ragdoll* ragdoll, float damp);

void PI_API pi_physics_ragdoll_set_angular_damp(Ragdoll* ragdoll, float damp);

PI_END_DECLS
#endif