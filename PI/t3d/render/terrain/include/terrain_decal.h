#ifndef INCLUDE_TERRAIN_MESH_DECAL_H
#define INCLUDE_TERRAIN_MESH_DECAL_H

#include <pi_lib.h>
#include <texture.h>
#include <rendermesh.h>
#include <entity.h>

typedef struct
{
	PiVector3 vertex[3];
	PiVector3 normal[3];
	PiVector3 segment[3][2];
} DecalTri;

typedef struct
{
	uint vertex_count;
	PiVector3 * vertex_buffer;
	PiVector3 * normal_buffer;
	PiVector3 * tangent_buffer;
	PiVector3 vertex0[3];
	PiVector3 vertex1[3];
	PiVector3 segment[4][2];
	float aabb_min_x;
	float aabb_min_z;
	float aabb_max_x;
	float aabb_max_z;
	uint tri_segment_count;
	PiVector3 **tri_segment;
} DecalRect;

typedef struct
{
	PiVector3 pos_a, pos_b, pos_c, pos_d;	/*矩形贴花网格的四个顶点*/
	PiRenderMesh *mesh;							/*贴花网格*/
	DecalRect rect;										/*贴花的矩形结构数据*/
	uint tri_count;										/*贴花的三角形数量*/
	DecalTri *tri_list;									/*三角形数据列表*/
	uint texCoord_type;								/*贴花纹理类型*/
	PiRenderMesh *normal_mesh;				/*法线网格,用于显示法线信息*/
	PiRenderMesh *tangent_mesh;				/*切线网格,用于显示切线信息*/
} TerrainDecal;

PI_BEGIN_DECLS

/**
 * 生成地形贴花数据结构TerrainDecal
 * pos_a, pos_b, pos_c, pos_d 为贴花四个顶点
 * vertex_buffer 用于贴花的地形顶点缓冲数据
 * normal_buffer 用于贴花的地形顶点法线缓冲数据
 * index_buffer 用于贴花的地形索引缓冲数据
 * index_count 用于贴花的地形索引数量
 **/
TerrainDecal *PI_API pi_terrain_decal_create(PiVector3 *pos_a, PiVector3 *pos_b, PiVector3 *pos_c, PiVector3 *pos_d, PiVector3 *vertex_buffer, PiVector3 *normal_buffer, uint *index_buffer, uint index_count);

/*更新贴花矩形*/
void PI_API pi_terrain_decal_update_rect(TerrainDecal *decal);

/*创建贴花网格*/
void PI_API pi_terrain_decal_create_mesh(TerrainDecal *decal);

PI_END_DECLS

#endif /* INCLUDE_TERRAIN_H */