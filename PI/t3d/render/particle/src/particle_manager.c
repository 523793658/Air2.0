#include "particle_manager.h"

static PiCompR PI_API _sort(void* userData, const ParticleCluster* c1, const ParticleCluster* c2)
{
	if (c1->priority > c2->priority)
	{
		return PI_COMP_LESS;
	}
	return PI_COMP_GREAT;
}

ParticleSystem* PI_API pi_particle_system_create()
{
	ParticleSystem* sys = pi_new0(ParticleSystem, 1);
	return sys;
}

void PI_API pi_particle_system_free(ParticleSystem* sys)
{
	int i;
	for (i = 0; i < CT_NUM; i++)
	{
		if (sys->clusters[i])
		{
			pi_vector_free(sys->clusters[i]);
		}
	}
	if (sys->emitters)
	{
		pi_vector_free(sys->emitters);
	}
	pi_free(sys);
}

void PI_API pi_particle_system_add_cluster(ParticleSystem* sys, ParticleClusterType type, ParticleCluster *cluster)
{
	if (sys->clusters[type] == NULL)
	{
		sys->clusters[type] = pi_vector_new();
	}
	pi_vector_push(sys->clusters[type], cluster);
}

void PI_API pi_particle_system_add_emitter(ParticleSystem* sys, Emitter* emitter)
{
	if (sys->emitters == NULL)
	{
		sys->emitters = pi_vector_new();
	}
	pi_vector_push(sys->emitters, emitter);
}

void PI_API pi_particle_system_sort(ParticleSystem* sys)
{
	if (sys->clusters[CT_NORMAL] != NULL)
	{
		pi_vector_sort(sys->clusters[CT_NORMAL], (PiCompareFunc)_sort, NULL);
	}
	if (sys->clusters[CT_FLUCTUATION] != NULL)
	{
		pi_vector_sort(sys->clusters[CT_FLUCTUATION], (PiCompareFunc)_sort, NULL);
	}
}

void PI_API pi_particle_system_reset(ParticleSystem* sys, ParticleManager *pmgr, float raped)
{
	sys->time = pmgr->time;
	sys->rapid = 1.0f / raped;
}

void PI_API pi_particle_system_update(ParticleSystem* sys, float mgr_time)
{
	uint j, size;
	int i;
	PiVector* cluster_list;
	float time = (mgr_time - sys->time) * sys->rapid;
	sys->time = mgr_time;
	for (i = CT_NUM - 1; i >= 0; i--)
	{
		if (sys->clusters[i] != NULL)
		{
			cluster_list = sys->clusters[i];
			size = pi_vector_size(cluster_list);
			for (j = 0; j < size; j++)
			{
				pi_particle_cluster_update(pi_vector_get(cluster_list, j), time);
			}
		}
	}
	size = pi_vector_size(sys->emitters);
	for (j = 0; j < size; j++)
	{
		pi_emitter_update(pi_vector_get(sys->emitters, j), time);
	}
}

void PI_API pi_particle_system_get_entity(ParticleSystem* sys, ParticleClusterType type, PiVector* entity_list)
{
	if (sys->clusters[type])
	{
		uint i, size;
		size = pi_vector_size(sys->clusters[type]);
		for (i = 0; i < size; i++)
		{
			pi_particle_cluster_get_entities(pi_vector_get(sys->clusters[type], i), entity_list);
		}
	}
}

void PI_API pi_particle_system_set_priority(ParticleSystem* sys, int priority)
{
	sys->priority = priority;
}

static PiCompR PI_API _manager_sort(PiCamera* camera, ParticleSystem* sys1, ParticleSystem* sys2)
{
	if (sys1->priority > sys2->priority)
	{
		return PI_COMP_GREAT;
	}
	else if (sys1->priority < sys2->priority)
	{
		return PI_COMP_LESS;
	}
	else
	{
		PiVector3 *pos1, *pos2;
		float dis1, dis2;
		pos1 = pi_spatial_get_world_translation(&sys1->spatial);
		pos2 = pi_spatial_get_world_translation(&sys2->spatial);
		dis1 = pi_vec3_distance_square(pos1, pi_camera_get_location(camera));
		dis2 = pi_vec3_distance_square(pos2, pi_camera_get_location(camera));
		if (dis1 > dis2)
		{
			return PI_COMP_LESS;
		}
		return PI_COMP_GREAT;
	}
	
}

ParticleManager* PI_API pi_particle_mananger_create(PiCamera* camera)
{
	ParticleManager* mgr = pi_new0(ParticleManager, 1);
	mgr->camera = camera;
	return mgr;
}

void PI_API pi_particle_manager_set_camera(ParticleManager* mgr, PiCamera* camera)
{
	mgr->camera = camera;
}

void PI_API pi_particle_manager_sort(ParticleManager* mgr, PiCamera* camera)
{
	pi_vector_sort(mgr->query_list, (PiCompareFunc)_manager_sort, camera);
}

void PI_API pi_particle_manager_set_normal_data(ParticleManager* mgr, PiVector* list)
{
	mgr->normal_avaliable = TRUE;
	mgr->normal_entity_list = list;
}

void PI_API pi_particle_manager_set_fluctuation_data(ParticleManager* mgr, PiVector* list)
{
	mgr->flucatuation_avaliable = TRUE;
	mgr->flucatuation_entity_list = list;
}

void PI_API pi_particle_manager_free(ParticleManager* mgr)
{
	pi_free(mgr);
}

void PI_API pi_particle_manager_set_query_list(ParticleManager* mgr, PiVector* query_list)
{
	mgr->query_list = query_list;
}