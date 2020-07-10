#ifndef INCLUDE_QUAD_PARTICLES_H
#define INCLUDE_QUAD_PARTICLES_H

#include "particles.h"
#include <billboard.h>

//方形布告板粒子

typedef enum
{
	EOT_CENTER,
	EOT_CENTER_LEFT,
	EOT_CENTER_RIGHT,
	EOT_TOP_CENTER,
	EOT_TOP_LEFT,
	EOT_TOP_RIGHT,
	EOT_BOTTOM_CENTER,
	EOT_BOTTOM_LEFT,
	EOT_BOTTOM_RIGHT,
	EOT_NUM
} EORIGINType;

PI_BEGIN_DECLS

ParticleCluster *PI_API pi_particle_quad_cluster_create();

void PI_API pi_particle_quad_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_quad_cluster_set_facing(ParticleCluster *cluster, PiCamera *cam, EFacingType type);

void PI_API pi_particle_quad_cluster_set_origin_type(ParticleCluster *cluster, EORIGINType origin_type);

PI_END_DECLS

#endif /* INCLUDE_QUAD_PARTICLES_H */