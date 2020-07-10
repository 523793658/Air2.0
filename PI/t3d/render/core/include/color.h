#ifndef INCLUDE_COLOR_H
#define INCLUDE_COLOR_H

#include <pi_lib.h>

/**
 * 颜色处理 
 */

typedef struct  
{
	float rgba[4];
}PiColor;

PI_BEGIN_DECLS

/* 设置颜色 */
static void color_set(PiColor *color, float r, float g, float b, float a);

/* 设置颜色 */
static void color_set_byte(PiColor *color, byte r, byte g, byte b, byte a);

/* rgba的整形表示，注意：大小端的区别 */
static void color_from_int(PiColor *color, uint32 rgba);

/* 颜色是否相等 */
static PiBool color_is_equal(const PiColor *c1, const PiColor *c2);

/* 颜色拷贝 */
static void color_copy(PiColor *dst, const PiColor *src);

PI_END_DECLS

#include <color.inl>

#endif /* INCLUDE_COLOR_H */