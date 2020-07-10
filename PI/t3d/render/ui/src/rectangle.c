
#include "rectangle.h"

PiRectangle *PI_API pi_rectangle_copy(PiRectangle *dst, PiRectangle *src)
{
	pi_memcpy(dst, src, sizeof(PiRectangle));

	return dst;
}

PiBool PI_API pi_rectangle_equal(PiRectangle *dst, PiRectangle *src)
{
	return pi_memcmp(dst, src, sizeof(PiRectangle));
}

PiRectangle *PI_API pi_rectangle_reset(PiRectangle *rect)
{
	rect->min_x = rect->min_y = 0x7fffffff;
	rect->max_x = rect->max_y = 0x80000000;

	return rect;
}

PiRectangle *PI_API pi_rectangle_set_max(PiRectangle *rect)
{
	rect->min_x = rect->min_y = 0x80000000;
	rect->max_x = rect->max_y = 0x7fffffff;

	return rect;
}

PiRectangle *PI_API pi_rectangle_merge_point(PiRectangle *dst, sint x, sint y)
{
	if (x < dst->min_x)
	{
		dst->min_x = x;
	}
	if (x > dst->max_x)
	{
		dst->max_x = x;
	}

	if (y < dst->min_y)
	{
		dst->min_y = y;
	}
	if (y > dst->max_y)
	{
		dst->max_y = y;
	}

	return dst;
}

PiRectangle *PI_API pi_rectangle_merge(PiRectangle *dst, PiRectangle *src)
{
	if (!pi_rectangle_is_void(src))
	{
		pi_rectangle_merge_point(dst, src->min_x, src->min_y);
		pi_rectangle_merge_point(dst, src->max_x, src->max_y);
	}

	return dst;
}

PiBool PI_API pi_rectangle_is_void(PiRectangle *rect)
{
	return rect->max_x < rect->min_x || rect->max_y < rect->min_y;
}

PiRectangle *PI_API pi_rectangle_intersection(PiRectangle *dst, PiRectangle *rect1, PiRectangle *rect2)
{
	dst->max_x = MIN(rect1->max_x, rect2->max_x);
	dst->max_y = MIN(rect1->max_y, rect2->max_y);
	dst->min_x = MAX(rect1->min_x, rect2->min_x);
	dst->min_y = MAX(rect1->min_y, rect2->min_y);

	return dst;
}