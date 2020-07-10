#include <image.h>
#include <FreeImage.h>
#include "dds.h"

static void convert_to_bgr(byte *data, uint width, uint height, uint byte_per_pixel)
{
	uint i, j;
	byte *pixels = data;
	for(j = 0; j < height; ++j)
	{
		for(i = 0; i < width; ++i)
		{
			byte d = pixels[0];
			pixels[0] = pixels[2];
			pixels[2] = d;
			
			pixels += byte_per_pixel;
		}
	}
}

/* 加载除DDS格式之外的图像数据 */
static PiBool _image_load_other(PiImage *image, byte *data, uint size)
{	
	uint bpp;
	FREE_IMAGE_TYPE image_type;
	FREE_IMAGE_FORMAT image_format;
	FREE_IMAGE_COLOR_TYPE colour_type;
	
	FIBITMAP *bitmap = NULL;
	FIMEMORY *file_mem = FreeImage_OpenMemory(data, size);
	if(file_mem == NULL)
	{
		pi_log_print(LOG_WARNING, "image file mem null");
		return FALSE;
	}
	image_format = FreeImage_GetFileTypeFromMemory(file_mem, 0);
	if(image_format == FIF_UNKNOWN)
	{
		pi_log_print(LOG_WARNING, "image format error");
		FreeImage_CloseMemory(file_mem);
		return FALSE;
	}

	bitmap = FreeImage_LoadFromMemory(image_format, file_mem, 0);
	FreeImage_CloseMemory(file_mem);

	if(bitmap == NULL)
	{
		pi_log_print(LOG_WARNING, "create bit map error");
		return FALSE; 
	}
	
	/* 默认就是2D纹理，没有array_size和array_size */
	image->type = TT_2D;
	image->num_mipmap = 1;
	image->array_size = 1;
	
	image->depth = 1;
	image->width = FreeImage_GetWidth(bitmap);
	image->height = FreeImage_GetHeight(bitmap);

	bpp = FreeImage_GetBPP(bitmap);
	colour_type = FreeImage_GetColorType(bitmap);
	image_type = FreeImage_GetImageType(bitmap);
	switch(image_type)
	{
	case FIT_BITMAP:
		// Standard image type
		// Perform any colour conversions for greyscale
		if (colour_type == FIC_MINISWHITE || colour_type == FIC_MINISBLACK)
		{
			FIBITMAP* new_bitmap = FreeImage_ConvertToGreyscale(bitmap);
			// free old bitmap and replace
			FreeImage_Unload(bitmap);
			bitmap = new_bitmap;
			// get new formats
			bpp = FreeImage_GetBPP(bitmap);
		}
		// Perform any colour conversions for RGB
		else if (bpp < 8 || colour_type == FIC_PALETTE || colour_type == FIC_CMYK)
		{
			FIBITMAP* new_bitmap =  NULL;    
			if (FreeImage_IsTransparent(bitmap))
			{
				// convert to 32 bit to preserve the transparency 
				// (the alpha byte will be 0 if pixel is transparent)
				new_bitmap = FreeImage_ConvertTo32Bits(bitmap);
			}
			else
			{
				// no transparency - only 3 bytes are needed
				new_bitmap = FreeImage_ConvertTo24Bits(bitmap);
			}

			// free old bitmap and replace
			FreeImage_Unload(bitmap);
			bitmap = new_bitmap;
			// get new formats
			bpp = FreeImage_GetBPP(bitmap);
		}

		if (bpp == 24)
		{
			FIBITMAP* new_bitmap = NULL;
			new_bitmap = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(bitmap);
			bitmap = new_bitmap;
			bpp = FreeImage_GetBPP(bitmap);
		}

		// by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
		switch(bpp)
		{
		case 8:
			image->format = RF_A8;
			break;
		case 32:
			image->format = RF_ABGR8;
			break;
		default:
			PI_ASSERT(FALSE, "bpp isn't valid");
			return FALSE;
		};
		break;
	case FIT_UINT16:
	case FIT_INT16:
		// 16-bit greyscale
		image->format = RF_R16;
		break;
	case FIT_FLOAT:
		// Single-component floating point data
		image->format = RF_R32F;
		break;
	case FIT_RGB16:
		image->format = RF_BGR16;
		break;
	case FIT_RGBA16:
		image->format = RF_ABGR16;
		break;
	case FIT_RGBF:
		image->format = RF_BGR32F;
		break;
	case FIT_RGBAF:
		image->format = RF_ABGR32F;
		break;
	default:
		PI_ASSERT(FALSE, "Unknown or unsupported image format");
		break;
	};

	/* 如用OpenGL，需要上下倒转 */

	{
		byte *dst, *src = FreeImage_GetBits(bitmap);
		uint src_pitch = FreeImage_GetPitch(bitmap);

		sint y;
		ImageInitData idata;

		idata.row_pitch = image->width * pi_renderformat_get_numbytes(image->format);
		
		idata.size = idata.slice_pitch = idata.row_pitch * image->height;
		dst = idata.data = pi_malloc0(idata.size);
		
		if (dst == NULL) 
		{
			pi_log_print(LOG_WARNING, "image malloc error!! size is %d", idata.size);
			FreeImage_Unload(bitmap);
			return FALSE;
		}

		for (y = image->height - 1; y >= 0; --y)
		{
			byte *s = src + y * src_pitch;
			pi_memcpy_inline(dst, s, idata.row_pitch);
			dst += idata.row_pitch;
		}
		pi_dvector_push(&image->data_vector, &idata);
	}	

	image->row_pitch = image->width * pi_renderformat_get_numbytes(image->format);
	image->slice_pitch = image->height * image->row_pitch;

	// 如果是DDS，需要将ARGB转成ABGR
	if(image_format == FIF_DDS)
	{
		ImageInitData *idata = (ImageInitData *)pi_dvector_get(&image->data_vector, 0);
		switch(image->format)
		{
		case RF_A8:
			break;
		case RF_BGR8:
			convert_to_bgr(idata->data, image->width, image->height, 3);
			break;
		case RF_ABGR8:
			convert_to_bgr(idata->data, image->width, image->height, 4);
			break;
		default:
			PI_ASSERT(FALSE, "dds's decompress failed, format isn't valid");
			FreeImage_Unload(bitmap);
			return FALSE;
		}
	}

	FreeImage_Unload(bitmap);
	return TRUE;
}

static PiBool _image_data_init(PiImage *image, byte *data)
{
	ImageInitData idata;
	uint texel_size = pi_renderformat_get_numbytes(image->format);
	
	idata.row_pitch = image->width * texel_size;
	idata.size = idata.slice_pitch = idata.row_pitch * image->height;
	idata.data = pi_malloc0(idata.size);
	if(idata.data != NULL)
	{
		if(data != NULL)
		{
			pi_memcpy_inline(idata.data, data, idata.size);
		}
		pi_dvector_push(&image->data_vector, &idata);
	}
	return idata.data != NULL;
}

static PiImage* _init_image(PiImage *image, uint w, uint h, RenderFormat format)
{
	image->is_remove = FALSE;
	image->type = TT_2D;
	image->format = format;

	image->num_mipmap = 1;
	image->array_size = 1;

	image->width = w;
	image->height = h;
	image->depth = 1;
	image->row_pitch = w * pi_renderformat_get_numbytes(format);
	image->slice_pitch = h * image->row_pitch;

	pi_dvector_init(&image->data_vector, sizeof(ImageInitData));
	return image;
}

PiImage* PI_API pi_render_image_new(uint w, uint h, RenderFormat format, byte *data)
{
	PiImage *image;
	if(pi_renderformat_is_compressed_format(format))
	{
		PI_ASSERT(FALSE, "format is invalid");
		return NULL;
	}

	image = pi_new0(PiImage, 1);
	_init_image(image, w, h, format);
	if(!_image_data_init(image, data))
	{
		pi_dvector_clear(&image->data_vector, TRUE);
		pi_free(image);
		image = NULL;
	}
	return image;
}

PiImage* PI_API pi_render_image_load(byte *data, uint size, PiBool is_decompress)
{
	PiBool r;
	PiImage *image = pi_new0(PiImage, 1);
	if(data == NULL || size == 0)
	{
		pi_log_print(LOG_WARNING, "data or size error");
		return NULL;
	}
	
	_init_image(image, 1, 1, RF_ABGR8);
	
	if(dds_is_valid(data, size) && !is_decompress)
	{
		r = dds_load(image, data, size);
	}
	else 
	{
		r = _image_load_other(image, data, size);
	}
	if(!r)
	{
		pi_log_print(LOG_WARNING, "render image load error!");
		pi_free(image);
		image = NULL;
	}
	return image;
}
static void _clear_image(PiImage *image)
{
	uint i, size = pi_dvector_size(&image->data_vector);
	for(i = 0; i < size; ++i)
	{
		ImageInitData *idata = pi_dvector_get(&image->data_vector, i);
		pi_free(idata->data);
	}
	pi_dvector_clear(&image->data_vector, TRUE);
}

void PI_API pi_render_image_free(PiImage *image)
{
	if(image != NULL)
	{
		_clear_image(image);
		image->is_remove = FALSE;
		pi_free(image);
	}	
}

byte* PI_API pi_render_image_encode(PiImage *image, ImageEncodeType type, uint flag, uint *p_size)
{
	uint size = 0;
	uint pixel = 4;	/* 一像素数据多少字节 */
	FREE_IMAGE_TYPE image_type;
	byte *result = NULL;
	FIBITMAP *bitmap = NULL;
	const uint BITS_PER_BYTE = 8;	/* 一个字节有8位 */
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	
	if(p_size != NULL)
	{
		*p_size = 0;
	}

	if(pi_renderformat_is_compressed_format(image->format))
	{
		pi_log_print(LOG_WARNING, "encode image failed, format is compress");
		return NULL;
	}

	if(flag > 100) 
	{
		flag = 100;
	}
	
	switch(type)
	{
	case IET_PNG:
		format = FIF_PNG;
		break;
	case IET_JPG:
		format = FIF_JPEG;
		break;
	case IET_BMP:
		format = FIF_BMP;
		break;
	case IET_EXR:
		format = FIF_EXR;
		break;
	default:
		return NULL;
	}
	switch(image->format)
	{
	case RF_A8:
		pixel = 1;
		image_type = FIT_BITMAP;
		break;
	case RF_ABGR8:
		pixel = 4;
		image_type = FIT_BITMAP;
		break;
	case RF_BGR8:
		pixel = 3;
		image_type = FIT_BITMAP;
		break;
	case RF_R32F:
		image_type = FIT_FLOAT;
		pixel = 4;
		break;
	case RF_BGR32F:
		image_type = FIT_RGBF;
		pixel = 12;
		break;
	default:
		pi_log_print(LOG_WARNING, "encode image failed, format is compress");
		return NULL;
	}

	bitmap = FreeImage_AllocateT(image_type, image->width, image->height, BITS_PER_BYTE * pixel, 0, 0, 0);
	if(bitmap == NULL)
	{
		return NULL;
	}
	
	{
		ImageInitData *idata;
		uint i, pitch = FreeImage_GetPitch(bitmap);
		BYTE *p_data = FreeImage_GetBits(bitmap);
		PI_ASSERT(pi_dvector_size(&image->data_vector) == 1, "size is out of range");
		
		idata = pi_dvector_get(&image->data_vector, 0);
		for(i = 0; i < image->height; ++i)
		{
			pi_memcpy_inline(p_data, idata->data + i * idata->row_pitch, idata->row_pitch);
			p_data += pitch;
		}
	}
		 
	{
		FIMEMORY *mem = FreeImage_OpenMemory(0, 0);
		FreeImage_FlipVertical(bitmap);
		if(FreeImage_SaveToMemory(format, bitmap, mem, flag))
		{
			BYTE *temp_data = NULL;
			FreeImage_AcquireMemory(mem, &temp_data, (DWORD *)&size);
			result = pi_malloc(size);
			pi_memcpy_inline(result, temp_data, size);
		}
		FreeImage_CloseMemory(mem);
	}
	
	FreeImage_Unload(bitmap);
	
	if(p_size)
		*p_size = size;
	return result;
}

TextureType PI_API pi_render_image_get_type(PiImage *image)
{
	return image->type;
}

uint PI_API pi_render_image_get_width(PiImage *image)
{
	return image->width;
}

uint PI_API pi_render_image_get_height(PiImage *image)
{
	return image->height;
}

uint PI_API pi_render_image_get_depth(PiImage *image)
{
	return image->depth;
}

RenderFormat PI_API pi_render_image_get_format(PiImage *image)
{
	return image->format;
}

uint PI_API pi_render_image_get_array_size(PiImage *image)
{
	return image->array_size;
}

uint PI_API pi_render_image_get_num_mipmap(PiImage *image)
{
	return image->num_mipmap;
}

uint PI_API pi_render_image_get_size(PiImage *image, uint array_index, uint level)
{
	ImageInitData *idata;
	uint index, size = pi_dvector_size(&image->data_vector);

	if(pi_renderformat_is_compressed_format(image->format))
	{
		return 0;
	}

	if(array_index >= image->array_size || level >= image->num_mipmap)
	{
		return 0;
	}

	if(image->type == TT_CUBE)
	{
		index = 6 * image->num_mipmap + level;
	}
	else
	{
		index = array_index * image->num_mipmap + level;
	}
	
	if(index >= size)
	{
		PI_ASSERT(FALSE, "size is out of range");
		return 0;
	}
	
	idata = pi_dvector_get(&image->data_vector, index);

	return idata->size;
}

void* PI_API pi_render_image_get_pointer(PiImage *image, uint array_index, uint level, uint *pt_size)
{
	ImageInitData *idata;
	uint index, size = pi_dvector_size(&image->data_vector);
	
	if(array_index >= image->array_size || level >= image->num_mipmap)
	{
		return NULL;
	}
	
	if(image->type == TT_CUBE)
	{
		index = 6 * image->num_mipmap + level;
	}
	else
	{
		index = array_index * image->num_mipmap + level;
	}

	idata = pi_dvector_get(&image->data_vector, index);

	if(index >= size)
	{
		PI_ASSERT(FALSE, "size is out of range");
		return NULL;
	}

	if(pt_size != NULL)
	{
		*pt_size = idata->size;
	}
	return idata->data;
}

void PI_API pi_render_image_vflip(PiImage *image)
{
	byte *temp;
	uint i, size;
	if(pi_renderformat_is_compressed_format(image->format))
	{
		PI_ASSERT(FALSE, "format is invalid");
		return;
	}
	
	size = image->width * pi_renderformat_get_numbytes(image->format);
	temp = pi_malloc(size);
	
	size = pi_dvector_size(&image->data_vector);
	for(i = 0; i < size; ++i)
	{
		ImageInitData *idata = pi_dvector_get(&image->data_vector, i);
		
		byte *head = idata->data;
		byte *tail = idata->data + (idata->slice_pitch - 1) * idata->row_pitch;
		while(tail > head)
		{
			pi_memcpy_inline(temp, head, idata->row_pitch);
			pi_memcpy_inline(head, tail, idata->row_pitch);
			pi_memcpy_inline(tail, temp, idata->row_pitch);
			head += idata->row_pitch;
			tail -= idata->row_pitch;
		}
	}
	pi_free(temp);
}