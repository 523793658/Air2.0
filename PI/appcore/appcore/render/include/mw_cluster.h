#ifndef MW_CLUSTER_H
#define MW_CLUSTER_H

#include <pi_lib.h>
#include <particles.h>
#include <camera.h>

typedef struct MWParticle
{
	PiVector *mw_cluster_list;
	PiAABBBox aabb;
	int priority;
} MWParticle;

typedef struct MWCluster
{
	MWParticle *parent;
	int priority;
	PiSpatial* spatial;
	ParticleCluster *cluster;
} MWCluster;

PI_BEGIN_DECLS

MWCluster *PI_API pi_mw_cluster_create(ParticleCluster *cluster, MWParticle *particle, int priority);

void PI_API pi_mw_cluster_free(MWCluster *cluster);

ParticleCluster *PI_API pi_mw_cluster_get_cluster(MWCluster *c);

MWParticle *PI_API pi_mw_particle_create();

PiSpatial* PI_API pi_mw_cluster_get_spatial(MWCluster* p);

void PI_API pi_mw_particle_free(MWParticle *p);

void pi_mw_particle_add_cluster(MWParticle *p, MWCluster *c);

PiAABBBox *PI_API pi_mw_particle_get_world_aabb(MWParticle *p);

PiCompR PI_API pi_mw_cluster_sort_fun(PiCamera *camera, const MWCluster *pa, const MWCluster *pb);

PiCompareFunc PI_API pi_mw_cluster_get_sort_func();

MWParticle *PI_API pi_mw_cluster_get_parent(MWCluster *cluster);

void PI_API pi_mw_particle_updata_aabb(MWParticle *p);

void PI_API pi_mw_particle_set_priority(MWParticle *P, int priority);

PI_END_DECLS

#endif /* MW_CLUSTER_H */