#ifndef INCLUDE_CPU_PARTICLES_H
#define INCLUDE_CPU_PARTICLES_H

#include "particles.h"

//CPU粒子 所有基于CPU实现的粒子簇由此粒子扩展

#define _CPU_PARTICLE_UPDATE_MASK_GRAVITATE 0x1
#define _CPU_PARTICLE_UPDATE_MASK_VORTEX 0x2
#define _CPU_PARTICLE_UPDATE_MASK_ROTATION 0x4
#define _CPU_PARTICLE_UPDATE_MASK_SIZE 0x8
#define _CPU_PARTICLE_UPDATE_MASK_COLOR 0x10
#define _CPU_PARTICLE_UPDATE_MASK_ALPHA 0x20

typedef struct CPUParticleCluster CPUParticleCluster;
typedef struct CPUParticle CPUParticle;

typedef void (*CpuClusterUpdateFunc)(CPUParticleCluster *cluster, float tpf);
typedef void (*CpuParticleInitFunc)(CPUParticleCluster *cluster, CPUParticle *particle);
typedef void (*CpuParticleDestroyFunc)(CPUParticleCluster *cluster, CPUParticle *particle);
typedef void (*CpuParticleUpdateFunc)(CPUParticleCluster *cluster, CPUParticle *particle, float tpf);
typedef void (*CpuParticleSpawnFunc)(CPUParticleCluster *cluster, CPUParticle *particle);
typedef void (*CpuParticleDieFunc)(CPUParticleCluster *cluster, CPUParticle *particle);

struct CPUParticleCluster
{
	ParticleCluster *cluster;

	void *impl;

	CpuClusterUpdateFunc cluster_update_func;
	CpuParticleInitFunc particle_init_func;
	CpuParticleDestroyFunc particle_destroy_func;
	CpuParticleUpdateFunc particle_update_func;
	CpuParticleSpawnFunc particle_spawn_func;
	CpuParticleDieFunc particle_die_func;
};

struct CPUParticle
{
	Particle *particle;
	PiSpatial *spatial;

	void *impl;

	PiVector4 color;
	float alpha;

	//Motion
	PiVector3 displacement;                //位移
	PiVector3 velocity;                    //速度

	//Rotation
	PiQuaternion rotation;

	uint update_mask;
};

ParticleCluster *particle_cpu_cluster_create(CpuClusterUpdateFunc cluster_update_func, CpuParticleInitFunc init_func, CpuParticleDestroyFunc destroy_func, CpuParticleUpdateFunc update_func, CpuParticleSpawnFunc spawn_func, CpuParticleDieFunc die_func, ParticleGetEntitiesFunc get_entities_func, void *impl);

void particle_cpu_cluster_free(ParticleCluster *cluster);

PiAABBBox *PI_API pi_particle_cluster_get_world_aabb(ParticleCluster *cluster);

#endif /* INCLUDE_CPU_PARTICLE_H */