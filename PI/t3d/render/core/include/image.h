#ifndef INCLUDE_IMAGE_H
#define INCLUDE_IMAGE_H

#include <pi_lib.h>
#include <renderformat.h>

/**
 * ��������
 */
typedef enum 
{
	TT_2D,
	TT_3D,
	TT_CUBE,
}TextureType;

/* �������ͣ�ֻ����ͼ����� */
typedef enum
{
	IET_PNG,	/* pngͼƬ��ʽ */
	IET_JPG,	/* jpegͼƬ��ʽ */
	IET_BMP,	/* bmpͼƬ��ʽ */
	IET_EXR		/* exrͼƬ��ʽ */
}ImageEncodeType;

typedef struct 
{
	byte *data;
	uint size;

	uint row_pitch;
	uint slice_pitch;
}ImageInitData;

typedef struct 
{
	/* �������� */
	TextureType type;

	/* ͼƬ���ݵĿ�ȣ���λ������ */
	uint width;

	/* ͼƬ���ݵĸ߶ȣ���λ������ */
	uint height;
	
	/* ͼƬ���ݵ���ȣ���λ������ */
	uint depth;
	
	/* �п�࣬��λ���ֽ� */
	uint row_pitch;
	
	/* ���࣬��λ���ֽ� */
	uint slice_pitch;

	/* �����С */
	uint array_size;

	/* mipmap���� */
	uint num_mipmap;

	/* ͼ�����Ⱦ��ʽ������RGB8�� */
	RenderFormat format;

	/**
	 * ͼ���������飬ÿ��Ԫ�ض���ImageData 
	 * ������֯˳�򣺶�ά���飬��face����array�����mipmap
	 */
	PiDvector data_vector;

	PiBool is_remove;
}PiImage;

PI_BEGIN_DECLS

/** 
 * ��ָ����߳�ʼ��ͼ��
 * ���ܴ���DDSͼ�� 
 * ���֮ǰ����image������init��load��Ҫ�ȵ���clear�ٵ��øú��� 
 */
PiImage* PI_API pi_render_image_new(uint w, uint h, RenderFormat format, byte *data);

/** 
 * �����ݵ���ͼƬ����
 * ���֮ǰ����image������init��load��Ҫ�ȵ���clear�ٵ��øú��� 
 */
PiImage* PI_API pi_render_image_load(byte *data, uint size, PiBool is_decompress);

/* ���ͼƬ���� */
void PI_API pi_render_image_free(PiImage *image);

/** 
 * ͼ�����
 * type����������
 * ���ļ�����Ϊjpgʱ��flag��ʾͼƬ��������ֵΪ1��100��Խ�߱�ʾ����Խ��
 * p_size: ���ڷ��ر���֮����ֽ���
 * ���أ��ڴ�飬��pi_free�����ͷ�
 */
byte* PI_API pi_render_image_encode(PiImage *image, ImageEncodeType type, uint flag, uint *p_size);

/**
 * ȡͼ�������
 */
TextureType PI_API pi_render_image_get_type(PiImage *image);

/**
 * ȡͼ��Ŀ�
 */
uint PI_API pi_render_image_get_width(PiImage *image);

/**
 * ȡͼ��ĸ�
 */
uint PI_API pi_render_image_get_height(PiImage *image);

/**
 * ȡͼ�����
 */
uint PI_API pi_render_image_get_depth(PiImage *image);

/**
 * ȡͼ��ĸ�ʽ
 */
RenderFormat PI_API pi_render_image_get_format(PiImage *image);

/**
 * ȡͼ�������С
 */
uint PI_API pi_render_image_get_array_size(PiImage *image);

/**
 * ȡmipmap����
 */
uint PI_API pi_render_image_get_num_mipmap(PiImage *image);

/**
 * ȡͼ�����ݵĴ�С
 * ��ѹ��
 */
uint PI_API pi_render_image_get_size(PiImage *image, uint array_index, uint level);

/**
 * ȡͼ�����ݵ�ָ��
 * ��ѹ��
 */
void* PI_API pi_render_image_get_pointer(PiImage *image, uint array_index, uint level, uint *pt_size);

/**
 * ���·�ת����������DDS��ʽ
 */
void PI_API pi_render_image_vflip(PiImage *image);

PI_END_DECLS

#endif /* INCLUDE_IMAGE_H */