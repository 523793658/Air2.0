/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __PI_MESH_H__
#define __PI_MESH_H__

#include <pi_lib.h>
#include <pi_renderdata.h>

#define MAX_BONEINFO_NUM 4

/* ���� */
typedef struct  
{
	PiRenderData data;		/* ��Ⱦ��Ӧ�����ݽṹ */
	int version;
}PiMesh;

PI_BEGIN_DECLS

/** 
 * ������λ����������
 * ��С��(-0.5, -0.5, -0.5), ����(0.5, 0.5, 0.5)
 * uv: 4�����uv����, [u_1, v_1, u_2, v_2, u_3, v_3, u_4, v_4]
 */
PiMesh *PI_API pi_mesh_create_quad(float color[16], float uv[8], float z);

/** 
 * ������λ������ƽ������
 * ��С��(-0.5, 0, -0.5), ����(0.5, 0, 0.5)
 * uv: 4�����uv����, [u_1, v_1, u_2, v_2, u_3, v_3, u_4, v_4]
 */
PiMesh *PI_API pi_mesh_create_plane(float color[16], float uv[8], float y);

/** 
 * ������λ����������
 * ��С��(-0.5, -0.5, -0.5), ����(0.5, 0.5, 0.5)
 * uv: 24�����㣬��Ϊÿ�������嶥��3���棬һ��8�����㣬��24������
 */
PiMesh *PI_API pi_mesh_create_cube(float color[96], float uv[48]);

PiMesh* PI_API pi_mesh_create_sphere(float r);

PiMesh* PI_API pi_mesh_create_capsule(float r, float h);

PiMesh* PI_API pi_mesh_create_cylinder(float r, float h);

/* ���ݶ���������������� */
PiMesh* PI_API pi_mesh_create(EGeometryType gType, 
	EIndexType iType, uint32 numVertex, void *pos, void *color,
	void *normal, void *texCoord, uint32 numIndex, void *indexs);

/* �������������ڸ߲��pi_mesh_load */
PiMesh* PI_API pi_mesh_create_empty(void);

/* �ͷ����� */
void PI_API pi_mesh_free(PiMesh *mesh);

/* �Ӷ���������data����ȡ����������� */
uint32 PI_API pi_mesh_num(byte *data, uint32 size); 

/* �Ӷ���������data��������������� */
PiBool PI_API pi_mesh_load(PiMesh **meshs, uint32 num, byte *data, uint32 size);

/* ����������д��buf */
void PI_API pi_mesh_write(PiBytes *dst, PiMesh **meshes, uint32 num);

/** �������� */
void PI_API pi_mesh_copy(PiMesh *dst, PiMesh *src);

/** �������� */
PiMesh* PI_API pi_mesh_gen_integral_mesh( PiMesh **src, uint32 num);

/** �����ֵ */
void PI_API pi_mesh_interpolate(PiMesh *dst_mesh, PiMesh *start_mesh, PiMesh *end_mesh, float t);

/* Ӧ�ù������������������������� */
PiRenderData* PI_API pi_mesh_apply(PiMesh *mesh, PiBool isNormal, PiAABBBox *box, uint32 numSkMat, PiMatrix4 *skMat);

/** 
 * �������񱻹�������֮���AABB�����룺aabb��
 * aabb����6��Ԫ�أ��ֱ��ǣ�xmin, ymin, zmin, xmax, ymax, zmax
 */
void PI_API pi_mesh_compute_aabb(PiMesh *mesh, 
	PiAABBBox *aabb, uint32 numSkMat, void *skMat, void *worldMat);

/* ������������ */
void PI_API pi_mesh_set_index(PiMesh *mesh, PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index);

/* ���������� */
void PI_API pi_mesh_set_vertex(PiMesh *mesh, uint32 vertex_num,
	PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex);

/** 
 * ȡ������Ķ������ݲ�����
 * gType: ���ض���ļ�������
 * numVertex�����ض��������
 * ��������ֵ�������λ������
 */
PiVector3* PI_API pi_mesh_get_pos(PiMesh *mesh, EGeometryType *gType, uint32 *numVertex);

/** 
 * ȡ��������������� 
 * iType: �������������ͣ�16����32λ
 * numIndex����������������
 * ��������ֵ����������
 * ע�����iType����16λ���ͣ���ô��������ֵ���;���uint16*��
 * ע�����iType����32λ���ͣ���ô��������ֵ���;���uint32*��
 */
void* PI_API pi_mesh_get_index(PiMesh *mesh, EIndexType *itype, uint32 *index_num);

/**
 * ��ȡ������������
 */
int32 PI_API pi_mesh_get_texcoord_count(PiMesh *mesh);

/**
 * ��ȡ����Ķ����ָ������������
 * @param mesh ����ָ��
 * @param layer �ڼ�������
 * @param texcoord_num ��������
 * @param size ÿһ������ĸò�ά��
 * @return �����������ݣ�����Ϊtexcoord_num * size
 */ 
float* PI_API pi_mesh_get_texcoord(PiMesh *mesh, uint layer, uint32* texcoord_num, uint* size);

/**
 * ��ȡ����Ķ���ķ���
 * ��������ֵ:��������
 */
PiVector3* pi_mesh_get_normal(PiMesh *mesh, uint32 *numVertex);

/**
 * ��ȡ������
*/
uint PI_API pi_mesh_get_vertex_num(PiMesh *mesh);

/**
 * ���� 
 */
uint PI_API pi_mesh_get_face_num(PiMesh *mesh);

/* ȡ����AABBָ�� */
PiAABBBox* PI_API pi_mesh_get_aabb(PiMesh *mesh);

uint PI_API pi_mesh_get_bone_num(PiMesh* mesh);

void PI_API pi_mesh_update_vertex(PiMesh* mesh, uint32 index, uint32 vertex_num, VertexSemantic semantic, uint32 src_stride, const void* vertex);

PI_END_DECLS

#endif /* __PI_MESH_H__ */
