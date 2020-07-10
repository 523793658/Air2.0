
#include "stretched_quad_particles.h"
#include "cpu_particles.h"
#include <entity.h>
#include <renderinfo.h>
#include <pi_random.h>

static PiRenderMesh* instance_mesh = NULL;


/* 常量字符串 */
typedef struct
{
	PiBool is_init;

	char *TEXTURE;
	char *TILED;
	char *BLEND;
	char *DISTORTION_EFFECT;

	char *STRETCHED;

	char *U_Texture;
	char *U_TextureTileInfo;
	char *U_PrivateInfo;
	char *U_Time;

	char *U_Color;
	char *U_ColorArray;
	char *U_SceneColorTex;
	char *U_Alpha;

	char *U_WorldPos;
	char *U_Dir;
	char *U_Scale;
	char *DEPTH;
	char *REVERSE;
} ConstString;

typedef struct
{
	PiEntity *entity;
	PiMaterial *material;
	PiVector3 last_pos;
	PiVector3 direction;
	float quad_scale[4];
} StretchedQuadParticle;

static ConstString s_const_str;

static void _init_const_str(void)
{
	if (!s_const_str.is_init)
	{
		s_const_str.TEXTURE = pi_conststr("TEXTURE");
		s_const_str.TILED = pi_conststr("TILED");
		s_const_str.BLEND = pi_conststr("BLEND");
		s_const_str.DISTORTION_EFFECT = pi_conststr("DISTORTION_EFFECT");
		s_const_str.STRETCHED = pi_conststr("STRETCHED");
		s_const_str.REVERSE = pi_conststr("REVERSE");


		s_const_str.U_Texture = pi_conststr("u_Texture");
		s_const_str.U_TextureTileInfo = pi_conststr("u_TextureTileInfo");
		s_const_str.U_PrivateInfo = pi_conststr("u_PrivateInfo");
		s_const_str.U_Time = pi_conststr("u_Time");

		s_const_str.U_Color = pi_conststr("u_Color");
		s_const_str.U_ColorArray = pi_conststr("u_ColorArray");
		s_const_str.U_Alpha = pi_conststr("u_Alpha");
		s_const_str.U_SceneColorTex = pi_conststr("u_SceneColorTex");
		s_const_str.U_WorldPos = pi_conststr("u_WorldPos");
		s_const_str.U_Dir = pi_conststr("u_Dir");
		s_const_str.U_Scale = pi_conststr("u_Scale");
		s_const_str.DEPTH = pi_conststr("DEPTH");
		s_const_str.is_init = TRUE;
	}
}

static PiMesh *_create_quad_mesh()
{
	PiMesh* mesh;
	float tcoord[4 * 2] =
	{
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	float pos[4 * 3] =
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};

	uint32 indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, NULL, NULL, tcoord, 6, indices);
	pi_mesh_set_vertex(mesh, MAX_INSTANCE_NUM, FALSE, EVS_INSTANCE, 2, EVT_SHORT, EVU_STATIC_DRAW, pi_instance_entity_get_instance_data());
	return mesh;
}

static void _cluster_update(CPUParticleCluster *cpu_impl, float tpf)
{
}

static void _config_common_material(ParticleCluster* cluster, PiMaterial* material)
{
	pi_material_set_def(material, s_const_str.STRETCHED, TRUE);

	pi_material_set_cull_mode(material, CM_NO);
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
		case EPBM_ALPHA_R:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_DST_ALPHA, BF_ONE, BF_DST_ALPHA, BF_ZERO);
			pi_material_set_def(material, s_const_str.REVERSE, TRUE);
			break;

		default:
			break;
		}
	}

	//TODO:纹理控制代码可以考虑抽离
	if (cluster->texture != NULL)
	{
		pi_material_set_def(material, s_const_str.TEXTURE, TRUE);
		if (cluster->tile_info.x * cluster->tile_info.y > 1)
		{
			pi_material_set_def(material, s_const_str.TILED, TRUE);

			if (cluster->blend_enable)
			{
				pi_material_set_def(material, s_const_str.BLEND, TRUE);
			}
		}
	}
}

static void _particle_gen_instance_entity(ParticleCluster* cluster, PiRenderMesh* mesh)
{
	PiEntity* entity = pi_entity_new();
	PiMaterial* material = pi_material_new("particle_instance.vs", "particle_instance.fs");
	PiVector* private_uniform = pi_vector_new();
	PiVector* public_uniform = NULL;
	InstanceUniform* uniform = pi_new0(InstanceUniform, 1);
	_config_common_material(cluster, material);
	pi_material_set_uniform(material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, TRUE);


	if (cluster->texture != NULL)
	{

		public_uniform = pi_vector_new();
		uniform = pi_new(InstanceUniform, 1);
		uniform->name = s_const_str.U_Texture;
		uniform->array_name = s_const_str.U_Texture;
		uniform->type = UT_SAMPLER_2D;
		uniform->value_size = sizeof(SamplerState);
		pi_vector_push(public_uniform, uniform);

	}

	uniform = pi_new(InstanceUniform, 1);
	uniform->name = s_const_str.U_PrivateInfo;
	uniform->type = UT_VEC2;
	uniform->array_name = s_const_str.U_PrivateInfo;
	uniform->value_size = sizeof(float) * 2;
	pi_vector_push(private_uniform, uniform);

	uniform = pi_new0(InstanceUniform, 1);
	uniform->name = s_const_str.U_Color;
	uniform->array_name = s_const_str.U_ColorArray;
	uniform->type = UT_VEC4;
	uniform->value_size = sizeof(float) * 4;
	pi_vector_push(private_uniform, uniform);

	uniform = pi_new0(InstanceUniform, 1);
	uniform->name = s_const_str.U_Dir;
	uniform->array_name = s_const_str.U_Dir;
	uniform->type = UT_VEC3;
	uniform->value_size = sizeof(float) * 3;
	pi_vector_push(private_uniform, uniform);

	uniform = pi_new0(InstanceUniform, 1);
	uniform->name = s_const_str.U_Scale;
	uniform->array_name = s_const_str.U_Scale;
	uniform->type = UT_VEC2;
	uniform->value_size = sizeof(float) * 2;
	pi_vector_push(private_uniform, uniform);


	if (cluster->appearance == EPA_FLUCTUATION)
	{
		if (public_uniform == NULL)
		{
			public_uniform = pi_vector_new();
		}

		uniform = pi_new(InstanceUniform, 1);
		uniform->name = s_const_str.U_SceneColorTex;
		uniform->array_name = s_const_str.U_SceneColorTex;
		uniform->type = UT_SAMPLER_2D;
		uniform->value_size = sizeof(SamplerState);
		pi_vector_push(public_uniform, uniform);
	}

	pi_instance_entity_set_render_entity(cluster->instance_entity, entity, private_uniform, public_uniform, "u_WorldMatrixArray");
	pi_entity_set_material(entity, material);
	pi_entity_set_mesh(entity, mesh);
}

static void _particle_init(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PiEntity *entity = pi_entity_new();
	ParticleCluster *cluster = cpu_impl->cluster;
	StretchedQuadParticle *p_impl = pi_new0(StretchedQuadParticle, 1);

	p_impl->material = pi_material_new("particle.vs", "particle.fs");
	_config_common_material(cluster, p_impl->material);
	pi_material_set_uniform(p_impl->material, s_const_str.U_Color, UT_VEC4, 1, &particle->color, FALSE);
	//TODO:纹理控制代码可以考虑抽离
	pi_material_set_uniform(p_impl->material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, FALSE);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_PrivateInfo, UT_VEC2, 1, &particle->particle->other_data, FALSE, TRUE);

	if (instance_mesh == NULL)
	{
		PiMesh *mesh = _create_quad_mesh();
		instance_mesh = pi_rendermesh_new(mesh, TRUE);
	}

	if (cluster->texture)
	{
		pi_material_set_uniform(p_impl->material, s_const_str.U_Texture, UT_SAMPLER_2D, 1, &cluster->ss_tex, FALSE);
	}

	pi_entity_set_mesh(entity, instance_mesh);
	pi_entity_set_material(entity, p_impl->material);
	pi_entity_set_reference_spatial(entity, particle->spatial);

	if (cluster->instance_entity)
	{
		if (cluster->instance_entity->entity == NULL)
		{
			_particle_gen_instance_entity(cluster, instance_mesh);
		}
		//pi_entity_set_bind(entity, EBT_INSTANCE, cluster->instance_entity);
	}


	p_impl->entity = entity;
	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	StretchedQuadParticle *p_impl = (StretchedQuadParticle *)particle->impl;
	pi_material_free(p_impl->material);
	pi_entity_free(p_impl->entity);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cpu_impl, CPUParticle *particle, float tpf)
{
	StretchedQuadParticle *p_impl = (StretchedQuadParticle *)particle->impl;
	PiVector3 *pos, *scaling;

	scaling = pi_spatial_get_world_scaling(particle->spatial);
	p_impl->quad_scale[0] = scaling->x;
	p_impl->quad_scale[1] = scaling->y;
	pos = pi_spatial_get_world_translation(particle->spatial);
	pi_vec3_sub(&p_impl->direction, pos, &p_impl->last_pos);
	pi_vec3_normalise(&p_impl->direction, &p_impl->direction);
	pi_vec3_copy(&p_impl->last_pos, pos);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_WorldPos, UT_VEC3, 1, pos, FALSE, TRUE);

}

static void _particle_spawn(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PiVector3 *pos;
	StretchedQuadParticle *p_impl = (StretchedQuadParticle *)particle->impl;
	ParticleCluster *cluster = cpu_impl->cluster;

	pos = pi_spatial_get_world_translation(particle->spatial);

	pi_vec3_copy(&p_impl->last_pos, pos);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_Dir, UT_VEC3, 1, &p_impl->direction, FALSE, TRUE);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_Scale, UT_VEC2, 1, p_impl->quad_scale, FALSE, TRUE);
	if (cluster->texture != NULL)
	{
		if (cluster->tile_info.x * cluster->tile_info.y> 1)
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
		StretchedQuadParticle *p_impl = (StretchedQuadParticle *)((CPUParticle *)particle->impl)->impl;
		pi_vector_push(dst, p_impl->entity);
	}
}

ParticleCluster *PI_API pi_particle_stretched_quad_cluster_create()
{
	ParticleCluster *cluster;
	_init_const_str();
	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, NULL);

	return cluster;
}

void PI_API pi_particle_stretched_quad_cluster_free(ParticleCluster *cluster)
{
	particle_cpu_cluster_free(cluster);
}
