
#include "quad_particles.h"
#include "cpu_particles.h"
#include <entity.h>
#include <renderinfo.h>
#include <pi_random.h>

static PiRenderMesh* instanceMesh[EOT_NUM] = { NULL };

/* 常量字符串 */
typedef struct
{
	PiBool is_init;

	char *TEXTURE;
	char *TILED;
	char *BLEND;
	char *DISTORTION_EFFECT;
	char *REVERSE;

	char *U_Texture;
	char *U_TextureTileInfo;
	char *U_TileStart;
	char *U_PrivateInfo;
	char *DEPTH;
	char *U_SceneColorTex;
	char *U_Color;
} ConstString;

typedef struct
{
	EORIGINType origin_type;
	PiController *billboard_controller;
	EFacingType billboard_type;
} QuadParticleCluster;

typedef struct
{
	PiEntity *entity;
	PiMaterial *material;
} QuadParticle;

static ConstString s_const_str;

static void _init_const_str(void)
{
	if (!s_const_str.is_init)
	{
		s_const_str.TEXTURE = pi_conststr("TEXTURE");
		s_const_str.TILED = pi_conststr("TILED");
		s_const_str.BLEND = pi_conststr("BLEND");
		s_const_str.DISTORTION_EFFECT = pi_conststr("DISTORTION_EFFECT");
		s_const_str.REVERSE = pi_conststr("REVERSE");

		s_const_str.U_Texture = pi_conststr("u_Texture");
		s_const_str.U_TextureTileInfo = pi_conststr("u_TextureTileInfo");
		s_const_str.U_TileStart = pi_conststr("u_TileStart");
		s_const_str.U_PrivateInfo = pi_conststr("u_PrivateInfo");

		s_const_str.U_Color = pi_conststr("u_Color");
		s_const_str.DEPTH = pi_conststr("DEPTH");
		s_const_str.U_SceneColorTex = pi_conststr("u_SceneColorTex");
		s_const_str.is_init = TRUE;
	}
}

static PiMesh *_create_quad_mesh(EORIGINType origin_type)
{
	float tableX[] = { 0, 0.5, -0.5, 0, 0.5, -0.5, 0, 0.5, -0.5 };
	float tableY[] = { 0, 0, 0, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5 };
	PiMesh* mesh;
	uint32 i;

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

	for (i = 0; i < 4; ++i)
	{
		pos[i * 3] += tableX[origin_type];
		pos[i * 3 + 1] += tableY[origin_type];
	}
	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, NULL, NULL, tcoord, 6, indices);
	pi_mesh_set_vertex(mesh, MAX_INSTANCE_NUM, FALSE, EVS_INSTANCE, 2, EVT_SHORT, EVU_STATIC_DRAW, pi_instance_entity_get_instance_data());
	return mesh;
}

static void _cluster_update(CPUParticleCluster *cpu_impl, float tpf)
{
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;
	pi_controller_update(impl->billboard_controller, tpf);
}

static void _config_common_material(ParticleCluster* cluster, PiMaterial* material)
{
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
		case EPBM_MODULATE_R:
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
		case EPBM_ADDITIVE_R:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_DST_ALPHA, BF_ONE, BF_ZERO, BF_ONE);
			pi_material_set_def(material, s_const_str.REVERSE, TRUE);
			break;
		case EPBM_NORMAL_R:
			pi_material_set_blend(material, TRUE);
			pi_material_set_blend_factor(material, BF_DST_ALPHA, BF_ONE, BF_ZERO, BF_ZERO);
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


		if (cluster->tile_info.x * cluster->tile_info.y> 1)
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
	uniform->name = pi_conststr("u_PrivateInfo");
	uniform->type = UT_VEC2;
	uniform->array_name = pi_conststr("u_PrivateInfo");
	uniform->value_size = sizeof(float) * 2;
	pi_vector_push(private_uniform, uniform);

	uniform = pi_new0(InstanceUniform, 1);
	uniform->name = pi_conststr("u_Color");
	uniform->type = UT_VEC4;
	uniform->array_name = pi_conststr("u_ColorArray");
	uniform->value_size = sizeof(float) * 4;
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
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;
	QuadParticle *p_impl = pi_new0(QuadParticle, 1);
	
	p_impl->material = pi_material_new("particle.vs", "particle.fs");

	_config_common_material(cluster, p_impl->material);
	pi_material_set_uniform(p_impl->material, s_const_str.U_Color, UT_VEC4, 1, &particle->color, FALSE);
	particle->particle->other_data.x = particle->particle->time;
	pi_material_set_uniform(p_impl->material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, FALSE);
	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_PrivateInfo, UT_VEC2, 1, &particle->particle->other_data, FALSE, TRUE);
	if (cluster->texture)
	{
		pi_material_set_uniform(p_impl->material, s_const_str.U_Texture, UT_SAMPLER_2D, 1, &cluster->ss_tex, FALSE);
	}
	if (instanceMesh[impl->origin_type] == NULL)
	{
		PiMesh *mesh = _create_quad_mesh(impl->origin_type);
		instanceMesh[impl->origin_type] = pi_rendermesh_new(mesh, TRUE);
	}

	if (cluster->instance_entity)
	{
		if (cluster->instance_entity->entity == NULL)
		{
			_particle_gen_instance_entity(cluster, instanceMesh[impl->origin_type]);
		}
		pi_entity_set_bind(entity, EBT_INSTANCE, cluster->instance_entity);
	}


	pi_entity_set_mesh(entity, instanceMesh[impl->origin_type]);
	pi_entity_set_material(entity, p_impl->material);
	pi_entity_set_reference_spatial(entity, particle->spatial);
	p_impl->entity = entity;
	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	QuadParticle *p_impl = (QuadParticle *)particle->impl;
	pi_material_free(p_impl->material);
	pi_entity_free(p_impl->entity);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cpu_impl, CPUParticle *particle, float tpf)
{
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;
	QuadParticle *p_impl = (QuadParticle *)particle->impl;
	PiEntity *entity = p_impl->entity;

	pi_controller_apply(impl->billboard_controller, CAT_ENTITY, entity);
	particle->particle->other_data.x = particle->particle->time;
	if (impl->billboard_type != EFT_FREE && particle->particle->info.hasRotation)
	{
		PiMatrix4 rotation_tmp; 
		PiVector3 translation_tmp, scale_tmp;
		PiMatrix4 *mat = pi_entity_get_world_matrix(entity);
		translation_tmp.x = mat->m[0][3];
		translation_tmp.y = mat->m[1][3];
		translation_tmp.z = mat->m[2][3];

		scale_tmp.x = mat->m[0][0] * mat->m[0][0] + mat->m[1][0] * mat->m[1][0] + mat->m[2][0] * mat->m[2][0];
		scale_tmp.y = mat->m[0][1] * mat->m[0][1] + mat->m[1][1] * mat->m[1][1] + mat->m[2][1] * mat->m[2][1];
		scale_tmp.z = mat->m[0][2] * mat->m[0][2] + mat->m[1][2] * mat->m[1][2] + mat->m[2][2] * mat->m[2][2];
		if (scale_tmp.x < 0.000001 || scale_tmp.y < 0.000001 || scale_tmp.z < 0.000001)
		{
			return;
		}
		scale_tmp.x = pi_math_sqrt(scale_tmp.x);
		scale_tmp.y = pi_math_sqrt(scale_tmp.y);
		scale_tmp.z = pi_math_sqrt(scale_tmp.z);

		mat->m[0][3] = 0;
		mat->m[1][3] = 0;
		mat->m[2][3] = 0;

		mat->m[0][0] /= scale_tmp.x;
		mat->m[1][0] /= scale_tmp.x;
		mat->m[2][0] /= scale_tmp.x;
		mat->m[0][1] /= scale_tmp.y;
		mat->m[1][1] /= scale_tmp.y;
		mat->m[2][1] /= scale_tmp.y;
		mat->m[0][2] /= scale_tmp.z;
		mat->m[1][2] /= scale_tmp.z;
		mat->m[2][2] /= scale_tmp.z;

		pi_mat4_build_rotate(&rotation_tmp, &particle->rotation);
		rotation_tmp.m[0][0] *= scale_tmp.x;
		rotation_tmp.m[1][0] *= scale_tmp.x;
		rotation_tmp.m[2][0] *= scale_tmp.x;
		rotation_tmp.m[0][1] *= scale_tmp.y;
		rotation_tmp.m[1][1] *= scale_tmp.y;
		rotation_tmp.m[2][1] *= scale_tmp.y;
		rotation_tmp.m[0][2] *= scale_tmp.z;
		rotation_tmp.m[1][2] *= scale_tmp.z;
		rotation_tmp.m[2][2] *= scale_tmp.z;

		pi_mat4_mul(mat, mat, &rotation_tmp);

		mat->m[0][3] = translation_tmp.x;
		mat->m[1][3] = translation_tmp.y;
		mat->m[2][3] = translation_tmp.z;
	}
}

static void _particle_spawn(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	ParticleCluster *cluster = cpu_impl->cluster;

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
		QuadParticle *p_impl = (QuadParticle *)((CPUParticle *)particle->impl)->impl;
		pi_vector_push(dst, p_impl->entity);
	}
}

ParticleCluster *PI_API pi_particle_quad_cluster_create()
{
	ParticleCluster *cluster;
	QuadParticleCluster *impl = pi_new0(QuadParticleCluster, 1);

	_init_const_str();

	impl->billboard_controller = pi_billboard_new();

	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, impl);

	return cluster;
}

void PI_API pi_particle_quad_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;

	particle_cpu_cluster_free(cluster);

	pi_billboard_free(impl->billboard_controller);

	pi_free(impl);
}

void PI_API pi_particle_quad_cluster_set_facing(ParticleCluster *cluster, PiCamera *cam, EFacingType type)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;
	pi_billboard_set_camera(impl->billboard_controller, cam);
	pi_billboard_set_facing(impl->billboard_controller, type);
	impl->billboard_type = type;
}

void PI_API pi_particle_quad_cluster_set_origin_type(ParticleCluster *cluster, EORIGINType origin_type)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	QuadParticleCluster *impl = (QuadParticleCluster *)cpu_impl->impl;
	impl->origin_type = origin_type;
}