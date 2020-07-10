#ifndef INCLUDE_TAIL_PARTICLES_H
#define INCLUDE_TAIL_PARTICLES_H

#include "particles.h"
#include <tail.h>

//мон╡аёвс

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_tail_cluster_create();

void PI_API pi_particle_tail_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_tail_cluster_set_type(ParticleCluster *cluster, ETailType type);

void PI_API pi_particle_tail_cluster_set_data(ParticleCluster *cluster, uint sample_step, float width, float life_time, PiBool head_follow);

PI_END_DECLS

#endif /* INCLUDE_TAIL_PARTICLES_H */