
#include "chain_particles.h"
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

	char *FACING_CAMERA;

	char *U_Texture;
	char *U_TextureTileInfo;
	char *U_PrivateInfo;
	char *U_Time;

	char *DEPTH;

	char *U_Color;

	char *U_Width;
} ConstString;

typedef struct
{
	float step_time;
	EChainFacingType type;
	float width;
	PiVector3 *target;
	PiVector3 *target_offset;
	PiVector3 *start_offset;
} ChainParticleCluster;

typedef struct
{
	PiEntity *entity;
	PiMaterial *material;
	PiController *chain_controller;
	PiVector *pos_list;
	PiVector *offset_list;
} ChainParticle;

static ConstString s_const_str;

static void _init_const_str(void)
{
	if (!s_const_str.is_init)
	{
		s_const_str.TEXTURE = pi_conststr("TEXTURE");
		s_const_str.TILED = pi_conststr("TILED");
		s_const_str.BLEND = pi_conststr("BLEND");
		s_const_str.DISTORTION_EFFECT = pi_conststr("DISTORTION_EFFECT");

		s_const_str.FACING_CAMERA = pi_conststr("FACING_CAMERA");

		s_const_str.U_Texture = pi_conststr("u_Texture");
		s_const_str.U_TextureTileInfo = pi_conststr("u_TextureTileInfo");
		s_const_str.U_PrivateInfo = pi_conststr("u_PrivateInfo");
		s_const_str.U_Time = pi_conststr("u_Time");

		s_const_str.U_Color = pi_conststr("u_Color");

		s_const_str.U_Width = pi_conststr("u_Width");

		s_const_str.DEPTH = pi_conststr("DEPTH");

		s_const_str.is_init = TRUE;
	}
}

static void _cluster_update(CPUParticleCluster *cpu_impl, float tpf)
{
}

static void _config_common_material(ParticleCluster* cluster, PiMaterial* material, ChainParticleCluster* impl)
{
	pi_material_set_cull_mode(material, CM_NO);

	if (impl->type == ECFT_FACING_CAMERA)
	{
		pi_material_set_def(material, s_const_str.FACING_CAMERA, TRUE);
		pi_material_set_uniform(material, s_const_str.U_Width, UT_FLOAT, 1, &impl->width, TRUE);
	}
	if (cluster->appearance == EPA_FLUCTUATION)
	{
		pi_material_set_def(material, s_const_str.DISTORTION_EFFECT, TRUE);
		pi_material_set_depthwrite_enable(material, FALSE);
	}
	else
	{
		pi_material_set_depth_enable(material, cluster->depth_enable);
		pi_material_set_depthwrite_enable(material, cluster->depth_write_enable);
		pi_material_set_def(material, s_const_str.DEPTH, cluster->depth_write_enable);

		switch (cluster->blend_mode)
		{
		case EPBM_ALPHA:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
			break;

		case EPBM_ADDITIVE:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_ONE, BF_ZERO, BF_ONE);
			break;

		case EPBM_MODULATE:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_DST_ALPHA, BF_ZERO, BF_DST_COLOR, BF_ZERO);
			break;

		case EPBM_NORMAL:
			pi_material_set_blend(material, FALSE);
			pi_material_set_blend_factor(material, BF_ONE, BF_ZERO, BF_ONE, BF_ZERO);
			break;
		default:
			break;
		}
	}

	//TODO:纹理控制代码可以考虑抽离
	if (cluster->texture != NULL)
	{
		pi_material_set_def(material, s_const_str.TEXTURE, TRUE);
		pi_material_set_uniform(material, s_const_str.U_Texture, UT_SAMPLER_2D, 1, &cluster->ss_tex, FALSE);

		if (cluster->tile_info.x * cluster->tile_info.y > 1.0)
		{
			pi_material_set_def(material, s_const_str.TILED, TRUE);

			if (cluster->blend_enable)
			{
				pi_material_set_def(material, s_const_str.BLEND, TRUE);
			}
			pi_material_set_uniform(material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, FALSE);
		}
	}
}

static void _particle_init(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PiEntity *entity = pi_entity_new();
	ParticleCluster *cluster = cpu_impl->cluster;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;
	ChainParticle *p_impl = pi_new0(ChainParticle, 1);

	p_impl->material = pi_material_new("chain.vs", "particle.fs");
	_config_common_material(cluster, p_impl->material, impl);
	pi_material_set_uniform(p_impl->material, s_const_str.U_Color, UT_VEC4, 1, &particle->color, FALSE);
	pi_entity_set_material(entity, p_impl->material);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_PrivateInfo, UT_VEC2, 1, &particle->particle->other_data, FALSE, TRUE);

	pi_spatial_update(particle->spatial);

	p_impl->chain_controller = pi_chain_new(impl->type);
	pi_chain_set_width(p_impl->chain_controller, impl->width);
	p_impl->pos_list = pi_vector_new();
	p_impl->offset_list = pi_vector_new();
	pi_chain_set_step_points(p_impl->chain_controller, p_impl->pos_list, p_impl->offset_list);
	impl->start_offset = pi_new0(PiVector3, 1);
	impl->start_offset->x = 0;
	impl->start_offset->z = 0;
	impl->start_offset->y = 0;

	p_impl->entity = entity;
	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	ChainParticle *p_impl = (ChainParticle *)particle->impl;
	pi_material_free(p_impl->material);
	pi_entity_free(p_impl->entity);
	pi_chain_free(p_impl->chain_controller);
	pi_vector_free(p_impl->pos_list);
	pi_vector_free(p_impl->offset_list);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cpu_impl, CPUParticle *particle, float tpf)
{
	ChainParticle *p_impl = (ChainParticle *)particle->impl;
	PiEntity *entity = p_impl->entity;

	if (particle->particle->info.emitter_spatial)
	{
		pi_vec3_copy((PiVector3 *)pi_vector_get(p_impl->pos_list, 0), &particle->particle->info.emitter_spatial->world_translation);
	}

	pi_controller_update(p_impl->chain_controller, tpf);
	pi_controller_apply(p_impl->chain_controller, CAT_ENTITY, entity);
}

static void _particle_spawn(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	ChainParticle *p_impl = (ChainParticle *)particle->impl;
	ParticleCluster *cluster = cpu_impl->cluster;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;

	pi_vector_clear(p_impl->pos_list, FALSE);
	pi_vector_push(p_impl->pos_list, &particle->particle->info.emitter_pos);
	pi_vector_push(p_impl->pos_list, impl->target);

	pi_vector_clear(p_impl->offset_list, FALSE);
	pi_vector_push(p_impl->offset_list, impl->start_offset);
	pi_vector_push(p_impl->offset_list, impl->target_offset);

	pi_chain_reset(p_impl->chain_controller, 2, impl->step_time, 0);

	if (cluster->texture != NULL)
	{
		if (cluster->tile_info.x * cluster->tile_info.y > 1)
		{
			particle->particle->other_data.y = cluster->random_start ? pi_random_float(0, (float)(cluster->tile_info.x * cluster->tile_info.y)) : 0;
		}
	}
}

static void _particle_die(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PI_USE_PARAM(cpu_impl);
	PI_USE_PARAM(particle);
}

static void _particle_get_entities(ParticleCluster *cluster, PiVector *dst)
{
	uint i, size;
	size = pi_vector_size(cluster->active_list);
	pi_renderinfo_add_particle_num(size);

	for (i = 0; i < size; i++)
	{
		Particle *particle = (Particle *)pi_vector_get(cluster->active_list, i);
		ChainParticle *p_impl = (ChainParticle *)((CPUParticle *)particle->impl)->impl;
		pi_vector_push(dst, p_impl->entity);
	}
}

ParticleCluster *PI_API pi_particle_chain_cluster_create()
{
	ParticleCluster *cluster;
	ChainParticleCluster *impl = pi_new0(ChainParticleCluster, 1);
	impl->step_time = 0.5f;
	impl->type = ECFT_FACING_CAMERA;
	impl->width = 0.3f;

	_init_const_str();

	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, impl);

	return cluster;
}

void PI_API pi_particle_chain_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;

	particle_cpu_cluster_free(cluster);
	pi_free(impl->start_offset);
	pi_free(impl);
}

void PI_API pi_particle_chain_cluster_set_facing(ParticleCluster *cluster, EChainFacingType type)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;

	impl->type = type;
}

void PI_API pi_particle_chain_cluster_set_data(ParticleCluster *cluster, float width, float step_time)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;

	impl->width = width;
	impl->step_time = step_time;
}

void PI_API pi_particle_chain_cluster_set_target(ParticleCluster *cluster, PiVector3 *target, PiVector3 *offset)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	ChainParticleCluster *impl = (ChainParticleCluster *)cpu_impl->impl;

	impl->target = target;
	impl->target_offset = offset;
}
