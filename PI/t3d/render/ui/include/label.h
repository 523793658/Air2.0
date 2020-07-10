#ifndef INCLUDE_LABEL_H
#define INCLUDE_LABEL_H

#include "component.h"
#include "font_manager.h"

PI_BEGIN_DECLS

/**
 * ����label���
 * @param manager ���������
 * @returns label������
 */
PiComponent *PI_API pi_label_new(PiFontManager *font_manager);

/**
 * ����label���
 * @param component label������
 */
void PI_API pi_label_delete(PiComponent *component);

/**
 * ����label�����font-family
 * @param component label������
 * @param font_family_type font-family������
 */
void PI_API pi_label_set_font_family(PiComponent *component, PiFontFamilyType font_family_type);

/**
 * ����label�����font-style
 * @param component label������
 * @param style font-style
 */
void PI_API pi_label_set_font_face_style(PiComponent *component, PiFontFaceStyle font_face_style);

/**
 * ����label����������С
 * @param component label������
 * @param point_size �������
 */
void PI_API pi_label_set_font_size(PiComponent *component, float point_size);

/**
 * ����label������Ƿ�Ӵ֣���Ч����face_style�޹�
 * @param component label������
 * @param enable �Ƿ�Ӵ�
 */
void PI_API pi_label_set_bold_enable(PiComponent *component, PiBool enable);

/**
 * ����label����ļӴ�ǿ��
 * @param component label������
 * @param strength ���ֵ����λ����
 */
void PI_API pi_label_set_bold_strength(PiComponent *component, float strength);

/**
 * ����label������Ƿ���б����Ч����face_style�޹�
 * @param component label������
 * @param italic �Ƿ���б
 */
void PI_API pi_label_set_italic_enable(PiComponent *component, PiBool enable);

/**
 * ����label�������б��
 * @param component label������
 * @param strength ��бֵ��ƫ�Ƶľ������ַ��߶ȵı�ֵ
 */
void PI_API pi_label_set_italic_lean(PiComponent *component, float lean);

/**
 * ����label������ı��ַ�
 * @param component label������
 * @param characters �ı��ַ���
 */
void PI_API pi_label_set_characters(PiComponent *component, const wchar *characters);

/**
 * ����label������ı���ɫ
 * @param component label������
 * @param r ��ɫ��red����[0.0,1.0]
 * @param g ��ɫ��green����[0.0,1.0]
 * @param b ��ɫ��blue����[0.0,1.0]
 */
void PI_API pi_label_set_color(PiComponent *component, float r, float g, float b);

/**
 * ����label������ı��Ĳ�͸����
 * @param component label������
 * @param opacity ��͸����[0.0,1.0]
 */
void PI_API pi_label_set_opacity(PiComponent *component, float opacity);

/**
 * ����label������ı��Ƿ����
 * @param component label������
 * @param enable �Ƿ����
 */
void PI_API pi_label_set_stroke_enable(PiComponent *component, PiBool enable);

/**
 * ����label������ı�����ߴ�С
 * @param component label������
 * @param size ��ߵĴ�С����λ������
 */
void PI_API pi_label_set_stroke_size(PiComponent *component, float size);

/**
 * ����label������ı��������ɫ
 * @param component label������
 * @param r ��ɫ��red����[0.0,1.0]
 * @param g ��ɫ��green����[0.0,1.0]
 * @param b ��ɫ��blue����[0.0,1.0]
 */
void PI_API pi_label_set_stroke_color(PiComponent *component, float r, float g, float b);

/**
 * ����label������ı��Ƿ�ͶӰ
 * @param component label������
 * @param enable �Ƿ�ͶӰ
 */
void PI_API pi_label_set_shadow_enable(PiComponent *component, PiBool enable);

/**
 * ����label������ı���ͶӰƫ��
 * @param component label������
 * @param offset_x ͶӰ����ƫ�ƣ���λ�����أ�����Ϊ����
 * @param offset_y ͶӰ����ƫ�ƣ���λ�����أ�����Ϊ����
 */
void PI_API pi_label_set_shadow_offset(PiComponent *component, sint offset_x, sint offset_y);

/**
 * ����label������ı���ͶӰ��ɫ
 * @param component label������
 * @param r ��ɫ��red����[0.0,1.0]
 * @param g ��ɫ��green����[0.0,1.0]
 * @param b ��ɫ��blue����[0.0,1.0]
 */
void PI_API pi_label_set_shadow_color(PiComponent *component, float r, float g, float b);

/**
 * ����label��������������ýӿں������ø��²�����Ч
 * @param component label������
 */
void PI_API pi_label_update(PiComponent *component);

PI_END_DECLS

#endif /* INCLUDE_LABEL_H */
