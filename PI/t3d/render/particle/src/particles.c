
#include "particles.h"
#include "material.h"
typedef struct
{
	float tpf;
	ParticleCluster *cluster;
} UpdatePredData;

/*
 * 返回false表示粒子生命周期结束
 */
static PiBool particle_update_base(ParticleCluster *cluster, Particle *particle, float tpf)
{
	particle->time += tpf;
	if (particle->time < particle->life_time)
	{
		cluster->particle_update_func(cluster, particle, tpf);
		return TRUE;
	}

	cluster->particle_die_func(cluster, particle);
	return FALSE;
}

static void particle_spawn_base(ParticleCluster *cluster, Particle *particle, EmitInfo *info, float start_time)
{
	particle->time = 0;
	particle->life_time = info->life_time;
	pi_memcpy_inline(&particle->info, info, sizeof(EmitInfo));

	cluster->particle_spawn_func(cluster, particle);

	particle_update_base(cluster, particle, start_time);
}

static PiBool PI_API _update_pred_func(void *user_data, const void *data)
{
	Particle *particle = (Particle *)data;
	UpdatePredData *pred_data = (UpdatePredData *)user_data;

	if (!particle_update_base(pred_data->cluster, particle, pred_data->tpf))
	{
		pi_vector_push(pred_data->cluster->idle_list, particle);
		return TRUE;
	}

	return FALSE;
}

void PI_API pi_particle_cluster_update(ParticleCluster *cluster, float tpf)
{
	UpdatePredData pred_data;
	pred_data.tpf = tpf;
	pred_data.cluster = cluster;
	cluster->cluster_update_func(cluster, tpf);
	pi_vector_remove_if(cluster->active_list, _update_pred_func, &pred_data);
}

void PI_API pi_particle_cluster_spawn(ParticleCluster *cluster)
{
	uint i, size;
	size = pi_vector_size(cluster->active_list);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_pop(cluster->active_list);
		cluster->particle_die_func(cluster, particle);
		pi_vector_push(cluster->idle_list, particle);
	}
}

void particle_cluster_emit(ParticleCluster *cluster, EmitInfo *info, float start_time)
{
	uint current_count = pi_vector_size(cluster->active_list);
	if (current_count < cluster->max_count)
	{
		Particle *particle = (Particle *)pi_vector_pop(cluster->idle_list);

		if (particle == NULL)
		{
			particle = pi_new0(Particle, 1);
			cluster->particle_init_func(cluster, particle);
		}

		particle_spawn_base(cluster, particle, info, start_time);
		pi_vector_push(cluster->active_list, particle);
	}
}

void PI_API pi_particle_cluster_set_priority(ParticleCluster* cluster, int priority)
{
	cluster->priority = priority;
}

ParticleCluster *particle_cluster_create(ClusterUpdateFunc cluster_update_func, ParticleInitFunc init_func, ParticleDestroyFunc destroy_func, ParticleUpdateFunc update_func, ParticleSpawnFunc spawn_func, ParticleDieFunc die_func, ParticleGetEntitiesFunc get_entities_func, void *impl)
{
	float defalut_sequence_value_one = 1;
	PiVector3 vec3;
	ParticleCluster *cluster = pi_new0(ParticleCluster, 1);
	cluster->active_list = pi_vector_new();
	cluster->idle_list = pi_vector_new();

	cluster->cluster_update_func = cluster_update_func;
	cluster->particle_init_func = init_func;
	cluster->particle_destroy_func = destroy_func;
	cluster->particle_spawn_func = spawn_func;
	cluster->particle_die_func = die_func;
	cluster->particle_update_func = update_func;
	cluster->particle_get_entities_func = get_entities_func;
	cluster->global_alpha = 1.0f;
	cluster->max_count = 1000;
	cluster->impl = impl;


	cluster->alpha_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_set_search_method(cluster->alpha_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->alpha_sequence, 128);
	pi_sequence_add_node(cluster->alpha_sequence, 0.0f, &defalut_sequence_value_one);

	vec3.x = 1.0f;
	vec3.y = 1.0f;
	vec3.z = 1.0f;

	cluster->size_sequence = pi_sequence_create(EST_VECTOR_3F);
	pi_sequence_set_search_method(cluster->size_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->size_sequence, 128);
	pi_sequence_add_node(cluster->size_sequence, 0.0f, &vec3);

	cluster->color_sequence = pi_sequence_create(EST_VECTOR_3F);
	pi_sequence_set_search_method(cluster->color_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->color_sequence, 128);
	pi_sequence_add_node(cluster->color_sequence, 0.0f, &vec3);

	cluster->depth_enable = TRUE;
	cluster->depth_write_enable = FALSE;

	return cluster;
}

void PI_API particle_cluster_free(ParticleCluster *cluster)
{
	uint i, size;

	size = pi_vector_size(cluster->active_list);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_pop(cluster->active_list);
		cluster->particle_die_func(cluster, particle);
		cluster->particle_destroy_func(cluster, particle);
		pi_free(particle);
	}

	size = pi_vector_size(cluster->idle_list);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_pop(cluster->idle_list);
		cluster->particle_destroy_func(cluster, particle);
		pi_free(particle);
	}

	pi_sequence_free(cluster->alpha_sequence);
	pi_sequence_free(cluster->color_sequence);
	pi_sequence_free(cluster->size_sequence);

	pi_vector_free(cluster->active_list);
	pi_vector_free(cluster->idle_list);

	pi_free(cluster);
}

void PI_API pi_particle_cluster_set_alpha_sequence(ParticleCluster *cluster, PiSequence *alpha_sequence)
{
	pi_sequence_copy(cluster->alpha_sequence, alpha_sequence);
	pi_sequence_set_search_method(cluster->alpha_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->alpha_sequence, 128);
}

void PI_API pi_particle_cluster_set_color_sequence(ParticleCluster *cluster, PiSequence *color_sequence)
{
	pi_sequence_copy(cluster->color_sequence, color_sequence);
	pi_sequence_set_search_method(cluster->color_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->color_sequence, 128);
}

void PI_API pi_particle_cluster_set_size_sequence(ParticleCluster *cluster, PiSequence *size_sequence)
{
	pi_sequence_copy(cluster->size_sequence, size_sequence);
	pi_sequence_set_search_method(cluster->size_sequence, PRE_TREATMENT);
	pi_sequence_set_pretreatment_length(cluster->size_sequence, 128);
}

void PI_API pi_particle_cluster_set_texture(ParticleCluster *cluster, PiTexture *tex, uint tile_x, uint tile_y, float rate, PiBool blend_enable, PiBool random_start)
{
	cluster->tile_info.x = (float)tile_x;
	cluster->tile_info.y = (float)tile_y;
	cluster->tile_info.z = rate;
	cluster->blend_enable = blend_enable;
	cluster->random_start = random_start;
	cluster->texture = tex;
	pi_renderstate_set_default_sampler(&cluster->ss_tex);
	pi_sampler_set_addr_mode(&cluster->ss_tex, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&cluster->ss_tex, TFO_MIN_MAG_MIP_LINEAR);
	pi_sampler_set_texture(&cluster->ss_tex, cluster->texture);
}

void PI_API pi_particle_cluster_set_blend_mode(ParticleCluster *cluster, EParticleBlendMode blend_mode)
{
	cluster->blend_mode = blend_mode;
}

void PI_API pi_particle_cluster_set_appearance(ParticleCluster *cluster, EParticleAppearance appearance)
{
	cluster->appearance = appearance;
}

void PI_API pi_particle_cluster_set_depth_enable(ParticleCluster *cluster, PiBool depth_enable)
{
	cluster->depth_enable = depth_enable;
}
void PI_API pi_particle_cluster_set_depth_write_enable(ParticleCluster* cluster, PiBool depth_write_enable)
{
	cluster->depth_write_enable = depth_write_enable;
}

void PI_API pi_particle_cluster_get_entities(ParticleCluster *cluster, PiVector *dst)
{
	cluster->particle_get_entities_func(cluster, dst);
}

void PI_API pi_particle_cluster_set_global_alpha(ParticleCluster* cluster, float global_alpha)
{
	cluster->global_alpha = global_alpha;
}

void PI_API particle_cluster_set_max_count(ParticleCluster *cluster, uint count)
{
	uint i, size = pi_vector_size(cluster->active_list);
	for (i = count; i < size; ++i)
	{
		pi_vector_push(cluster->idle_list, pi_vector_pop(cluster->active_list));
	}
	cluster->max_count = count;
	
}

void PI_API pi_particle_cluster_set_instance(ParticleCluster* cluster, InstanceEntity* instance)
{
	cluster->instance_entity = instance;
}