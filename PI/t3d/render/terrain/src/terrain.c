#include "terrain.h"

#define MIN_GRAD_ABS 2.0f

Terrain* PI_API pi_terrain_new(uint width, uint height, uint block_unit, sint altitude_min, sint altitude_max)
{
	uint i, j;
	Terrain* terrain = pi_new0(Terrain, 1);
	terrain->width = width;
	terrain->height = height;
	terrain->unit = block_unit;
	terrain->elevation_min = altitude_min;
	terrain->elevation_max = altitude_max;
	terrain->tile_width = (uint)pi_math_ceil(width / (float)block_unit);
	terrain->tile_height = (uint)pi_math_ceil(height / (float)block_unit);
	terrain->blocks = pi_new0(TerrainBlock, terrain->tile_width * terrain->tile_height);
	terrain->layer_sequence = pi_vector_new();
	terrain->active_blocks = pi_vector_new();

	for(j = 0; j < terrain->tile_width; j++) {
		for(i = 0; i < terrain->tile_height; i++) {
			TerrainBlock* block = &terrain->blocks[j * terrain->tile_height + i];
			block->tile_w = i;
			block->tile_h = j;
			block->min_x = block->tile_w * block_unit;
			block->min_y = block->tile_h * block_unit;
			block->max_x = block->min_x + block_unit;
			block->max_y = block->min_y + block_unit;
			block->terrain = terrain;
		}
	}

	return terrain;
}

void PI_API pi_terrain_free(Terrain* terrain)
{
	uint i, j;
	for (i = 0; i < terrain->tile_height; ++i)
	{
		for (j = 0; j < terrain->tile_width; ++j)
		{
			TerrainBlock *block = pi_terrain_get_block_tile(terrain, j, i);
			pi_terrain_block_build(block, FALSE);
		}
	}

	pi_free(terrain->blocks);
	pi_vector_free(terrain->layer_sequence);
	pi_vector_free(terrain->active_blocks);
	pi_free(terrain);
}

TerrainLayer* PI_API pi_terrain_enable_layer(Terrain* terrain, uint id, PiBool is_enable)
{
	TerrainLayer* layer = &terrain->layers[id];
	layer->is_enable = is_enable;
	return layer;
}

void PI_API pi_terrain_refresh_layer(Terrain* terrain, uint id)
{

}

uint PI_API pi_terrain_get_layer_reference_count(Terrain* terrain, uint id)
{
	return terrain->layers[id].using_count;
}

TerrainBlock* PI_API pi_terrain_get_block_tile(Terrain* terrain, uint tile_w, uint tile_h)
{
	return &terrain->blocks[tile_h * terrain->tile_height + tile_w];
}

TerrainBlock* PI_API pi_terrain_get_block(Terrain* terrain, uint width, uint height)
{
	uint tile_w = (uint)pi_math_floor(width / (float)terrain->unit);
	uint tile_h = (uint)pi_math_floor(height / (float)terrain->unit);
	return pi_terrain_get_block_tile(terrain, tile_w, tile_h);
}

static void _block_build_mesh(TerrainBlock* block)
{
	PiImage* hight_map = block->height_map;
	byte* data = pi_render_image_get_pointer(hight_map, 0, 0, NULL);
	uint i, j;
	uint vertex_resolution;
	uint map_size = hight_map->width;
	sint elevation_min = block->terrain->elevation_min;
	sint elevation_max = block->terrain->elevation_max;

	PI_ASSERT(hight_map->width == hight_map->height, "HightMap must have same width and hight!");

	vertex_resolution = map_size;
	{
		uint vertex_count = (vertex_resolution + 1) * (vertex_resolution + 1);
		uint index_count = vertex_resolution * vertex_resolution * 2 * 3;
		float *vertex_buffer = pi_new0(float, vertex_count * 3); 
		uint *index_buffer = pi_new0(uint, index_count); 
		float unit = block->terrain->unit / (float)vertex_resolution;
		uint map_unit = map_size / vertex_resolution;
		uint i_edge, j_edge;

		for(i = 0; i <= vertex_resolution; i++)
		{
			for(j = 0; j <= vertex_resolution; j++)
			{
				i_edge = i == vertex_resolution ? 0 : i;
				j_edge = j == vertex_resolution ? 0 : j;
				vertex_buffer[(i * (vertex_resolution + 1) + j) * 3 + 0] = block->min_x + unit * j;
				vertex_buffer[(i * (vertex_resolution + 1) + j) * 3 + 1] = data[(i_edge * map_unit) * map_size + j_edge * map_unit] / 255.0f * (elevation_max - elevation_min) + elevation_min;
				vertex_buffer[(i * (vertex_resolution + 1) + j) * 3 + 2] = block->min_y + unit * i;
			}
		}

		for(i = 0; i < vertex_resolution; i++)
		{
			for(j = 0; j < vertex_resolution; j++)
			{
				index_buffer[(i * vertex_resolution + j) * 6 + 0] = (i + 1) * (vertex_resolution + 1) + j + 1;
				index_buffer[(i * vertex_resolution + j) * 6 + 1] = i * (vertex_resolution + 1) + j + 1;
				index_buffer[(i * vertex_resolution + j) * 6 + 2] = i * (vertex_resolution + 1) + j;
				index_buffer[(i * vertex_resolution + j) * 6 + 3] = i * (vertex_resolution + 1) + j;
				index_buffer[(i * vertex_resolution + j) * 6 + 4] = (i + 1) * (vertex_resolution + 1) + j;
				index_buffer[(i * vertex_resolution + j) * 6 + 5] = (i + 1) * (vertex_resolution + 1) + j + 1;
			}
		}

		block->mesh = pi_rendermesh_new(pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, vertex_count, vertex_buffer, NULL, NULL, NULL, index_count, index_buffer), TRUE);
		pi_free(vertex_buffer); //TODO:Ê¹ÓÃ»º´æ
		pi_free(index_buffer);
	}
}

static void _block_build_textures(TerrainBlock* block)
{

}

void PI_API pi_terrain_block_build(TerrainBlock* block, PiBool is_build)
{
	if(block->ready == is_build) {
		return;
	}
	else
	{
		if(is_build) {
			_block_build_mesh(block);
			_block_build_textures(block);
			block->ready = TRUE;
		}
		else
		{
			uint i;
			if (block->mesh != NULL)
			{
				pi_mesh_free(block->mesh->mesh);
				pi_rendermesh_free(block->mesh);
			}			
			pi_texture_free(block->blend_tex);
			block->mesh = NULL;
			block->blend_tex = NULL;
			for(i = 0; i < MAX_TERRAIN_LAYER; i++) {
				if(block->index_texes[i] != NULL) {
					pi_texture_free(block->index_texes[i]);
					block->index_texes[i] = NULL;
				}
			}
			pi_terrain_block_set_visible(block, FALSE);
			block->ready = FALSE;
		}
	}
}

static void _terrain_update_active_blocks(Terrain* terrain)
{
	uint i, j;
	pi_vector_clear(terrain->active_blocks, FALSE);
	for(i = 0; i < terrain->tile_width; i++) {
		for(j = 0; j < terrain->tile_height; j++) {
			TerrainBlock* block = pi_terrain_get_block_tile(terrain, i, j);
			if(block->visible) {
				pi_vector_push(terrain->active_blocks, block);
			}
		}
	}
}

void PI_API pi_terrain_block_set_visible(TerrainBlock* block, PiBool is_visible)
{
	if(block->ready) {
		block->visible = is_visible;
		_terrain_update_active_blocks(block->terrain);
	}
}