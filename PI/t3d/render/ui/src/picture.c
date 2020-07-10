#include "picture.h"

static PiRenderMesh *PI_PICTURE_QUAD_MESH = NULL;

static const char *RS_PICTURE_VS = "simplest.vs";
static const char *RS_PICTURE_FS = "picture.fs";

static PiRenderMesh *_create_picture_mesh()
{
	PiMesh *mesh;

	float tcoord[4 * 2] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	float pos[4 * 3] =
	{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	uint32 index[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, NULL, NULL, tcoord, 6, index);

	PI_PICTURE_QUAD_MESH = pi_rendermesh_new(mesh, TRUE);

	return PI_PICTURE_QUAD_MESH;
}

static void _update_transform(PiPicture *picture)
{
	PiSpatial *spatial = pi_entity_get_spatial(picture->entity);
	pi_spatial_set_local_translation(spatial, (float)picture->x, (float)picture->y, 0);
	pi_spatial_set_local_scaling(spatial, (float)picture->width, (float)picture->height, 1);

	pi_spatial_update(spatial);
}

PiPicture *PI_API pi_picture_new()
{
	PiPicture *picture = pi_new0(PiPicture, 1);
	PiRenderMesh *mesh = PI_PICTURE_QUAD_MESH;
	PiMaterial *material;

	picture->conststr_texture = pi_conststr("TEXTURE");
	picture->conststr_flip_y = pi_conststr("FLIP_Y");
	picture->conststr_uv_anim = pi_conststr("UV_ANIM");
	picture->conststr_tile_anim = pi_conststr("TILE_ANIM");
	picture->conststr_tile_blend = pi_conststr("TILE_BLEND");
	picture->conststr_u_color = pi_conststr("u_Color");
	picture->conststr_u_texture = pi_conststr("u_Texture");
	picture->conststr_u_uv_anim = pi_conststr("u_UVAnim");
	picture->conststr_u_tile_anim = pi_conststr("u_TileAnim");

	picture->entity = pi_entity_new();

	picture->width = 256;
	picture->height = 256;
	_update_transform(picture);

	if (!mesh)
	{
		mesh = _create_picture_mesh();
	}
	pi_entity_set_mesh(picture->entity, mesh);

	material = pi_material_new(RS_PICTURE_VS, RS_PICTURE_FS);
	pi_entity_set_material(picture->entity, material);
	pi_renderstate_set_default_sampler(&picture->sampler);
	pi_sampler_set_filter(&picture->sampler, TFO_MIN_MAG_LINEAR);

	picture->color[0] = 1.0f;
	picture->color[1] = 1.0f;
	picture->color[2] = 1.0f;
	picture->color[3] = 1.0f;

	pi_material_set_uniform(material, picture->conststr_u_color, UT_VEC4, 1, picture->color, FALSE);
	pi_material_set_depth_enable(material, FALSE);

	return picture;
}

void PI_API pi_picture_free(PiPicture *picture)
{
	pi_material_free(picture->entity->material);
	pi_entity_free(picture->entity);
	pi_free(picture);
}

void PI_API pi_picture_set_size(PiPicture *picture, uint width, uint height)
{
	picture->width = width;
	picture->height = height;
	_update_transform(picture);
}

void PI_API pi_picture_set_location(PiPicture *picture, sint x, sint y)
{
	picture->x = x;
	picture->y = y;
	_update_transform(picture);
}

void PI_API pi_picture_set_color(PiPicture *picture, float r, float g, float b, float a)
{
	PiMaterial *material = picture->entity->material;
	picture->color[0] = r;
	picture->color[1] = g;
	picture->color[2] = b;
	picture->color[3] = a;
	pi_material_set_uniform(material, picture->conststr_u_color, UT_VEC4, 1, picture->color, FALSE);
}

void PI_API pi_picture_set_texture(PiPicture *picture, PiTexture *texture, PiBool flip_y)
{
	PiMaterial *material = picture->entity->material;
	picture->texture = texture;

	if (texture)
	{
		pi_material_set_def(material, picture->conststr_texture, TRUE);
		pi_material_set_def(material, picture->conststr_flip_y, flip_y);
		pi_sampler_set_texture(&picture->sampler, texture);
		pi_material_set_uniform(material, picture->conststr_u_texture, UT_SAMPLER_2D, 1, &picture->sampler, FALSE);
	}
	else
	{
		pi_material_set_def(material, picture->conststr_texture, FALSE);
	}
}

void PI_API pi_picture_set_blend_mode(PiPicture *picture, EBlendMode blend_mode)
{
	PiMaterial *material = picture->entity->material;
	picture->blend_mode = blend_mode;

	switch (blend_mode)
	{
	case EBM_ALPHA:
		pi_material_set_blend(material, TRUE);
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case EBM_ALPHA_ASSOCIATIVE_INIT:
		pi_material_set_blend(material, TRUE);
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_ZERO, BF_ONE, BF_ZERO);
		break;
	case EBM_ALPHA_ASSOCIATIVE:
		pi_material_set_blend(material, TRUE);
		pi_material_set_blend_factor(material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ONE, BF_INV_SRC_ALPHA);
		break;
	case EBM_ALPHA_ASSOCIATIVE_FINISH:
		pi_material_set_blend(material, TRUE);
		pi_material_set_blend_factor(material, BF_ONE, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case EBM_NONE:
		pi_material_set_blend(material, FALSE);
		break;
	default:
		break;
	}
}

EBlendMode PI_API pi_picture_get_blend_mode(PiPicture *picture)
{
	return picture->blend_mode;
}

void PI_API pi_picture_set_texture_uv_anim(PiPicture *picture, float u, float v)
{
	PiMaterial *material = picture->entity->material;
	picture->uv_anim[0] = u;
	picture->uv_anim[1] = v;

	if (u != 0 || v != 0)
	{
		pi_material_set_def(material, picture->conststr_uv_anim, TRUE);
		pi_material_set_def(material, picture->conststr_tile_anim, FALSE);
		pi_material_set_uniform(material, picture->conststr_u_uv_anim, UT_VEC2, 1, picture->uv_anim, FALSE);
	}
	else
	{
		pi_material_set_def(material, picture->conststr_uv_anim, FALSE);
	}
}

void PI_API pi_picture_set_texture_tile_anim(PiPicture *picture, uint tile_x, uint tile_y, float frame_time, uint tile_count, PiBool is_blend)
{
	PiMaterial *material = picture->entity->material;

	PI_ASSERT(tile_x * tile_y > tile_count, "TileCount must less than the result of 'tile_x * tile_y'.");

	picture->tile_anim[0] = (float)tile_x;
	picture->tile_anim[1] = (float)tile_y;
	picture->tile_anim[2] = (float)frame_time;
	picture->tile_anim[3] = (float)(tile_count ? tile_count : tile_x * tile_y);

	if (tile_x > 1 || tile_y > 1)
	{
		pi_material_set_def(material, picture->conststr_tile_blend, is_blend);
		pi_material_set_def(material, picture->conststr_uv_anim, FALSE);
		pi_material_set_def(material, picture->conststr_tile_anim, TRUE);
		pi_material_set_uniform(material, picture->conststr_u_tile_anim, UT_VEC4, 1, picture->tile_anim, FALSE);
	}
	else
	{
		pi_material_set_def(material, picture->conststr_uv_anim, FALSE);
	}
}
