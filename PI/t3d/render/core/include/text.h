#ifndef INCLUDE_TEXT_H
#define INCLUDE_TEXT_H

#include "font_manager.h"
#include "entity.h"

/* text的混合模式 */
typedef enum
{
	TBM_NONE,							/* 不混合 */
	TBM_ALPHA,							/* alpha混合 */
	TBM_ALPHA_ASSOCIATIVE				/* alpha混合的结合，用于ui，把多个alpha混合的text进行纹理化，然后把纹理混合到最终目标上 */
} PiTextBlendMode;

/* text的布局信息，text的矩形区域和他的原点偏移 */
typedef struct
{
	sint left;				/* text左边界相对原点的位置 */
	sint right;				/* text右边界相对原点的位置 */
	sint top;				/* text上边界相对原点的位置 */
	sint bottom;			/* text下边界相对原点的位置 */
} PiTextLayout;

typedef struct PiText PiText;

PI_BEGIN_DECLS

/**
 * 创建文本渲染对象
 * @param manager 字体管理器
 * @returns 文本渲染对象句柄
 */
PiText *PI_API pi_text_new(PiFontManager *font_manager);

/**
 * 销毁文本渲染对象
 * @param text 文本渲染对象句柄
 */
void PI_API pi_text_delete(PiText *text);

/**
 * 设置文本渲染对象的font-family，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param type font-family的type
 */
void PI_API pi_text_set_font_family(PiText *text, PiFontFamilyType type);

/**
 * 设置文本渲染对象的font-style，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param style font-style
 */
void PI_API pi_text_set_font_face_style(PiText *text, PiFontFaceStyle style);

/**
 * 设置文本渲染对象的字体大小，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param point_size 字体的磅数
 */
void PI_API pi_text_set_point_size(PiText *text, float point_size);

/**
 * 设置文本渲染对象的轮廓类型，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param outline_type 轮廓类型，默认是OT_NONE
 */
void PI_API pi_text_set_outline_type(PiText *text, PiOutlineType outline_type);

/**
 * 设置文本渲染对象的轮廓厚度，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param outline_thickness 厚度值，单位像素
 */
void PI_API pi_text_set_outline_thickness(PiText *text, float outline_thickness);

/**
 * 设置文本渲染对象是否加粗，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param enable 是否加粗
 */
void PI_API pi_text_set_bold_enable(PiText *text, PiBool enable);

/**
 * 设置文本渲染对象的加粗强度，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param strength 加粗厚度值，单位像素
 */
void PI_API pi_text_set_bold_strength(PiText *text, float strength);

/**
 * 设置文本渲染对象是否倾斜，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param enable 是否倾斜
 */
void PI_API pi_text_set_italic_enable(PiText *text, PiBool enable);

/**
 * 设置文本渲染对象的倾斜度，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param strength 倾斜度，偏移的距离与字符高度的比值
 */
void PI_API pi_text_set_italic_lean(PiText *text, float lean);

/**
 * 给文本渲染对象设置字符，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param characters 字符
 */
void PI_API pi_text_set_characters(PiText *text, const wchar *characters);

/**
 * 给文本渲染对象设置中心点，需要调用pi_text_update才能生效
 * @param text 文本渲染对象句柄
 * @param enable 是否让text的entity的模型空间的中心在中间，默认FALSE
 */
void PI_API pi_text_set_center_enable(PiText *text, PiBool enable);

/**
 * 给文本渲染对象设置文字颜色
 * @param text 文本渲染对象句柄
 * @param color rgba的颜色，默认[1.0, 1.0, 1.0, 1.0]
 */
void PI_API pi_text_set_color(PiText *text, float color[4]);

/**
 * 设置文本渲染对象的深度开关
 * @param text 文本渲染对象句柄
 * @param enable 深度开关，默认TRUE
 */
void PI_API pi_text_set_depth_enable(PiText *text, PiBool enable);

/**
 * 设置文本渲染对象的混合模式
 * @param text 文本渲染对象句柄
 * @param blend_mode 混合模式，默认TBM_ALPHA
 */
void PI_API pi_text_set_blend_mode(PiText *text, PiTextBlendMode blend_mode);

/**
 * 更新文本渲染对象，一些参数设置后需要更新才能起效
 * @param text 文本渲染对象句柄
 */
void PI_API pi_text_update(PiText *text);

/**
 * 获取文本渲染对象的排版布局
 * @param text 文本渲染对象句柄
 * @returns 文本渲染对象的布局对象
 */
PiTextLayout *PI_API pi_text_layout_get(PiText *text);

/**
 * 获取文本渲染对象的entity
 * @param text 文本渲染对象句柄
 * @returns entity
 */
PiEntity *PI_API pi_text_entity_get(PiText *text);

PI_END_DECLS

#endif /* INCLUDE_TEXT_H */
