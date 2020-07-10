#ifndef INCLUDE_NESTING_PARTICLES_H
#define INCLUDE_NESTING_PARTICLES_H

#include "particles.h"
#include "particle_emitter.h"

//Ç¶Ì×Á£×Ó

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_nesting_cluster_create();

void PI_API pi_particle_nesting_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_nesting_cluster_add_emitter(ParticleCluster *cluster, Emitter *emitter);

PI_END_DECLS

#endif /* INCLUDE_NESTING_PARTICLES_H */