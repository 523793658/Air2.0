
#include "text.h"

struct PiText
{
	PiFontManager *font_manager;
	PiFontFamilyType family_type;
	PiFontFaceStyle face_style;
	sint point_size;
	PiOutlineType outline_type;
	sint outline_thickness;
	PiBool bold;
	sint strength;
	PiBool italic;
	sint lean;

	PiBool center_enable;
	uint num_characters;
	wchar *characters;

	float color[4];

	PiBool available;

	PiBool is_font_face_size_update;
	PiFontFaceSize *font_face_size;

	PiBool is_mesh_update;
	PiRenderMesh *mesh;

	PiBool is_material_update;
	PiMaterial *material;

	PiEntity *entity;

	PiTextLayout layout;
};

static const char *RS_TEXT_VS = "text.vs";
static const char *RS_TEXT_FS = "text.fs";

PiText *PI_API pi_text_new(PiFontManager *font_manager)
{
	PiText *text;

	if (font_manager == NULL)
	{
		return NULL;
	}

	text = pi_new0(PiText, 1);

	text->font_manager = font_manager;
	text->family_type = FFT_INVALID;
	text->face_style = FFS_REGULAR;
	text->point_size = 9 * 64;
	text->outline_type = OT_NONE;
	text->outline_thickness = 1 * 64;
	text->bold = FALSE;
	text->strength = 1 * 64;
	text->italic = FALSE;
	text->lean = 0x10000L;

	text->center_enable = FALSE;

	text->color[3] = 1.0f;

	text->entity = pi_entity_new();
	text->material = pi_material_new(RS_TEXT_VS, RS_TEXT_FS);
	pi_entity_set_material(text->entity, text->material);

	pi_material_set_uniform(text->material, "u_Color", UT_VEC4, 1, text->color, FALSE);

	pi_material_set_blend(text->material, TRUE);
	pi_material_set_blend_factor(text->material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);

	pi_material_set_depthwrite_enable(text->material, FALSE);

	return text;
}

void PI_API pi_text_delete(PiText *text)
{
	if (text->characters != NULL)
	{
		pi_free(text->characters);
	}
	pi_material_free(text->material);
	pi_entity_free(text->entity);
	if (text->font_face_size != NULL)
	{
		pi_font_face_size_delete(text->font_face_size);
	}
	if (text->mesh != NULL)
	{
		pi_mesh_free(text->mesh->mesh);
		pi_rendermesh_free(text->mesh);
	}
	pi_free(text);
}

void PI_API pi_text_set_font_family(PiText *text, PiFontFamilyType type)
{
	if (text->family_type != type)
	{
		text->family_type = type;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_font_face_style(PiText *text, PiFontFaceStyle style)
{
	if (text->face_style != style)
	{
		text->face_style = style;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_point_size(PiText *text, float point_size)
{
	sint fixed_point_size = (sint)(point_size * 64.0f);
	if (text->point_size != fixed_point_size)
	{
		text->point_size = fixed_point_size;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_outline_type(PiText *text, PiOutlineType outline_type)
{
	if (text->outline_type != outline_type)
	{
		text->outline_type = outline_type;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_outline_thickness(PiText *text, float outline_thickness)
{
	sint fixed_outline_thickness = (sint)(64.0f * outline_thickness);
	if (text->outline_thickness != fixed_outline_thickness)
	{
		text->outline_thickness = fixed_outline_thickness;
		if (text->outline_type != OT_NONE)
		{
			text->is_font_face_size_update = TRUE;
		}
	}
}

void PI_API pi_text_set_bold_enable(PiText *text, PiBool enable)
{
	if (!!text->bold != !!enable)
	{
		text->bold = enable;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_bold_strength(PiText *text, float strength)
{
	sint fixed_strength = (sint)(64.0f * strength);
	if (text->strength != fixed_strength)
	{
		text->strength = fixed_strength;
		if (text->bold)
		{
			text->is_font_face_size_update = TRUE;
		}
	}
}

void PI_API pi_text_set_italic_enable(PiText *text, PiBool enable)
{
	if (!!text->italic != !!enable)
	{
		text->italic = enable;
		text->is_font_face_size_update = TRUE;
	}
}

void PI_API pi_text_set_italic_lean(PiText *text, float lean)
{
	sint fixed_lean = (sint)(65536.0f * lean);
	if (text->lean != lean)
	{
		text->lean = fixed_lean;
		if (text->italic)
		{
			text->is_font_face_size_update = TRUE;
		}
	}
}

void PI_API pi_text_set_characters(PiText *text, const wchar *characters)
{
	if (text->characters != NULL)
	{
		pi_free(text->characters);
	}

	text->characters = pi_wstr_dup(characters);
	text->num_characters = pi_wstrlen(text->characters);

	text->is_mesh_update = TRUE;
}

void PI_API pi_text_set_center_enable(PiText *text, PiBool enable)
{
	if (!!text->center_enable != !!enable)
	{
		text->center_enable = enable;
		text->is_mesh_update = TRUE;
	}
}

void PI_API pi_text_set_color(PiText *text, float color[4])
{
	text->color[0] = color[0];
	text->color[1] = color[1];
	text->color[2] = color[2];
	text->color[3] = color[3];
}

void PI_API pi_text_set_depth_enable(PiText *text, PiBool enable)
{
	pi_material_set_depth_enable(text->material, enable);
}

void PI_API pi_text_set_blend_mode(PiText *text, PiTextBlendMode blend_mode)
{
	switch (blend_mode)
	{
	case TBM_ALPHA:
		pi_material_set_blend(text->material, TRUE);
		pi_material_set_blend_factor(text->material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case TBM_ALPHA_ASSOCIATIVE:
		pi_material_set_blend(text->material, TRUE);
		pi_material_set_blend_factor(text->material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ONE, BF_INV_SRC_ALPHA);
		break;
	case TBM_NONE:
		pi_material_set_blend(text->material, FALSE);
		break;
	default:
		break;
	}
}

static PiRenderMesh *_create_glyph_mesh(PiCharGlyph **glyphs, uint num_glyphs, PiBool center_enable, PiTextLayout *layout)
{
	PiMesh *mesh;
	uint i, num_vertices = 4 * num_glyphs, num_indices = 3 * 2 * num_glyphs;
	sint pen_x = 0, min_y = MAX_INT32, max_y = MIN_INT32, min_x = MAX_INT32, max_x = MIN_INT32;
	/* 使用float4的EVS_DIFFUSE流作为顶点流，xy表示网格的位置，zw表示uv坐标，因为字形网格的z没有意义 */
	float *position = pi_new0(float, 4 * num_vertices);
	uint32 *indices = pi_new0(uint32, num_indices);

	for (i = 0; i < num_glyphs; i++)
	{
		PiCharGlyph *glyph = glyphs[i];
		uint vertex_index = 4 * i;
		float *glyph_pos = position + 4 * vertex_index;
		uint32 *glyph_indices = indices + 6 * i;

		float left = (float)(pen_x + glyph->left);
		float right = (float)(pen_x + glyph->right);
		float bottom = (float)glyph->bottom;
		float top = (float)glyph->top;

		min_x = MIN(min_x, pen_x + glyph->left);
		max_x = MAX(max_x, pen_x + glyph->right);
		min_y = MIN(min_y, glyph->bottom);
		max_y = MAX(max_y, glyph->top);

		pen_x += glyph->advance_x;

		/* 左下角顶点 */
		glyph_pos[0] = left;
		glyph_pos[1] = bottom;
		glyph_pos[2] = glyph->s0;
		glyph_pos[3] = glyph->t0;

		/* 右下角顶点 */
		glyph_pos[4] = right;
		glyph_pos[5] = bottom;
		glyph_pos[6] = glyph->s1;
		glyph_pos[7] = glyph->t0;

		/* 右上角顶点 */
		glyph_pos[8] = right;
		glyph_pos[9] = top;
		glyph_pos[10] = glyph->s1;
		glyph_pos[11] = glyph->t1;

		/* 左上角顶点 */
		glyph_pos[12] = left;
		glyph_pos[13] = top;
		glyph_pos[14] = glyph->s0;
		glyph_pos[15] = glyph->t1;

		glyph_indices[0] = vertex_index;
		glyph_indices[1] = vertex_index + 1;
		glyph_indices[2] = vertex_index + 2;
		glyph_indices[3] = vertex_index;
		glyph_indices[4] = vertex_index + 2;
		glyph_indices[5] = vertex_index + 3;
	}

	/* 网格的原点以及包围和 */
	if (center_enable)
	{
		sint origin_x = (min_x + max_x) / 2;
		sint origin_y = (min_y + max_y) / 2;

		float offset_x = -(float)origin_x;
		float offset_y = -(float)origin_y;

		for (i = 0; i < num_vertices; i++)
		{
			position[4 * i] += offset_x;
			position[4 * i + 1] += offset_y;
		}

		layout->left = min_x - origin_x;
		layout->right = max_x - origin_x;
		layout->top = max_y - origin_y;
		layout->bottom = min_y - origin_y;
	}
	else
	{
		layout->left = min_x;
		layout->right = max_x;
		layout->top = max_y;
		layout->bottom = min_y;
	}

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, num_vertices, NULL, position, NULL, NULL, num_indices, indices);

	pi_free(position);
	pi_free(indices);

	return pi_rendermesh_new(mesh, TRUE);
}

static PiRenderMesh *_create_text_mesh(PiText *text)
{
	PiRenderMesh *mesh = NULL;
	uint i, num_glyphs = 0;
	PiCharGlyph **glyphs = pi_new0(PiCharGlyph *, text->num_characters);
	for (i = 0; i < text->num_characters; i++)
	{
		PiCharGlyph *glyph = pi_font_face_size_load_char_glyph(text->font_face_size, text->characters[i]);
		if (glyph)
		{
			glyphs[num_glyphs] = glyph;
			num_glyphs++;
		}
	}

	if (num_glyphs > 0)
	{
		mesh = _create_glyph_mesh(glyphs, num_glyphs, text->center_enable, &text->layout);
	}
	pi_free(glyphs);

	return mesh;
}

static PiBool _updata_font_face_size(PiText *text)
{
	PiFontFamily *font_family;
	PiFontFace *font_face;

	if (text->font_face_size != NULL)
	{
		pi_font_face_size_delete(text->font_face_size);
		text->font_face_size = NULL;
	}

	font_family = pi_font_family_get(text->font_manager, text->family_type);
	if (font_family == NULL)
	{
		return FALSE;
	}

	font_face = pi_font_face_get(font_family, text->face_style);
	if (font_face == NULL)
	{
		return FALSE;
	}

	text->font_face_size = pi_font_face_size_new(font_face, text->point_size, text->outline_type, text->outline_thickness, text->bold, text->strength, text->italic, text->lean);

	if (text->font_face_size == NULL)
	{
		return FALSE;
	}

	text->is_mesh_update = TRUE;
	text->is_material_update = TRUE;

	return TRUE;
}

static PiBool _update_mesh(PiText *text)
{
	if (text->mesh != NULL)
	{
		pi_mesh_free(text->mesh->mesh);
		pi_rendermesh_free(text->mesh);
	}

	text->mesh = _create_text_mesh(text);
	if (text->mesh == NULL)
	{
		return FALSE;
	}

	pi_entity_set_mesh(text->entity, text->mesh);

	return TRUE;
}

static void _update_material(PiText *text)
{
	PiTextureAtlasData *atlas_data = pi_font_face_size_get_glyphs_texture(text->font_face_size);
	pi_material_set_uniform(text->material, "u_Texture", UT_SAMPLER_2D, 1, &atlas_data->atlas_sampler, FALSE);
	pi_material_set_uniform(text->material, "u_TextureSize", UT_VEC2, 1, atlas_data->atlas_size, FALSE);
}

void PI_API pi_text_update(PiText *text)
{
	if (text->is_font_face_size_update)
	{
		text->available = _updata_font_face_size(text);
		text->is_font_face_size_update = FALSE;
	}

	if (text->available && text->is_mesh_update)
	{
		text->available = _update_mesh(text);
		text->is_mesh_update = FALSE;
	}

	if (text->available && text->is_material_update)
	{
		_update_material(text);
		text->is_material_update = FALSE;
	}
}

PiTextLayout *PI_API pi_text_layout_get(PiText *text)
{
	if (text->available)
	{
		return &text->layout;
	}
	return NULL;
}

PiEntity *PI_API pi_text_entity_get(PiText *text)
{
	if (text->available)
	{
		return text->entity;
	}
	return NULL;
}
