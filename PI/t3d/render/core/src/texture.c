#include <texture.h>
#include <renderwrap.h>

PiBool PI_API pi_texture_free(PiTexture *texture)
{
	PiBool r = TRUE;
	if (texture != NULL)
	{
		r = render_texture_clear(texture);
		pi_free(texture->level_size);
		pi_free(texture);
	}
	return r;
}

PiTexture *PI_API pi_texture_clone(PiTexture *tex)
{
	uint i, j;
	PiTexture *dst = NULL;
	switch (tex->type)
	{
	case TT_2D:
		dst = pi_texture_2d_create(tex->format, tex->usage, tex->array_size, tex->num_mipmap, tex->width, tex->height, TRUE);
		for (i = 0; i < tex->array_size; ++i)
		{
			for (j = 0; j < tex->num_mipmap; ++j)
			{
				TextureLevelSize *size = &tex->level_size[j];
				pi_texture_2d_copy(dst, tex, i, j, 0, 0, i, j, 0, 0, size->width, size->height);
			}
		}
		break;
	case TT_3D:
		dst = pi_texture_3d_create(tex->format, tex->usage, tex->num_mipmap, tex->width, tex->height, tex->depth, TRUE);
		for (j = 0; j < tex->num_mipmap; ++j)
		{
			TextureLevelSize *size = &tex->level_size[j];
			pi_texture_3d_copy(dst, tex, j, 0, 0, 0, j, 0, 0, 0, size->width, size->height, size->depth);
		}
		break;
	case TT_CUBE:
		dst = pi_texture_cube_create(tex->format, tex->usage, tex->num_mipmap, tex->width, TRUE);
		for (i = 0; i < 6; ++i)
		{
			for (j = 0; j < tex->num_mipmap; ++j)
			{
				TextureCubeFace face = i + CF_POSITIVE_X;
				TextureLevelSize *size = &tex->level_size[j];
				pi_texture_cube_copy(dst, tex, face, j, 0, 0, face, j, 0, 0, size->width, size->width);
			}
		}
		break;
	default:
		break;
	}
	return dst;
}

static uint _get_compress_block_size(RenderFormat format)
{
	uint size = 0;
	switch (format)
	{
	case RF_BC1:
	case RF_SIGNED_BC1:
	case RF_BC1_SRGB:
	case RF_BC4:
	case RF_SIGNED_BC4:
	case RF_BC4_SRGB:
		size = 8;
		break;
	default:
		size = 16;
		break;
	}
	return size;
}

static PiTexture *_create_texture(TextureType type, RenderFormat format, TextureUsage usage, uint array_size, uint num_mipmap, uint width, uint height, uint depth)
{
	PiTexture *texture = pi_new0(PiTexture, 1);

	texture->type = type;
	texture->format = format;
	texture->usage = usage;
	texture->array_size = array_size;
	texture->num_mipmap = num_mipmap;

	texture->width = width;
	texture->height = height;
	texture->depth = depth;

	texture->is_compressed_format = pi_renderformat_is_compressed_format(format);
	if (texture->is_compressed_format)
	{
		texture->compress_block_size = _get_compress_block_size(format);
	}
	return texture;
}

PiTexture *PI_API pi_texture_2d_create(RenderFormat format, TextureUsage usage, uint array_size, uint num_mipmap, uint width, uint height, PiBool is_create_handle)
{
	PiTexture *texture = _create_texture(TT_2D, format, usage, array_size, num_mipmap, width, height, 0);

	if (texture->num_mipmap == 0)
	{
		uint w = width, h = height;
		texture->num_mipmap = 1;
		while (w > 1 || h > 1)
		{
			++texture->num_mipmap;
			w = MAX(1, w / 2);
			h = MAX(1, h / 2);
		}
	}

	texture->level_size = pi_new0(TextureLevelSize, texture->num_mipmap);
	{
		uint level, w = width, h = height;
		for (level = 0; level < texture->num_mipmap; ++level)
		{
			texture->level_size[level].width = w;
			texture->level_size[level].height = h;
			w = MAX(1, w / 2);
			h = MAX(1, h / 2);
		}
	}

	if (is_create_handle)
	{
		if (!pi_texture_init(texture))
		{
			pi_free(texture->level_size);
			pi_free(texture);
			texture = NULL;
		}
	}
	return texture;
}

PiTexture *PI_API pi_texture_3d_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint width, uint height, uint depth, PiBool is_create_handle)
{
	PiTexture *texture = _create_texture(TT_3D, format, usage, 1, num_mipmap, width, height, depth);

	if (texture->num_mipmap == 0)
	{
		uint w = width, h = height, d = depth;
		texture->num_mipmap = 1;
		while (w > 1 || h > 1 || d > 1)
		{
			++texture->num_mipmap;
			w = MAX(1, w / 2);
			h = MAX(1, h / 2);
			d = MAX(1, d / 2);
		}
	}

	texture->level_size = pi_new0(TextureLevelSize, texture->num_mipmap);
	{
		uint level, w = width, h = height, d = depth;
		for (level = 0; level < texture->num_mipmap; ++level)
		{
			texture->level_size[level].width = w;
			texture->level_size[level].height = h;
			texture->level_size[level].depth = d;
			w = MAX(1, w / 2);
			h = MAX(1, h / 2);
			d = MAX(1, d / 2);
		}
	}

	if (is_create_handle)
	{
		if (!pi_texture_init(texture))
		{
			pi_free(texture->level_size);
			pi_free(texture);
			texture = NULL;
		}
	}
	return texture;
}

PiTexture *PI_API pi_texture_cube_create(RenderFormat format, TextureUsage usage, uint num_mipmap, uint size, PiBool is_create_handle)
{
	PiTexture *texture = _create_texture(TT_CUBE, format, usage, 1, num_mipmap, size, size, 0);

	if (texture->num_mipmap == 0)
	{
		uint s = size;
		texture->num_mipmap = 1;
		while (s > 1)
		{
			++texture->num_mipmap;
			s = MAX(1, s / 2);
		}
	}

	texture->level_size = pi_new0(TextureLevelSize, texture->num_mipmap);
	{
		uint level, s = size;
		for (level = 0; level < texture->num_mipmap; ++level)
		{
			texture->level_size[level].width = s;
			s = MAX(1, s / 2);
		}
	}

	if (is_create_handle)
	{
		if (!pi_texture_init(texture))
		{
			pi_free(texture->level_size);
			pi_free(texture);
			texture = NULL;
		}
	}
	return texture;
}

uint PI_API pi_texture_get_type(PiTexture *texture)
{
	return texture->type;
}

uint PI_API pi_texture_get_width(PiTexture *texture)
{

	return texture->width;
}

uint PI_API pi_texture_get_height(PiTexture *texture)
{
	return texture->height;
}

uint PI_API pi_texture_get_depth(PiTexture *texture)
{

	return texture->depth;
}

uint PI_API pi_texture_get_format(PiTexture *texture)
{

	return texture->format;
}

uint PI_API pi_texture_get_array_size(PiTexture *texture)
{

	return texture->array_size;
}

uint PI_API pi_texture_get_num_mipmap(PiTexture *texture)
{
	return texture->num_mipmap;
}

PiBool PI_API pi_texture_init(PiTexture *tex)
{
	return render_texture_init(tex);
}

PiBool PI_API pi_texture_build_mipmap(PiTexture *texture)
{
	return render_texture_build_mipmap(texture);
}

static PiBool _is_2d_texture_valid(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h)
{
	uint width, height;
	if (texture->type != TT_2D)
	{
		pi_log_print(LOG_WARNING, "type isn't 2d");
		return FALSE;
	}
	if (array_index >= texture->array_size)
	{
		pi_log_print(LOG_WARNING, "array_index out of range, array_index = %d", array_index);
		return FALSE;
	}

	if (level >= texture->num_mipmap)
	{
		pi_log_print(LOG_WARNING, "level out of range, level = %d", level);
		return FALSE;
	}

	width = texture->level_size[level].width;
	if (x + w > width)
	{
		pi_log_print(LOG_WARNING, "x + w out of range, x = %d, w = %d", x, w);
		return FALSE;
	}

	height = texture->level_size[level].height;
	if (y + h > height)
	{
		pi_log_print(LOG_WARNING, "y + h out of range, y = %d, h = %d", y, h);
		return FALSE;
	}

	return TRUE;
}

static PiBool _is_3d_texture_valid(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d)
{
	uint width, height, depth;
	if (texture->type != TT_3D)
	{
		pi_log_print(LOG_WARNING, "type isn't 3d");
		return FALSE;
	}
	if (level >= texture->num_mipmap)
	{
		pi_log_print(LOG_WARNING, "level out of range, level = %d", level);
		return FALSE;
	}

	width = texture->level_size[level].width;
	if (x + w > width)
	{
		pi_log_print(LOG_WARNING, "x + w out of range, x = %d, w = %d", x, w);
		return FALSE;
	}

	height = texture->level_size[level].height;
	if (y + h > height)
	{
		pi_log_print(LOG_WARNING, "y + h out of range, y = %d, h = %d", y, h);
		return FALSE;
	}

	depth = texture->level_size[level].depth;
	if (z + d > depth)
	{
		pi_log_print(LOG_WARNING, "z + d out of range, z = %d, d = %d", z, d);
		return FALSE;
	}

	return TRUE;
}

static PiBool _is_cube_texture_valid(PiTexture *texture, uint level, uint x, uint y, uint w, uint h)
{
	uint width;
	if (texture->type != TT_CUBE)
	{
		pi_log_print(LOG_WARNING, "type isn't cube");
		return FALSE;
	}

	if (level >= texture->num_mipmap)
	{
		pi_log_print(LOG_WARNING, "level out of range, level = %d", level);
		return FALSE;
	}

	width = texture->level_size[level].width;
	if (x + w > width)
	{
		pi_log_print(LOG_WARNING, "x + w out of range, x = %d, w = %d", x, w);
		return FALSE;
	}

	return TRUE;
}

PiBool PI_API pi_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data)
{
	return render_texture_2d_update(texture, array_index, level, x, y, w, h, data_size, data);
}

PiBool PI_API pi_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data)
{
	return render_texture_3d_update(texture, level, x, y, z, w, h, d, data_size, data);
}

PiBool PI_API pi_texture_3d_update_by_clut(PiTexture* texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, PiColorLookUpTable* clut)
{
	return render_texture_3d_update(texture, level, x, y, z, w, h, d, clut->data_size, (byte*)clut->data);
}

PiBool PI_API pi_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data)
{
	return render_texture_cube_update(texture, level, face, x, y, w, h, data_size, data);
}

PiImage *PI_API pi_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h)
{
	return render_texture_2d_get(texture, array_index, level, x, y, w, h);
}

PiImage *PI_API pi_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h)
{
	return render_texture_cube_get(texture, level, face, x, y, w, h);
}

PiBool PI_API pi_texture_2d_copy(PiTexture *dst, PiTexture *src,
                                 uint dst_array_index, uint dst_level, uint dst_x, uint dst_y,
                                 uint src_array_index, uint src_level, uint src_x, uint src_y, uint w, uint h)
{
	if (dst->format != src->format)
	{
		pi_log_print(LOG_WARNING, "copy type or format is different between dst and src");
		return FALSE;
	}

	if (pi_renderformat_is_compressed_format(dst->format))
	{
		pi_log_print(LOG_WARNING, "copy texture format must not be compress");
		return FALSE;
	}

	if (!_is_2d_texture_valid(dst, dst_array_index, dst_level, dst_x, dst_y, w, h))
	{
		pi_log_print(LOG_WARNING, "copy dst is invalid texture");
		return FALSE;
	}

	if (!_is_2d_texture_valid(src, src_array_index, src_level, src_x, src_y, w, h))
	{
		pi_log_print(LOG_WARNING, "copy src is invalid texture");
		return FALSE;
	}

	return render_texture_2d_copy(dst, src, dst_array_index, dst_level, dst_x, dst_y, src_array_index, src_level, src_x, src_y, w, h);
}

PiBool PI_API pi_texture_3d_copy(PiTexture *dst, PiTexture *src,
                                 uint dst_level, uint dst_x, uint dst_y, uint dst_z,
                                 uint src_level, uint src_x, uint src_y, uint src_z, uint w, uint h, uint d)
{
	if (dst->format != src->format)
	{
		pi_log_print(LOG_WARNING, "copy type or format is different between dst and src");
		return FALSE;
	}

	if (pi_renderformat_is_compressed_format(dst->format))
	{
		pi_log_print(LOG_WARNING, "copy texture format must not be compress");
		return FALSE;
	}

	if (!_is_3d_texture_valid(dst, dst_level, dst_x, dst_y, dst_z, w, h, d))
	{
		pi_log_print(LOG_WARNING, "copy dst is invalid texture");
		return FALSE;
	}

	if (!_is_3d_texture_valid(src, src_level, src_x, src_y, src_z, w, h, d))
	{
		pi_log_print(LOG_WARNING, "copy src is invalid texture");
		return FALSE;
	}

	return render_texture_3d_copy(dst, src, dst_level, dst_x, dst_y, dst_z, src_level, src_x, src_y, src_z, w, h, d);
}

PiBool PI_API pi_texture_cube_copy(PiTexture *dst, PiTexture *src,
                                   TextureCubeFace dst_face, uint dst_level, uint dst_x, uint dst_y,
                                   TextureCubeFace src_face, uint src_level, uint src_x, uint src_y, uint w, uint h)
{
	if (dst->format != src->format)
	{
		pi_log_print(LOG_WARNING, "copy type or format is different between dst and src");
		return FALSE;
	}

	if (pi_renderformat_is_compressed_format(dst->format))
	{
		pi_log_print(LOG_WARNING, "copy texture format must not be compress");
		return FALSE;
	}

	if (!_is_cube_texture_valid(dst, dst_level, dst_x, dst_y, w, h))
	{
		pi_log_print(LOG_WARNING, "copy dst is invalid texture");
		return FALSE;
	}

	if (!_is_cube_texture_valid(src, src_level, src_x, src_y, w, h))
	{
		pi_log_print(LOG_WARNING, "copy src is invalid texture");
		return FALSE;
	}

	return render_texture_cube_copy(dst, src, dst_face, dst_level, dst_x, dst_y, src_face, src_level, src_x, src_y, w, h);
}

void *PI_API texture_get_curr_sampler(PiTexture *tex)
{
	return render_texture_get_curr_sampler(tex);
}

PiBool PI_API pi_texture_update_sub_image(PiTexture *texture,
        uint array_index, uint level, TextureCubeFace face, PiImage *image)
{
	uint size = 0;
	PiBool r = FALSE;
	void *img_data = pi_render_image_get_pointer(image, 0, 0, &size);
	switch (texture->type)
	{
	case TT_2D:
		r = pi_texture_2d_update(texture, array_index, level, 0, 0, image->width, image->height, size, img_data);
		break;
	case TT_3D:
		r = pi_texture_3d_update(texture, level, 0, 0, 0, image->width, image->height, image->depth, size, img_data);
		break;
	case TT_CUBE:
		r = pi_texture_cube_update(texture, level, face, 0, 0, image->width, image->height, size, img_data);
		break;
	default:
		break;
	}
	return r;
}

PiBool PI_API pi_texture_update_image(PiTexture *texture, PiImage *image, uint from_level)
{
	PiBool r = FALSE;
	TextureCubeFace face;
	uint i, level, array_index;
	uint d, w = image->width, h = image->height;

	for (i = 0; i < from_level; ++i)
	{
		w /= 2;
		h /= 2;
	}

	if (w != texture->width || h != texture->height)
	{
		pi_log_print(LOG_WARNING, "update image failed, width or height is different");
		return FALSE;
	}

	switch (texture->type)
	{
	case TT_2D:
		for (array_index = 0; array_index < texture->array_size; ++array_index)
		{
			w = image->width, h = image->height;
			for (level = 0; level < texture->num_mipmap; ++level)
			{
				uint image_size;
				void *image_data = pi_render_image_get_pointer(image, array_index, level + from_level, &image_size);
				if (image_data != NULL)
				{
					r = pi_texture_2d_update(texture, array_index, level, 0, 0, w, h, image_size, image_data);
				}
				w = MAX(1, w / 2);
				h = MAX(1, h / 2);
			}
		}
		break;
	case TT_3D:
		i = 0, w = image->width, h = image->height, d = image->depth;
		for (level = 0; level < texture->num_mipmap; ++level)
		{
			uint image_size;
			void *image_data = pi_render_image_get_pointer(image, 0, level + from_level, &image_size);
			r = pi_texture_3d_update(texture, level, 0, 0, 0, w, h, d, image_size, image_data);

			w = MAX(1, w / 2);
			h = MAX(1, h / 2);
			d = MAX(1, d / 2);
		}
		break;
	case TT_CUBE:
		for (face = 0; face < 6; ++face)
		{
			w = image->width, h = image->height;
			for (level = 0; level < texture->num_mipmap; ++level)
			{
				uint image_size;
				void *image_data = pi_render_image_get_pointer(image, 0, level + from_level, &image_size);
				if (image_data != NULL)
				{
					r = pi_texture_cube_update(texture, level, face, 0, 0, w, h, image_size, image_data);
				}

				w = MAX(1, w / 2);
				h = MAX(1, h / 2);
			}
		}
		break;
	default:
		break;
	}
	return r;
}