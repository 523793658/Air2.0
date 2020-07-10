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

// Texture����;������ͨ������ΪcolorĿ�꣬��Ϊdepth-stencilĿ��
// ��OpenGL����
typedef enum
{
	TU_NORMAL = 0,			// ��ͨ����
	TU_COLOR = 1,			// ��ΪcolorĿ�������
	TU_DEPTH_STENCIL = 2	// ��Ϊdepth-stencilĿ�������
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
 * ����ģ��
 * ע������Ŀ��ߣ����ʽ��mipmap�����ڳ�ʼ��ʱȫ��ȷ�����޷��޸�
 */
typedef struct 
{
	TextureType	type;
	RenderFormat format;

	uint array_size;				/* ��������ĸ���������1����ʾ����һ���������� */
	uint num_mipmap;

	uint32 width;
	uint32 height;
	uint32 depth;
	
	uint compress_block_size;
	PiBool is_compressed_format;
	
	TextureUsage usage;

	TextureLevelSize *level_size;	/* ����Ԫ�ظ���Ϊnum_mipmap */

	void *impl;
}PiTexture;

PI_BEGIN_DECLS

/* �������� */
PiBool PI_API pi_texture_free(PiTexture *texture);

/**
 * ��¡��������������
 */
PiTexture* PI_API pi_texture_clone(PiTexture *texture);

/** 
 * ����2D����
 * usage: ������;
 * format: �����ʽ
 * array_size: ����1��ʾ��������
 * num_mipmap: mipmap�������Ϊ0��ʾ�������е�mipmap
 * width, height: �����ȣ��߶�
 * is_create_handle: �Ƿ���ó�ʼ������
 * ���أ�2D����ָ��
 */ 
PiTexture* PI_API pi_texture_2d_create(RenderFormat format, TextureUsage usage, uint array_size, uint num_mipmap, uint width, uint height, PiBool is_create_handle);

/** 
 * ����3D����
 * usage: ������;
 * format: �����ʽ
 * num_mipmap: mipmap�������Ϊ0��ʾ�������е�mipmap
 * width, height, depth: �����ȣ��߶ȣ����
 * is_create_handle: �Ƿ���ó�ʼ������
 * ���أ�3D����ָ��
 */ 
PiTexture* PI_API pi_texture_3d_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint width, uint height, uint depth, PiBool is_create_handle);

/** 
 * ����Cube����
 * usage: ������;
 * format: �����ʽ
 * num_mipmap: mipmap�������Ϊ0��ʾ�������е�mipmap
 * size: �����С
 * is_create_handle: �Ƿ���ó�ʼ������
 * ���أ�3D����ָ��
 */ 
PiTexture* PI_API pi_texture_cube_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint size, PiBool is_create_handle);

/* ����������� */
uint PI_API pi_texture_get_type(PiTexture *texture);

/* �������Ŀ� */
uint PI_API pi_texture_get_width(PiTexture *texture);

/* �������ĸ� */
uint PI_API pi_texture_get_height(PiTexture *texture);

/* ���������� */
uint PI_API pi_texture_get_depth(PiTexture *texture);

/* �������ĸ�ʽ */
uint PI_API pi_texture_get_format(PiTexture *texture);

/* ������������С */
uint PI_API pi_texture_get_array_size(PiTexture *texture);

/* �������mipmap���� */
uint PI_API pi_texture_get_num_mipmap(PiTexture *texture);

/* �����Կ���Դ */
PiBool PI_API pi_texture_init(PiTexture *texture);

/** 
 * ����mipmap
 */
PiBool PI_API pi_texture_build_mipmap(PiTexture *texture);

/**
 * ������������
 */

PiBool PI_API pi_texture_update_image(PiTexture *texture, PiImage *image, uint level);

PiBool PI_API pi_texture_update_sub_image(PiTexture *texture, uint array_index, uint level, TextureCubeFace face, PiImage *image);

PiBool PI_API pi_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data);

PiBool PI_API pi_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data);

PiBool PI_API pi_texture_3d_update_by_clut(PiTexture* texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, PiColorLookUpTable* clut);

PiBool PI_API pi_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data);

/**
 * ȡ�������ݣ�Ŀǰֻ֧�ַ�ѹ����ARBG��ʽ�Ķ�ȡ
 */
PiImage* PI_API pi_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h);

PiImage* PI_API pi_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h);

/**
 * ������
 * ע�����ߣ����ʽ����������ͬ
 * ע������ʹѹ����ʽ
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

/* ���½ӿڽ����ڲ�ʹ�� */

/* ȡ���������µĲ�����״̬ */
void* PI_API texture_get_curr_sampler(PiTexture *texture);

PI_END_DECLS

#endif /* INCLUDE_TEXTURE_H */