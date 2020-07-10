#ifndef INCLUDE_IMAGE_H
#define INCLUDE_IMAGE_H

#include <pi_lib.h>
#include <renderformat.h>

/**
 * 纹理类型
 */
typedef enum 
{
	TT_2D,
	TT_3D,
	TT_CUBE,
}TextureType;

/* 编码类型：只用于图像编码 */
typedef enum
{
	IET_PNG,	/* png图片格式 */
	IET_JPG,	/* jpeg图片格式 */
	IET_BMP,	/* bmp图片格式 */
	IET_EXR		/* exr图片格式 */
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
	/* 纹理类型 */
	TextureType type;

	/* 图片数据的宽度，单位：像素 */
	uint width;

	/* 图片数据的高度，单位：像素 */
	uint height;
	
	/* 图片数据的深度，单位：像素 */
	uint depth;
	
	/* 行跨距，单位：字节 */
	uint row_pitch;
	
	/* 面跨距，单位：字节 */
	uint slice_pitch;

	/* 数组大小 */
	uint array_size;

	/* mipmap数量 */
	uint num_mipmap;

	/* 图像的渲染格式，比如RGB8等 */
	RenderFormat format;

	/**
	 * 图像数据数组，每个元素都是ImageData 
	 * 数据组织顺序：二维数组，先face，再array，最后mipmap
	 */
	PiDvector data_vector;

	PiBool is_remove;
}PiImage;

PI_BEGIN_DECLS

/** 
 * 用指定宽高初始化图像
 * 不能创建DDS图像 
 * 如果之前已用image调用了init或load，要先调用clear再调用该函数 
 */
PiImage* PI_API pi_render_image_new(uint w, uint h, RenderFormat format, byte *data);

/** 
 * 从数据导入图片数据
 * 如果之前已用image调用了init或load，要先调用clear再调用该函数 
 */
PiImage* PI_API pi_render_image_load(byte *data, uint size, PiBool is_decompress);

/* 清空图片数据 */
void PI_API pi_render_image_free(PiImage *image);

/** 
 * 图像编码
 * type：编码类型
 * 当文件类型为jpg时候，flag表示图片的质量，值为1到100，越高表示质量越好
 * p_size: 用于返回编码之后的字节数
 * 返回：内存块，用pi_free函数释放
 */
byte* PI_API pi_render_image_encode(PiImage *image, ImageEncodeType type, uint flag, uint *p_size);

/**
 * 取图像的类型
 */
TextureType PI_API pi_render_image_get_type(PiImage *image);

/**
 * 取图像的宽
 */
uint PI_API pi_render_image_get_width(PiImage *image);

/**
 * 取图像的高
 */
uint PI_API pi_render_image_get_height(PiImage *image);

/**
 * 取图像的深
 */
uint PI_API pi_render_image_get_depth(PiImage *image);

/**
 * 取图像的格式
 */
RenderFormat PI_API pi_render_image_get_format(PiImage *image);

/**
 * 取图像数组大小
 */
uint PI_API pi_render_image_get_array_size(PiImage *image);

/**
 * 取mipmap数量
 */
uint PI_API pi_render_image_get_num_mipmap(PiImage *image);

/**
 * 取图像数据的大小
 * 非压缩
 */
uint PI_API pi_render_image_get_size(PiImage *image, uint array_index, uint level);

/**
 * 取图像数据的指针
 * 非压缩
 */
void* PI_API pi_render_image_get_pointer(PiImage *image, uint array_index, uint level, uint *pt_size);

/**
 * 上下翻转，不适用于DDS格式
 */
void PI_API pi_render_image_vflip(PiImage *image);

PI_END_DECLS

#endif /* INCLUDE_IMAGE_H */