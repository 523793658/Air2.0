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

/* 网格 */
typedef struct  
{
	PiRenderData data;		/* 渲染对应的数据结构 */
	int version;
}PiMesh;

PI_BEGIN_DECLS

/** 
 * 创建单位正方形网格
 * 最小点(-0.5, -0.5, -0.5), 最大点(0.5, 0.5, 0.5)
 * uv: 4个点的uv坐标, [u_1, v_1, u_2, v_2, u_3, v_3, u_4, v_4]
 */
PiMesh *PI_API pi_mesh_create_quad(float color[16], float uv[8], float z);

/** 
 * 创建单位正方形平面网格
 * 最小点(-0.5, 0, -0.5), 最大点(0.5, 0, 0.5)
 * uv: 4个点的uv坐标, [u_1, v_1, u_2, v_2, u_3, v_3, u_4, v_4]
 */
PiMesh *PI_API pi_mesh_create_plane(float color[16], float uv[8], float y);

/** 
 * 创建单位立方体网格
 * 最小点(-0.5, -0.5, -0.5), 最大点(0.5, 0.5, 0.5)
 * uv: 24个顶点，因为每个立方体顶点3个面，一共8个顶点，供24个顶点
 */
PiMesh *PI_API pi_mesh_create_cube(float color[96], float uv[48]);

PiMesh* PI_API pi_mesh_create_sphere(float r);

PiMesh* PI_API pi_mesh_create_capsule(float r, float h);

PiMesh* PI_API pi_mesh_create_cylinder(float r, float h);

/* 根据顶点和索引创建网格 */
PiMesh* PI_API pi_mesh_create(EGeometryType gType, 
	EIndexType iType, uint32 numVertex, void *pos, void *color,
	void *normal, void *texCoord, uint32 numIndex, void *indexs);

/* 创建空网格，用于高层的pi_mesh_load */
PiMesh* PI_API pi_mesh_create_empty(void);

/* 释放网格 */
void PI_API pi_mesh_free(PiMesh *mesh);

/* 从二进制数据data里面取的网格的数量 */
uint32 PI_API pi_mesh_num(byte *data, uint32 size); 

/* 从二进制数据data里面解析网格数据 */
PiBool PI_API pi_mesh_load(PiMesh **meshs, uint32 num, byte *data, uint32 size);

/* 将网格数据写入buf */
void PI_API pi_mesh_write(PiBytes *dst, PiMesh **meshes, uint32 num);

/** 拷贝网格 */
void PI_API pi_mesh_copy(PiMesh *dst, PiMesh *src);

/** 网格整合 */
PiMesh* PI_API pi_mesh_gen_integral_mesh( PiMesh **src, uint32 num);

/** 网格插值 */
void PI_API pi_mesh_interpolate(PiMesh *dst_mesh, PiMesh *start_mesh, PiMesh *end_mesh, float t);

/* 应用骨骼矩阵和世界矩阵计算骨骼数据 */
PiRenderData* PI_API pi_mesh_apply(PiMesh *mesh, PiBool isNormal, PiAABBBox *box, uint32 numSkMat, PiMatrix4 *skMat);

/** 
 * 计算网格被骨骼作用之后的AABB，放入：aabb中
 * aabb至少6个元素，分别是：xmin, ymin, zmin, xmax, ymax, zmax
 */
void PI_API pi_mesh_compute_aabb(PiMesh *mesh, 
	PiAABBBox *aabb, uint32 numSkMat, void *skMat, void *worldMat);

/* 设置索引数据 */
void PI_API pi_mesh_set_index(PiMesh *mesh, PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index);

/* 设置流数据 */
void PI_API pi_mesh_set_vertex(PiMesh *mesh, uint32 vertex_num,
	PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex);

/** 
 * 取得网格的顶点数据并返回
 * gType: 返回顶点的几何类型
 * numVertex：返回顶点的数量
 * 函数返回值：顶点的位置数据
 */
PiVector3* PI_API pi_mesh_get_pos(PiMesh *mesh, EGeometryType *gType, uint32 *numVertex);

/** 
 * 取得网格的索引数组 
 * iType: 返回索引的类型：16或者32位
 * numIndex：返回索引的数量
 * 函数返回值：索引数据
 * 注：如果iType返回16位类型，那么函数返回值类型就是uint16*；
 * 注：如果iType返回32位类型，那么函数返回值类型就是uint32*；
 */
void* PI_API pi_mesh_get_index(PiMesh *mesh, EIndexType *itype, uint32 *index_num);

/**
 * 获取网格的纹理层数
 */
int32 PI_API pi_mesh_get_texcoord_count(PiMesh *mesh);

/**
 * 获取网格的定点的指定层纹理数据
 * @param mesh 网格指针
 * @param layer 第几层纹理
 * @param texcoord_num 顶点数量
 * @param size 每一个顶点的该层维数
 * @return 纹理坐标数据，长度为texcoord_num * size
 */ 
float* PI_API pi_mesh_get_texcoord(PiMesh *mesh, uint layer, uint32* texcoord_num, uint* size);

/**
 * 获取网格的顶点的法线
 * 函数返回值:法线数组
 */
PiVector3* pi_mesh_get_normal(PiMesh *mesh, uint32 *numVertex);

/**
 * 获取顶点数
*/
uint PI_API pi_mesh_get_vertex_num(PiMesh *mesh);

/**
 * 面数 
 */
uint PI_API pi_mesh_get_face_num(PiMesh *mesh);

/* 取网格AABB指针 */
PiAABBBox* PI_API pi_mesh_get_aabb(PiMesh *mesh);

uint PI_API pi_mesh_get_bone_num(PiMesh* mesh);

void PI_API pi_mesh_update_vertex(PiMesh* mesh, uint32 index, uint32 vertex_num, VertexSemantic semantic, uint32 src_stride, const void* vertex);

PI_END_DECLS

#endif /* __PI_MESH_H__ */
