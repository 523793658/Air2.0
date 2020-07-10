
#include "texture_atlas.h"

#define TEXTURE_INIT_SIZE (512)

struct PiTextureAtlas
{
	uint width, height;							/* 当前纹理的宽高 */
	uint current_x, current_y;					/* 当前可以区域的位置 */
	uint row_offset;							/* 换行之后的横向偏移 */
	uint row_height;							/* 当前行的高度 */
	PiTexture *texture;
	PiTextureAtlasData data;
};

PiTextureAtlas *PI_API pi_texture_atlas_new()
{
	byte *texture_data;
	PiTextureAtlas *atlas = pi_new0(PiTextureAtlas, 1);

	atlas->width = TEXTURE_INIT_SIZE;
	atlas->height = TEXTURE_INIT_SIZE;

	atlas->data.atlas_size[0] = (float)atlas->width;
	atlas->data.atlas_size[1] = (float)atlas->height;

	atlas->texture = pi_texture_2d_create(RF_A8, TU_NORMAL, 1, 1, TEXTURE_INIT_SIZE, TEXTURE_INIT_SIZE, TRUE);
	texture_data = pi_new0(byte, atlas->width * atlas->height);
	pi_texture_2d_update(atlas->texture, 0, 0, 0, 0, atlas->width, atlas->height, atlas->width * atlas->height, texture_data);
	pi_free(texture_data);

	pi_renderstate_set_default_sampler(&atlas->data.atlas_sampler);

	pi_sampler_set_filter(&atlas->data.atlas_sampler, TFO_MIN_MAG_POINT);
	pi_sampler_set_addr_mode(&atlas->data.atlas_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&atlas->data.atlas_sampler, atlas->texture);

	return atlas;
}

void PI_API pi_texture_atlas_delete(PiTextureAtlas *atlas)
{
	pi_texture_free(atlas->texture);
	pi_free(atlas);
}

static void _extend_texture(PiTextureAtlas *atlas)
{
	uint width = atlas->width, height = atlas->height;		/* width/height表示扩展前的宽高 */
	byte *texture_data;
	PiTexture *texture;

	/* 高度扩展 */
	if (height < width)
	{
		atlas->height = 2 * height;

		atlas->row_offset = 0;

		atlas->current_x = 0;
		atlas->current_y = height;
	}
	/* 宽度扩展 */
	else
	{
		atlas->width = 2 * width;

		atlas->row_offset = width;

		atlas->current_x = width;
		atlas->current_y = 0;
	}

	atlas->row_height = 0;

	texture = pi_texture_2d_create(RF_A8, TU_NORMAL, 1, 1, atlas->width, atlas->height, TRUE);
	texture_data = pi_new0(byte, atlas->width * atlas->height);
	pi_texture_2d_update(texture, 0, 0, 0, 0, atlas->width, atlas->height, atlas->width * atlas->height, texture_data);
	pi_free(texture_data);

	pi_texture_2d_copy(texture, atlas->texture, 0, 0, 0, 0, 0, 0, 0, 0, width, height);

	pi_sampler_set_texture(&atlas->data.atlas_sampler, texture);
	pi_texture_free(atlas->texture);
	atlas->texture = texture;

	atlas->data.atlas_size[0] = (float)atlas->width;
	atlas->data.atlas_size[1] = (float)atlas->height;
}

void PI_API pi_texture_atlas_add_tile(PiTextureAtlas *atlas, byte *bitmap, uint width, uint height, uint offset[2])
{
	if (atlas->current_x + width > atlas->width)
	{
		atlas->current_x = atlas->row_offset;
		atlas->current_y += atlas->row_height;
		atlas->row_height = 0;
	}
	if (atlas->current_y + height > atlas->height)
	{
		_extend_texture(atlas);
	}

	offset[0] = atlas->current_x;
	offset[1] = atlas->current_y;

	pi_texture_2d_update(atlas->texture, 0, 0, atlas->current_x, atlas->current_y, width, height, width * height, bitmap);

	atlas->current_x += width;

	atlas->row_height = MAX(atlas->row_height, height);
}

PiTextureAtlasData *PI_API pi_texture_atlas_get_data(PiTextureAtlas *atlas)
{
	return &atlas->data;
}
