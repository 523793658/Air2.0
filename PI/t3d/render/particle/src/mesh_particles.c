
#include "mesh_particles.h"
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

	char *U_Texture;
	char *U_TextureTileInfo;
	char *U_PrivateInfo;
	char *U_Time;
	char *DEPTH;

	char *U_Color;
	char *U_ColorArray;
	char *U_SceneColorTex;
} ConstString;

typedef struct
{
	PiRenderMesh *mesh;
} MeshParticleCluster;

typedef struct
{
	PiEntity *entity;
	PiMaterial *material;
} MeshParticle;

static ConstString s_const_str;

static void _init_const_str(void)
{
	if (!s_const_str.is_init)
	{
		s_const_str.TEXTURE = pi_conststr("TEXTURE");
		s_const_str.TILED = pi_conststr("TILED");
		s_const_str.BLEND = pi_conststr("BLEND");
		s_const_str.DISTORTION_EFFECT = pi_conststr("DISTORTION_EFFECT");

		s_const_str.U_Texture = pi_conststr("u_Texture");
		s_const_str.U_TextureTileInfo = pi_conststr("u_TextureTileInfo");
		s_const_str.U_PrivateInfo = pi_conststr("u_PrivateInfo");


		s_const_str.U_Time = pi_conststr("u_Time");
		s_const_str.DEPTH = pi_conststr("DEPTH");
		s_const_str.U_Color = pi_conststr("u_Color");
		s_const_str.U_ColorArray = pi_conststr("u_ColorArray");
		s_const_str.U_SceneColorTex = pi_conststr("u_SceneColorTex");
		s_const_str.is_init = TRUE;
	}
}

static void _cluster_update(CPUParticleCluster *cpu_impl, float tpf)
{
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

static void _gen_instance_entity(ParticleCluster* cluster, PiRenderMesh* mesh)
{
	PiEntity* entity = pi_entity_new();
	PiMaterial* material = pi_material_new("particle_instance.vs", "particle_instance.fs");
	PiVector* private_uniform = pi_vector_new();
	PiVector* public_uniform = NULL;
	InstanceUniform* uniform = pi_new0(InstanceUniform, 1);
	_config_common_material(cluster, material);
	pi_material_set_uniform(material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, TRUE);
	
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
	pi_mesh_set_vertex(mesh->mesh, MAX_INSTANCE_NUM, FALSE, EVS_INSTANCE, 2, EVT_SHORT, EVU_STATIC_DRAW, pi_instance_entity_get_instance_data());
	pi_rendermesh_update(mesh);
	pi_entity_set_mesh(entity, mesh);

}

static void _particle_init(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	PiEntity *entity = pi_entity_new();
	ParticleCluster *cluster = cpu_impl->cluster;
	MeshParticleCluster *impl = (MeshParticleCluster *)cpu_impl->impl;
	MeshParticle *p_impl = pi_new0(MeshParticle, 1);

	PI_ASSERT(impl->mesh != NULL, "mesh particle has no mesh");

	p_impl->material = pi_material_new("particle.vs", "particle.fs");
	_config_common_material(cluster, p_impl->material);
	pi_material_set_uniform(p_impl->material, s_const_str.U_Color, UT_VEC4, 1, &particle->color, FALSE);
	pi_material_set_uniform(p_impl->material, s_const_str.U_TextureTileInfo, UT_VEC4, 1, &cluster->tile_info, FALSE);
	//TODO:纹理控制代码可以考虑抽离
	if (cluster->texture)
	{
		pi_material_set_uniform(p_impl->material, s_const_str.U_Texture, UT_SAMPLER_2D, 1, &cluster->ss_tex, FALSE);
	}

	pi_material_set_uniform_pack_flag(p_impl->material, s_const_str.U_PrivateInfo, UT_VEC2, 1, &particle->particle->other_data, FALSE, TRUE);
	if (cluster->instance_entity)
	{
		if (cluster->instance_entity->entity == NULL)
		{
			_gen_instance_entity(cluster, impl->mesh);
		}
		pi_entity_set_bind(entity, EBT_INSTANCE, cluster->instance_entity);
	}
	pi_entity_set_mesh(entity, impl->mesh);
	pi_entity_set_material(entity, p_impl->material);
	pi_entity_set_reference_spatial(entity, particle->spatial);
	p_impl->entity = entity;
	particle->impl = p_impl;
}

static void _particle_destroy(CPUParticleCluster *cpu_impl, CPUParticle *particle)
{
	MeshParticle *p_impl = (MeshParticle *)particle->impl;
	pi_material_free(p_impl->material);
	pi_entity_free(p_impl->entity);
	pi_free(p_impl);
}

static void _particle_update(CPUParticleCluster *cpu_impl, CPUParticle *particle, float tpf)
{
	PiQuaternion rotation;
	MeshParticle *p_impl = (MeshParticle *)particle->impl;
	PiEntity *entity = p_impl->entity;
	PiVector3 *svec = pi_spatial_get_world_scaling(particle->spatial);
	PiVector3 *tvec = pi_spatial_get_world_translation(particle->spatial);
	particle->particle->other_data.x = particle->particle->time;
	PI_USE_PARAM(cpu_impl);
	pi_quat_mul(&rotation, pi_spatial_get_world_rotation(particle->spatial), &particle->rotation);
	pi_spatial_set_local_translation(entity->spatial, tvec->x, tvec->y, tvec->z);
	pi_spatial_set_local_rotation(entity->spatial, rotation.w, rotation.x, rotation.y, rotation.z);
	pi_spatial_set_local_scaling(entity->spatial, svec->x, svec->y, svec->z);
	pi_spatial_update(entity->spatial);
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
		MeshParticle *p_impl = (MeshParticle *)((CPUParticle *)particle->impl)->impl;
		pi_vector_push(dst, p_impl->entity);
	}
}

ParticleCluster *PI_API pi_particle_mesh_cluster_create()
{
	ParticleCluster *cluster;
	MeshParticleCluster *impl = pi_new0(MeshParticleCluster, 1);

	_init_const_str();

	cluster = particle_cpu_cluster_create(_cluster_update, _particle_init, _particle_destroy, _particle_update, _particle_spawn, _particle_die, _particle_get_entities, impl);

	return cluster;
}

void PI_API pi_particle_mesh_cluster_free(ParticleCluster *cluster)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	MeshParticleCluster *impl = (MeshParticleCluster *)cpu_impl->impl;

	particle_cpu_cluster_free(cluster);

	pi_free(impl);
}

void PI_API pi_particle_mesh_cluster_set_mesh(ParticleCluster *cluster, PiRenderMesh *mesh)
{
	CPUParticleCluster *cpu_impl = (CPUParticleCluster *)cluster->impl;
	MeshParticleCluster *impl = (MeshParticleCluster *)cpu_impl->impl;
	impl->mesh = mesh;
}
