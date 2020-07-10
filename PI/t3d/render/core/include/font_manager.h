#ifndef INCLUDE_FONT_MANAGER_H
#define INCLUDE_FONT_MANAGER_H

#include "texture_atlas.h"

/* Windows系统常见中文字体英文名称 */
typedef enum
{
	FFT_INVALID = -1,			/* 无效的 */
	FFT_MICROSOFT_YA_HEI,		/* 微软雅黑 */
	FFT_MICROSOFT_YA_HEI_UI,	/* 微软雅黑UI */
	FFT_SIM_HEI,				/* 黑体 */
	FFT_KAI_TI,					/* 楷体 */
	FFT_SIM_SUN,				/* 宋体 */
	FFT_N_SIM_SUN,				/* 新宋体 */
	FFT_FANG_SONG,				/* 仿宋 */
	FFT_Li_Su,					/* 隶书 */
	FFT_You_Yuan,				/* 幼圆 */
	FFT_NUM
} PiFontFamilyType;

typedef enum
{
	FFS_INVALID = -1,			/* 无效的 */
	FFS_REGULAR,				/* 常规 */
	FFS_BOLD,					/* 粗体 */
	FFS_ITALIC,					/* 斜体 */
	FFS_BLACK,					/* 黑体 */
	FFS_SEMIBOLD,				/* 半粗体 */
	FFS_MEDIUM,					/* 中等 */
	FFS_LIGHT,					/* 细体 */
	FFS_EXTRA_LIGHT,			/* 特细 */
	FFS_NUM
} PiFontFaceStyle;

typedef enum
{
	OT_NONE,					/* 无轮廓，标准的字形 */
	OT_LINE,					/* 线轮廓，字形只有线描绘的轮廓，也就是内部是空，可以给线设置厚度 */
	OT_INNER,					/* 轮廓向内收缩 */
	OT_OUTER					/* 轮廓向外延展 */
} PiOutlineType;

typedef struct PiFontManager PiFontManager;

typedef struct PiFontFamily PiFontFamily;

typedef struct PiFontFace PiFontFace;

typedef struct PiFontFaceSize PiFontFaceSize;

/* 字符的字形属性 */
typedef struct
{
	wchar charcode;			/* 字符码 */
	uint advance_x;			/* 字符的排版宽度 */

	/* 字形图像相对基线的坐标，用于生成字符的网格，这里采用sint类型，避免加减后的浮点误差，在创建网格时需要进行创建类型转换 */
	sint left;				/* 字符的纹理的左边界距离字符原点的距离，单位像素 */
	sint right;				/* 字符的纹理的右边界距离字符原点的距离，单位像素 */
	sint top;				/* 字符的纹理的上边界距离字符基线的距离，单位像素 */
	sint bottom;			/* 字符的纹理的下边界距离字符基点的距离，单位像素 */

	/* 字形图像在纹理的坐标，用于生成字符的uv坐标，这里直接采用了float类型，创建网格的时候就不需要进行强制类型转换 */
	float s0;
	float t0;
	float s1;
	float t1;
} PiCharGlyph;

PI_BEGIN_DECLS

/**
 * 创建字体管理器
 * @param horz_resolution 屏幕的水平DPI，Windows默认96
 * @param vert_resolution 屏幕的垂直DPI，Windows默认96
 * @returns 字体管理器句柄
 */
PiFontManager *PI_API pi_font_manager_new(uint horz_resolution, uint vert_resolution);

/**
 * 销毁字体管理器
 * @param manager 字体管理器句柄
 */
void PI_API pi_font_manager_delete(PiFontManager *manager);

/**
 * 加载字体数据
 * @param manager 字体管理器句柄
 * @param data 字体数据
 * @param data_size 字体数据的大小
 * @param force 强制加载，默认FALSE
 */
void PI_API pi_font_manager_load_data(PiFontManager *manager, const byte *data, uint data_size, PiBool force);

/**
 * 获取font-family
 * @param manager 字体管理器句柄
 * @param type font-family的类型，比如FFT_MICROSOFT_YA_HEI
 * @returns font-family句柄
 */
PiFontFamily *PI_API pi_font_family_get(PiFontManager *manager, PiFontFamilyType type);

/**
 * 销毁font-family
 * @param family font-family句柄
 */
void PI_API pi_font_family_delete(PiFontFamily *family);

/**
 * 获取font-face
 * @param family font-family句柄
 * @param style face的风格，比如"FFS_ITALIC"表示斜体
 * @returns font-face句柄
 */
PiFontFace *PI_API pi_font_face_get(PiFontFamily *family, PiFontFaceStyle style);

/**
 * 销毁font-face
 * @param face font-face句柄
 */
void PI_API pi_font_face_delete(PiFontFace *face);

/**
 * 创建face-size
 * @param face font-face句柄
 * @param point_size 字符的磅数
 * @param outline_type 轮廓的类型
 * @param outline_thickness 轮廓厚度
 * @param bold 是否加粗
 * @param strength 加粗厚度，单位像素，26.6像素格式
 * @param italic 是否倾斜
 * @param lean 倾斜度，偏移的距离与字符高度的比值，16.16的定点数格式
 * @returns face-size句柄
 */
PiFontFaceSize *PI_API pi_font_face_size_new(PiFontFace *face, sint point_size, PiOutlineType outline_type, sint outline_thickness, PiBool bold, sint strength, PiBool italic, sint lean);

/**
 * 销毁face-size
 * @param size face-size句柄
 */
void PI_API pi_font_face_size_delete(PiFontFaceSize *size);

/**
 * 加载字符的字形
 * @param size face-size句柄
 * @param charcode 字符
 * @returns 字形句柄
 */
PiCharGlyph *PI_API pi_font_face_size_load_char_glyph(PiFontFaceSize *size, wchar charcode);

/**
 * 获取face-size字形纹理
 * @param size face-size句柄
 * @returns 字形纹理句柄
 */
PiTextureAtlasData *PI_API pi_font_face_size_get_glyphs_texture(PiFontFaceSize *size);

PI_END_DECLS

#endif /* INCLUDE_FONT_MANAGER_H */
