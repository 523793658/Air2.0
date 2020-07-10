#include "radar_map.h"
#include "app_component.h"
#include "picture.h"

static PiRenderMesh *RADAR_MAP_MESH = NULL;

#define MAX_NPC_TEXTRUE_SIZE 20


static const char *RS_RADAR_MAP_VS = "simplest.vs";
static const char *RS_RADAR_MAP_FS = "radar_map.fs";

static const char *RS_RADAR_MAP_NPC_VS = "radar_map_npc.vs"; 
static const char *RS_RADAR_MAP_NPC_FS = "radar_map_npc.fs";

static const char *RS_RADAR_MAP_LINE_VS = "simplest.vs";
static const char *RS_RADAR_MAP_LINE_FS = "radar_map_npc.fs";

static float LINE_COLOR[4] = {1.0f, 1.0f, 0.0f, 1.0};


static const char *TEXTURE = "TEXTURE";



static PiRenderMesh *_create_mesh()
{
	PiMesh *mesh;

	float tcoord[4 * 2] =
	{
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f 
	};

	float pos[4 * 3] =
	{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	uint32 index[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, NULL, NULL, tcoord, 6, index);

	RADAR_MAP_MESH = pi_rendermesh_new(mesh, TRUE);

	return RADAR_MAP_MESH;
}

typedef struct
{
	uint map_width;				//地图实际宽度
	uint map_height;			//地图实际高度
	uint map_texture_width;		//地图图片宽度
	uint map_texture_height;	//地图图片高度
	uint component_width;
	uint component_height;
	uint current_npc_type_count;
	uint current_npc_count[MAX_NPC_TEXTRUE_SIZE];
	float center[2];
	float scale[2];


	PiEntity* instance_entity_list[MAX_NPC_TEXTRUE_SIZE];

	PiEntity* lines;
	PiMaterial* line_material;
	PiRenderMesh* line_mesh;

	PiTexture* npc_texture[MAX_NPC_TEXTRUE_SIZE];

	PiTexture* shape;
	SamplerState* shapeSampler;

	PiTexture* border;
	SamplerState* borderSampler;
	PiEntity *bg_entity;
	PiMaterial *bg_material;
	PiTexture *bg_texture;
	SamplerState* bg_sampler;

	char *conststr_bg_texture;
	char *conststr_shape;
	char *conststr_border;
	char *conststr_color;

	char *conststr_u_diffuse_map;
	char *conststr_u_center;
	char *conststr_u_scale;
	char *conststr_u_shape;
	char *conststr_u_border;
	char *conststr_u_npc_positions;
	char *conststr_u_color;

	PiBool is_init;

}AppRadarMap;


static PiRenderMesh* _create_lines_mesh(float* points, uint count, AppRadarMap* radar_map)
{
	PiMesh *mesh;
	uint i;
	float* pos = pi_new0(float, count * 3);
	for(i = 0; i < count; i++)
	{
		pos[i*3] = points[i*2]* radar_map->scale[0];
		pos[i*3+1] = (-points[i*2+1]) * radar_map->scale[1];
		pos[i*3+2] = 0.0f;

	}

	mesh = pi_mesh_create(EGOT_LINE_STRIP, EINDEX_32BIT, count, pos, NULL, NULL, NULL, count, NULL);
	pi_free(pos);
	return pi_rendermesh_new(mesh, TRUE);
}

static void _component_draw(PiComponent *component, AppRadarMap *map)
{
	uint width;
	uint height;
	PiSpatial *spatial;
	float offset[2];
	uint i;
	if(!map->is_init){
		return;
	}
	width = component->global_bounds.max_x - component->global_bounds.min_x;
	height = component->global_bounds.max_y - component->global_bounds.min_y;
	spatial = pi_entity_get_spatial(map->bg_entity);
	pi_spatial_set_local_translation(spatial, (float)component->global_bounds.min_x, (float)component->global_bounds.min_y, 0);
	pi_spatial_set_local_scaling(spatial, (float)width, (float)height, 1);
	pi_spatial_update(spatial);
	pi_entity_draw(map->bg_entity);

	offset[0] = (float)component->global_bounds.min_x;
	offset[1] = (float)component->global_bounds.min_y;
	for(i = 0; i < MAX_NPC_TEXTRUE_SIZE; ++i)
	{
		if(map->current_npc_count[i] > 0)
		{
			pi_material_set_uniform(map->instance_entity_list[i]->material, "u_Offset", UT_VEC2, 1, offset, TRUE);
			pi_entity_draw_instance(map->instance_entity_list[i], map->current_npc_count[i]);
		}
	}

	if(map->line_mesh)
	{
		spatial = pi_entity_get_spatial(map->lines);
		pi_spatial_set_local_translation(spatial, (component->global_bounds.max_x + component->global_bounds.min_x)/2.0f , (component->global_bounds.max_y + component->global_bounds.min_y)/2.0f, 0.0f);
		pi_spatial_update(spatial);
		pi_entity_draw(map->lines);
	}
}

PiComponent *PI_API app_ui_radar_map_new(PiTexture* shape, PiTexture* border)
{
	AppRadarMap *radar_map;
	PiComponent *component;
	PiRenderMesh* mesh = RADAR_MAP_MESH;

	radar_map = pi_new0(AppRadarMap, 1);

	radar_map->bg_entity = pi_entity_new();

	radar_map->conststr_bg_texture = pi_conststr("MAP_TEXTURE");
	radar_map->conststr_shape = pi_conststr("SHAPE");
	radar_map->conststr_border = pi_conststr("BORDER");
	radar_map->conststr_color = pi_conststr("COLOR");



	radar_map->conststr_u_diffuse_map = pi_conststr("u_DiffuseMap");
	radar_map->conststr_u_scale = pi_conststr("u_Scale");
	radar_map->conststr_u_center = pi_conststr("u_Center");
	radar_map->conststr_u_shape = pi_conststr("u_Shape");
	radar_map->conststr_u_border = pi_conststr("u_Border");
	radar_map->conststr_u_npc_positions = pi_conststr("u_NpcPosition");
	radar_map->conststr_u_color = pi_conststr("u_Color");

	if(!mesh)
	{
		mesh = _create_mesh();
	}

	pi_entity_set_mesh(radar_map->bg_entity, mesh);
	radar_map->scale[0] = 1.0f;
	radar_map->scale[1] = 1.0f;
	radar_map->center[0] = 0.0f;
	radar_map->center[1] = 0.0f;

	radar_map->bg_material = pi_material_new(RS_RADAR_MAP_VS, RS_RADAR_MAP_FS);
	pi_entity_set_material(radar_map->bg_entity, radar_map->bg_material);
	radar_map->bg_sampler = pi_sampler_new();
	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_scale, UT_VEC2, 1, radar_map->scale, FALSE);
	pi_material_set_depth_enable(radar_map->bg_material, FALSE);
	pi_material_set_blend(radar_map->bg_material, TRUE);
	pi_material_set_blend_factor(radar_map->bg_material, BF_SRC_ALPHA, BF_ZERO, BF_ONE, BF_ZERO);

	if(shape)
	{
		radar_map->shape = shape;
		radar_map->shapeSampler = pi_sampler_new();
		pi_sampler_set_texture(radar_map->shapeSampler, radar_map->shape);
		pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_shape, UT_SAMPLER_2D, 1, radar_map->shapeSampler, FALSE);
		pi_material_set_def(radar_map->bg_material, radar_map->conststr_shape, TRUE);
	}

	if(border)
	{
		radar_map->border = border;
		radar_map->borderSampler = pi_sampler_new();
		pi_sampler_set_texture(radar_map->borderSampler, radar_map->border);
		pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_border, UT_SAMPLER_2D, 1, radar_map->borderSampler, FALSE);
		pi_material_set_def(radar_map->bg_material, radar_map->conststr_border, TRUE);
	}
	
	radar_map->lines = pi_entity_new();
	radar_map->line_material = pi_material_new(RS_RADAR_MAP_LINE_VS, RS_RADAR_MAP_LINE_FS);
	pi_material_set_def(radar_map->line_material, radar_map->conststr_color, TRUE);
	pi_material_set_uniform(radar_map->line_material, radar_map->conststr_u_color, UT_VEC4, 1, LINE_COLOR, FALSE);
	pi_entity_set_material(radar_map->lines, radar_map->line_material);


	component = pi_component_new((EWidgetsType)EWT_RADAR_MAP, (ComponentDrawFunc)_component_draw, NULL, radar_map);
	pi_component_set_translucent(component, FALSE);
	return component;
}

static PiMaterial* _create_material(PiTexture* tex, float scale[2], uint size[2], AppRadarMap *radar_map)
{
	PiMaterial* material = pi_material_new(RS_RADAR_MAP_NPC_VS, RS_RADAR_MAP_NPC_FS);
	SamplerState* sample = pi_sampler_new();
	float center[2] = {0};
	float sizef[2];
	float flagSize[2];

	sizef[0] = (float)size[0];
	sizef[1] = (float)size[1];
	flagSize[0] = (float)pi_texture_get_width(tex);
	flagSize[1] = (float)pi_texture_get_height(tex);


	pi_sampler_set_texture(sample, tex);
	pi_material_set_depth_enable(material, FALSE);
	pi_material_set_blend(material, FALSE);
	pi_material_set_uniform(material, radar_map->conststr_u_diffuse_map, UT_SAMPLER_2D, 1, sample, TRUE);
	pi_material_set_uniform(material, radar_map->conststr_u_scale, UT_VEC2, 1, scale, TRUE);
	pi_material_set_uniform(material, radar_map->conststr_u_center, UT_VEC2, 1, center, TRUE);
	pi_material_set_uniform(material, "u_Size", UT_VEC2, 1, sizef, TRUE);
	pi_material_set_uniform(material, "u_FlagSize", UT_VEC2, 1, flagSize, TRUE);

	pi_material_set_blend(material, FALSE);
	pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);

	pi_sampler_free(sample);
	return material;
}

static PiEntity* _create_instance_entity(PiTexture* tex, float scale[2], uint size[2], AppRadarMap* radar_map)
{
	PiEntity* entity;
	entity = pi_entity_new();
	pi_entity_set_mesh(entity, _create_mesh());
	pi_entity_set_material(entity, _create_material(tex, scale, size, radar_map));
	return entity;
}

static void _free_instance_entity(AppRadarMap* radar_map)
{
	uint i;
	for(i = 0; i < radar_map->current_npc_type_count; ++i)
	{
		pi_material_free(radar_map->instance_entity_list[i]->material);
		pi_entity_free(radar_map->instance_entity_list[i]);
	}
	pi_memset(radar_map->current_npc_count, 0, MAX_NPC_TEXTRUE_SIZE*sizeof(float));
}


void PI_API app_ui_radar_map_init(PiComponent* component, uint width, uint height, PiTexture*bg_texture, PiTexture** npc_Texture, uint typeSize)
{
	AppRadarMap* radar_map = (AppRadarMap*)component->impl;
	uint size[2];
	float scale[2];
	uint i;
	radar_map->map_width = width;
	radar_map->map_height = height;

	radar_map->bg_texture = bg_texture;
	radar_map->map_texture_width = pi_texture_get_width(bg_texture);
	radar_map->map_texture_height = pi_texture_get_height(bg_texture);
	

	pi_component_get_size(component, size);
	radar_map->component_width = size[0];
	radar_map->component_height = size[1];
	scale[0] = ((float)radar_map->map_texture_width) / size[0];
	scale[1] = ((float)radar_map->map_texture_height) / size[1];

	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_scale, UT_VEC2, 1, scale, TRUE);
	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_center, UT_VEC2, 1, radar_map->center, TRUE);

	pi_material_set_def(radar_map->bg_material, radar_map->conststr_bg_texture, TRUE);
	pi_sampler_set_texture(radar_map->bg_sampler, radar_map->bg_texture);
	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_diffuse_map, UT_SAMPLER_2D, 1, radar_map->bg_sampler, TRUE);

	radar_map->scale[0] = (float)radar_map->map_texture_width / radar_map->map_width;
	radar_map->scale[1] = (float)radar_map->map_texture_height / radar_map->map_height;

	_free_instance_entity(radar_map);
	for(i = 0; i < typeSize && i < MAX_NPC_TEXTRUE_SIZE; i++)
	{
		radar_map->npc_texture[i] = npc_Texture[i];
		radar_map->instance_entity_list[i] = _create_instance_entity(npc_Texture[i], radar_map->scale, size, radar_map);
	}
	radar_map->current_npc_type_count = typeSize < MAX_NPC_TEXTRUE_SIZE ? typeSize : MAX_NPC_TEXTRUE_SIZE;
	radar_map->is_init = TRUE;
	pi_component_reprint(component);
}

void PI_API app_ui_radar_map_set_npc_data(PiComponent* component, float* position_array, uint8* indexArray, uint count)
{
	uint i;
	AppRadarMap* radar_map = (AppRadarMap*)component->impl;

	float* position[MAX_NPC_TEXTRUE_SIZE] = {0};
	uint counter[MAX_NPC_TEXTRUE_SIZE] = {0};
	PiMaterial* material;
	uint8 index;   //类型索引
	uint size[2];
	if(!radar_map->is_init)
	{
		return;
	}
	pi_component_get_size(component, size);

	pi_memset(&radar_map->current_npc_count, 0, sizeof(uint)*MAX_NPC_TEXTRUE_SIZE);
	for(i = 0; i < count; i++)
	{
		index = indexArray[i];
		if(index >= radar_map->current_npc_type_count)
		{
			continue;
		}
		++radar_map->current_npc_count[index];
	}
	for(i = 0 ; i < count; i++)
	{
		index = indexArray[i];
		if(index >= radar_map->current_npc_type_count)
		{
			continue;
		}
		if(position[index] == NULL)
		{
			position[index] = pi_new0(float, radar_map->current_npc_count[index]*2);
		}

		position[index][counter[index]*2] = position_array[i*2];
		position[index][counter[index]*2 + 1] = position_array[i*2 + 1];

		counter[index]++;
	}
	for(i = 0 ; i < MAX_NPC_TEXTRUE_SIZE; ++i)
	{
		if(radar_map->current_npc_count[i] != 0)
		{
			material = radar_map->instance_entity_list[i]->material;
			pi_material_set_uniform(material, radar_map->conststr_u_npc_positions, UT_VEC2, radar_map->current_npc_count[i], position[i], TRUE);
		}
		if(position[i] != NULL)
		{
			pi_free(position[i]);
			position[i] = NULL;
		}
	}
	pi_component_reprint(component);
}

void PI_API app_ui_radar_map_set_pos(PiComponent* component, float x, float y)
{
	uint i;
	AppRadarMap* radar_map = (AppRadarMap*)component->impl;
	float worldPos[2];
	

	if(!radar_map->is_init)
	{
		return ;
	}
	worldPos[0] = x;
	worldPos[1] = y;

	radar_map->center[0] = x / radar_map->map_width;
	radar_map->center[1] = y / radar_map->map_height;

	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_center, UT_VEC2, 1, radar_map->center, FALSE);

	for(i = 0 ; i< radar_map->current_npc_type_count; ++i)
	{
		pi_material_set_uniform(radar_map->instance_entity_list[i]->material, radar_map->conststr_u_center, UT_VEC2, 1, worldPos, TRUE);
	}
	pi_component_reprint(component);
}

void PI_API app_ui_radar_map_set_background_texture(PiComponent* component, PiTexture* bg_texture)
{
	AppRadarMap* radar_map = (AppRadarMap*)component->impl;
	radar_map->bg_texture = bg_texture;
	pi_sampler_set_texture(radar_map->bg_sampler, radar_map->bg_texture);
	pi_material_set_def(radar_map->bg_material, radar_map->conststr_bg_texture, TRUE);
	pi_material_set_uniform(radar_map->bg_material, radar_map->conststr_u_diffuse_map, UT_SAMPLER_2D, 1, radar_map->bg_sampler, FALSE);
	pi_component_reprint(component);
}

void PI_API app_ui_radar_map_set_line_points(PiComponent* component, float* points, uint count)
{
	AppRadarMap* radar_map = (AppRadarMap*)component->impl;
	if(!radar_map->is_init)
	{
		return;
	}
	if(radar_map->line_mesh != NULL)
	{
		pi_mesh_free(radar_map->line_mesh->mesh);
		pi_rendermesh_free(radar_map->line_mesh);
		radar_map->line_mesh = NULL;
	}
	if(count > 1)
	{
		radar_map->line_mesh = _create_lines_mesh(points, count, radar_map);
		pi_entity_set_mesh(radar_map->lines, radar_map->line_mesh);
	}
}



void PI_API app_ui_radar_map_free(PiComponent* component)
{
	AppRadarMap* randar_map = (AppRadarMap*)component->impl;
	pi_component_delete(component);
	pi_entity_free(randar_map->bg_entity);
	pi_material_free(randar_map->bg_material);
	pi_sampler_free(randar_map->borderSampler);
	pi_sampler_free(randar_map->shapeSampler);
	pi_sampler_free(randar_map->bg_sampler);

	if(randar_map->is_init)
	{
		_free_instance_entity(randar_map);
	}
}