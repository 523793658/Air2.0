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

static void pi_plane_set(PiPlane *plane, PiVector3 *v, float d)
{
	plane->d = d;
	pi_vec3_copy(&plane->v, v);
}

static void pi_plane_by_3points(PiPlane *plane, PiVector3* p1, PiVector3* p2, PiVector3* p3)
{
	PiVector3 v1, v2, n;
	float d;

	pi_vec3_sub(&v1, p1, p2);
	pi_vec3_sub(&v2, p1, p3);
	pi_vec3_cross(&n, &v1, &v2);
	pi_vec3_normalise(&n, &n);
	d = pi_vec3_dot(&n, p1);

	plane->d = -d;
	pi_vec3_copy(&plane->v, &n);
}

/* 拷贝平面数据 */
static void pi_plane_copy(PiPlane* dst, PiPlane* src)
{
	dst->d = src->d;
	pi_vec3_copy(&dst->v, &src->v);
}

static float pi_plane_dot(PiPlane *plane, float x, float y, float z)
{
	float r;
	PiVector3 v;
	pi_vec3_set(&v, x, y, z);
	r = plane->d + pi_vec3_dot(&v, &plane->v);
	return r;
}

static void pi_plane_normalise(PiPlane *dst, PiPlane *src)
{
	float len = pi_vec3_len(&src->v);

	pi_vec3_normalise(&dst->v, &src->v);
	if(!IS_FLOAT_EQUAL(0.0f, len))
		dst->d = src->d / len;
}

static void pi_plane_symmetry_point(PiVector3* dst, PiVector3* src, PiPlane *plane)
{
	PiVector3 v;
	float d = pi_vec3_dot(&plane->v, src) + plane->d;
	d /= pi_vec3_len(&plane->v);
	pi_vec3_normalise(&v, &plane->v);
	pi_vec3_scale(&v, &v, -2.0f * d);
	pi_vec3_add(dst, src, &v);
}

static void pi_plane_symmetry_vector(PiVector3* dst, PiVector3* src, PiPlane *plane)
{
	float d = pi_vec3_dot(src, &plane->v);
	PiVector3 v;
	pi_vec3_normalise(&v, &plane->v);
	pi_vec3_scale(&v, &v, -2.0f * d);
	pi_vec3_add(dst, src, &v);
}
