#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include <pi_lib.h>

#include <image.h>
#include "clut.h"
#include <renderformat.h>

typedef enum 
{
	TMA_READ_ONLY,
	TMA_WRITE_ONLY
}TextureMapAccess;

// Texture的用途，是普通纹理，作为color目标，作为depth-stencil目标
// 仅OpenGL可用
typedef enum
{
	TU_NORMAL = 0,			// 普通纹理
	TU_COLOR = 1,			// 作为color目标的纹理
	TU_DEPTH_STENCIL = 2	// 作为depth-stencil目标的纹理
}TextureUsage;


typedef enum 
{
	CF_POSITIVE_X = 0,
	CF_NEGATIVE_X = 1,
	CF_POSITIVE_Y = 2,
	CF_NEGATIVE_Y = 3,
	CF_POSITIVE_Z = 4,
	CF_NEGATIVE_Z = 5,
	CF_NONE = 6
}TextureCubeFace;

typedef struct  
{
	uint width;
	uint height;
	uint depth;
}TextureLevelSize;

/**
 * 纹理模块
 * 注：纹理的宽，高，深，格式，mipmap数量在初始化时全部确定，无法修改
 */
typedef struct 
{
	TextureType	type;
	RenderFormat format;

	uint array_size;				/* 纹理数组的个数，大于1，表示这是一个纹理数组 */
	uint num_mipmap;

	uint32 width;
	uint32 height;
	uint32 depth;
	
	uint compress_block_size;
	PiBool is_compressed_format;
	
	TextureUsage usage;

	TextureLevelSize *level_size;	/* 数组元素个数为num_mipmap */

	void *impl;
}PiTexture;

PI_BEGIN_DECLS

/* 清理纹理 */
PiBool PI_API pi_texture_free(PiTexture *texture);

/**
 * 克隆纹理，返回新纹理
 */
PiTexture* PI_API pi_texture_clone(PiTexture *texture);

/** 
 * 创建2D纹理
 * usage: 纹理用途
 * format: 纹理格式
 * array_size: 大于1表示纹理数组
 * num_mipmap: mipmap层次数，为0表示创建所有的mipmap
 * width, height: 纹理宽度，高度
 * is_create_handle: 是否调用初始化函数
 * 返回：2D纹理指针
 */ 
PiTexture* PI_API pi_texture_2d_create(RenderFormat format, TextureUsage usage, uint array_size, uint num_mipmap, uint width, uint height, PiBool is_create_handle);

/** 
 * 创建3D纹理
 * usage: 纹理用途
 * format: 纹理格式
 * num_mipmap: mipmap层次数，为0表示创建所有的mipmap
 * width, height, depth: 纹理宽度，高度，深度
 * is_create_handle: 是否调用初始化函数
 * 返回：3D纹理指针
 */ 
PiTexture* PI_API pi_texture_3d_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint width, uint height, uint depth, PiBool is_create_handle);

/** 
 * 创建Cube纹理
 * usage: 纹理用途
 * format: 纹理格式
 * num_mipmap: mipmap层次数，为0表示创建所有的mipmap
 * size: 纹理大小
 * is_create_handle: 是否调用初始化函数
 * 返回：3D纹理指针
 */ 
PiTexture* PI_API pi_texture_cube_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint size, PiBool is_create_handle);

/* 获得纹理类型 */
uint PI_API pi_texture_get_type(PiTexture *texture);

/* 获得纹理的宽 */
uint PI_API pi_texture_get_width(PiTexture *texture);

/* 获得纹理的高 */
uint PI_API pi_texture_get_height(PiTexture *texture);

/* 获得纹理的深 */
uint PI_API pi_texture_get_depth(PiTexture *texture);

/* 获得纹理的格式 */
uint PI_API pi_texture_get_format(PiTexture *texture);

/* 获得纹理数组大小 */
uint PI_API pi_texture_get_array_size(PiTexture *texture);

/* 获得纹理mipmap数量 */
uint PI_API pi_texture_get_num_mipmap(PiTexture *texture);

/* 生成显卡资源 */
PiBool PI_API pi_texture_init(PiTexture *texture);

/** 
 * 生成mipmap
 */
PiBool PI_API pi_texture_build_mipmap(PiTexture *texture);

/**
 * 更新纹理数据
 */

PiBool PI_API pi_texture_update_image(PiTexture *texture, PiImage *image, uint level);

PiBool PI_API pi_texture_update_sub_image(PiTexture *texture, uint array_index, uint level, TextureCubeFace face, PiImage *image);

PiBool PI_API pi_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data);

PiBool PI_API pi_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data);

PiBool PI_API pi_texture_3d_update_by_clut(PiTexture* texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, PiColorLookUpTable* clut);

PiBool PI_API pi_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data);

/**
 * 取纹理数据，目前只支持非压缩的ARBG格式的读取
 */
PiImage* PI_API pi_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h);

PiImage* PI_API pi_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h);

/**
 * 纹理拷贝
 * 注：宽，高，深，格式，都必须相同
 * 注：不能使压缩格式
 */
PiBool PI_API pi_texture_2d_copy(PiTexture *dst, PiTexture *src,
	uint dst_array_index, uint dst_level, uint dst_x, uint dst_y,
	uint src_array_index, uint src_level, uint src_x, uint src_y, uint w, uint h);

PiBool PI_API pi_texture_3d_copy(PiTexture *dst, PiTexture *src,
	uint dst_level, uint dst_x, uint dst_y, uint dst_z, 
	uint src_level, uint src_x, uint src_y, uint src_z, uint w, uint h, uint d);

PiBool PI_API pi_texture_cube_copy(PiTexture *dst, PiTexture *src,
	TextureCubeFace dst_face, uint dst_level, uint dst_x, uint dst_y, 
	TextureCubeFace src_face, uint src_level, uint src_x, uint src_y, uint w, uint h);

/* 以下接口仅供内部使用 */

/* 取出纹理最新的采样器状态 */
void* PI_API texture_get_curr_sampler(PiTexture *texture);

PI_END_DECLS

#endif /* INCLUDE_TEXTURE_H */