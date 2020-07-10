#ifndef INCLUDE_COLOR_H
#define INCLUDE_COLOR_H

#include <pi_lib.h>

/**
 * ��ɫ���� 
 */

typedef struct  
{
	float rgba[4];
}PiColor;

PI_BEGIN_DECLS

/* ������ɫ */
static void color_set(PiColor *color, float r, float g, float b, float a);

/* ������ɫ */
static void color_set_byte(PiColor *color, byte r, byte g, byte b, byte a);

/* rgba�����α�ʾ��ע�⣺��С�˵����� */
static void color_from_int(PiColor *color, uint32 rgba);

/* ��ɫ�Ƿ���� */
static PiBool color_is_equal(const PiColor *c1, const PiColor *c2);

/* ��ɫ���� */
static void color_copy(PiColor *dst, const PiColor *src);

PI_END_DECLS

#include <color.inl>

#endif /* INCLUDE_COLOR_H */