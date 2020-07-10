#ifndef INCLUDE_MOVEMENT_H
#define INCLUDE_MOVEMENT_H

#include "cpu_particles.h"

PI_BEGIN_DECLS

/**
 * 计算粒子的位移、速度、加速度  s=v0*t+a0*t*t/2 v=v0+a0*t
 */
void pi_particle_move(CPUParticle *particle, EmitInfo *info, float tpf);

void pi_particle_set_size(PiSpatial *spatial, PiSequence *size_sequence, EmitInfo *info, float time);

void pi_particle_vortex(PiSpatial *spatial, CPUParticle *particle, EmitInfo *info, float time, PiVector3 *dst);

void pi_particle_rotate(CPUParticle *particle, EmitInfo *info, float time);

PI_END_DECLS

#endif /* INCLUDE_MOVEMENT_H */