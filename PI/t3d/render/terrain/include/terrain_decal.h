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
	PiVector3 pos_a, pos_b, pos_c, pos_d;	/*��������������ĸ�����*/
	PiRenderMesh *mesh;							/*��������*/
	DecalRect rect;										/*�����ľ��νṹ����*/
	uint tri_count;										/*����������������*/
	DecalTri *tri_list;									/*�����������б�*/
	uint texCoord_type;								/*������������*/
	PiRenderMesh *normal_mesh;				/*��������,������ʾ������Ϣ*/
	PiRenderMesh *tangent_mesh;				/*��������,������ʾ������Ϣ*/
} TerrainDecal;

PI_BEGIN_DECLS

/**
 * ���ɵ����������ݽṹTerrainDecal
 * pos_a, pos_b, pos_c, pos_d Ϊ�����ĸ�����
 * vertex_buffer ���������ĵ��ζ��㻺������
 * normal_buffer ���������ĵ��ζ��㷨�߻�������
 * index_buffer ���������ĵ���������������
 * index_count ���������ĵ�����������
 **/
TerrainDecal *PI_API pi_terrain_decal_create(PiVector3 *pos_a, PiVector3 *pos_b, PiVector3 *pos_c, PiVector3 *pos_d, PiVector3 *vertex_buffer, PiVector3 *normal_buffer, uint *index_buffer, uint index_count);

/*������������*/
void PI_API pi_terrain_decal_update_rect(TerrainDecal *decal);

/*������������*/
void PI_API pi_terrain_decal_create_mesh(TerrainDecal *decal);

PI_END_DECLS

#endif /* INCLUDE_TERRAIN_H */