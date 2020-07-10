#ifndef INCLUDE_CHAIN_PARTICLES_H
#define INCLUDE_CHAIN_PARTICLES_H

#include "particles.h"
#include <chain.h>

//Á´Á£×Ó´Ø

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_chain_cluster_create();

void PI_API pi_particle_chain_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_chain_cluster_set_facing(ParticleCluster *cluster, EChainFacingType type);

void PI_API pi_particle_chain_cluster_set_data(ParticleCluster *cluster, float width, float step_time);

void PI_API pi_particle_chain_cluster_set_target(ParticleCluster *cluster, PiVector3 *target, PiVector3 *offset);

PI_END_DECLS

#endif /* INCLUDE_CHAIN_PARTICLES_H */