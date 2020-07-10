
#include "nesting_particles.h"
#include "cpu_particles.h"

typedef struct
{
	PiVector *prototype;
} NestingParticleCluster;

typedef struct
{
	PiVector *instance;
} NestingParticle;

static void _cluster_update(CPUParticleCluster *cluster, float tpf)
{
}

static void _particle_init(CPUParticleCluster *cluster, CPUParticle *particle)
{
	NestingParticleCluster *impl = (NestingParticleCluster *)cluster->impl;
	NestingParticle *p_impl = pi_new0(NestingParticle, 1);
	uint size;
	uint i;
	p_impl->instance = pi_vector_new();
	size = pi_vector_size(impl->prototype);

	for (i = 0 ; i < size; i++)
	{
		Emitter *emitter = (Emitter *)pi_vector_get(impl->prototype, i);
		Emitter *p_emitter = pi_emitter_clone(emitter);
		pi_spatial_attach_sync(particle->spatial, p_emitter->spatial);
		pi_vector_push(p_impl->instance, p_emitter);
	}

	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cluster, CPUParticle *particle)
{
	NestingParticle *p_impl = (NestingParticle *)particle->impl;
	uint size;
	uint i;
	size = pi_vector_size(p_impl->instance);

	for (i = 0 ; i < size; i++)
	{
		Emitter *emitter = (Emitter *)pi_vector_get(p_impl->instance, i);
		pi_spatial_detach_sync(particle->spatial, emitter->spatial);
		pi_emitter_free(emitter);
	}

	pi_vector_free(p_impl->instance);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cluster, CPUParticle *particle, float tpf)
{
	NestingParticle *p_impl = (NestingParticle *)particle->impl;
	uint size;
	uint i;
	PI_USE_PARAM(cluster);
	size = pi_vector_size(p_impl->instance);

	for (i = 0 ; i < size; i++)
	{
		Emitter *emitter = (Emitter *)pi_vector_get(p_impl->instance, i);
		pi_emitter_update(emitter, tpf);
	}
}

static void _particle_spawn(CPUParticleCluster *cluster, CPUParticle *particle)
{
	NestingParticle *p_impl = (NestingParticle *)particle->impl;
	uint size;
	uint i;
	PI_USE_PARAM(cluster);
	size = pi_vector_size(p_impl->instance);

	for (i = 0; i < size; i++)
	{
		Emitter *emitter = (Emitter *)pi_vector_get(p_impl->instance, i);
		pi_emitter_spawn(emitter);
	}
}

static void _particle_die(CPUParticleCluster *cluster, CPUParticle *particle)
{
	PI_USE_PARAM(cluster);
	PI_USE_PARAM(particle);
}

static void _particle_get_entities(ParticleCluster *cluster, PiVector *dst)
{
}

ParticleCluster *PI_API pi_particle_nesting_cluster_create()
{
	ParticleCluster *cluster;
	NestingParticleCluster *nestingParticle = pi_new0(NestingParticleCluster, 1);
	nestingParticle->prototype = pi_vector_new();
	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, nestingParticle);

	return cluster;
}

void PI_API pi_particle_nesting_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	NestingParticleCluster *impl = (NestingParticleCluster *)cpu_impl->impl;
	pi_vector_free(impl->prototype);
	pi_free(impl);

	particle_cpu_cluster_free(cluster);
}

void PI_API pi_particle_nesting_cluster_add_emitter(ParticleCluster *cluster, Emitter *emitter)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	NestingParticleCluster *impl = (NestingParticleCluster *)cpu_impl->impl;
	pi_vector_push(impl->prototype, emitter);
}
