
#include "tail_particles.h"
#include "cpu_particles.h"
#include <entity.h>
#include <renderinfo.h>
#include <pi_random.h>

/* 常量字符串 */
typedef struct
{
	PiBool is_init;

	char *TEXTURE;
	char *TILED;
	char *BLEND;
	char *DISTORTION_EFFECT;

	char *TAIL;
	char *FACING_CAMERA;
	char *DEPTH;
	char *U_Texture;
	char *U_TextureTileInfo;
	char *U_PrivateInfo;
	char *U_Time;

	char *U_Color;

	char *U_Alpha;

	char *U_Width;
} ConstString;

typedef struct
{
	ETailType type;
	uint sample_step;
	float width;
	float life_time;
	PiBool head_follow;
} TailParticleCluster;

typedef struct
{
	PiEntity *entity;
	PiMaterial *material;
	PiController *tail_controller;
} TailParticle;

static ConstString s_const_str;

static void _init_const_str(void)
{
	if (!s_const_str.is_init)
	{
		s_const_str.TEXTURE = pi_conststr("TEXTURE");
		s_const_str.TILED = pi_conststr("TILED");
		s_const_str.BLEND = pi_conststr("BLEND");
		s_const_str.DISTORTION_EFFECT = pi_conststr("DISTORTION_EFFECT");
		s_const_str.DEPTH = pi_conststr("DEPTH");
		s_const_str.TAIL = pi_conststr("TAIL");
		s_const_str.FACING_CAMERA = pi_conststr("FACING_CAMERA");

		s_const_str.U_Texture = pi_conststr("u_Texture");
		s_const_str.U_TextureTileInfo = pi_conststr("u_TextureTileInfo");
		s_const_str.U_PrivateInfo = pi_conststr("u_PrivateInfo");
		s_const_str.U_Time = pi_conststr("u_Time");

		s_const_str.U_Color = pi_conststr("u_Color");

		s_const_str.U_Alpha = pi_conststr("u_Alpha");

		s_const_str.U_Width = pi_conststr("u_Width");

		s_const_str.is_init = TRUE;
	}
}

static void _cluster_update(CPUParticleCluster *cpu_impl, float tpf)
{
}

static void _particle_init(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PiEntity *entity = pi_entity_new();
	ParticleCluster *cluster = cpu_impl->cluster;
	TailParticleCluster *impl = (TailParticleCluster *)cpu_impl->impl;
	TailParticle *p_impl = pi_new0(TailParticle, 1);

	p_impl->material = pi_material_new("tail.vs", "particle.fs");

	pi_material_set_def(p_impl->material, s_const_str.TAIL, TRUE);

	pi_material_set_cull_mode(p_impl->material, CM_NO);

	if (impl->type == ETT_RIBBON_FACING_CAMERA)
	{
		pi_material_set_def(p_impl->material, s_const_str.FACING_CAMERA, TRUE);
		pi_material_set_uniform(p_impl->material, s_const_str.U_Width, UT_FLOAT, 1, &impl->width, FALSE);
	}
	pi_material_set_uniform(p_impl->material, s_const_str.U_Color, UT_VEC4, 1, &particle->color, FALSE);
	if (cluster->appearance == EPA_FLUCTUATION)
	{
		pi_material_set_def(p_impl->material, s_const_str.DISTORTION_EFFECT, TRUE);
		pi_material_set_depthwrite_enable(p_impl->material, FALSE);
	}
	else
	{

		pi_material_set_depth_enable(p_impl->material, cluster->depth_enable);
		pi_material_set_depthwrite_enable(p_impl->material, cluster->depth_write_enable);
		pi_material_set_def(p_impl->material, s_const_str.DEPTH, cluster->depth_write_enable);

		switch (cluster->blend_mode)
		{
		case EPBM_ALPHA:
			pi_material_set_blend(p_impl->material, TRUE);
			pi_material_set_blend_factor(p_impl->material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
			break;
		case EPBM_ADDITIVE:
			pi_material_set_blend(p_impl->material, TRUE);
			pi_material_set_blend_factor(p_impl->material, BF_SRC_ALPHA, BF_ONE, BF_ZERO, BF_ONE);
			break;
		case EPBM_MODULATE:
			pi_material_set_blend(p_impl->material, TRUE);
			pi_material_set_blend_factor(p_impl->material, BF_DST_ALPHA, BF_ZERO, BF_DST_COLOR, BF_ZERO);
			break;
		case EPBM_NORMAL:
			pi_material_set_blend(p_impl->material, FALSE);
			pi_material_set_blend_factor(p_impl->material, BF_ONE, BF_ZERO, BF_ONE, BF_ZERO);
			break;
		default:
			break;
		}
	}

	//TODO:纹理控制代码可以考虑抽离
	if (cluster->texture != NULL)
	{
		pi_material_set_def(p_impl->material, s_const_str.TEXTURE, TRUE);
		pi_material_set_uniform(p_impl->material, s_const_str.U_Texture, UT_SAMPLER_2D, 1, &cluster->ss_tex, FALSE);

		if (cluster->tile_info.x * cluster->tile_info.y> 1)
		{
			pi_material_set_def(p_impl->material, s_const_str.TILED, TRUE);

			if (cluster->blend_enable)
			{
				pi_material_set_def(p_impl->material, s_const_str.BLEND, TRUE);
			}

			pi_material_set_uniform(p_impl->material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, FALSE);
			pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_PrivateInfo, UT_VEC2, 1, &particle->particle->other_data, FALSE, TRUE);
		}
	}

	pi_entity_set_material(entity, p_impl->material);

	p_impl->tail_controller = pi_tail_new(impl->type, impl->sample_step, impl->width, impl->life_time, impl->head_follow);
	pi_tail_set_spatial(p_impl->tail_controller, particle->spatial);

	p_impl->entity = entity;
	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	TailParticle *p_impl = (TailParticle *)particle->impl;
	pi_material_free(p_impl->material);
	pi_entity_free(p_impl->entity);
	pi_tail_free(p_impl->tail_controller);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cpu_impl, CPUParticle *particle, float tpf)
{
	TailParticle *p_impl = (TailParticle *)particle->impl;
	PiEntity *entity = p_impl->entity;

	pi_controller_update(p_impl->tail_controller, tpf);
	pi_controller_apply(p_impl->tail_controller, CAT_ENTITY, entity);
}

static void _particle_spawn(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	TailParticle *p_impl = (TailParticle *)particle->impl;
	ParticleCluster *cluster = cpu_impl->cluster;
	TailParticleCluster *impl = (TailParticleCluster *)cpu_impl->impl;

	pi_tail_reset(p_impl->tail_controller);

	if (cluster->texture != NULL)
	{
		if (cluster->tile_info.x * cluster->tile_info.y> 1)
		{
			particle->particle->other_data.y = cluster->random_start ? pi_random_float(0, (float)(cluster->tile_info.x * cluster->tile_info.y)) : 0;
		}
	}

	if (particle->particle->info.emitter_spatial && !impl->head_follow)
	{
		pi_node_attach_child(particle->particle->info.emitter_spatial, p_impl->entity->spatial);
		pi_spatial_update(p_impl->entity->spatial);
	}
}

static void _particle_die(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	TailParticle *p_impl = (TailParticle *)particle->impl;

	pi_spatial_detach_from_parent(p_impl->entity->spatial);
}

static void _particle_get_entities(ParticleCluster *cluster, PiVector *dst)
{
	uint i, size;
	size = pi_vector_size(cluster->active_list);
	pi_renderinfo_add_particle_num(size);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_get(cluster->active_list, i);
		TailParticle *p_impl = (TailParticle *)((CPUParticle *)particle->impl)->impl;
		pi_vector_push(dst, p_impl->entity);
	}
}

ParticleCluster *PI_API pi_particle_tail_cluster_create()
{
	ParticleCluster *cluster;
	TailParticleCluster *impl = pi_new0(TailParticleCluster, 1);
	impl->type = ETT_LINE;

	_init_const_str();

	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, impl);

	return cluster;
}

void PI_API pi_particle_tail_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	TailParticleCluster *impl = (TailParticleCluster *)cpu_impl->impl;
	particle_cpu_cluster_free(cluster);
	pi_free(impl);
}

void PI_API pi_particle_tail_cluster_set_type(ParticleCluster *cluster, ETailType type)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	TailParticleCluster *impl = (TailParticleCluster *)cpu_impl->impl;
	impl->type = type;
}

void PI_API pi_particle_tail_cluster_set_data(ParticleCluster *cluster, uint sample_step, float width, float life_time, PiBool head_follow)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	TailParticleCluster *impl = (TailParticleCluster *)cpu_impl->impl;
	impl->sample_step = sample_step;
	impl->width = width;
	impl->life_time = life_time;
	impl->head_follow = head_follow;
}