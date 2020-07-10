#ifndef INCLUDE_RENDERVIEW_H
#define INCLUDE_RENDERVIEW_H

#include <pi_lib.h>

#include <renderformat.h>
#include <texture.h>

/**
 * 视图是资源的逻辑结构的封装，用于渲染目标上
 */
typedef enum
{
	RVT_COLOR,
	RVT_DEPTH_STENCIL
}RenderViewType;

/**
 * Renderview的来源
 */
typedef enum
{
	RVS_OFFSCREEN,		/* 来自离线缓存 */
	RVS_TEXTURE_2D,		/* 来自2D纹理 */
	RVS_TEXTURE_CUBE,	/* 来自cube纹理 */
}RenderViewSource;

typedef struct  
{
	PiTexture *tex;			/* 2D纹理 */
	uint level;				/* 纹理的mipmap层次 */
	uint array_index;		/* 纹理数组索引 */
}Texture2DView;

typedef struct  
{
	PiTexture *tex;			/* 3D纹理 */
	uint level;				/* 纹理的mipmap层次 */
	uint slice;				/* 深度层次 */
}Texture3DView;

typedef struct  
{
	PiTexture *tex;			/* Cube纹理 */
	uint level;				/* 纹理的mipmap层次 */
	TextureCubeFace face;	/* 立方体纹理的某个面 */
}TextureCubeView;

typedef struct  
{
	void *impl;

	uint width;					/* 视图的宽 */
	uint height;				/* 视图的高 */
	RenderFormat format;		/* 视图的格式 */

	RenderViewType type;		/* 类型 */
	RenderViewSource source;	/* 来源 */
	union {
		Texture2DView tex_2d;
		Texture3DView tex_3d;
		TextureCubeView tex_cube;
	}data;						/* 由来源决定 */
}PiRenderView;

PI_BEGIN_DECLS

/** 
 * 创建目标视图
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
 * 初始化
 */
PiBool PI_API pi_renderview_init(PiRenderView *view);

/**
 * 释放
 */
PiBool PI_API pi_renderview_free(PiRenderView *view);

PI_END_DECLS

#endif /* INCLUDE_RENDERVIEW_H */