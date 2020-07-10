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

#include "pi_vector3.h"

const PiVector3 g_zero_vec = {0, 0, 0};
const PiVector3 g_xunit_vec = {1, 0, 0};
const PiVector3 g_yunit_vec = {0, 1, 0};
const PiVector3 g_zunit_vec = {0, 0, 1};
const PiVector3 g_scale_unit = {1, 1, 1};

 const PiVector3* pi_vec3_get_zero(void)
{
	return &g_zero_vec;
}

const PiVector3* pi_vec3_get_xunit(void)
{
	return &g_xunit_vec;
}

const PiVector3* pi_vec3_get_yunit(void)
{
	return &g_yunit_vec;
}

const PiVector3* pi_vec3_get_zunit(void)
{
	return &g_zunit_vec;
}

const PiVector3* pi_vec3_get_scale_unit(void)
{
	return &g_scale_unit;
}

PiBool pi_vec3_normalise(PiVector3 *dst, const PiVector3 *src)
{
	float fLen = pi_vec3_len(src);
	if(IS_FLOAT_EQUAL(fLen, 0.0f))
	{
		pi_memset_inline(dst, 0, sizeof(PiVector3));
		return FALSE;
	}
	pi_vec3_scale(dst, src, 1.0f / fLen);
	return TRUE;
}

float pi_vec3_angle(const PiVector3 *src1, const PiVector3 *src2)
{
	float f = pi_vec3_len(src1) * pi_vec3_len(src2);
	
	if(IS_FLOAT_EQUAL(f, 0.0f))
		return 0.0f;
	
	f = pi_vec3_dot(src1, src2) / f;
	f = pi_clamp_float(f, -1.0f, 1.0f);
	return pi_math_acos(f);
}

