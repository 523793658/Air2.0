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

static void pi_mat4_copy(PiMatrix4 *dst, const PiMatrix4 *src)
{
	if(dst != src)
	{
		dst->m[0][0] = src->m[0][0];	dst->m[0][1] = src->m[0][1];	dst->m[0][2] = src->m[0][2];	dst->m[0][3] = src->m[0][3];
		dst->m[1][0] = src->m[1][0];	dst->m[1][1] = src->m[1][1];	dst->m[1][2] = src->m[1][2];	dst->m[1][3] = src->m[1][3];
		dst->m[2][0] = src->m[2][0];	dst->m[2][1] = src->m[2][1];	dst->m[2][2] = src->m[2][2];	dst->m[2][3] = src->m[2][3];
		dst->m[3][0] = src->m[3][0];	dst->m[3][1] = src->m[3][1];	dst->m[3][2] = src->m[3][2];	dst->m[3][3] = src->m[3][3];
	}
}

static PiBool pi_mat4_is_equal(const PiMatrix4 *src1, const PiMatrix4 *src2)
{
	int i = 0, j = 0;
	for(i = 0; i < 4; ++i)
		for(j = 0; j < 4; ++j)
			if(!IS_FLOAT_EQUAL(src1->m[i][j], src2->m[i][j]))
				return FALSE;
	return TRUE;
}

static void pi_mat4_add(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2)
{
	dst->m[0][0] = src1->m[0][0] + src2->m[0][0];	dst->m[0][1] = src1->m[0][1] + src2->m[0][1];
	dst->m[0][2] = src1->m[0][2] + src2->m[0][2];	dst->m[0][3] = src1->m[0][3] + src2->m[0][3];
	
	dst->m[1][0] = src1->m[1][0] + src2->m[1][0];	dst->m[1][1] = src1->m[1][1] + src2->m[1][1];
	dst->m[1][2] = src1->m[1][2] + src2->m[1][2];	dst->m[1][3] = src1->m[1][3] + src2->m[1][3];
	
	dst->m[2][0] = src1->m[2][0] + src2->m[2][0];	dst->m[2][1] = src1->m[2][1] + src2->m[2][1];
	dst->m[2][2] = src1->m[2][2] + src2->m[2][2];	dst->m[2][3] = src1->m[2][3] + src2->m[2][3];
	
	dst->m[3][0] = src1->m[3][0] + src2->m[3][0];	dst->m[3][1] = src1->m[3][1] + src2->m[3][1];
	dst->m[3][2] = src1->m[3][2] + src2->m[3][2];	dst->m[3][3] = src1->m[3][3] + src2->m[3][3];
}

static void pi_mat4_scale(PiMatrix4 *dst, const PiMatrix4 *src, float scale)
{
	dst->m[0][0] = scale * src->m[0][0];	dst->m[0][1] = scale * src->m[0][1];	dst->m[0][2] = scale * src->m[0][2];	dst->m[0][3] = scale * src->m[0][3];
	dst->m[1][0] = scale * src->m[1][0];	dst->m[1][1] = scale * src->m[1][1];	dst->m[1][2] = scale * src->m[1][2];	dst->m[1][3] = scale * src->m[1][3];
	dst->m[2][0] = scale * src->m[2][0];	dst->m[2][1] = scale * src->m[2][1];	dst->m[2][2] = scale * src->m[2][2];	dst->m[2][3] = scale * src->m[2][3];
	dst->m[3][0] = scale * src->m[3][0];	dst->m[3][1] = scale * src->m[3][1];	dst->m[3][2] = scale * src->m[3][2];	dst->m[3][3] = scale * src->m[3][3];
}

static void pi_mat4_transpose(PiMatrix4 *dst, const PiMatrix4 *src)
{
	float tmp;
	
	dst->m[0][0] = src->m[0][0];	dst->m[1][1] = src->m[1][1];	dst->m[2][2] = src->m[2][2];	dst->m[3][3] = src->m[3][3];

	tmp = src->m[0][1];		dst->m[0][1] = src->m[1][0];	dst->m[1][0] = tmp;
	tmp = src->m[0][2];		dst->m[0][2] = src->m[2][0];	dst->m[2][0] = tmp;
	tmp = src->m[0][3];		dst->m[0][3] = src->m[3][0];	dst->m[3][0] = tmp;

	tmp = src->m[1][2];		dst->m[1][2] = src->m[2][1];	dst->m[2][1] = tmp;
	tmp = src->m[1][3];		dst->m[1][3] = src->m[3][1];	dst->m[3][1] = tmp;
	
	tmp = src->m[2][3];		dst->m[2][3] = src->m[3][2];	dst->m[3][2] = tmp;
}

static void pi_mat4_apply_point(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat)
{
	float x = src->x, y = src->y, z = src->z;
	float fInvW = 1.0f / (mat->m[3][3] + mat->m[3][0] * x + mat->m[3][1] * y + mat->m[3][2] * z);
	dst->x = fInvW * (mat->m[0][3] + x * mat->m[0][0] + y * mat->m[0][1] + z * mat->m[0][2]);
	dst->y = fInvW * (mat->m[1][3] + x * mat->m[1][0] + y * mat->m[1][1] + z * mat->m[1][2]);
	dst->z = fInvW * (mat->m[2][3] + x * mat->m[2][0] + y * mat->m[2][1] + z * mat->m[2][2]);	
}

static void pi_mat4_apply_vector(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat)
{
	float x = src->x, y = src->y, z = src->z;

	dst->x = x * mat->m[0][0] + y * mat->m[0][1] + z * mat->m[0][2];
	dst->y = x * mat->m[1][0] + y * mat->m[1][1] + z * mat->m[1][2];
	dst->z = x * mat->m[2][0] + y * mat->m[2][1] + z * mat->m[2][2];
}

static void pi_mat4_set_translate(PiMatrix4 *dst, const PiVector3 *translate)
{
	dst->m[0][3] = translate->x;
	dst->m[1][3] = translate->y;
	dst->m[2][3] = translate->z;
}

static void pi_mat4_extract_translate( PiVector3 *dst, const PiMatrix4 *mat )
{
	dst->x = mat->m[0][3];
	dst->y = mat->m[1][3];
	dst->z = mat->m[2][3];
}

static void pi_mat4_set_scale(PiMatrix4 *dst, const PiVector3 *scale)
{
	dst->m[0][0] = scale->x;
	dst->m[1][1] = scale->y;
	dst->m[2][2] = scale->z;
}
