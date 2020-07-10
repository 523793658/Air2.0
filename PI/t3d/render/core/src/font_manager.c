
#include "font_manager.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include FT_STROKER_H

#define GLYPHS_BUFFER_COUNT 32				/* 定义的字符坐标缓冲数量 */

struct PiFontFaceSize
{
	uint ref_count;							/* 引用计数 */

	sint point_size;						/* 磅数 */

	PiOutlineType outline_type;				/* 轮廓类型 */
	sint outline_thickness;					/* 轮廓厚度 */
	PiBool bold;							/* 是否加粗 */
	sint strength;							/* 加粗厚度 */
	PiBool italic;							/* 是否倾斜 */
	sint lean;								/* 倾斜度 */

	PiDvector glyphs[GLYPHS_BUFFER_COUNT];	/* 字符坐标缓冲 */
	PiTextureAtlas *atlas;					/* 字形纹理贴图集 */

	FT_Size ft_size;						/* freetype的size句柄 */
	FT_Stroker stroker;						/* freetype的stroekr句柄 */
	FT_Matrix matrix;						/* freetype的矩阵，用于倾斜 */

	PiFontFace *face;						/* size所属的face */
};

struct PiFontFace
{
	PiFontFaceStyle style;

	FT_Face ft_face;

	uint num_sizes;
	PiVector sizes;

	PiFontFamily *family;
};

struct PiFontFamily
{
	PiFontFamilyType type;

	PiFontFace *faces[FFS_NUM];

	PiFontManager *manager;
};

struct PiFontManager
{
	uint horz_resolution;
	uint vert_resolution;

	FT_Library ft_library;

	PiFontFamily *families[FFT_NUM];

	uint num_files;
	PiVector files;
};

PiFontManager *PI_API pi_font_manager_new(uint horz_resolution, uint vert_resolution)
{
	PiFontManager *font_manager;

	FT_Library ft_library;
	FT_Error error = FT_Init_FreeType(&ft_library);
	if (error)
	{
		return NULL;
	}

	font_manager = pi_new0(PiFontManager, 1);

	font_manager->ft_library = ft_library;

	font_manager->horz_resolution = horz_resolution;
	font_manager->vert_resolution = vert_resolution;

	pi_vector_init(&font_manager->files);

	return font_manager;
}

static void _font_face_size_delete(PiFontFaceSize *size)
{
	uint i;
	for (i = 0; i < GLYPHS_BUFFER_COUNT; i++)
	{
		pi_dvector_clear(&size->glyphs[i], TRUE);
	}
	FT_Done_Size(size->ft_size);
	if (size->outline_type != OT_NONE)
	{
		FT_Stroker_Done(size->stroker);
	}
	pi_texture_atlas_delete(size->atlas);
	pi_free(size);
}

static void _font_face_delete(PiFontFace *face)
{
	uint i;
	for (i = 0; i < face->num_sizes; i++)
	{
		PiFontFaceSize *size = pi_vector_get(&face->sizes, i);
		_font_face_size_delete(size);
	}

	pi_vector_clear(&face->sizes, TRUE);
	FT_Done_Face(face->ft_face);
	pi_free(face);
}

static void _font_family_delete(PiFontFamily *family)
{
	uint i;
	for (i = 0; i < FFS_NUM; i++)
	{
		PiFontFace *face = family->faces[i];
		if (face != NULL)
		{
			_font_face_delete(face);
		}
	}

	pi_free(family);
}

void PI_API pi_font_manager_delete(PiFontManager *manager)
{
	uint i;

	for (i = 0; i < FFT_NUM; i++)
	{
		PiFontFamily *family = manager->families[i];
		if (family != NULL)
		{
			_font_family_delete(family);
		}
	}
	FT_Done_FreeType(manager->ft_library);
	pi_free(manager);
}

static PiFontFamily *_create_font_family(PiFontManager *manager, PiFontFamilyType type)
{
	PiFontFamily *family = pi_new0(PiFontFamily, 1);

	family->manager = manager;
	manager->families[type] = family;

	return family;
}

static PiFontFamilyType _get_family_type(const char *family_name)
{
	if (pi_str_equal("Microsoft YaHei", family_name, TRUE))
	{
		return FFT_MICROSOFT_YA_HEI;
	}
	else if (pi_str_equal("Microsoft YaHei UI", family_name, TRUE))
	{
		return FFT_MICROSOFT_YA_HEI_UI;
	}
	else if (pi_str_equal("SimHei", family_name, TRUE))
	{
		return FFT_SIM_HEI;
	}
	else if (pi_str_equal("KaiTi", family_name, TRUE))
	{
		return FFT_KAI_TI;
	}
	else if (pi_str_equal("SimSun", family_name, TRUE))
	{
		return FFT_SIM_SUN;
	}
	else if (pi_str_equal("NSimSun", family_name, TRUE))
	{
		return FFT_N_SIM_SUN;
	}
	else if (pi_str_equal("FangSong", family_name, TRUE))
	{
		return FFT_FANG_SONG;
	}
	else if (pi_str_equal("LiSu", family_name, TRUE))
	{
		return FFT_Li_Su;
	}
	else if (pi_str_equal("YouYuan", family_name, TRUE))
	{
		return FFT_You_Yuan;
	}
	else
	{
		return FFT_INVALID;
	}
}

static PiFontFaceStyle _get_face_style(const char *style_name)
{
	if (pi_str_equal("Regular", style_name, TRUE))
	{
		return FFS_REGULAR;
	}
	else if (pi_str_equal("Bold", style_name, TRUE))
	{
		return FFS_BOLD;
	}
	else if (pi_str_equal("Italic", style_name, TRUE))
	{
		return FFS_ITALIC;
	}
	else if (pi_str_equal("Black", style_name, TRUE))
	{
		return FFS_BLACK;
	}
	else if (pi_str_equal("Semibold", style_name, TRUE))
	{
		return FFS_SEMIBOLD;
	}
	else if (pi_str_equal("Medium", style_name, TRUE))
	{
		return FFS_MEDIUM;
	}
	else if (pi_str_equal("Light", style_name, TRUE))
	{
		return FFS_LIGHT;
	}
	else if (pi_str_equal("ExtraLight", style_name, TRUE))
	{
		return FFS_EXTRA_LIGHT;
	}
	else
	{
		return FFS_INVALID;
	}
}

static void _create_font_face(PiFontFamily *family, FT_Face ft_face)
{
	PiFontFaceStyle style;
	PiFontFace *face;

	style = _get_face_style(ft_face->style_name);

	if (style == FFS_INVALID || family->faces[style] != NULL)
	{
		FT_Done_Face(ft_face);
		return;
	}

	FT_Select_Charmap(ft_face, FT_ENCODING_UNICODE);

	face = pi_new0(PiFontFace, 1);
	face->style = style;
	face->ft_face = ft_face;
	face->family = family;
	pi_vector_init(&face->sizes);

	family->faces[style] = face;
}

static void _load_font_face(PiFontManager *manager, FT_Face ft_face)
{
	PiFontFamilyType type = _get_family_type(ft_face->family_name);
	if (type != FFT_INVALID)
	{
		PiFontFamily *family = manager->families[type];
		if (family == NULL)
		{
			family = _create_font_family(manager, type);
		}
		_create_font_face(family, ft_face);
	}
}

void PI_API pi_font_manager_load_data(PiFontManager *manager, const byte *data, uint data_size, PiBool force)
{
	uint i, num_faces;
	FT_Face ft_face;

	if (!force)
	{
		for (i = 0; i < manager->num_files; i++)
		{
			const byte *file = pi_vector_get(&manager->files, i);
			if (file == data)
			{
				return;
			}
		}
	}

	FT_New_Memory_Face(manager->ft_library, data, data_size, 0, &ft_face);
	num_faces = ft_face->num_faces;

	_load_font_face(manager, ft_face);

	for (i = 1; i < num_faces; i++)
	{
		FT_New_Memory_Face(manager->ft_library, data, data_size, i, &ft_face);

		_load_font_face(manager, ft_face);
	}

	pi_vector_push(&manager->files, (void *)data);
	manager->num_files++;
}

PiFontFamily *PI_API pi_font_family_get(PiFontManager *manager, PiFontFamilyType type)
{
	return manager->families[type];
}

void PI_API pi_font_family_delete(PiFontFamily *family)
{
	family->manager->families[family->type] = NULL;
	_font_family_delete(family);
}

PiFontFace *PI_API pi_font_face_get(PiFontFamily *family, PiFontFaceStyle style)
{
	return family->faces[style];
}

void PI_API pi_font_face_delete(PiFontFace *face)
{
	face->family->faces[face->style] = NULL;
	_font_face_delete(face);
}

static void _add_glyph_image(PiTextureAtlas *atlas, FT_Bitmap *ft_bitmap, PiCharGlyph *glyph)
{
	uint i;
	uint offset[2];
	byte *glyph_texture_data;

	uint pitch = ft_bitmap->pitch;
	uint width = ft_bitmap->width;
	uint height = ft_bitmap->rows;

	glyph_texture_data = pi_new0(byte, width * height);

	switch (ft_bitmap->pixel_mode)
	{
	case FT_PIXEL_MODE_GRAY:
		for (i = 0; i < height; i++)
		{
			pi_memcpy_inline(glyph_texture_data + i * width, ft_bitmap->buffer + (height - 1 - i) * width, width);
		}
		break;
	case FT_PIXEL_MODE_MONO:
		/* A monochrome bitmap, using 1~bit per pixel. */
		/* Note that pixels are stored in most-significant order (MSB), which means that the left-most pixel in a byte has value 128. */
		for (i = 0; i < height; i++)
		{
			uint j;
			byte *dest = glyph_texture_data + i * width;
			byte *src = ft_bitmap->buffer + (height - 1 - i) * pitch;

			for (j = 0; j < width; j++)
			{
				dest[j] = (src[j / 8] & (128 >> (j % 8))) ? 255 : 0;
			}
		}
		break;
	default:
		PI_ASSERT(FALSE, "unsupported freetype glyph bitmap pixel mode!");
		return;
	}

	pi_texture_atlas_add_tile(atlas, glyph_texture_data, width, height, offset);

	pi_free(glyph_texture_data);

	glyph->s0 = (float)offset[0];
	glyph->t0 = (float)offset[1];

	glyph->s1 = (float)(offset[0] + width);
	glyph->t1 = (float)(offset[1] + height);
}

static void _load_fill_glyph_bitmap(PiFontFaceSize *size, FT_GlyphSlot slot, PiCharGlyph *glyph)
{
	FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

	glyph->left = slot->bitmap_left;
	glyph->top = slot->bitmap_top;
	glyph->right = slot->bitmap_left + (FT_Int)slot->bitmap.width;
	glyph->bottom = slot->bitmap_top - (FT_Int)slot->bitmap.rows;

	_add_glyph_image(size->atlas, &slot->bitmap, glyph);
}

static void _load_stroke_glyph_bitmap(PiFontFaceSize *size, FT_GlyphSlot slot, PiCharGlyph *glyph)
{
	FT_Glyph ft_glyph;
	FT_BitmapGlyph ft_bitmap_glyph;

	FT_Get_Glyph(slot, &ft_glyph);

	if (size->outline_type == OT_LINE)
	{
		FT_Glyph_Stroke(&ft_glyph, size->stroker, 1);
	}
	else if (size->outline_type == OT_INNER)
	{
		FT_Glyph_StrokeBorder(&ft_glyph, size->stroker, 1, 1);
	}
	else if (size->outline_type == OT_OUTER)
	{
		FT_Glyph_StrokeBorder(&ft_glyph, size->stroker, 0, 1);
	}

	FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);

	ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;

	glyph->left = ft_bitmap_glyph->left;
	glyph->top = ft_bitmap_glyph->top;
	glyph->right = ft_bitmap_glyph->left + (FT_Int)ft_bitmap_glyph->bitmap.width;
	glyph->bottom = ft_bitmap_glyph->top - (FT_Int)ft_bitmap_glyph->bitmap.rows;

	_add_glyph_image(size->atlas, &ft_bitmap_glyph->bitmap, glyph);

	FT_Done_Glyph(ft_glyph);
}

static PiFontFaceSize *_create_font_face_size(PiFontFace *face, sint point_size, PiOutlineType outline_type, sint outline_thickness, PiBool bold, sint strength, PiBool italic, sint lean)
{
	uint i;
	PiFontFaceSize *size = pi_new0(PiFontFaceSize, 1);

	size->face = face;

	size->point_size = point_size;
	size->outline_type = outline_type;
	size->outline_thickness = outline_thickness;
	size->bold = bold;
	size->strength = strength;
	size->italic = italic;
	size->lean = lean;

	FT_New_Size(face->ft_face, &size->ft_size);
	FT_Activate_Size(size->ft_size);
	FT_Set_Char_Size(size->face->ft_face, point_size, point_size, face->family->manager->horz_resolution, face->family->manager->vert_resolution);

	if (italic)
	{
		size->matrix.xx = 0x10000L;
		size->matrix.xy = lean;
		size->matrix.yx = 0;
		size->matrix.yy = 0x10000L;
	}

	if (outline_type != OT_NONE)
	{
		FT_Stroker_New(face->family->manager->ft_library, &size->stroker);
		FT_Stroker_Set(size->stroker, size->outline_thickness, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	size->atlas = pi_texture_atlas_new();

	pi_vector_push(&face->sizes, size);
	face->num_sizes++;

	for (i = 0; i < GLYPHS_BUFFER_COUNT; i++)
	{
		pi_dvector_init(&size->glyphs[i], sizeof(PiCharGlyph));
	}

	return size;
}

PiFontFaceSize *PI_API pi_font_face_size_new(PiFontFace *face, sint point_size, PiOutlineType outline_type, sint outline_thickness, PiBool bold, sint strength, PiBool italic, sint lean)
{
	uint i;
	PiFontFaceSize *size;

	if (face == NULL)
	{
		return NULL;
	}

	for (i = 0; i < face->num_sizes; i++)
	{
		size = pi_vector_get(&face->sizes, i);
		/* todo:这个判断有点复杂 */
		if (size->point_size == point_size &&
			size->outline_type == outline_type &&
			!!size->bold == !!bold &&
			!!size->italic == !!italic &&
			(size->outline_thickness == outline_thickness || outline_type == OT_NONE) &&
			(size->strength == strength || !bold) &&
			(size->lean == lean || !italic))
		{
			size->ref_count++;
			return size;
		}
	}

	size = _create_font_face_size(face, point_size, outline_type, outline_thickness, bold, strength, italic, lean);
	size->ref_count++;
	return size;
}

void PI_API pi_font_face_size_delete(PiFontFaceSize *size)
{
	uint i;
	size->ref_count--;
	if (size->ref_count > 0)
	{
		return;
	}
	for (i = 0; i < size->face->num_sizes; i++)
	{
		PiFontFaceSize *elem = pi_vector_get(&size->face->sizes, i);
		if (elem == size)
		{
			pi_vector_remove(&size->face->sizes, i);
			size->face->num_sizes--;
			_font_face_size_delete(size);
			return;
		}
	}
}

static PiCharGlyph *_load_glyph(PiFontFaceSize *size, wchar charcode, PiDvector *glyphs)
{
	PiCharGlyph glyph;
	FT_UInt glyph_index;
	FT_GlyphSlot slot;
	FT_Int32 load_flags = FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT;

	//load_flags |= FT_LOAD_TARGET_MONO;

	glyph_index = FT_Get_Char_Index(size->face->ft_face, charcode);
	if (!glyph_index)
	{
		return NULL;
	}

	FT_Activate_Size(size->ft_size);

	if (size->italic)
	{
		FT_Set_Transform(size->face->ft_face, &size->matrix, 0);
	}
	else
	{
		load_flags |= FT_LOAD_IGNORE_TRANSFORM;
	}

	if (FT_Load_Glyph(size->face->ft_face, glyph_index, load_flags))
	{
		return NULL;
	}

	slot = size->face->ft_face->glyph;

	glyph.advance_x = slot->advance.x / 64;

	if (size->bold)
	{
		FT_Outline_Embolden(&slot->outline, size->strength);
	}

	if (size->outline_type == OT_NONE)
	{
		_load_fill_glyph_bitmap(size, slot, &glyph);
	}
	else
	{
		_load_stroke_glyph_bitmap(size, slot, &glyph);
	}

	glyph.charcode = charcode;

	return pi_dvector_push(glyphs, &glyph);
}

PiCharGlyph *PI_API pi_font_face_size_load_char_glyph(PiFontFaceSize *size, wchar charcode)
{
	uint i, num_glyphs;
	PiDvector *glyphs = &size->glyphs[charcode % GLYPHS_BUFFER_COUNT];

	num_glyphs = pi_dvector_size(glyphs);
	for (i = 0; i < num_glyphs; i++)
	{
		PiCharGlyph *glyph = pi_dvector_get(glyphs, i);

		if (charcode == glyph->charcode)
		{
			return glyph;
		}
	}

	return _load_glyph(size, charcode, glyphs);
}

PiTextureAtlasData *PI_API pi_font_face_size_get_glyphs_texture(PiFontFaceSize *size)
{
	return pi_texture_atlas_get_data(size->atlas);
}
