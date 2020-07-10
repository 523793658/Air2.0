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

/* �������� */
typedef enum
{
	EGOT_UNKOWN,
	EGOT_POINT_LIST,		/* ���б� */ 
	EGOT_LINE_LIST,			/* ���б� */ 
	EGOT_LINE_STRIP,		/* �ߴ� */
	EGOT_TRIANGLE_LIST,		/* �������б� */
	EGOT_TRIANGLE_STRIP,	/* �����δ� */
	EGOT_TRIANGLE_FAN,		/* �������� */
	EGOT_QUAD_LIST,			/* �ı����б� */
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

/* ������ÿ���������� */
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

/* ����Ԫ�ص����� */
typedef enum
{
	EVS_POSITION,	    	/* λ�ã�3��float */
	EVS_NORMAL,				/* ��������3��float */
	EVS_DIFFUSE,			/* ��������ɫ����Ҫ�淶����4��byte��rgba */
	EVS_SPECULAR,			/* ���淴����ɫ����Ҫ�淶����4��byte��rgba */
	EVS_BINORMAL,			/* 3��float��Binormal (Y axis if normal is Z) */
	EVS_TANGENT,			/* 4��float��Tangent (X axis if normal is Z) */
	EVS_BLEND_WEIGHTS,	 	/* �������Ȩ��, 4��float */
	EVS_BLEND_INDICES,	 	/* �������������4��short */
	EVS_TEXCOORD_0,			/* ��0���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_1,			/* ��1���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_2,			/* ��2���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_3,			/* ��3���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_4,			/* ��4���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_5,			/* ��5���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_TEXCOORD_6,			/* ��6���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_INSTANCE,			/* ��7���������꣬float��Ԫ��ֵ���ⲿ���� */
	EVS_NUM
}  VertexSemantic;

/* �������Ԫ */
#define MAX_TEXCOORD_NUM (EVS_TEXCOORD_6 - EVS_TEXCOORD_0 + 1)

/* ���������� */
typedef struct
{
	/* semantic��Ϊhash�ļ���������VertexElement�ĵ�һ��Ԫ�� */
	VertexSemantic semantic;	/* ������������ */
	void* impl;			/**/
	PiBool is_init;
	void *data;			/* ���������� */
	PiBool is_copy;		/* ��û�п��� */
	PiBool is_dirty;	/* ��û�и��� */
	uint32 size;		/* ���������ֽ��� */
	uint32 num;			/* ÿ������Ļ������͵����� */
	EVertexType type;	/* ÿ������Ļ������� */
	EBufferUsage usage;	/* ��;��������Ⱦ�Ż����� */
} VertexElement;

/* �������� */
typedef enum
{
	EINDEX_16BIT = 1,	/* 16λ���� */
	EINDEX_32BIT = 2,	/* 32λ���� */
}EIndexType;

/* �������� */
typedef struct 
{
	void *data;			/* �������� */
	void *impl;
	uint32 num;			/* �������� */
	PiBool is_copy;		/* ��û�п��� */
	PiBool is_dirty;	/* ��û�и��� */
	EIndexType type;	/* �������� */
	EBufferUsage usage;	/* ��;��������Ⱦ�Ż����� */
} IndexData;

/* ��Ⱦ���� */
typedef struct
{
	PiBool is_dirty;		/* �Ƿ��и��� */
	uint32 vertex_num;		/* �������� */ 
	PiDhash vertex_map;		/* key: VertexSemantic, value: VertexElement */

	IndexData idata;		/* �������� */
	float bone_offset;		/*��������ƫ��*/
	uint bone_num;			/*��������*/
	
	EGeometryType type;		/* ���ݵļ������� */
	
	PiAABBBox box;			/* ����Ⱦ���ݵİ�Χ���� */
} PiRenderData;

PI_BEGIN_DECLS

/* ���� */
PiRenderData* PI_API pi_renderdata_new(EGeometryType type);

/* �ͷ� */
void PI_API pi_renderdata_free(PiRenderData *data);

/* ��ʼ�� */
void PI_API pi_renderdata_init(PiRenderData *data, EGeometryType type);

/* ����RenderData */
void PI_API pi_renderdata_clear(PiRenderData *data);

/* ����RenderData */
void PI_API pi_renderdata_set(PiRenderData *dst, PiRenderData *src, PiBool is_copy);

/* ���ö������� */
void PI_API pi_renderdata_set_vertex_num(PiRenderData *data, uint32 vertex_num);

/* ������������ */
void PI_API pi_renderdata_set_index(PiRenderData *data, 
	PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index);

/*����������*/
void PI_API pi_renderdata_update_vertex(PiRenderData* data, uint32 offset, uint32_t vertex_num, VertexSemantic semantic, uint32_t srcStride, const void* src_data);

/* ���������� */
void PI_API pi_renderdata_set_vertex(PiRenderData *data, uint32 vertex_num,
	PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex);

/*��ȡ�������ʹ�С*/
uint PI_API pi_get_vertex_type_size(EVertexType type, uint num);

/* ��ȡ������ */
VertexElement* pi_renderdata_get_vertex(PiRenderData *data, VertexSemantic semantic);

PI_END_DECLS

#endif /* __PI_RENDERDATA_H__ */