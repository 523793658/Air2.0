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

static void pi_vec3_set(PiVector3 *dst, float x, float y, float z)
{
	dst->x = x;
	dst->y = y;
	dst->z = z;
}

static void pi_vec3_copy(PiVector3 *dst, const PiVector3 *src)
{
	if(dst != src)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
	}
}

static PiBool pi_vec3_is_equal(const PiVector3 *src1, const PiVector3 *src2)
{
	if( IS_FLOAT_EQUAL(src1->x, src2->x) && 
		IS_FLOAT_EQUAL(src1->y, src2->y) &&
		IS_FLOAT_EQUAL(src1->z, src2->z))
		return TRUE;
	return FALSE;
}

static void pi_vec3_add(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2)
{	
	dst->x = src1->x + src2->x;
	dst->y = src1->y + src2->y;
	dst->z = src1->z + src2->z;
}

static void pi_vec3_sub(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2)
{
	dst->x = src1->x - src2->x;
	dst->y = src1->y - src2->y;
	dst->z = src1->z - src2->z;
}

static void pi_vec3_mul(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2)
{
	dst->x = src1->x * src2->x;
	dst->y = src1->y * src2->y;
	dst->z = src1->z * src2->z;
}

static void pi_vec3_div(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2)
{
	dst->x = src1->x / src2->x;
	dst->y = src1->y / src2->y;
	dst->z = src1->z / src2->z;
}

static void pi_vec3_scale(PiVector3 *dst, const PiVector3 *src, float scale)
{
	dst->x = scale * src->x;
	dst->y = scale * src->y;
	dst->z = scale * src->z;
}

static float pi_vec3_dot(const PiVector3 *src1, const PiVector3 *src2)
{
	return src1->x * src2->x + src1->y * src2->y + src1->z * src2->z;
}

static void pi_vec3_cross(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2)
{
	float x = src1->y * src2->z - src1->z * src2->y;
	float y = src1->z * src2->x - src1->x * src2->z;
	float z = src1->x * src2->y - src1->y * src2->x;

	dst->x = x;
	dst->y = y;
	dst->z = z;
}

static float pi_vec3_len_square(const PiVector3 *src)
{
	return pi_vec3_dot(src, src);
}

static float pi_vec3_len(const PiVector3 *src)
{
	return pi_math_sqrt(pi_vec3_len_square(src));
}

static float pi_vec3_distance_square(const PiVector3 *src1, const PiVector3 *src2)
{
	PiVector3 v;
	pi_vec3_sub(&v, src1, src2);
	return pi_vec3_len_square(&v);
}

static float pi_vec3_distance(const PiVector3 *src1, const PiVector3 *src2)
{
	return pi_math_sqrt(pi_vec3_distance_square(src1, src2));
}

static void pi_vec3_lerp(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2, float frac)
{
	pi_vec3_set(dst, 
		pi_lerp_float(src1->x, src2->x, frac), 
		pi_lerp_float(src1->y, src2->y, frac), 
		pi_lerp_float(src1->z, src2->z, frac));
}
