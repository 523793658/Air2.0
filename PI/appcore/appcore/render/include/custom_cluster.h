#ifndef INCLUDE_CUSTOM_CLUSTER_H
#define INCLUDE_CUSTOM_CLUSTER_H
#include "particles.h"

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_custom_cluster_create();

void PI_API pi_particle_custom_cluster_free(ParticleCluster *cluster);

PI_END_DECLS

#endif