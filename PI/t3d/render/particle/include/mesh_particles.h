#ifndef INCLUDE_MESH_PARTICLES_H
#define INCLUDE_MESH_PARTICLES_H

#include "particles.h"
#include <pi_rendermesh.h>

//自定义网格粒子

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_mesh_cluster_create();

void PI_API pi_particle_mesh_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_mesh_cluster_set_mesh(ParticleCluster *cluster, PiRenderMesh *mesh);

PI_END_DECLS

#endif /* INCLUDE_MESH_PARTICLES_H */