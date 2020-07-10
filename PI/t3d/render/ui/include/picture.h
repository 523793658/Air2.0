#ifndef INCLUDE_PICTURE_H
#define INCLUDE_PICTURE_H

#include <entity.h>
#include <texture.h>

/**
 * 此文件定义了用于2D平面绘制的Picture模块,提供2D化的基础绘制功能
 */

typedef enum
{
	EBM_NONE = 0,
	EBM_ALPHA,
	/* 用来处理标准alpha混合的结合，当要渲染多个alpha混合时，如果想把他们预先渲染到一张纹理上，再把这个纹理跟最终的目标做混合，分别要使用下面三个混合方式 */
	EBM_ALPHA_ASSOCIATIVE_INIT,
	EBM_ALPHA_ASSOCIATIVE,
	EBM_ALPHA_ASSOCIATIVE_FINISH
} EBlendMode;

typedef struct
{
	PiEntity *entity;
	PiTexture *texture;
	SamplerState sampler;
	float color[4];
	float uv_anim[2];
	float tile_anim[4];
	sint x;
	sint y;
	sint z;
	uint width;
	uint height;
	EBlendMode blend_mode;

	char *conststr_texture;
	char *conststr_flip_y;
	char *conststr_uv_anim;
	char *conststr_tile_anim;
	char *conststr_tile_blend;
	char *conststr_u_color;
	char *conststr_u_texture;
	char *conststr_u_uv_anim;
	char *conststr_u_tile_anim;
} PiPicture;

PI_BEGIN_DECLS

PiPicture *PI_API pi_picture_new();

void PI_API pi_picture_free(PiPicture *picture);

void PI_API pi_picture_set_size(PiPicture *picture, uint width, uint height);

void PI_API pi_picture_set_location(PiPicture *picture, sint x, sint y);

void PI_API pi_picture_set_color(PiPicture *picture, float r, float g, float b, float a);

void PI_API pi_picture_set_texture(PiPicture *picture, PiTexture *texture, PiBool flip_y);

void PI_API pi_picture_set_texture_uv_anim(PiPicture *picture, float u, float v);

void PI_API pi_picture_set_texture_tile_anim(PiPicture *picture, uint tile_x, uint tile_y, float frame_time, uint tile_count, PiBool is_blend);

void PI_API pi_picture_set_blend_mode(PiPicture *picture, EBlendMode blend_mode);

EBlendMode PI_API pi_picture_get_blend_mode(PiPicture *picture);

PI_END_DECLS

#endif /* INCLUDE_PICTURE_H */
