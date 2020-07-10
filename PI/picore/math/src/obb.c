#include "pi_obb.h"

void PI_API pi_obb_copy(PiOBBBox *dst, PiOBBBox *src)
{
	dst->center.x = src->center.x;		dst->center.y = src->center.y;		dst->center.z = src->center.z;
	dst->extent[0] = src->extent[0];	dst->extent[1] = src->extent[1];	dst->extent[2] = src->extent[2];
	
	dst->axis[0].x = src->axis[0].x;	dst->axis[0].y = src->axis[0].y;	dst->axis[0].z = src->axis[0].z;
	dst->axis[1].x = src->axis[1].x;	dst->axis[1].y = src->axis[1].y;	dst->axis[1].z = src->axis[1].z;
	dst->axis[2].x = src->axis[2].x;	dst->axis[2].y = src->axis[2].y;	dst->axis[2].z = src->axis[2].z;
}

void PI_API pi_obb_get_points(PiOBBBox *box, PiVector3 *buffer)
{
	int i;
	PiVector3 tmp[3];
	for(i = 0; i < 3; i++) {
		tmp[i].x = box->axis[i].x * box->extent[i];
		tmp[i].y = box->axis[i].y * box->extent[i];
		tmp[i].z = box->axis[i].z * box->extent[i];
	}
	pi_vec3_add(&buffer[0], &box->center, &tmp[0]);
	pi_vec3_add(&buffer[0], &buffer[0], &tmp[1]);
	pi_vec3_add(&buffer[0], &buffer[0], &tmp[2]);

	pi_vec3_sub(&buffer[1], &box->center, &tmp[0]);
	pi_vec3_add(&buffer[1], &buffer[1], &tmp[1]);
	pi_vec3_add(&buffer[1], &buffer[1], &tmp[2]);

	pi_vec3_add(&buffer[2], &box->center, &tmp[0]);
	pi_vec3_sub(&buffer[2], &buffer[2], &tmp[1]);
	pi_vec3_add(&buffer[2], &buffer[2], &tmp[2]);

	pi_vec3_add(&buffer[3], &box->center, &tmp[0]);
	pi_vec3_add(&buffer[3], &buffer[3], &tmp[1]);
	pi_vec3_sub(&buffer[3], &buffer[3], &tmp[2]);

	pi_vec3_add(&buffer[4], &box->center, &tmp[0]);
	pi_vec3_sub(&buffer[4], &buffer[4], &tmp[1]);
	pi_vec3_sub(&buffer[4], &buffer[4], &tmp[2]);

	pi_vec3_sub(&buffer[5], &box->center, &tmp[0]);
	pi_vec3_add(&buffer[5], &buffer[5], &tmp[1]);
	pi_vec3_sub(&buffer[5], &buffer[5], &tmp[2]);

	pi_vec3_sub(&buffer[6], &box->center, &tmp[0]);
	pi_vec3_sub(&buffer[6], &buffer[6], &tmp[1]);
	pi_vec3_add(&buffer[6], &buffer[6], &tmp[2]);

	pi_vec3_sub(&buffer[7], &box->center, &tmp[0]);
	pi_vec3_sub(&buffer[7], &buffer[7], &tmp[1]);
	pi_vec3_sub(&buffer[7], &buffer[7], &tmp[2]);
}

void PI_API pi_obb_get_aabb(PiOBBBox *box, PiAABBBox *dst)
{
	int i;
	PiVector3 points[8];
	pi_obb_get_points(box, points);
	pi_aabb_init(dst);
	for(i = 0; i < 8; i++) {
		pi_aabb_add_point(dst, &points[i]);
	}
}

void PI_API pi_obb_transform(PiOBBBox *box, PiMatrix4 *mat)
{
	float length_0, length_1, length_2;
	//Rotation
	pi_mat4_apply_vector(&box->axis[0],  &box->axis[0],  mat);
	pi_mat4_apply_vector(&box->axis[1],  &box->axis[1],  mat);
	pi_mat4_apply_vector(&box->axis[2],  &box->axis[2],  mat);
	length_0 = pi_vec3_len(&box->axis[0]);
	length_1 = pi_vec3_len(&box->axis[1]);
	length_2 = pi_vec3_len(&box->axis[2]);
	box->axis[0].x /= length_0;
	box->axis[0].y /= length_0;
	box->axis[0].z /= length_0;
	box->axis[1].x /= length_1;
	box->axis[1].y /= length_1;
	box->axis[1].z /= length_1;
	box->axis[2].x /= length_2;
	box->axis[2].y /= length_2;
	box->axis[2].z /= length_2;
	//Scaling
	box->extent[0] *= length_0;
	box->extent[1] *= length_1;
	box->extent[2] *= length_2;
	//Translation
	pi_mat4_apply_point(&box->center, &box->center, mat);
}
