#ifndef INCLUDE_STRETCHED_QUAD_PARTICLES_H
#define INCLUDE_STRETCHED_QUAD_PARTICLES_H

#include "particles.h"

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_stretched_quad_cluster_create();

void PI_API pi_particle_stretched_quad_cluster_free(ParticleCluster *cluster);

PI_END_DECLS

#endif /* INCLUDE_STRETCHED_QUAD_PARTICLES_H */