#include "terrain_renderer.h"

#include <entity.h>
#include <terrain.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * µØÐÎäÖÈ¾Æ÷
 */
typedef struct
{
	PiEntity* terrain_entity;
	PiMaterial* blend_material;
	PiMaterial* detal_material;

	char* block_list_name;
	char* view_cam_name;
	char* output_name;
	PiBool is_deploy;
} TerrainRenderer;

static void _type_check(PiRenderer* renderer)
{
	PI_ASSERT((renderer)->type == ERT_TERRAIN, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash *resources)
{
	TerrainRenderer *impl = (TerrainRenderer*)renderer->impl;

	if(!impl->is_deploy)
	{
		return FALSE;
	}

	impl->terrain_entity = pi_entity_new();
	impl->blend_material = pi_material_new("terrain.vs", "terrain.fs");
	impl->detal_material = pi_material_new("terrain.vs", "terrain.fs");

	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	TerrainRenderer* impl = (TerrainRenderer*)renderer->impl;
	uint i, n;
	PiRenderTarget* target;
	PiCamera* view_camera;
	PiVector* block_list;

	pi_hash_lookup(resources, impl->block_list_name, (void**)&block_list);
	pi_hash_lookup(resources, impl->view_cam_name, (void**)&view_camera);
	pi_hash_lookup(resources, impl->output_name, (void**)&target);

	pi_rendersystem_set_target(target);
	n = pi_vector_size(block_list);
	pi_rendersystem_set_camera(view_camera);

	for(i = 0; i < n; ++i)
	{
		TerrainBlock* block = (TerrainBlock*)pi_vector_get(block_list, i);
		PiMatrix4 mat;
		pi_entity_set_mesh(impl->terrain_entity, block->mesh);
		pi_entity_set_material(impl->terrain_entity, impl->blend_material);
		pi_mat4_set_identity(&mat);
		pi_entity_draw(impl->terrain_entity);
	}
}

static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer* PI_API pi_terrain_renderer_new()
{
	PiRenderer* renderer;
	TerrainRenderer* impl = pi_new0(TerrainRenderer, 1);

	renderer = pi_renderer_create(ERT_TERRAIN, "terrain", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_terrain_renderer_deploy(PiRenderer* renderer, char* block_list_name, char* view_cam_name, char* output_name)
{
	TerrainRenderer* impl;
	_type_check(renderer);
	impl = (TerrainRenderer*)renderer->impl;

	pi_free(impl->block_list_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->output_name);

	impl->block_list_name = pi_str_dup(block_list_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_terrain_renderer_free(PiRenderer* renderer)
{
	TerrainRenderer* impl;
	_type_check(renderer);
	impl = (TerrainRenderer*)renderer->impl;

	pi_entity_free(impl->terrain_entity);
	pi_material_free(impl->blend_material);
	pi_material_free(impl->detal_material);

	pi_free(impl->block_list_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->output_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_terrain_renderer_set_wireframe(PiRenderer* renderer, PiBool is_wireframe)
{
	TerrainRenderer* impl;
	_type_check(renderer);
	impl = (TerrainRenderer*)renderer->impl;

	if(is_wireframe)
	{
		pi_material_set_polygon_mode(impl->blend_material, PM_LINE);
		pi_material_set_polygon_mode(impl->detal_material, PM_LINE);
	}
	else
	{
		pi_material_set_polygon_mode(impl->blend_material, PM_FILL);
		pi_material_set_polygon_mode(impl->detal_material, PM_LINE);
	}
}