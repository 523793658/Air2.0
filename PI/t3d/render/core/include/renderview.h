#ifndef INCLUDE_RENDERVIEW_H
#define INCLUDE_RENDERVIEW_H

#include <pi_lib.h>

#include <renderformat.h>
#include <texture.h>

/**
 * ��ͼ����Դ���߼��ṹ�ķ�װ��������ȾĿ����
 */
typedef enum
{
	RVT_COLOR,
	RVT_DEPTH_STENCIL
}RenderViewType;

/**
 * Renderview����Դ
 */
typedef enum
{
	RVS_OFFSCREEN,		/* �������߻��� */
	RVS_TEXTURE_2D,		/* ����2D���� */
	RVS_TEXTURE_CUBE,	/* ����cube���� */
}RenderViewSource;

typedef struct  
{
	PiTexture *tex;			/* 2D���� */
	uint level;				/* �����mipmap��� */
	uint array_index;		/* ������������ */
}Texture2DView;

typedef struct  
{
	PiTexture *tex;			/* 3D���� */
	uint level;				/* �����mipmap��� */
	uint slice;				/* ��Ȳ�� */
}Texture3DView;

typedef struct  
{
	PiTexture *tex;			/* Cube���� */
	uint level;				/* �����mipmap��� */
	TextureCubeFace face;	/* �����������ĳ���� */
}TextureCubeView;

typedef struct  
{
	void *impl;

	uint width;					/* ��ͼ�Ŀ� */
	uint height;				/* ��ͼ�ĸ� */
	RenderFormat format;		/* ��ͼ�ĸ�ʽ */

	RenderViewType type;		/* ���� */
	RenderViewSource source;	/* ��Դ */
	union {
		Texture2DView tex_2d;
		Texture3DView tex_3d;
		TextureCubeView tex_cube;
	}data;						/* ����Դ���� */
}PiRenderView;

PI_BEGIN_DECLS

/** 
 * ����Ŀ����ͼ
 */

 PiRenderView* PI_API pi_renderview_new(RenderViewType type, 
 uint width, uint height, RenderFormat format, PiBool is_create_handle);
 
PiRenderView* PI_API pi_renderview_new_tex2d(RenderViewType type, 
	PiTexture *texture, uint array_index, uint level, PiBool is_create_handle);

PiRenderView* PI_API pi_renderview_new_texcube(RenderViewType type, 
	PiTexture *texture, TextureCubeFace face, uint level, PiBool is_create_handle);

uint PI_API pi_renderview_get_width(PiRenderView *view);

uint PI_API pi_renderview_get_height(PiRenderView *view);

RenderFormat PI_API pi_renderview_get_format(PiRenderView *view);

/**
 * ��ʼ��
 */
PiBool PI_API pi_renderview_init(PiRenderView *view);

/**
 * �ͷ�
 */
PiBool PI_API pi_renderview_free(PiRenderView *view);

PI_END_DECLS

#endif /* INCLUDE_RENDERVIEW_H */