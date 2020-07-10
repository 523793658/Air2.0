
#include <cpu_particles.h>
#include "mw_cluster.h"

MWCluster *PI_API pi_mw_cluster_create(ParticleCluster *cluster, MWParticle *particle, int priority)
{
	MWCluster *impl = pi_new0(MWCluster, 1);
	impl->cluster = cluster;
	impl->parent = particle;
	impl->priority = priority;
	impl->spatial = pi_spatial_node_create();
	pi_mw_particle_add_cluster(impl->parent, impl);
	return impl;
}

ParticleCluster *PI_API pi_mw_cluster_get_cluster(MWCluster *c)
{
	return c->cluster;
}

void PI_API pi_mw_cluster_free(MWCluster *cluster)
{
	pi_spatial_destroy(cluster->spatial);
	pi_free(cluster);
}

MWParticle *PI_API pi_mw_particle_create()
{
	MWParticle *impl = pi_new0(MWParticle, 1);
	impl->mw_cluster_list = pi_vector_new();
	return impl;
}

PiSpatial* PI_API pi_mw_cluster_get_spatial(MWCluster* c)
{
	return c->spatial;
}

void PI_API pi_mw_particle_free(MWParticle *p)
{
	pi_vector_free(p->mw_cluster_list);
	pi_free(p);
}

void pi_mw_particle_add_cluster(MWParticle *p, MWCluster *c)
{
	pi_vector_push(p->mw_cluster_list, c);
}

PiAABBBox *PI_API pi_mw_particle_get_world_aabb(MWParticle *p)
{
	int i, size;
	MWCluster *cluster;
	pi_aabb_init(&p->aabb);
	size = pi_vector_size(p->mw_cluster_list);

	for (i = 0; i < size; i++)
	{
		cluster = (MWCluster *)pi_vector_get(p->mw_cluster_list, i);
		pi_aabb_merge(&p->aabb, &p->aabb, pi_particle_cluster_get_world_aabb(cluster->cluster));
	}

	return &p->aabb;
}

PiCompR  PI_API pi_mw_cluster_sort_fun(PiCamera *camera, const MWCluster *pa, const MWCluster *pb)
{
	float dis_sq_a, dis_sq_b;
	PiAABBBox *particle1_aabb ;
	PiAABBBox *particle2_aabb ;
	PiVector3* pos_a;
	PiVector3* pos_b;

	if (pa->parent == pb->parent)
	{
		if (pa->priority > pb->priority)
		{
			return PI_COMP_LESS;
		}
		else if (pa->priority < pb->priority)
		{
			return PI_COMP_GREAT;
		}
		else
		{
			return PI_COMP_EQUAL;
		}
	}
	else
	{
		if (pa->parent->priority > pb->parent->priority)
		{
			return PI_COMP_GREAT;
		}
		else if (pa->parent->priority < pb->parent->priority)
		{
			return PI_COMP_LESS;
		}
		pos_a = pi_spatial_get_world_translation(pa->spatial);
		pos_b = pi_spatial_get_world_translation(pb->spatial);

		dis_sq_a = pi_vec3_distance_square(pos_a, pi_camera_get_location(camera));
		dis_sq_b = pi_vec3_distance_square(pos_b, pi_camera_get_location(camera));

		if (dis_sq_a > dis_sq_b)
		{
			return PI_COMP_LESS;
		}
		else if (dis_sq_a < dis_sq_b)
		{
			return PI_COMP_GREAT;
		}
		else
		{
			return PI_COMP_EQUAL;
		}
	}
}

PiCompareFunc PI_API pi_mw_cluster_get_sort_func()
{
	PiCompareFunc func = NULL;
	func = (PiCompareFunc)pi_mw_cluster_sort_fun;
	return func;
}

void PI_API pi_mw_particle_updata_aabb(MWParticle *p)
{
	uint size = pi_vector_size(p->mw_cluster_list);
	uint i;
	MWCluster *cluster;
	pi_aabb_init(&p->aabb);

	for (i = 0 ; i < size; i++)
	{
		cluster = (MWCluster *)pi_vector_get(p->mw_cluster_list, i);
		pi_aabb_merge(&p->aabb, &p->aabb, &cluster->cluster->aabb);
	}
}

MWParticle *PI_API pi_mw_cluster_get_parent(MWCluster *cluster)
{
	return cluster->parent;
}

void PI_API pi_mw_particle_set_priority(MWParticle *P, int priority)
{
	P->priority = priority;
}
