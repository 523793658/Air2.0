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

/** 
 * ����ƽ��ͷ���ƽ�� 
 * ԭ���� v = (x, y, z, 1) Ϊ���������е�һ��
 * ���裺M = ProjMat * ViewMat
 * ��v' = (x', y', z', w') = M * v
 * v'Ϊ�ü��ռ�CVV�еĵ㣺��M.row[i]Ϊ����M�ĵ�i��Ԫ����ɵ�����
 * x' = v.dot(M.row[1])
 * y' = v.dot(M.row[2])
 * z' = v.dot(M.row[3])
 * w' = v.dot(M.row[4])
 
 * ��ƽ�淽��Ϊ��x'/w' = -1  --> v.dot(M.row[4] + M.row[1]) = 0
 * չ��Ϊ��x * (M30 + M00) + y * (M31 + M01) + z * (M32 + M02) + (M33 + M03) = 0
 
 * ��ƽ�淽�̣�x'/w' = 1
 * չ��Ϊ��x * (M30 - M00) + y * (M31 - M01) + z * (M32 - M02) + (M33 - M03) = 0

 * ��ƽ�淽�̣�y'/w' = 1
 * չ��Ϊ��x * (M30 - M10) + y * (M31 - M11) + z * (M32 - M12) + (M33 - M13) = 0

 * ��ƽ�淽�̣�y'/w' = -1
 * չ��Ϊ��x * (M30 + M10) + y * (M31 + M11) + z * (M32 + M12) + (M33 + M13) = 0

 * ��ƽ�淽�̣�z'/w' = -1
 * չ��Ϊ��x * (M30 + M20) + y * (M31 + M21) + z * (M32 + M22) + (M33 + M23) = 0

 * Զƽ�淽�̣�z'/w' = 1
 * չ��Ϊ��x * (M30 - M20) + y * (M31 - M21) + z * (M32 - M22) + (M33 - M23) = 0
 */
static void frustum_update_plane(PiFrustum* frustum, PiMatrix4* viewMat, PiMatrix4* projMat)
{
	float d;
	PiVector3 v;
	PiMatrix4 *mat = &frustum->pvMat;

	pi_mat4_copy(&frustum->projMat, projMat);
	pi_mat4_copy(&frustum->viewMat, viewMat);
	pi_mat4_inverse(&frustum->inverseViewMat, viewMat);
	pi_mat4_mul(mat, &frustum->projMat, &frustum->viewMat);
	pi_mat4_inverse(&frustum->inversePVM, mat);
	pi_mat4_inverse(&frustum->inverseProjMat, &frustum->projMat);

	d = mat->m[3][3] + mat->m[0][3];
	pi_vec3_set(&v, mat->m[3][0] + mat->m[0][0],
				mat->m[3][1] + mat->m[0][1],
				mat->m[3][2] + mat->m[0][2]);
	pi_plane_set(&frustum->plane[EFP_LEFT], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_LEFT], &frustum->plane[EFP_LEFT]);

	d = mat->m[3][3] - mat->m[0][3];
	pi_vec3_set(&v, mat->m[3][0] - mat->m[0][0],
				mat->m[3][1] - mat->m[0][1],
				mat->m[3][2] - mat->m[0][2]);
	pi_plane_set(&frustum->plane[EFP_RIGHT], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_RIGHT], &frustum->plane[EFP_RIGHT]);

	d = mat->m[3][3] - mat->m[1][3];
	pi_vec3_set(&v, mat->m[3][0] - mat->m[1][0],
				mat->m[3][1] - mat->m[1][1],
				mat->m[3][2] - mat->m[1][2]);
	pi_plane_set(&frustum->plane[EFP_TOP], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_TOP], &frustum->plane[EFP_TOP]);

	d = mat->m[3][3] + mat->m[1][3];
	pi_vec3_set(&v, mat->m[3][0] + mat->m[1][0],
				mat->m[3][1] + mat->m[1][1],
				mat->m[3][2] + mat->m[1][2]);
	pi_plane_set(&frustum->plane[EFP_BOTTOM], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_BOTTOM], &frustum->plane[EFP_BOTTOM]);

	d = mat->m[3][3] + mat->m[2][3];
	pi_vec3_set(&v, mat->m[3][0] + mat->m[2][0],
				mat->m[3][1] + mat->m[2][1],
				mat->m[3][2] + mat->m[2][2]);
	pi_plane_set(&frustum->plane[EFP_NEAR], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_NEAR], &frustum->plane[EFP_NEAR]);

	d = mat->m[3][3] - mat->m[2][3];
	pi_vec3_set(&v, mat->m[3][0] - mat->m[2][0],
				mat->m[3][1] - mat->m[2][1],
				mat->m[3][2] - mat->m[2][2]);
	pi_plane_set(&frustum->plane[EFP_FAR], &v, d);
	pi_plane_normalise(&frustum->plane[EFP_FAR], &frustum->plane[EFP_FAR]);
}

/* ����ƽ��ͷ��� 6 ���� */
static void pi_frustum_set_planes(PiFrustum* frustum, PiPlane* planes)
{
	uint32 i;

	for (i = EFP_LEFT; i < EFP_COUNT; i++)
		pi_plane_copy(&frustum->plane[i], &planes[i]);
}

/* ����ͶӰ���� */
static void pi_frustum_set_proj(PiFrustum* frustum, PiMatrix4 *mat)
{
	pi_mat4_copy(&frustum->projMat, mat);
}

/* ����ƽ��ͷ�� */
static void pi_frustum_update(PiFrustum* frustum, PiMatrix4* viewMat, PiMatrix4* projMat)
{	
	frustum_update_plane(frustum, viewMat, projMat);
}

/* �ж�ƽ��ͷ���Ƿ�� aabb �ཻ����� */
static PiBool pi_frustum_is_aabb_visible(PiFrustum* frustum, PiAABBBox* box)
{
	int i;
	float d;
	for(i = 0; i < EFP_COUNT; ++i)
	{
		d = pi_plane_dot(&frustum->plane[i], box->minPt.x, box->minPt.y, box->minPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->minPt.x, box->minPt.y, box->maxPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->minPt.x, box->maxPt.y, box->minPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->minPt.x, box->maxPt.y, box->maxPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->maxPt.x, box->minPt.y, box->minPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->maxPt.x, box->minPt.y, box->maxPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->maxPt.x, box->maxPt.y, box->minPt.z);
		if(0.0f <= d)
			continue;
		d = pi_plane_dot(&frustum->plane[i], box->maxPt.x, box->maxPt.y, box->maxPt.z);
		if(0.0f <= d)
			continue;

		return FALSE;	/* ��Χ�е�8���㶼��ͬһ�������⣬���ɼ� */
	}
	return TRUE;
}

/* �ж�ƽ��ͷ���Ƿ�������� */
static PiBool pi_frustum_is_point_visible(PiFrustum* frustum, PiVector3* point)
{
	int i;
	float d;
	for(i = 0; i < EFP_COUNT; ++i)
	{
		d = pi_plane_dot(&frustum->plane[i], point->x, point->y, point->z);
		if(d < 0.0f)
			return FALSE;
	}
	return TRUE;
}
