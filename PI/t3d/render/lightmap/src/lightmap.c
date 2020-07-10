
#include "stdio.h"
#include "stdlib.h"
#include "renderutil.h"
#include "renderwrap.h"
#include "lightmap.h"


static const char* BEAST_SHADING_SHADER_VS = "beast.vs";
static const char* BEAST_SHADING_SHADER_FS = "beast.fs";

static const char* BEAST_DEFAULT_SHADER_VS = "default.vs";
static const char* BEAST_EXTENSION_SHADER_FS = "boundary_extension.fs";

static const char* BEAST_UV_0 = "BEAST_UV_0";
static const char* MIRRORING = "MIRRORING";

static const char* U_ShadowMap[9] = {
	"u_ShadowMap0",
	"u_ShadowMap1",
	"u_ShadowMap2",
	"u_ShadowMap3",
	"u_ShadowMap4",
	"u_ShadowMap5",
	"u_ShadowMap6",
	"u_ShadowMap7",
	"u_ShadowMap8"
};

static const char* U_ShadowMatrix[9] = {
	"u_ShadowMatrix0",
	"u_ShadowMatrix1",
	"u_ShadowMatrix2",
	"u_ShadowMatrix3",
	"u_ShadowMatrix4",
	"u_ShadowMatrix5",
	"u_ShadowMatrix6",
	"u_ShadowMatrix7",
	"u_ShadowMatrix8"
};

static const char* SHADOW_MAP_NUM[9] = {
	"SHADOW_MAP_0",
	"SHADOW_MAP_1",
	"SHADOW_MAP_2",
	"SHADOW_MAP_3",
	"SHADOW_MAP_4",
	"SHADOW_MAP_5",
	"SHADOW_MAP_6",
	"SHADOW_MAP_7",
	"SHADOW_MAP_8"
};

static const char* U_UVMatrix = "u_UVMatrix";

void PI_API pi_texture_save(wchar* path, PiTexture* texture)
{
	PiImage* image = pi_texture_2d_get(texture, 0, 0, 0, 0, texture->width, texture->height);
	uint size;
	void* data = pi_render_image_encode(image, IET_PNG, 0, &size);
	void* handle = pi_file_open(path, FILE_OPEN_WRITE | FILE_OPEN_WRITE_CLEAR);
	pi_file_write(handle, 0, FALSE, data, size);
	pi_file_close(handle);
	pi_render_image_free(image);
}


LightMapDevice* PI_API pi_light_map_device_create()
{
	LightMapDevice* device = pi_new0(LightMapDevice, 1);
	device->map_manager = pi_new0(LightMapManager, 1);
	device->map_manager->lightmap_list = pi_vector_new();
	device->map_manager->shadow_map_list = pi_vector_new();
	device->map_manager->shadow_map_sampler_list = pi_vector_new();
	device->map_manager->temp_map = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, 1024, 1024, TRUE);


	device->viewManager = pi_new0(RenderViewManager, 1);
	device->viewManager->lightmap_view_list = pi_vector_new();
	device->viewManager->defaultView = pi_renderview_new(RVT_COLOR, 4096, 4096, RF_ABGR8, TRUE);
	device->viewManager->temp_view = pi_renderview_new_tex2d(RVT_COLOR, device->map_manager->temp_map, 0, 0, TRUE);


	device->targetManager = pi_new0(RenderTargetManager, 1);
	device->dispenser = pi_new0(LightMapDispenser, 1);
	device->dispenser->available_list = pi_vector_new();

	device->work_list = pi_vector_new();

	device->shadow_camera = pi_camera_new();

	device->targetManager->shadowTarget = pi_rendertarget_new(TT_MRT, TRUE);
	pi_rendertarget_attach(device->targetManager->shadowTarget, ATT_COLOR0, device->viewManager->defaultView);

	device->targetManager->shadingTarget = pi_rendertarget_new(TT_MRT, TRUE);

	device->transformManager = pi_new0(TransformManager, 1);
	device->transformManager->shadow_camera_view_preject_matrix_list = pi_vector_new();
	device->beastMaterialManager = pi_new0(BeastMaterialManager, 1);

	device->beastMaterialManager->shading_material = pi_material_new(BEAST_SHADING_SHADER_VS, BEAST_SHADING_SHADER_FS);
	pi_material_set_depth_enable(device->beastMaterialManager->shading_material, FALSE);
	pi_material_set_depthwrite_enable(device->beastMaterialManager->shading_material, FALSE);
	pi_material_set_blend(device->beastMaterialManager->shading_material, FALSE);
	pi_material_set_cull_mode(device->beastMaterialManager->shading_material, CM_NO);
	device->magnifier = pi_lightmap_magnifier_create();

	device->map_manager->shadow_map_texture = pi_texture_2d_create(RF_D32F, TU_DEPTH_STENCIL, 1, 1, 4096, 4096, TRUE);
	device->viewManager->shadow_map_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, device->map_manager->shadow_map_texture, 0, 0, TRUE);
	device->viewManager->shadow_map_color_view = pi_renderview_new(RVT_COLOR, 4096, 4096, RF_A8, TRUE);
	pi_renderstate_set_default_sampler(&device->map_manager->shadow_map_sampler);
	pi_sampler_set_texture(&device->map_manager->shadow_map_sampler, device->map_manager->shadow_map_texture);
	PiColor border_color = { 1.0, 1.0, 1.0, 1.0 };
	pi_sampler_set_border_color(&device->map_manager->shadow_map_sampler, &border_color);
	pi_sampler_set_addr_mode(&device->map_manager->shadow_map_sampler, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_filter(&device->map_manager->shadow_map_sampler, TFO_CMP_MIN_MAG_MIP_LINEAR);
	return device;
}

void PI_API pi_light_map_device_init(LightMapDevice* device, uint size, GaussianBlurLevel level)
{
	device->map_manager->width = size;
	device->map_manager->height = size;
	device->borderExtensionManager = pi_light_map_post_process_manager_create(level);
}


void _uv_pack(LightMapDevice* device, PackInfo* info)
{
	uint i, map_count, j, blockNum;
	PiBool nfind = TRUE;
	LightMapDispenser* dispenser = device->dispenser;
	int compSize = (int)(info->origin_size / 1.2);

	map_count = pi_vector_size(device->map_manager->lightmap_list);
	for (i = 0; i < map_count && nfind; i++)
	{
		PiVector* blockList = pi_vector_get(dispenser->available_list, i);
		blockNum = pi_vector_size(blockList);
		for (j = 0; j < blockNum && nfind; j++)
		{
			Block* block = pi_vector_get(blockList, j);
			//找到一个可容纳的块
			if (block->width >= compSize && block->height >= compSize)
			{
				pi_vector_remove(blockList, j);

				if (block->width <= info->origin_size)
				{
					info->block.width = block->width;
				}
				else
				{
					info->block.width = info->origin_size;
				}
				if(block->height <= info->origin_size)
				{
					info->block.height = block->height;
				}
				else
				{
					info->block.height = info->origin_size;
				}

				info->block.x = block->x;
				info->block.y = block->y;
				//右下都有剩余
				if (block->width - info->block.width > 20 && block->height - info->block.height > 20)
				{
					Block* right, *bottom;
					right = pi_new0(Block, 1);
					right->x = block->x + info->block.width + 4;
					right->y = block->y;
					right->width = block->width - info->block.width - 4;
					right->height = info->block.height;

					pi_vector_push(blockList, right);

					bottom = pi_new0(Block, 1);
					bottom->x = block->x;
					bottom->width = block->width;
					bottom->y = block->y + info->block.height + 4;
					bottom->height = block->height-info->block.height-4;
					pi_vector_push(blockList, bottom);
				}
				else if (block->width - info->block.width > 20)
				{
					Block* right;
					right = pi_new0(Block, 1);
					right->x = block->x + info->block.width + 4;
					right->y = block->y;
					right->width = block->width - info->block.width -4;
					right->height = block->height;
					pi_vector_push(blockList, right);
				}
				else if (block->height - info->block.height > 20)
				{
					Block* bottom;
					bottom = pi_new0(Block, 1);
					bottom->x = block->x;
					bottom->width = block->width;
					bottom->y = block->y + info->block.height + 4;
					bottom->height = block->height - info->block.height -4;
					pi_vector_push(blockList, bottom);
				}
				pi_free(block);

				info->index = i;
				nfind = FALSE;
			}
		}
	}
	if (nfind)
	{
		PiTexture* texture = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, device->map_manager->width, device->map_manager->height, TRUE);
		PiRenderView* view = pi_renderview_new_tex2d(RVT_COLOR, texture, 0, 0, TRUE);


		PiVector* list = pi_vector_new();
		Block *right = pi_new0(Block, 1);
		Block *bottom = pi_new0(Block, 1);
		PiColor color = { 1.0f, 0.0f, 0.0f, 0.0f };

		info->index = map_count;
		right->x = info->origin_size + 4;
		right->y = 0;
		right->width = device->map_manager->width - right->x;
		right->height = info->origin_size;

		bottom->x = 0;
		bottom->y = info->origin_size + 4;
		bottom->width = device->map_manager->width;
		bottom->height = device->map_manager->height - bottom->y;

		info->block.width = info->origin_size;
		info->block.height = info->origin_size;
		info->block.x = 0;
		info->block.y = 0;

		pi_vector_push(list, right);
		pi_vector_push(list, bottom);
		pi_vector_push(dispenser->available_list, list);
		pi_vector_push(device->map_manager->lightmap_list, texture);
		pi_vector_push(device->viewManager->lightmap_view_list, view);
		pi_rendertarget_attach(device->targetManager->shadingTarget, ATT_COLOR0, view);
		pi_rendersystem_set_target(device->targetManager->shadingTarget);
		pi_rendersystem_clearview(TBM_COLOR, &color, 1.0f, 1);
	}
	info->scale_x = (float)info->block.width / device->map_manager->width;
	info->scale_y = (float)info->block.height / device->map_manager->height;
	info->offset_x = (float)info->block.x / device->map_manager->width;
	info->offset_y = (float)info->block.y / device->map_manager->height;
}

static PiBool _use_beast_ui_1(PiMaterial* material)
{
	uint i;
	for (i = 0; i < material->num_defs; i++)
	{
		if (pi_str_equal(material->def_names[i], BEAST_UV_0, FALSE))
		{
			return TRUE;
		}
	}
	return FALSE;
}

static PiBool has_defined(PiMaterial* material, char const * name)
{
	uint i;
	for (i = 0; i < material->num_defs; i++)
	{
		if (pi_str_equal(material->def_names[i], name, FALSE))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void _shadow_map_generate(LightMapDevice* device, BeastWork* work)
{
	PiVector3* dir, up;
	PI_ASSERT(device != NULL, "environment is null");
	pi_camera_set_location(device->shadow_camera, 0, 0, 0);
	dir = &work->render_environment->default_light.diffuse_dir;
	pi_vec3_set(&up, 1.0f, 0.0f, 0.0f);
	if (fabs(fabs(pi_vec3_dot(dir, &up)) - pi_vec3_len(dir)) < 0.0001)
	{
		pi_vec3_set(&up, 0.0f, 1.0, 1.0f);
	}

	pi_camera_set_up(device->shadow_camera, up.x, up.y, up.z);


	pi_camera_set_direction(device->shadow_camera, -dir->x, -dir->y, -dir->z);
	//viewMtr = pi_camera_get_view_matrix(device->shadow_camera);





	/*count = pi_vector_size(work->cast_shadow_entity_list);
	pi_aabb_init(&aabb);
	for (i = 0; i < count; i++)
	{
	entity = pi_vector_get(work->cast_shadow_entity_list, i);
	pi_aabb_copy(&world_aabb, pi_entity_get_world_aabb(entity));
	pi_aabb_transform(&world_aabb, viewMtr);
	pi_aabb_merge(&aabb, &aabb, &world_aabb);
	}
	block_x = (uint)pi_math_ceil((aabb.maxPt.x - aabb.minPt.y) / 100);
	block_y = (uint)pi_math_ceil((aabb.maxPt.y - aabb.minPt.y) / 100);
	device->block_x = block_x;
	device->block_y = block_y;
	block_aabb = pi_new0(PiAABBBox, block_x * block_y);
	temp_aabb = pi_new0(PiAABBBox, block_x * block_y);
	left = aabb.minPt.x;
	bottom = aabb.minPt.y;
	for (i = 0; i < block_y; i++)
	{
	for (j = 0; j < block_x; j++)
	{
	temp_aabb[i * block_x + j].minPt.x = left + j * 100;
	temp_aabb[i * block_x + j].minPt.y = bottom + i * 100;
	temp_aabb[i * block_x + j].minPt.z = aabb.minPt.z;
	temp_aabb[i * block_x + j].maxPt.x = left + (j + 1) * 100;
	temp_aabb[i * block_x + j].maxPt.y = bottom + (i + 1) * 100;
	temp_aabb[i * block_x + j].maxPt.z = aabb.maxPt.z;
	pi_aabb_init(&block_aabb[i * block_x + j]);
	}
	}
	for (k = 0; k < count; k++)
	{
	for (i = 0; i < block_y; i++)
	{
	for (j = 0; j < block_x; j++)
	{
	entity = pi_vector_get(work->cast_shadow_entity_list, k);
	pi_aabb_copy(&world_aabb, pi_entity_get_world_aabb(entity));
	pi_aabb_transform(&world_aabb, viewMtr);
	if (pi_aabb_is_overlapped(&temp_aabb[i * block_x + j], &world_aabb))
	{
	pi_aabb_merge(&block_aabb[i * block_x + j], &block_aabb[i * block_x + j], &world_aabb);
	}
	}
	}
	}
	aabb.minPt.x = 10000000.0f;
	aabb.minPt.y = 10000000.0f;
	aabb.minPt.z = 10000000.0f;
	aabb.maxPt.x = 10000001.0f;
	aabb.maxPt.y = 10000001.0f;
	aabb.maxPt.z = 10000001.0f;


	for (i = 0; i < block_y; i++)
	{
	for (j = 0; j < block_x; j++)
	{

	if (!_is_aabb_valid(&block_aabb[i * block_x + j]))
	{
	pi_aabb_merge(&block_aabb[i * block_x + j], &block_aabb[i * block_x + j], &aabb);
	}
	}
	}*/


// 	for (i = 0; i < block_y * block_x; i++)
// 	{
// 		texture = pi_texture_2d_create(RF_D32F, TU_DEPTH_STENCIL, 1, 1, 4096, 4096, TRUE);
// 		renderView = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, texture, 0, 0, TRUE);
// 		sampler = pi_sampler_new();
// 		pi_renderstate_set_default_sampler(sampler);
// 		pi_sampler_set_texture(sampler, texture);
// 		pi_sampler_set_border_color(sampler, &border_color);
// 		pi_vector_push(device->map_manager->shadow_map_sampler_list, sampler);
// 		pi_vector_push(device->map_manager->shadow_map_list, texture);
// 		pi_vector_push(device->transformManager->shadow_camera_view_preject_matrix_list, pi_new0(PiMatrix4, 1));
// 		pi_vector_push(shadow_map_view_list, renderView);
// 	}
// 	for (i = 0; i < block_y; i++)
// 	{
// 		for (j = 0; j < block_x; j++)
// 		{
// 			PiColor color = { 1.0, 1.0, 1.0, 1.0 };
// 			PiMatrix4* shadow_matrix = pi_vector_get(device->transformManager->shadow_camera_view_preject_matrix_list, i * block_x + j);
// 			renderView = pi_vector_get(shadow_map_view_list, i * block_x + j);
//  			//shadow_map_color_texture = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, 1024, 1024, TRUE);
//  			//shadow_map_color_view = pi_renderview_new_tex2d(RVT_COLOR, shadow_map_color_texture, 0, 0, TRUE);
// 			pi_rendertarget_attach(device->targetManager->shadowTarget, ATT_DEPTHSTENCIL, renderView);
// 			pi_rendertarget_attach(device->targetManager->shadowTarget, ATT_COLOR0, device->viewManager->defaultView);
// 			pi_camera_set_frustum(device->shadow_camera,
// 				block_aabb[block_x * i + j].minPt.x,
// 				block_aabb[block_x * i + j].maxPt.x,
// 				block_aabb[block_x * i + j].minPt.y,
// 				block_aabb[block_x * i + j].maxPt.y,
// 				block_aabb[block_x * i + j].minPt.z - 50,
// 				block_aabb[block_x * i + j].maxPt.z + 50, TRUE);
// 			pi_rendersystem_set_camera(device->shadow_camera);
// 			pi_rendersystem_set_target(device->targetManager->shadowTarget);
// 			pi_mat4_copy(shadow_matrix, pi_camera_get_view_projection_matrix(device->shadow_camera));
// 			pi_rendersystem_begin_draw();
// 
// 			pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &color, 1.0, 0);
// 			for (k = 0; k < count; k++)
// 			{
// 				entity = pi_vector_get(work->cast_shadow_entity_list, k);
// 				pi_aabb_copy(&world_aabb, pi_entity_get_world_aabb(entity));
// 				pi_aabb_transform(&world_aabb, viewMtr);
// 				if (pi_aabb_is_overlapped(&block_aabb[block_x * i + j], &world_aabb))
// 				{
// 					pi_entity_draw(entity);
// 				}
// 			}
// 
// 			pi_rendersystem_end_draw();
// 			pi_rendersystem_swapbuffer();
// 
// 			//pi_texture_save(L"t3d\\demo\\resource\\model\\shadow0.png", shadow_map_color_texture);
// 
// 		}
// 	}

// 	count = pi_vector_size(shadow_map_view_list);
// 	for (i = 0; i < count; i++)
// 	{
// 		renderView = pi_vector_get(shadow_map_view_list, i);
// 		pi_renderview_free(renderView);
// 	}
// 	pi_vector_free(shadow_map_view_list);
// 	pi_free(block_aabb);
// 	pi_free(temp_aabb);
}

static void _generate_shadow_map(LightMapDevice* device, PiEntity* entity, BeastWork* work)
{
	uint i, count;
	PiAABBBox shadow_aabb;
	PiMatrix4* view_mat;
	PiColor color = { 0.0, 0.0, 0.0, 1.0 };
	PiVector3* pos = pi_spatial_get_world_translation(entity->reference_spatial == NULL ? entity->spatial : entity->reference_spatial);
	pi_camera_set_location(device->shadow_camera, pos->x, pos->y, pos->z);
	view_mat = pi_camera_get_view_matrix(device->shadow_camera);
	pi_aabb_copy(&shadow_aabb, pi_entity_get_world_aabb(entity));
	pi_aabb_transform(&shadow_aabb, view_mat);
	shadow_aabb.minPt.x -= 10;
	shadow_aabb.minPt.y -= 10;
	shadow_aabb.minPt.z -= 10;
	shadow_aabb.maxPt.x += 10;
	shadow_aabb.maxPt.y += 10;
	shadow_aabb.maxPt.z += 10;

	fit_shadow_to_pixel(&shadow_aabb, 4096, 4096);
	pi_camera_set_frustum(device->shadow_camera, shadow_aabb.minPt.x, shadow_aabb.maxPt.x, shadow_aabb.minPt.y, shadow_aabb.maxPt.y, shadow_aabb.minPt.z - 100, shadow_aabb.maxPt.z + 100, TRUE);

	pi_rendertarget_attach(device->targetManager->shadowTarget, ATT_DEPTHSTENCIL, device->viewManager->shadow_map_view);
	pi_rendertarget_attach(device->targetManager->shadowTarget, ATT_COLOR0, device->viewManager->shadow_map_color_view);
	pi_rendersystem_set_camera(device->shadow_camera);
	pi_rendersystem_set_target(device->targetManager->shadowTarget);
	pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &color, 1.0, 0);

	pi_rendersystem_begin_draw();

	count = pi_vector_size(work->cast_shadow_entity_list);
	for (i = 0; i < count; i++)
	{
		entity = pi_vector_get(work->cast_shadow_entity_list, i);
// 		pi_aabb_copy(&world_aabb, pi_entity_get_world_aabb(entity));
// 		pi_aabb_transform(&world_aabb, view_mat);
// 		if (pi_aabb_is_overlapped(&shadow_aabb, &world_aabb))
// 		{
			pi_entity_draw(entity);
//		}
	}


	pi_rendersystem_end_draw();
	pi_rendersystem_swapbuffer();
}
void static _print_info(char* state, float process)
{
	pi_log_print(LOG_INFO, "烘焙阶段----【%s】---已完成：%d%%", state, (int)(process * 100));
}

void _render_entity(LightMapDevice* device, BeastWork* work)
{
	BeastResult* result = pi_new0(BeastResult, 1);
	uint i, num;
	PiEntity* entity;
	PiMaterial* originMaterial;
	PiAABBBox aabb;
	PiMatrix4 worldTransform;
	PiVector3 scale, pos;
	PiQuaternion rotate;
	uint process = 0;

	result->reference_info = pi_vector_new();

	float areaSize;
	num = pi_vector_size(work->beast_entity_list);
	
	for (i = 0; i < num; i++)
	{
		PackInfo* info = pi_new0(PackInfo, 1);
		PiMatrix4 uvMatrix;
		PiRenderView* renderview;
		PiColor color = { 1.0, 0.0, 0.0, 0.0 };
		BakeObj* bakeObj;
		bakeObj = pi_vector_get(work->beast_entity_list, i);
		entity = bakeObj->entity;
		pi_aabb_copy(&aabb, pi_mesh_get_aabb(entity->mesh->mesh));
		pi_mat4_decompose(&pos, &scale, &rotate, pi_entity_get_world_matrix(entity));
		pi_mat4_build_transform(&worldTransform, pi_vec3_get_zero(), &scale, pi_quat_get_unit());
		pi_aabb_transform(&aabb, &worldTransform);

		//分配lightmap
		areaSize = (aabb.maxPt.x - aabb.minPt.x) * (aabb.maxPt.y - aabb.minPt.y);
		areaSize += (aabb.maxPt.x - aabb.minPt.x) * (aabb.maxPt.z - aabb.minPt.z);
		areaSize += (aabb.maxPt.z - aabb.minPt.z) * (aabb.maxPt.y - aabb.minPt.y);
		info->origin_size = (uint) (areaSize / 2 * bakeObj->scale_in_map);
		info->origin_size = min(max(info->origin_size, 64), 1024);

		_uv_pack(device, info);

		//生成阴影贴图
		_generate_shadow_map(device, entity, work);

		PiMatrix4* shadow_matrix = pi_camera_get_view_projection_matrix(device->shadow_camera);
		pi_material_set_uniform_pack_flag(device->beastMaterialManager->shading_material, U_ShadowMatrix[0], UT_MATRIX4, 1, shadow_matrix, FALSE, TRUE);
		pi_material_set_uniform(device->beastMaterialManager->shading_material, U_ShadowMap[0], UT_SAMPLER_2D, 1, &device->map_manager->shadow_map_sampler, FALSE);

		//找到对应的shadow map;
// 		shadow_map_num = pi_vector_size(device->transformManager->shadow_camera_view_preject_matrix_list);
// 		for (j = 0; j < shadow_map_num; j++)
// 		{
// 			PiMatrix4* shadow_matrix = pi_vector_get(device->transformManager->shadow_camera_view_preject_matrix_list, j);
// 			pi_mat4_apply_point(&pos, &pos, shadow_matrix);
// 			if (pos.x >= -1.0f && pos.x <= 1.0f && pos.y >= -1.0f && pos.y <= 1.0f)
// 			{
// 				int block_x, block_y, map_num = 0, lx, ly;
// 				block_x = j % device->block_x;
// 				block_y = j / device->block_x;
// 
// 				lx = block_x - 1 >= 0 ? block_x - 1 : 0;
// 				ly = block_y - 1 >= 0 ? block_y - 1 : 0;
// 				block_x = block_x + 1 > device->block_x-1 ? device->block_x : block_x + 2;
// 				block_y = block_y + 1 > device->block_y-1 ? device->block_y : block_y + 2;
// 				for (n = ly ; n < block_y; n++)
// 				{
// 					for (m = lx ; m < block_x; m++)
// 					{
// 						pi_material_set_uniform(device->beastMaterialManager->shading_material, U_ShadowMap[map_num], UT_SAMPLER_2D, 1, pi_vector_get(device->map_manager->shadow_map_sampler_list, n * device->block_x + m), FALSE);
// 						pi_material_set_uniform_pack_flag(device->beastMaterialManager->shading_material, U_ShadowMatrix[map_num], UT_MATRIX4, 1, pi_vector_get(device->transformManager->shadow_camera_view_preject_matrix_list, n * device->block_x + m), FALSE, TRUE);
// 						pi_material_set_def(device->beastMaterialManager->shading_material, SHADOW_MAP_NUM[map_num], TRUE);
// 						map_num++;
// 					}
// 				}
// 				for (m = map_num; m < 9; m++)
// 				{
// 					pi_material_set_def(device->beastMaterialManager->shading_material, SHADOW_MAP_NUM[m], FALSE);
// 				}
// 				break;
// 			}
// 		}




		pi_vec3_set(&pos, info->offset_x, info->offset_y, 0.0);
		pi_vec3_set(&scale, info->scale_x, info->scale_y, 1.0);
		pi_mat4_build_transform(&uvMatrix, &pos, &scale, pi_quat_get_unit());

		pi_material_set_uniform(device->beastMaterialManager->shading_material, U_UVMatrix, UT_MATRIX4, 1, &uvMatrix, TRUE);
		pi_vector_push(result->reference_info, info);
		originMaterial = entity->material;
		if (has_defined(originMaterial, BEAST_UV_0))
		{
			pi_material_set_def(device->beastMaterialManager->shading_material, BEAST_UV_0, TRUE);
		}
		else
		{
			pi_material_set_def(device->beastMaterialManager->shading_material, BEAST_UV_0, FALSE);
		}
		if (has_defined(originMaterial, MIRRORING))
		{
			pi_material_set_def(device->beastMaterialManager->shading_material, MIRRORING, TRUE);
			Uniform* uniform = pi_material_get_uniform(originMaterial, pi_conststr("u_Mirroring"));
			pi_material_set_uniform(device->beastMaterialManager->shading_material, pi_conststr("u_Mirroring"), uniform->type, uniform->count, uniform->value, TRUE);
		}
		else
		{
			pi_material_set_def(device->beastMaterialManager->shading_material, MIRRORING, FALSE);
		}
		pi_entity_set_material(entity, device->beastMaterialManager->shading_material);
		renderview = pi_vector_get(device->viewManager->lightmap_view_list, info->index);
		pi_rendertarget_attach(device->targetManager->shadingTarget, ATT_COLOR0, device->viewManager->temp_view);
		pi_rendersystem_set_target(device->targetManager->shadingTarget);
		pi_rendertarget_set_viewport(device->targetManager->shadingTarget, 0, 0, device->viewManager->temp_view->width, device->viewManager->temp_view->height);
		pi_rendersystem_clearview(TBM_COLOR, &color, 1.0, 1);
		pi_rendersystem_begin_draw();
		pi_entity_draw(entity);
		pi_rendersystem_end_draw();
		pi_rendersystem_swapbuffer();



		//缩放到指定纹理
		pi_rendertarget_attach(device->targetManager->shadingTarget, ATT_COLOR0, renderview);
		pi_rendersystem_set_target(device->targetManager->shadingTarget);
		pi_rendertarget_set_viewport(device->targetManager->shadingTarget, info->block.x, renderview->height - info->block.y - info->block.height, info->block.width, info->block.height);
		pi_lightmap_magnifier_work(device->magnifier, device->map_manager->temp_map, device->targetManager->shadingTarget);

		entity->material = originMaterial;
		work->precess = (i + 1.0f) / num * 0.9f;
		if (process != ((uint)(100 * work->precess)))
		{
			_print_info("正在烘焙", work->precess);
			process = ((uint)(100 * work->precess));
		}
	}







	work->result = result;
	



}

BeastWork* PI_API pi_light_map_work_create(PiVector* shadow_list, PiVector* entity_list, PiEnvironment* environment)
{
	BeastWork* work = pi_new0(BeastWork, 1);
	work->cast_shadow_entity_list = shadow_list;
	work->beast_entity_list = entity_list;
	work->render_environment = environment;
	return work;
}

void PI_API pi_light_map_add_work(LightMapDevice* device, BeastWork* work)
{
	work->state = BS_WAITING;
	pi_vector_push(device->work_list, work);
}

void static _prepare_for_work(LightMapDevice* device)
{
	//清除上一个任务的shadowmap
	uint size, i;
	PiTexture* texture;
	PiMatrix4* mat;
	SamplerState* sampler;
	PiRenderView* view;
	size = pi_vector_size(device->map_manager->shadow_map_list);
	for (i = 0; i < size; i++)
	{
		texture = pi_vector_get(device->map_manager->shadow_map_list, i);
		pi_texture_free(texture);
		mat = pi_vector_get(device->transformManager->shadow_camera_view_preject_matrix_list, i);
		pi_free(mat);
		sampler = pi_vector_get(device->map_manager->shadow_map_sampler_list, i);
		pi_sampler_free(sampler);
	}
	pi_vector_clear(device->map_manager->shadow_map_list, FALSE);
	pi_vector_clear(device->map_manager->shadow_map_sampler_list, FALSE);
	pi_vector_clear(device->transformManager->shadow_camera_view_preject_matrix_list, FALSE);

	size = pi_vector_size(device->map_manager->lightmap_list);
	for (i = 0; i < size; i++)
	{
		texture = pi_vector_get(device->map_manager->lightmap_list, i);
		view = pi_vector_get(device->viewManager->lightmap_view_list, i);
		pi_renderview_free(view);
		pi_texture_free(texture);
	}
	pi_vector_clear(device->map_manager->lightmap_list, FALSE);
	pi_vector_clear(device->viewManager->lightmap_view_list, FALSE);
	size = pi_vector_size(device->dispenser->available_list);
	for (i = 0; i < size; i++)
	{
		PiVector* block_list = pi_vector_get(device->dispenser->available_list, i);
		uint j, num = pi_vector_size(block_list);
		for (j = 0; j < num; j++)
		{
			Block* block = pi_vector_get(block_list, j);
			pi_free(block);
		}
		pi_vector_free(block_list);
	}
	pi_vector_clear(device->dispenser->available_list, FALSE);
}

PiVector* _post_process_lightmap(LightMapDevice* device, BeastWork* work)
{
	pi_light_map_post_process_manager_set_input(device->borderExtensionManager, device->viewManager->lightmap_view_list);
	return pi_light_map_post_process_mananger_begin(device->borderExtensionManager);
}

void PI_API pi_light_map_start(LightMapDevice* device)
{
	BeastWork* work = pi_vector_pop(device->work_list);
	wchar path[256];
	while (work )
	{
		uint i, num;
		PiTexture* texture;
		PiVector* output;
		work->state = BS_WORKING;
		
		//生成阴影贴图
		_shadow_map_generate(device, work);

		_render_entity(device, work);

		//lightmap后期处理
		output = _post_process_lightmap(device, work);
		_print_info("正在烘焙", 1.0f);
		_print_info("保存贴图", 0.0f);
		//获取lightmap并保存
		num = pi_vector_size(output);
		for (i = 0; i < num; i++)
		{
			texture = pi_vector_get(output, i);
			pi_log_2_wbuffer(path, 256, L"%s\\shadow_ao_map_%d.png", work->result_path, i);
			pi_texture_save(path, texture);
			pi_texture_free(texture);
			_print_info("保存贴图", (i + 1.0f) / num);
		}
		pi_vector_free(output);
		//清理内存
		_prepare_for_work(device);
		work->texture_num = num;
		work->state = BS_FINISHED;
		work = pi_vector_pop(device->work_list);
	}
}

void PI_API pi_light_map_work_free(BeastWork* work)
{
	uint i, size;
	if (work->result)
	{
		size = pi_vector_size(work->result->reference_info);
		for (i = 0; i < size; i++)
		{
			PackInfo* info = pi_vector_get(work->result->reference_info, i);
			pi_free(info);
		}
		pi_vector_free(work->result->reference_info);
		pi_free(work->result);
	}

	if (work->result_path)
	{
		pi_free(work->result_path);
	}
	pi_free(work);
}

void PI_API pi_light_map_device_free(LightMapDevice* device)
{
	pi_rendersystem_set_target(NULL);
	pi_light_map_post_process_manager_free(device->borderExtensionManager);
	pi_material_free(device->beastMaterialManager->shading_material);
	pi_free(device->beastMaterialManager);
	pi_vector_free(device->transformManager->shadow_camera_view_preject_matrix_list);
	pi_free(device->transformManager);
	pi_rendertarget_free(device->targetManager->shadingTarget);
	pi_rendertarget_free(device->targetManager->shadowTarget);
	pi_camera_free(device->shadow_camera);
	pi_vector_free(device->work_list);
	pi_vector_free(device->dispenser->available_list);
	pi_free(device->dispenser);
	pi_free(device->targetManager);
	pi_renderview_free(device->viewManager->temp_view);
	pi_renderview_free(device->viewManager->defaultView);
	pi_vector_free(device->viewManager->lightmap_view_list);
	pi_renderview_free(device->viewManager->shadow_map_color_view);
	pi_renderview_free(device->viewManager->shadow_map_view);
	pi_free(device->viewManager);
	pi_texture_free(device->map_manager->temp_map);
	pi_vector_free(device->map_manager->shadow_map_sampler_list);
	pi_vector_free(device->map_manager->shadow_map_list);
	pi_vector_free(device->map_manager->lightmap_list);
	pi_texture_free(device->map_manager->shadow_map_texture);
	pi_free(device->map_manager);
	pi_free(device);
}

PiMesh** _load_mesh(wchar* meshPath, uint *num)
{
	void *data = NULL;
	uint size;
	void *file = pi_file_open(meshPath, FILE_OPEN_READ);
	size = 0;

	if (file != NULL)
	{
		int64 size64 = 0;
		pi_file_size(file, &size64);
		size = (uint)size64;
		data = pi_malloc0(size);
		pi_file_read(file, 0, FALSE, (char *)data, size);
		pi_file_close(file);
	}

	if (size > 0)
	{
		uint mesh_num = pi_mesh_num(data, size);
		if (mesh_num > 0)
		{
			uint i;
			PiMesh **meshes = pi_new0(PiMesh *, mesh_num);
			for (i = 0; i < mesh_num; ++i)
			{
				meshes[i] = pi_new0(PiMesh, 1);
			}

			pi_mesh_load(meshes, mesh_num, data, size);

			if (num != NULL)
			{
				*num = mesh_num;
			}

			pi_free(data);
			return meshes;
		}
		pi_free(data);
		return NULL;
	}
	return NULL;
}


PiBool PI_API pi_mesh_atlas_create(wchar* mesh_path, PiBool isAuto)
{
	uint num, i;
	PiMesh* temp;
	PiMesh** meshes = _load_mesh(mesh_path, &num);
	if (meshes == NULL)
	{
		return FALSE;
	}
	float* data;
	PiBool modified = FALSE;
	void* handle;
	for (i = 0; i < num; i++)
	{
		
		data = pi_mesh_get_texcoord(meshes[i], 1, NULL, NULL);
		if (data == NULL || isAuto)
		{
			temp = mesh_atlas_create(meshes[i]);
			pi_mesh_free(meshes[i]);
			meshes[i] = temp;
			modified = TRUE;
		}
	}
	if (modified)
	{
		PiBytes* buffer = pi_bytes_new();
		pi_mesh_write(buffer, meshes, num);
		handle = pi_file_open(mesh_path, FILE_OPEN_WRITE | FILE_OPEN_WRITE_CLEAR);
		pi_file_write(handle, 0, FALSE, pi_bytes_array(buffer, 0), pi_bytes_size(buffer));
		pi_file_close(handle);
	}
	for (i = 0; i < num; i++)
	{
		pi_mesh_free(meshes[i]);
	}
	pi_free(meshes);
	return modified;
}

void PI_API pi_light_map_work_set_output_path(BeastWork* work, wchar* path)
{
	work->result_path = pi_wstr_dup(path);
}

PiBool PI_API pi_light_map_check_work_end(BeastWork* work)
{
	return work->state == BS_FINISHED;
}

uint PI_API pi_light_map_get_result_info(BeastWork* work, uint index, float offset[4])
{
	PackInfo* info = pi_vector_get(work->result->reference_info, index);
	offset[0] = info->offset_x;
	offset[1] = info->offset_y;
	offset[2] = info->scale_x;
	offset[3] = info->scale_y;
	return info->index;
}
BakeObj* PI_API pi_light_map_create_bake_obj(PiEntity* entity, float scale)
{
	BakeObj* obj = pi_new0(BakeObj, 1);
	obj->entity = entity;
	obj->scale_in_map = scale;
	return obj;
}

void PI_API pi_light_map_bake_obj_free(BakeObj* obj)
{
	pi_free(obj);
}