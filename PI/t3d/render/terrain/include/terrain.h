#ifndef INCLUDE_TERRAIN_H
#define INCLUDE_TERRAIN_H

#include <pi_lib.h>
#include <texture.h>
#include <rendermesh.h>
#include <entity.h>

#define MAX_TERRAIN_LAYER 32

typedef struct Terrain Terrain;

typedef struct TerrainLayer
{
	uint id;
	PiBool is_enable;
	PiTexture* diffuse_tex;
	PiTexture* bump_tex;
	PiTexture* specular_tex;
	PiBool* vertical_mapping;
	uint scale;
	uint using_count;
}TerrainLayer;

typedef struct TerrainBlock
{
	uint tile_w;
	uint tile_h;
	uint min_x;
	uint min_y;
	uint max_x;
	uint max_y;
	Terrain* terrain;
	PiImage* index_map;
	PiImage* blend_map;
	PiImage* height_map;
	PiBool ready;
	PiBool visible;
	PiTexture* index_texes[MAX_TERRAIN_LAYER];
	PiTexture* blend_tex;
	PiRenderMesh* mesh; 
}TerrainBlock;

typedef struct Terrain
{
	uint width;
	uint height;
	uint unit;
	uint tile_width;
	uint tile_height;
	sint elevation_min;
	sint elevation_max;
	TerrainLayer layers[MAX_TERRAIN_LAYER];
	TerrainBlock* blocks;
	PiVector* layer_sequence;
	PiVector* active_blocks;
}Terrain;

PI_BEGIN_DECLS

Terrain* PI_API pi_terrain_new(uint width, uint height, uint block_unit, sint altitude_min, sint altitude_max);

void PI_API pi_terrain_free(Terrain* terrain);

TerrainLayer* PI_API pi_terrain_enable_layer(Terrain* terrain, uint id, PiBool is_enable);

void PI_API pi_terrain_refresh_layer(Terrain* terrain, uint id);

uint PI_API pi_terrain_get_layer_reference_count(Terrain* terrain, uint id);

TerrainBlock* PI_API pi_terrain_get_block(Terrain* terrain, uint width, uint height);

TerrainBlock* PI_API pi_terrain_get_block_tile(Terrain* terrain, uint tile_w, uint tile_h);

void PI_API pi_terrain_block_build(TerrainBlock* block, PiBool is_build);

void PI_API pi_terrain_block_set_visible(TerrainBlock* block, PiBool is_visible);

PI_END_DECLS

#endif /* INCLUDE_TERRAIN_H */