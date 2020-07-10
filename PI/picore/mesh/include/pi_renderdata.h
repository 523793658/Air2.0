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

#ifndef __PI_RENDERDATA_H__
#define __PI_RENDERDATA_H__

#include <pi_lib.h>
#include <pi_aabb.h>

/* 几何类型 */
typedef enum
{
	EGOT_UNKOWN,
	EGOT_POINT_LIST,		/* 点列表 */ 
	EGOT_LINE_LIST,			/* 线列表 */ 
	EGOT_LINE_STRIP,		/* 线带 */
	EGOT_TRIANGLE_LIST,		/* 三角形列表 */
	EGOT_TRIANGLE_STRIP,	/* 三角形带 */
	EGOT_TRIANGLE_FAN,		/* 三角形扇 */
	EGOT_QUAD_LIST,			/* 四边形列表 */
	EGOT_NUM,
}EGeometryType;

typedef enum
{
	EVU_STREAM_DRAW,	
	EVU_STREAM_READ,	
	EVU_STREAM_COPY,

	EVU_STATIC_DRAW,
	EVU_STATIC_READ,
	EVU_STATIC_COPY,

	EVU_DYNAMIC_DRAW,
	EVU_DYNAMIC_READ,
	EVU_DYNAMIC_COPY
} EBufferUsage;

/* 属性流每个数的类型 */
typedef enum
{
	EVT_BYTE,
	EVT_UNSIGNED_BYTE,
	EVT_SHORT,
	EVT_UNSIGNED_SHORT,
	EVT_INT,
	EVT_UNSIGNED_INT,
	EVT_FLOAT,
	EVT_DOUBLE,
	EVT_UNSPECIFIED
} EVertexType;

/* 顶点元素的语义 */
typedef enum
{
	EVS_POSITION,	    	/* 位置，3个float */
	EVS_NORMAL,				/* 法向量，3个float */
	EVS_DIFFUSE,			/* 漫反射颜色，需要规范化的4个byte，rgba */
	EVS_SPECULAR,			/* 镜面反射颜色，需要规范化的4个byte，rgba */
	EVS_BINORMAL,			/* 3个float，Binormal (Y axis if normal is Z) */
	EVS_TANGENT,			/* 4个float，Tangent (X axis if normal is Z) */
	EVS_BLEND_WEIGHTS,	 	/* 骨骼混合权重, 4个float */
	EVS_BLEND_INDICES,	 	/* 骨骼混合索引，4个short */
	EVS_TEXCOORD_0,			/* 第0层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_1,			/* 第1层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_2,			/* 第2层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_3,			/* 第3层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_4,			/* 第4层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_5,			/* 第5层纹理坐标，float，元素值由外部给定 */
	EVS_TEXCOORD_6,			/* 第6层纹理坐标，float，元素值由外部给定 */
	EVS_INSTANCE,			/* 第7层纹理坐标，float，元素值由外部给定 */
	EVS_NUM
}  VertexSemantic;

/* 最大纹理单元 */
#define MAX_TEXCOORD_NUM (EVS_TEXCOORD_6 - EVS_TEXCOORD_0 + 1)

/* 顶点属性流 */
typedef struct
{
	/* semantic作为hash的键，必须是VertexElement的第一个元素 */
	VertexSemantic semantic;	/* 顶点流的语义 */
	void* impl;			/**/
	PiBool is_init;
	void *data;			/* 该流的数据 */
	PiBool is_copy;		/* 有没有拷贝 */
	PiBool is_dirty;	/* 有没有更新 */
	uint32 size;		/* 该流的总字节数 */
	uint32 num;			/* 每个顶点的基本类型的数量 */
	EVertexType type;	/* 每个顶点的基本类型 */
	EBufferUsage usage;	/* 用途，用于渲染优化建议 */
} VertexElement;

/* 索引类型 */
typedef enum
{
	EINDEX_16BIT = 1,	/* 16位索引 */
	EINDEX_32BIT = 2,	/* 32位索引 */
}EIndexType;

/* 索引数据 */
typedef struct 
{
	void *data;			/* 索引数据 */
	void *impl;
	uint32 num;			/* 索引数量 */
	PiBool is_copy;		/* 有没有拷贝 */
	PiBool is_dirty;	/* 有没有更新 */
	EIndexType type;	/* 索引类型 */
	EBufferUsage usage;	/* 用途，用于渲染优化建议 */
} IndexData;

/* 渲染数据 */
typedef struct
{
	PiBool is_dirty;		/* 是否有更新 */
	uint32 vertex_num;		/* 顶点数量 */ 
	PiDhash vertex_map;		/* key: VertexSemantic, value: VertexElement */

	IndexData idata;		/* 索引数据 */
	float bone_offset;		/*骨骼索引偏移*/
	uint bone_num;			/*骨骼数量*/
	
	EGeometryType type;		/* 数据的几何意义 */
	
	PiAABBBox box;			/* 该渲染数据的包围网格 */
} PiRenderData;

PI_BEGIN_DECLS

/* 创建 */
PiRenderData* PI_API pi_renderdata_new(EGeometryType type);

/* 释放 */
void PI_API pi_renderdata_free(PiRenderData *data);

/* 初始化 */
void PI_API pi_renderdata_init(PiRenderData *data, EGeometryType type);

/* 清理RenderData */
void PI_API pi_renderdata_clear(PiRenderData *data);

/* 拷贝RenderData */
void PI_API pi_renderdata_set(PiRenderData *dst, PiRenderData *src, PiBool is_copy);

/* 设置顶点数量 */
void PI_API pi_renderdata_set_vertex_num(PiRenderData *data, uint32 vertex_num);

/* 设置索引数据 */
void PI_API pi_renderdata_set_index(PiRenderData *data, 
	PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index);

/*更新流数据*/
void PI_API pi_renderdata_update_vertex(PiRenderData* data, uint32 offset, uint32_t vertex_num, VertexSemantic semantic, uint32_t srcStride, const void* src_data);

/* 设置流数据 */
void PI_API pi_renderdata_set_vertex(PiRenderData *data, uint32 vertex_num,
	PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex);

/*获取数据类型大小*/
uint PI_API pi_get_vertex_type_size(EVertexType type, uint num);

/* 获取流数据 */
VertexElement* pi_renderdata_get_vertex(PiRenderData *data, VertexSemantic semantic);

PI_END_DECLS

#endif /* __PI_RENDERDATA_H__ */