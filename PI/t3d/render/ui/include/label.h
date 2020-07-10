#ifndef INCLUDE_LABEL_H
#define INCLUDE_LABEL_H

#include "component.h"
#include "font_manager.h"

PI_BEGIN_DECLS

/**
 * 创建label组件
 * @param manager 字体管理器
 * @returns label组件句柄
 */
PiComponent *PI_API pi_label_new(PiFontManager *font_manager);

/**
 * 销毁label组件
 * @param component label组件句柄
 */
void PI_API pi_label_delete(PiComponent *component);

/**
 * 设置label组件的font-family
 * @param component label组件句柄
 * @param font_family_type font-family的类型
 */
void PI_API pi_label_set_font_family(PiComponent *component, PiFontFamilyType font_family_type);

/**
 * 设置label组件的font-style
 * @param component label组件句柄
 * @param style font-style
 */
void PI_API pi_label_set_font_face_style(PiComponent *component, PiFontFaceStyle font_face_style);

/**
 * 设置label组件的字体大小
 * @param component label组件句柄
 * @param point_size 字体磅数
 */
void PI_API pi_label_set_font_size(PiComponent *component, float point_size);

/**
 * 设置label组件的是否加粗，该效果跟face_style无关
 * @param component label组件句柄
 * @param enable 是否加粗
 */
void PI_API pi_label_set_bold_enable(PiComponent *component, PiBool enable);

/**
 * 设置label组件的加粗强度
 * @param component label组件句柄
 * @param strength 厚度值，单位像素
 */
void PI_API pi_label_set_bold_strength(PiComponent *component, float strength);

/**
 * 设置label组件的是否倾斜，该效果跟face_style无关
 * @param component label组件句柄
 * @param italic 是否倾斜
 */
void PI_API pi_label_set_italic_enable(PiComponent *component, PiBool enable);

/**
 * 设置label组件的倾斜度
 * @param component label组件句柄
 * @param strength 倾斜值，偏移的距离与字符高度的比值
 */
void PI_API pi_label_set_italic_lean(PiComponent *component, float lean);

/**
 * 设置label组件的文本字符
 * @param component label组件句柄
 * @param characters 文本字符串
 */
void PI_API pi_label_set_characters(PiComponent *component, const wchar *characters);

/**
 * 设置label组件的文本颜色
 * @param component label组件句柄
 * @param r 颜色的red分量[0.0,1.0]
 * @param g 颜色的green分量[0.0,1.0]
 * @param b 颜色的blue分量[0.0,1.0]
 */
void PI_API pi_label_set_color(PiComponent *component, float r, float g, float b);

/**
 * 设置label组件的文本的不透明度
 * @param component label组件句柄
 * @param opacity 不透明度[0.0,1.0]
 */
void PI_API pi_label_set_opacity(PiComponent *component, float opacity);

/**
 * 设置label组件的文本是否描边
 * @param component label组件句柄
 * @param enable 是否描边
 */
void PI_API pi_label_set_stroke_enable(PiComponent *component, PiBool enable);

/**
 * 设置label组件的文本的描边大小
 * @param component label组件句柄
 * @param size 描边的大小，单位是像素
 */
void PI_API pi_label_set_stroke_size(PiComponent *component, float size);

/**
 * 设置label组件的文本的描边颜色
 * @param component label组件句柄
 * @param r 颜色的red分量[0.0,1.0]
 * @param g 颜色的green分量[0.0,1.0]
 * @param b 颜色的blue分量[0.0,1.0]
 */
void PI_API pi_label_set_stroke_color(PiComponent *component, float r, float g, float b);

/**
 * 设置label组件的文本是否投影
 * @param component label组件句柄
 * @param enable 是否投影
 */
void PI_API pi_label_set_shadow_enable(PiComponent *component, PiBool enable);

/**
 * 设置label组件的文本的投影偏移
 * @param component label组件句柄
 * @param offset_x 投影向右偏移，单位是像素，可以为负数
 * @param offset_y 投影向下偏移，单位是像素，可以为负数
 */
void PI_API pi_label_set_shadow_offset(PiComponent *component, sint offset_x, sint offset_y);

/**
 * 设置label组件的文本的投影颜色
 * @param component label组件句柄
 * @param r 颜色的red分量[0.0,1.0]
 * @param g 颜色的green分量[0.0,1.0]
 * @param b 颜色的blue分量[0.0,1.0]
 */
void PI_API pi_label_set_shadow_color(PiComponent *component, float r, float g, float b);

/**
 * 更新label组件，调用了设置接口后必须调用更新才能起效
 * @param component label组件句柄
 */
void PI_API pi_label_update(PiComponent *component);

PI_END_DECLS

#endif /* INCLUDE_LABEL_H */
