#include <entity.h>
#include <rendersystem.h>
#include <renderinfo.h>
#include "renderwrap.h"
#include "instance_entity.h"
#include "instance_manager.h"

PiEntity *PI_API pi_entity_new(void)
{
	PiEntity *entity = pi_new0(PiEntity, 1);
	entity->spatial = pi_spatial_geometry_create();
	return entity;
}

void PI_API pi_entity_free(PiEntity *entity)
{
	if (entity != NULL)
	{
		pi_spatial_destroy(entity->spatial);
		if (entity->skinedData)
		{
			if (entity->skinedData->renderMesh)
			{
				pi_rendermesh_free(entity->skinedData->renderMesh);
			}
			if (entity->skinedData->renderData)
			{
				pi_renderdata_free(entity->skinedData->renderData);
			}
			if (entity->skinedData->mesh)
			{
				pi_free(entity->skinedData->mesh);
			}
			pi_free(entity->skinedData);
		}
		pi_free(entity);
	}
}

void PI_API pi_entity_set_mesh(PiEntity *entity, PiRenderMesh *mesh)
{
	entity->mesh = mesh;
	//规定：挂靠到note上的entity不影响node节点的aabb大小
	if (mesh->mesh)
	{
		pi_geometry_set_local_aabb(entity->spatial, pi_mesh_get_aabb(mesh->mesh));
	}
}

void PI_API pi_entity_set_material(PiEntity *entity, PiMaterial *material)
{
	entity->material = material;
}

PiMatrix4 *PI_API pi_entity_get_world_matrix(PiEntity *entity)
{
	if (entity->reference_spatial)
	{
		return &entity->reference_spatial->world_transform_matrix;
	}
	return &entity->spatial->world_transform_matrix;
}

PiSpatial *PI_API pi_entity_get_spatial(PiEntity *entity)
{
	return entity->spatial;
}

void PI_API pi_entity_set_reference_spatial(PiEntity* entity, PiSpatial* spatial)
{
	entity->reference_spatial = spatial;
}

PiAABBBox* PI_API pi_entity_get_world_aabb(PiEntity* entity)
{
	if (entity->reference_spatial)
	{
		return pi_spatial_get_world_aabb(entity->reference_spatial);
	}
	else
	{
		return pi_spatial_get_world_aabb(entity->spatial);
	}
}

void PI_API pi_entity_draw(PiEntity *entity)
{
	PiBool is_draw = TRUE;
	PiRenderSystem *sys = pi_rendersystem_get_instance();
	if (entity->hws_required > 0)
	{
		return;
	}
	sys->gdv.wvp_mask &= ~WVP_NORMAL;
	sys->gdv.wvp_mask &= ~WVP_WORLD_VIEW;
	sys->gdv.wvp_mask &= ~WVP_VIEW_NORMAL;
	sys->gdv.wvp_mask &= ~WVP_WORLD_VIEW_PROJ;
	sys->gdv.g_world = pi_entity_get_world_matrix(entity);
	sys->gdv.g_alpha_cull_off = entity->material->cull_off;

	is_draw = pi_material_update(entity->material);

	if (is_draw)
	{
		pi_rendersystem_draw(entity, 0);
	}
	else
	{
		const char *name = pi_renderinfo_get_current_renderer_name();
		pi_log_print(LOG_WARNING, "ignore entity because updating material failed, renderer's name = %s, vs_key = %s, fs_key = %s\n", name, entity->material->vs_key, entity->material->fs_key);
	}
}

void PI_API pi_entity_draw_instance(PiEntity *entity, uint n)
{
	PiBool is_draw = TRUE;
	PiRenderSystem *sys = pi_rendersystem_get_instance();
	sys->gdv.wvp_mask &= ~WVP_NORMAL;
	sys->gdv.wvp_mask &= ~WVP_WORLD_VIEW;
	sys->gdv.wvp_mask &= ~WVP_VIEW_NORMAL;
	sys->gdv.wvp_mask &= ~WVP_WORLD_VIEW_PROJ;
	sys->gdv.g_world = pi_entity_get_world_matrix(entity);
	sys->gdv.g_alpha_cull_off = entity->material->cull_off;

	is_draw = pi_material_update(entity->material);

	if (is_draw)
	{
		pi_rendersystem_draw(entity, n);
	}
	else
	{
		const char *name = pi_renderinfo_get_current_renderer_name();
		pi_log_print(LOG_WARNING, "ignore entity because updating material failed, renderer's name = %s, vs_key = %s, fs_key = %s\n", name, entity->material->vs_key, entity->material->fs_key);
	}
}

void PI_API pi_entity_draw_list_back(PiVector *entity_list)
{
	int i, num;
	InstanceCache* cache = pi_instance_manager_get_cache();

	num = pi_vector_size(entity_list);

	for (i = num - 1; i >= 0; i--)
	{
		PiEntity *entity = (PiEntity *)pi_vector_get(entity_list, i);
		InstanceEntity* instance = pi_entity_get_bind(entity, EBT_INSTANCE);
		if (instance)
		{
			pi_vector_push(instance->subEntity, entity);
			if (!pi_hash_lookup(cache->instance_map, (void*)instance->id, NULL))
			{
				pi_hash_insert(cache->instance_map, (void*)instance->id, instance);
				pi_vector_push(cache->instance_list, instance);
			}
		}
		else
		{
			pi_entity_draw(entity);
		}
	}

	num = pi_vector_size(cache->instance_list);
	for (i = 0; i < num; i++)
	{
		InstanceEntity* instance = pi_vector_get(cache->instance_list, i);
		pi_instance_entity_draw(instance);
	}
	pi_vector_clear(cache->instance_list, FALSE);
	pi_hash_clear(cache->instance_map, FALSE);
}

void PI_API pi_entity_draw_list(PiVector *entity_list)
{
	uint i, num;
	InstanceCache* cache = pi_instance_manager_get_cache();

	num = pi_vector_size(entity_list);

	for (i = 0; i < num; i++)
	{
		PiEntity *entity = (PiEntity *)pi_vector_get(entity_list, i);
		InstanceEntity* instance = pi_entity_get_bind(entity, EBT_INSTANCE);
		if (instance)
		{
			pi_vector_push(instance->subEntity, entity);
			if (!pi_hash_lookup(cache->instance_map, (void*)instance->id, NULL))
			{
				pi_hash_insert(cache->instance_map, (void*)instance->id, instance);
				pi_vector_push(cache->instance_list, instance);
			}
		}
		else
		{
			pi_entity_draw(entity);
		}
	}

	num = pi_vector_size(cache->instance_list);
	for (i = 0; i < num; i++)
	{
		InstanceEntity* instance = pi_vector_get(cache->instance_list, i);
		pi_instance_entity_draw(instance);
	}
	pi_vector_clear(cache->instance_list, FALSE);
	pi_hash_clear(cache->instance_map, FALSE);
}

void PI_API pi_entity_set_bind(PiEntity *entity, EntityBindType type, void *data)
{
	entity->bind_data[type] = data;
}

void *PI_API pi_entity_get_bind(PiEntity *entity, EntityBindType type)
{
	return entity->bind_data[type];
}

void PI_API pi_entity_add_hws_requied(PiEntity* entity, int count)
{
	entity->hws_required += count;
}