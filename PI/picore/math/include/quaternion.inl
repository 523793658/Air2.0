/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

static void pi_quat_set(PiQuaternion *dst, float w, float x, float y, float z)
{
	dst->w = w;
	dst->x = x;
	dst->y = y;
	dst->z = z;
}

static void pi_quat_copy(PiQuaternion *dst, const PiQuaternion *src)
{
	if(dst != src)
	{
		dst->w = src->w;
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
	}
}

static PiBool pi_quat_is_equal(const PiQuaternion *src1, const PiQuaternion *src2)
{
	if( IS_FLOAT_EQUAL(src1->w, src2->w) && 
		IS_FLOAT_EQUAL(src1->x, src2->x) && 
		IS_FLOAT_EQUAL(src1->y, src2->y) && 
		IS_FLOAT_EQUAL(src1->z, src2->z))
		return TRUE;
	return FALSE;
}

static void pi_quat_add(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2)
{
	dst->w = src1->w + src2->w;
	dst->x = src1->x + src2->x;
	dst->y = src1->y + src2->y;
	dst->z = src1->z + src2->z;
}

static void pi_quat_sub(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2)
{
	dst->w = src1->w - src2->w;
	dst->x = src1->x - src2->x;
	dst->y = src1->y - src2->y;
	dst->z = src1->z - src2->z;
}

static void pi_quat_scale(PiQuaternion *dst, const PiQuaternion *src, float scale)
{
	dst->w = scale * src->w;
	dst->x = scale * src->x;
	dst->y = scale * src->y;
	dst->z = scale * src->z;
}

static float pi_quat_dot(const PiQuaternion *src1, const PiQuaternion *src2)
{
	return src1->w * src2->w + src1->x * src2->x + src1->y * src2->y + src1->z * src2->z;
}

static float pi_quat_len_square(const PiQuaternion *src)
{
	return pi_quat_dot(src, src);
}

static float pi_quat_len(const PiQuaternion *src)
{
	return pi_math_sqrt(pi_quat_len_square(src));
}

static void pi_quat_conjugate(PiQuaternion *dst, const PiQuaternion *src)
{
	pi_quat_set(dst, src->w, -src->x, -src->y, -src->z);
}
