#ifndef INCLUDE_RECTANGLE_H
#define INCLUDE_RECTANGLE_H

#include "pi_lib.h"

/**
 * 此文件定义了2DUI绘制所需的区域矩形
 */

typedef struct
{
	sint min_x;
	sint min_y;
	sint max_x;
	sint max_y;
} PiRectangle;

PI_BEGIN_DECLS

PiRectangle *PI_API pi_rectangle_copy(PiRectangle *dst, PiRectangle *src);

PiBool PI_API pi_rectangle_equal(PiRectangle *dst, PiRectangle *src);

PiRectangle *PI_API pi_rectangle_reset(PiRectangle *rect);

PiRectangle *PI_API pi_rectangle_set_max(PiRectangle *rect);

PiRectangle *PI_API pi_rectangle_merge(PiRectangle *dst, PiRectangle *src);

PiBool PI_API pi_rectangle_is_void(PiRectangle *rect);

PiRectangle *PI_API pi_rectangle_intersection(PiRectangle *dst, PiRectangle *rect1, PiRectangle *rect2);

PI_END_DECLS

#endif /* INCLUDE_RECTANGLE_H */
