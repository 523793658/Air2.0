#ifndef INCLUDE_TEXTURE_ATLAS_H
#define INCLUDE_TEXTURE_ATLAS_H

#include "material.h"

typedef struct PiTextureAtlas PiTextureAtlas;		/* 纹理贴图集 */

typedef struct
{
	SamplerState atlas_sampler;						/* 纹理贴图集的纹理采样器，采用nearest */
	float atlas_size[2];							/* 纹理贴图集的纹理的宽高 */
} PiTextureAtlasData;

PI_BEGIN_DECLS

/**
 * 创建纹理贴图集
 * @returns 纹理贴图集句柄
 */
PiTextureAtlas *PI_API pi_texture_atlas_new();

/**
 * 销毁纹理贴图集
 * @param atlas 纹理贴图集句柄
 */
void PI_API pi_texture_atlas_delete(PiTextureAtlas *atlas);

/**
 * 向纹理贴图集中添加一个分块
 * @param atlas 纹理贴图集句柄
 * @param bitmap 分块的数据
 * @param width 分块的宽度
 * @param height 分块的高度
 * @param offset 返回分块在纹理贴图集中偏移量
 */
void PI_API pi_texture_atlas_add_tile(PiTextureAtlas *atlas, byte *bitmap, uint width, uint height, uint offset[2]);

/**
 * 获取纹理贴图集的渲染数据
 * @returns 纹理贴图集的渲染数据的句柄
 */
PiTextureAtlasData *PI_API pi_texture_atlas_get_data(PiTextureAtlas *atlas);

PI_END_DECLS

#endif /* INCLUDE_TEXTURE_ATLAS_H */
