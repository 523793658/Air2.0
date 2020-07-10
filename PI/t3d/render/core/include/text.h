#ifndef INCLUDE_TEXT_H
#define INCLUDE_TEXT_H

#include "font_manager.h"
#include "entity.h"

/* text�Ļ��ģʽ */
typedef enum
{
	TBM_NONE,							/* ����� */
	TBM_ALPHA,							/* alpha��� */
	TBM_ALPHA_ASSOCIATIVE				/* alpha��ϵĽ�ϣ�����ui���Ѷ��alpha��ϵ�text����������Ȼ��������ϵ�����Ŀ���� */
} PiTextBlendMode;

/* text�Ĳ�����Ϣ��text�ľ������������ԭ��ƫ�� */
typedef struct
{
	sint left;				/* text��߽����ԭ���λ�� */
	sint right;				/* text�ұ߽����ԭ���λ�� */
	sint top;				/* text�ϱ߽����ԭ���λ�� */
	sint bottom;			/* text�±߽����ԭ���λ�� */
} PiTextLayout;

typedef struct PiText PiText;

PI_BEGIN_DECLS

/**
 * �����ı���Ⱦ����
 * @param manager ���������
 * @returns �ı���Ⱦ������
 */
PiText *PI_API pi_text_new(PiFontManager *font_manager);

/**
 * �����ı���Ⱦ����
 * @param text �ı���Ⱦ������
 */
void PI_API pi_text_delete(PiText *text);

/**
 * �����ı���Ⱦ�����font-family����Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param type font-family��type
 */
void PI_API pi_text_set_font_family(PiText *text, PiFontFamilyType type);

/**
 * �����ı���Ⱦ�����font-style����Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param style font-style
 */
void PI_API pi_text_set_font_face_style(PiText *text, PiFontFaceStyle style);

/**
 * �����ı���Ⱦ����������С����Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param point_size ����İ���
 */
void PI_API pi_text_set_point_size(PiText *text, float point_size);

/**
 * �����ı���Ⱦ������������ͣ���Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param outline_type �������ͣ�Ĭ����OT_NONE
 */
void PI_API pi_text_set_outline_type(PiText *text, PiOutlineType outline_type);

/**
 * �����ı���Ⱦ�����������ȣ���Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param outline_thickness ���ֵ����λ����
 */
void PI_API pi_text_set_outline_thickness(PiText *text, float outline_thickness);

/**
 * �����ı���Ⱦ�����Ƿ�Ӵ֣���Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param enable �Ƿ�Ӵ�
 */
void PI_API pi_text_set_bold_enable(PiText *text, PiBool enable);

/**
 * �����ı���Ⱦ����ļӴ�ǿ�ȣ���Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param strength �Ӵֺ��ֵ����λ����
 */
void PI_API pi_text_set_bold_strength(PiText *text, float strength);

/**
 * �����ı���Ⱦ�����Ƿ���б����Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param enable �Ƿ���б
 */
void PI_API pi_text_set_italic_enable(PiText *text, PiBool enable);

/**
 * �����ı���Ⱦ�������б�ȣ���Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param strength ��б�ȣ�ƫ�Ƶľ������ַ��߶ȵı�ֵ
 */
void PI_API pi_text_set_italic_lean(PiText *text, float lean);

/**
 * ���ı���Ⱦ���������ַ�����Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param characters �ַ�
 */
void PI_API pi_text_set_characters(PiText *text, const wchar *characters);

/**
 * ���ı���Ⱦ�����������ĵ㣬��Ҫ����pi_text_update������Ч
 * @param text �ı���Ⱦ������
 * @param enable �Ƿ���text��entity��ģ�Ϳռ���������м䣬Ĭ��FALSE
 */
void PI_API pi_text_set_center_enable(PiText *text, PiBool enable);

/**
 * ���ı���Ⱦ��������������ɫ
 * @param text �ı���Ⱦ������
 * @param color rgba����ɫ��Ĭ��[1.0, 1.0, 1.0, 1.0]
 */
void PI_API pi_text_set_color(PiText *text, float color[4]);

/**
 * �����ı���Ⱦ�������ȿ���
 * @param text �ı���Ⱦ������
 * @param enable ��ȿ��أ�Ĭ��TRUE
 */
void PI_API pi_text_set_depth_enable(PiText *text, PiBool enable);

/**
 * �����ı���Ⱦ����Ļ��ģʽ
 * @param text �ı���Ⱦ������
 * @param blend_mode ���ģʽ��Ĭ��TBM_ALPHA
 */
void PI_API pi_text_set_blend_mode(PiText *text, PiTextBlendMode blend_mode);

/**
 * �����ı���Ⱦ����һЩ�������ú���Ҫ���²�����Ч
 * @param text �ı���Ⱦ������
 */
void PI_API pi_text_update(PiText *text);

/**
 * ��ȡ�ı���Ⱦ������Ű沼��
 * @param text �ı���Ⱦ������
 * @returns �ı���Ⱦ����Ĳ��ֶ���
 */
PiTextLayout *PI_API pi_text_layout_get(PiText *text);

/**
 * ��ȡ�ı���Ⱦ�����entity
 * @param text �ı���Ⱦ������
 * @returns entity
 */
PiEntity *PI_API pi_text_entity_get(PiText *text);

PI_END_DECLS

#endif /* INCLUDE_TEXT_H */
