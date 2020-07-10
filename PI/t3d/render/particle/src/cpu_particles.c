
#include "cpu_particles.h"
#include "movement.h"

static void _cluster_update(ParticleCluster *cluster, float tpf)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	impl->cluster_update_func(impl, tpf);
}

static void _particle_init(ParticleCluster *cluster, Particle *particle)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	CPUParticle *p_impl = pi_new0(CPUParticle, 1);

	p_impl->spatial = pi_spatial_geometry_create();

	p_impl->particle = particle;

	particle->impl = p_impl;
	impl->particle_init_func(impl, p_impl);
}

static void _particle_destroy(ParticleCluster *cluster, Particle *particle)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	CPUParticle *p_impl = (CPUParticle *)particle->impl;

	impl->particle_destroy_func(impl, p_impl);
	pi_spatial_destroy(p_impl->spatial);
	pi_free(p_impl);
}

static PiBool _check_varying_sequence(PiSequence *sequence)
{
	return !pi_sequence_is_same(sequence);
}

static void _check_update_mask(ParticleCluster *cluster, Particle *particle)
{
	CPUParticle *p_impl = (CPUParticle *)particle->impl;
	EmitInfo *info = &particle->info;
	float rotation_rate_length = pi_vec3_len_square(&info->rotation_rate);

	p_impl->update_mask = 0;

	p_impl->update_mask |= (info->gravitate_force ? _CPU_PARTICLE_UPDATE_MASK_GRAVITATE : 0);
	p_impl->update_mask |= (info->vortex_rate ? _CPU_PARTICLE_UPDATE_MASK_VORTEX : 0);
	p_impl->update_mask |= (rotation_rate_length ? _CPU_PARTICLE_UPDATE_MASK_ROTATION : 0);
	p_impl->update_mask |= (_check_varying_sequence(cluster->size_sequence) ? _CPU_PARTICLE_UPDATE_MASK_SIZE : 0);
	p_impl->update_mask |= (_check_varying_sequence(cluster->color_sequence) ? _CPU_PARTICLE_UPDATE_MASK_COLOR : 0);
	p_impl->update_mask |= (_check_varying_sequence(cluster->alpha_sequence) ? _CPU_PARTICLE_UPDATE_MASK_ALPHA : 0);
}

static void _particle_update(ParticleCluster *cluster, Particle *particle, float tpf)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	CPUParticle *p_impl = (CPUParticle *)particle->impl;
	PiSpatial *spatial = p_impl->spatial;
	float time = particle->time / particle->life_time;

	pi_particle_move(p_impl, &particle->info, tpf);

	if (p_impl->update_mask & _CPU_PARTICLE_UPDATE_MASK_VORTEX)
	{
		PiVector3 dstPos;
		pi_particle_vortex(spatial, p_impl, &particle->info, particle->time, &dstPos);
		pi_spatial_set_local_translation(spatial, dstPos.x, dstPos.y, dstPos.z);
	}
	else
	{
		pi_spatial_set_local_translation(spatial, p_impl->displacement.x, p_impl->displacement.y, p_impl->displacement.z);
	}

	if (p_impl->update_mask & _CPU_PARTICLE_UPDATE_MASK_ROTATION)
	{
		pi_particle_rotate(p_impl, &particle->info, particle->time);
	}

	if (p_impl->update_mask & _CPU_PARTICLE_UPDATE_MASK_SIZE)
	{
		pi_particle_set_size(spatial, cluster->size_sequence, &particle->info, time);
	}

	if (p_impl->update_mask & _CPU_PARTICLE_UPDATE_MASK_COLOR)
	{
		pi_sequence_set_time(cluster->color_sequence, time);
		pi_math_vec4_copy_from_vec3(&p_impl->color, (PiVector3 *)pi_sequence_get_value(cluster->color_sequence));
	}

	if (p_impl->update_mask & _CPU_PARTICLE_UPDATE_MASK_ALPHA)
	{
		pi_sequence_set_time(cluster->alpha_sequence, time);
		p_impl->alpha = particle->info.alpha * (*(float *)pi_sequence_get_value(cluster->alpha_sequence));
	}
	p_impl->color.w = p_impl->alpha * cluster->global_alpha;
	pi_spatial_update(spatial);

	impl->particle_update_func(impl, p_impl, tpf);
}

static void _particle_spawn(ParticleCluster *cluster, Particle *particle)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	CPUParticle *p_impl = (CPUParticle *)particle->impl;
	PiSpatial *spatial = p_impl->spatial;
	PiVector3 dstPos;

	_check_update_mask(cluster, particle);

	//初始的位移
	pi_vec3_copy(&p_impl->displacement, &particle->info.position);

	//初始的速度
	pi_vec3_scale(&p_impl->velocity, &particle->info.direction, particle->info.velocity);

	pi_particle_vortex(spatial, p_impl, &particle->info, 0.0f, &dstPos);
	pi_spatial_set_local_translation(spatial, dstPos.x, dstPos.y, dstPos.z);

	pi_particle_rotate(p_impl, &particle->info, 0.0f);
	
	pi_particle_set_size(spatial, cluster->size_sequence, &particle->info, 0.0f);

	pi_sequence_set_time(cluster->color_sequence, 0.0f);
	pi_math_vec4_copy_from_vec3(&p_impl->color, (PiVector3 *)pi_sequence_get_value(cluster->color_sequence));

	pi_sequence_set_time(cluster->alpha_sequence, 0.0f);
	p_impl->alpha = particle->info.alpha * (*(float *)pi_sequence_get_value(cluster->alpha_sequence));

	if (particle->info.emitter_spatial)
	{
		pi_node_attach_child(particle->info.emitter_spatial, p_impl->spatial);
	}

	pi_spatial_update(spatial);

	impl->particle_spawn_func(impl, p_impl);
}

static void _particle_die(ParticleCluster *cluster, Particle *particle)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	CPUParticle *p_impl = (CPUParticle *)particle->impl;

	if (particle->info.emitter_spatial)
	{
		pi_spatial_detach_from_parent(p_impl->spatial);
	}

	impl->particle_die_func(impl, p_impl);
}

ParticleCluster *particle_cpu_cluster_create(CpuClusterUpdateFunc cluster_update_func, CpuParticleInitFunc init_func, CpuParticleDestroyFunc destroy_func, CpuParticleUpdateFunc update_func, CpuParticleSpawnFunc spawn_func, CpuParticleDieFunc die_func, ParticleGetEntitiesFunc get_entities_func, void *impl)
{
	ParticleCluster *cluster;
	CPUParticleCluster *cpu_particle_cluster = pi_new0(CPUParticleCluster, 1);

	cpu_particle_cluster->impl = impl;
	cpu_particle_cluster->cluster_update_func = cluster_update_func;
	cpu_particle_cluster->particle_init_func = init_func;
	cpu_particle_cluster->particle_destroy_func = destroy_func;
	cpu_particle_cluster->particle_update_func = update_func;
	cpu_particle_cluster->particle_spawn_func = spawn_func;
	cpu_particle_cluster->particle_die_func = die_func;
	
	cluster = particle_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, get_entities_func, cpu_particle_cluster);
	cpu_particle_cluster->cluster = cluster;

	return cluster;
}

void particle_cpu_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *impl = (CPUParticleCluster *)cluster->impl;
	particle_cluster_free(cluster);
	pi_free(impl);
}

PiAABBBox *PI_API pi_particle_cluster_get_world_aabb(ParticleCluster *cluster)
{
	uint i, size;

	pi_aabb_init(&cluster->aabb);
	size = pi_vector_size(cluster->active_list);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_get(cluster->active_list, i);
		CPUParticle *p_impl = (CPUParticle *)particle->impl;
		PiVector3 *pos = pi_spatial_get_world_translation(p_impl->spatial);
		pi_aabb_add_point(&cluster->aabb, pos);

		if (i == size - 1)
		{
			PiVector3 s;
			pi_vec3_scale(&s, &particle->info.size, 0.5);
			pi_vec3_sub(&cluster->aabb.minPt, &cluster->aabb.minPt, &s);
			pi_vec3_add(&cluster->aabb.maxPt, &cluster->aabb.maxPt, &s);
		}
	}

	return &cluster->aabb;
}
