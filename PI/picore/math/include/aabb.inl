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

static PiBool _is_aabb_valid(PiAABBBox *box)
{
	return box->maxPt.x >= box->minPt.x ||
		box->maxPt.y >= box->minPt.y ||
		box->maxPt.z >= box->minPt.z;
}

static void pi_aabb_init(PiAABBBox *box)
{
	box->maxPt.x = -MAX_FLOAT;
	box->maxPt.y = -MAX_FLOAT;
	box->maxPt.z = -MAX_FLOAT;
	box->minPt.x = MAX_FLOAT;
	box->minPt.y = MAX_FLOAT;
	box->minPt.z = MAX_FLOAT;
}

static void pi_aabb_clear(PiAABBBox *box)
{
	pi_memset_inline(box, 0, sizeof(PiAABBBox));
}

static void pi_aabb_add_point(PiAABBBox *box, PiVector3 *pt)
{
	if(pt->x < box->minPt.x)
		box->minPt.x = pt->x;
	if (pt->x > box->maxPt.x)
		box->maxPt.x = pt->x;

	if(pt->y < box->minPt.y)
		box->minPt.y = pt->y;
	if (pt->y > box->maxPt.y)
		box->maxPt.y = pt->y;

	if(pt->z < box->minPt.z)
		box->minPt.z = pt->z;
	if (pt->z > box->maxPt.z)
		box->maxPt.z = pt->z;
}

static void pi_aabb_merge(PiAABBBox *dst, PiAABBBox *src1, PiAABBBox *src2)
{
	PiAABBBox box;
	PiBool r1 = _is_aabb_valid(src1);
	PiBool r2 = _is_aabb_valid(src2);

	pi_aabb_init(&box);

	if (r1)
	{
		pi_aabb_copy(&box, src1);
		if (r2)
		{
			pi_aabb_add_point(&box, &src2->minPt);
			pi_aabb_add_point(&box, &src2->maxPt);
		}
	}
	else 
	{
		if (r2)
		{
			pi_aabb_copy(&box, src2);
		}
	}

	pi_aabb_copy(dst, &box);
}

static void pi_aabb_get_center(PiAABBBox *box, PiVector3 *dst)
{
	pi_vec3_add(dst, &box->minPt, &box->maxPt);
	pi_vec3_scale(dst, dst, 0.5f);
}

static void pi_aabb_copy(PiAABBBox *dst, PiAABBBox *src)
{
	if(dst != src)
	{
		dst->minPt.x = src->minPt.x;
		dst->minPt.y = src->minPt.y;
		dst->minPt.z = src->minPt.z;

		dst->maxPt.x = src->maxPt.x;
		dst->maxPt.y = src->maxPt.y;
		dst->maxPt.z = src->maxPt.z;
	}
}

static void pi_aabb_transform(PiAABBBox *box, PiMatrix4 *mat)
{
	PiVector3 v;
	PiAABBBox temp;

	if (!_is_aabb_valid(box))
	{
		return;
	}

	pi_aabb_init(&temp);

	pi_vec3_set(&v, box->minPt.x, box->minPt.y, box->minPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->minPt.x, box->minPt.y, box->maxPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->minPt.x, box->maxPt.y, box->minPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->minPt.x, box->maxPt.y, box->maxPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->maxPt.x, box->minPt.y, box->minPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->maxPt.x, box->minPt.y, box->maxPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->maxPt.x, box->maxPt.y, box->minPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_vec3_set(&v, box->maxPt.x, box->maxPt.y, box->maxPt.z);
	pi_mat4_apply_point(&v, &v, mat);
	pi_aabb_add_point(&temp, &v);

	pi_aabb_copy(box, &temp);
}

static PiBool pi_aabb_is_overlapped(PiAABBBox* b1, PiAABBBox* b2)
{
	return !(b1->minPt.x >= b2->maxPt.x
		|| b1->maxPt.x <= b2->minPt.x
		|| b1->minPt.y >= b2->maxPt.y
		|| b1->maxPt.y <= b2->minPt.y
		|| b1->minPt.z >= b2->maxPt.z
		|| b1->maxPt.z <= b2->minPt.z);
}

static PiBool pi_aabb_is_2in1(PiAABBBox* b1, PiAABBBox* b2)
{
	return (b2->maxPt.x <= b1->maxPt.x
		&& b2->minPt.x >= b1->minPt.x
		&& b2->maxPt.y <= b1->maxPt.y
		&& b2->minPt.y >= b1->minPt.y
		&& b2->maxPt.z <= b1->maxPt.z
		&& b2->minPt.z >= b1->minPt.z);
}

static PiBool pi_aabb_is_contian_point(PiAABBBox *box, PiVector3 *pt)
{
	return  pt->x >= box->minPt.x && pt->x <= box->maxPt.x && 
			pt->y >= box->minPt.y && pt->y <= box->maxPt.y && 
			pt->z >= box->minPt.z && pt->z <= box->maxPt.z;
}
