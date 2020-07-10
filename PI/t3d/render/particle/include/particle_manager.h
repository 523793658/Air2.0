#ifndef _ParticleManager_H_
#define _ParticleManager_H_
#include "pi_lib.h"
#include "particles.h"
#include "particle_emitter.h"
#include "pi_spatial.h"
#include "camera.h"
#define MAX_CLUSTER 

typedef enum
{
	CT_NORMAL,
	CT_FLUCTUATION,
	CT_NEST,
	CT_NUM
}ParticleClusterType;

typedef struct  
{
	PiBool normal_avaliable;
	PiVector* normal_entity_list;

	PiBool flucatuation_avaliable;
	PiVector* flucatuation_entity_list;
	PiVector* query_list;
	PiCamera* camera;
	float time;
}ParticleManager;


typedef struct  
{
	PiVector* clusters[CT_NUM];
	PiVector* emitters;
	PiSpatial spatial;
	float time;
	int priority;
	float rapid;
}ParticleSystem;





PI_BEGIN_DECLS

ParticleSystem* PI_API pi_particle_system_create();

void PI_API pi_particle_system_free(ParticleSystem* sys);

void PI_API pi_particle_system_sort(ParticleSystem* sys);

void PI_API pi_particle_system_reset(ParticleSystem* sys, ParticleManager *pmgr, float rapid);

void PI_API pi_particle_system_add_cluster(ParticleSystem* sys, ParticleClusterType type, ParticleCluster *cluster);

void PI_API pi_particle_system_add_emitter(ParticleSystem* sys, Emitter* emitter);

void PI_API pi_particle_system_update(ParticleSystem* sys, float tpf);

void PI_API pi_particle_system_get_entity(ParticleSystem* sys, ParticleClusterType type, PiVector* entity_list);

void PI_API pi_particle_system_set_priority(ParticleSystem* sys, int priority);


ParticleManager* PI_API pi_particle_mananger_create(PiCamera* camera);

void PI_API pi_particle_manager_set_camera(ParticleManager* mgr, PiCamera* camera);

void PI_API pi_particle_manager_set_normal_data(ParticleManager* mgr, PiVector* list);

void PI_API pi_particle_manager_set_fluctuation_data(ParticleManager* mgr, PiVector* list);

void PI_API pi_particle_manager_free(ParticleManager* mgr);

void PI_API pi_particle_manager_sort(ParticleManager* mgr, PiCamera* camera);

void PI_API pi_particle_manager_set_query_list(ParticleManager* mgr, PiVector* query_list);


PI_END_DECLS

#endif