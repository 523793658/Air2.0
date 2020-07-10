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

#include "pi_matrix4.h"

const PiMatrix4 g_identity_mat4 = 
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

void pi_mat4_set_identity(PiMatrix4 *dst)
{
	dst->m[0][1] = dst->m[0][2] = dst->m[0][3] = 0.0f;
	dst->m[1][0] = dst->m[1][2] = dst->m[1][3] = 0.0f;
	dst->m[2][0] = dst->m[2][1] = dst->m[2][3] = 0.0f;
	dst->m[3][0] = dst->m[3][1] = dst->m[3][2] = 0.0f;
	dst->m[0][0] = dst->m[1][1] = dst->m[2][2] = dst->m[3][3] = 1.0f;
}

PiMatrix4* pi_mat4_get_identity(void)
{
	return (PiMatrix4*)&g_identity_mat4;
}

void pi_mat4_mul(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2)
{
#ifdef _SSE_
	float* result = &dst->m[0][0];
	const float* left = &src1->m[0][0];
	const float* right = &src2->m[0][0];
	__asm
	{
		mov eax, left
			mov edi, right
			mov edx, result
			movups xmm4, [edi]
			movups xmm5, [edi + 16]
			movups xmm6, [edi + 32]
			movups xmm7, [edi + 48]
			mov edi, 0
			mov ecx, 4
		l_:     movups xmm0, [eax + edi]
				movaps xmm1, xmm0
				movaps xmm2, xmm0
				movaps xmm3, xmm0
				shufps xmm0, xmm0, 00000000b
				shufps xmm1, xmm1, 01010101b
				shufps xmm2, xmm2, 10101010b
				shufps xmm3, xmm3, 11111111b
				mulps xmm0, xmm4
				mulps xmm1, xmm5
				mulps xmm2, xmm6
				mulps xmm3, xmm7
				addps xmm2, xmm0
				addps xmm3, xmm1
				addps xmm3, xmm2
				movups[edx + edi], xmm3
				add edi, 16
				loop l_
	}
#else
	PiMatrix4 temp;
	temp.m[0][0] = src1->m[0][0] * src2->m[0][0] + src1->m[0][1] * src2->m[1][0] +
		src1->m[0][2] * src2->m[2][0] + src1->m[0][3] * src2->m[3][0];
	temp.m[0][1] = src1->m[0][0] * src2->m[0][1] + src1->m[0][1] * src2->m[1][1] + 
		src1->m[0][2] * src2->m[2][1] + src1->m[0][3] * src2->m[3][1];
	temp.m[0][2] = src1->m[0][0] * src2->m[0][2] + src1->m[0][1] * src2->m[1][2] + 
		src1->m[0][2] * src2->m[2][2] + src1->m[0][3] * src2->m[3][2];
	temp.m[0][3] = src1->m[0][0] * src2->m[0][3] + src1->m[0][1] * src2->m[1][3] + 
		src1->m[0][2] * src2->m[2][3] + src1->m[0][3] * src2->m[3][3];

	temp.m[1][0] = src1->m[1][0] * src2->m[0][0] + src1->m[1][1] * src2->m[1][0] + 
		src1->m[1][2] * src2->m[2][0] + src1->m[1][3] * src2->m[3][0];
	temp.m[1][1] = src1->m[1][0] * src2->m[0][1] + src1->m[1][1] * src2->m[1][1] + 
		src1->m[1][2] * src2->m[2][1] + src1->m[1][3] * src2->m[3][1];
	temp.m[1][2] = src1->m[1][0] * src2->m[0][2] + src1->m[1][1] * src2->m[1][2] + 
		src1->m[1][2] * src2->m[2][2] + src1->m[1][3] * src2->m[3][2];
	temp.m[1][3] = src1->m[1][0] * src2->m[0][3] + src1->m[1][1] * src2->m[1][3] + 
		src1->m[1][2] * src2->m[2][3] + src1->m[1][3] * src2->m[3][3];

	temp.m[2][0] = src1->m[2][0] * src2->m[0][0] + src1->m[2][1] * src2->m[1][0] + 
		src1->m[2][2] * src2->m[2][0] + src1->m[2][3] * src2->m[3][0];
	temp.m[2][1] = src1->m[2][0] * src2->m[0][1] + src1->m[2][1] * src2->m[1][1] + 
		src1->m[2][2] * src2->m[2][1] + src1->m[2][3] * src2->m[3][1];
	temp.m[2][2] = src1->m[2][0] * src2->m[0][2] + src1->m[2][1] * src2->m[1][2] + 
		src1->m[2][2] * src2->m[2][2] + src1->m[2][3] * src2->m[3][2];
	temp.m[2][3] = src1->m[2][0] * src2->m[0][3] + src1->m[2][1] * src2->m[1][3] + 
		src1->m[2][2] * src2->m[2][3] + src1->m[2][3] * src2->m[3][3];

	temp.m[3][0] = src1->m[3][0] * src2->m[0][0] + src1->m[3][1] * src2->m[1][0] + 
		src1->m[3][2] * src2->m[2][0] + src1->m[3][3] * src2->m[3][0];
	temp.m[3][1] = src1->m[3][0] * src2->m[0][1] + src1->m[3][1] * src2->m[1][1] + 
		src1->m[3][2] * src2->m[2][1] + src1->m[3][3] * src2->m[3][1];
	temp.m[3][2] = src1->m[3][0] * src2->m[0][2] + src1->m[3][1] * src2->m[1][2] +
		src1->m[3][2] * src2->m[2][2] + src1->m[3][3] * src2->m[3][2];
	temp.m[3][3] = src1->m[3][0] * src2->m[0][3] + src1->m[3][1] * src2->m[1][3] + 
		src1->m[3][2] * src2->m[2][3] + src1->m[3][3] * src2->m[3][3];

	dst->m[0][0] = temp.m[0][0];	dst->m[0][1] = temp.m[0][1];	dst->m[0][2] = temp.m[0][2];	dst->m[0][3] = temp.m[0][3];
	dst->m[1][0] = temp.m[1][0];	dst->m[1][1] = temp.m[1][1];	dst->m[1][2] = temp.m[1][2];	dst->m[1][3] = temp.m[1][3];
	dst->m[2][0] = temp.m[2][0];	dst->m[2][1] = temp.m[2][1];	dst->m[2][2] = temp.m[2][2];	dst->m[2][3] = temp.m[2][3];
	dst->m[3][0] = temp.m[3][0];	dst->m[3][1] = temp.m[3][1];	dst->m[3][2] = temp.m[3][2];	dst->m[3][3] = temp.m[3][3];
#endif
}

void pi_mat4_inverse(PiMatrix4 *dst, const PiMatrix4 *src)
{
	float m00 = src->m[0][0], m01 = src->m[0][1], m02 = src->m[0][2], m03 = src->m[0][3];
	float m10 = src->m[1][0], m11 = src->m[1][1], m12 = src->m[1][2], m13 = src->m[1][3];
	float m20 = src->m[2][0], m21 = src->m[2][1], m22 = src->m[2][2], m23 = src->m[2][3];
	float m30 = src->m[3][0], m31 = src->m[3][1], m32 = src->m[3][2], m33 = src->m[3][3];

	float v0 = m20 * m31 - m21 * m30;
	float v1 = m20 * m32 - m22 * m30;
	float v2 = m20 * m33 - m23 * m30;


	float v3 = m21 * m32 - m22 * m31;
	float v4 = m21 * m33 - m23 * m31;
	float v5 = m22 * m33 - m23 * m32;

	float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
	float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
	float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
	float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

	float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	dst->m[0][0] = t00 * invDet;
	dst->m[1][0] = t10 * invDet;
	dst->m[2][0] = t20 * invDet;
	dst->m[3][0] = t30 * invDet;

	dst->m[0][1] = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	dst->m[1][1] = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	dst->m[2][1] = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	dst->m[3][1] = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	dst->m[0][2] = + invDet * (v5 * m01 - v4 * m02 + v3 * m03);
	dst->m[1][2] = - invDet * (v5 * m00 - v2 * m02 + v1 * m03);
	dst->m[2][2] = + invDet * (v4 * m00 - v2 * m01 + v0 * m03);
	dst->m[3][2] = - invDet * (v3 * m00 - v1 * m01 + v0 * m02);

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	dst->m[0][3] = - invDet * (v5 * m01 - v4 * m02 + v3 * m03);
	dst->m[1][3] = + invDet * (v5 * m00 - v2 * m02 + v1 * m03);
	dst->m[2][3] = - invDet * (v4 * m00 - v2 * m01 + v0 * m03);
	dst->m[3][3] = + invDet * (v3 * m00 - v1 * m01 + v0 * m02);
}

void pi_mat4_build_rotate(PiMatrix4 *dst, const struct PiQuaternion *rotate)
{
	float fTx  = rotate->x + rotate->x;
	float fTy  = rotate->y + rotate->y;
	float fTz  = rotate->z + rotate->z;

	float fTwx = fTx * rotate->w;
	float fTwy = fTy * rotate->w;
	float fTwz = fTz * rotate->w;
	float fTxx = fTx * rotate->x;
	float fTxy = fTy * rotate->x;
	float fTxz = fTz * rotate->x;
	float fTyy = fTy * rotate->y;
	float fTyz = fTz * rotate->y;
	float fTzz = fTz * rotate->z;

	dst->m[0][0] = 1.0f - (fTyy + fTzz);
	dst->m[0][1] = fTxy - fTwz;
	dst->m[0][2] = fTxz + fTwy;
	dst->m[0][3] = 0.0f;

	dst->m[1][0] = fTxy + fTwz;
	dst->m[1][1] = 1.0f - (fTxx + fTzz);
	dst->m[1][2] = fTyz - fTwx;
	dst->m[1][3] = 0.0f;

	dst->m[2][0] = fTxz - fTwy;
	dst->m[2][1] = fTyz + fTwx;
	dst->m[2][2] = 1.0f - (fTxx + fTyy);
	dst->m[2][3] = 0.0f;

	dst->m[3][0] = 0.0f;
	dst->m[3][1] = 0.0f;
	dst->m[3][2] = 0.0f;
	dst->m[3][3] = 1.0f;

}

void pi_mat4_build_transform(PiMatrix4 *dst, 
	const PiVector3 *translate, const PiVector3 *scale, const struct PiQuaternion *rotate)
{
	pi_mat4_build_rotate(dst, rotate);
	
	dst->m[0][0] *= scale->x;	dst->m[0][1] *= scale->y;	dst->m[0][2] *= scale->z;
	dst->m[1][0] *= scale->x; 	dst->m[1][1] *= scale->y; 	dst->m[1][2] *= scale->z; 
	dst->m[2][0] *= scale->x; 	dst->m[2][1] *= scale->y;	dst->m[2][2] *= scale->z; 
	dst->m[0][3] = translate->x;	dst->m[1][3] = translate->y;	dst->m[2][3] = translate->z;
}

void pi_mat4_decompose(PiVector3 *pos, PiVector3 *scale, PiQuaternion *rotate, const PiMatrix4 *mat)
{
	//首先这个矩阵须是affine的
	//拿其中3X3的部分做分解

	// Factor M = QR = QDU where Q is orthogonal, D is diagonal,
	// and U is upper triangular with ones on its diagonal.  Algorithm uses
	// Gram-Schmidt orthogonalization (the QR algorithm).
	//
	// If M = [ m0 | m1 | m2 ] and Q = [ q0 | q1 | q2 ], then
	//
	//   q0 = m0/|m0|
	//   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
	//   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
	//
	// where |V| indicates length of vector V and A*B indicates dot
	// product of vectors A and B.  The matrix R has entries
	//
	//   r00 = q0*m0  r01 = q0*m1  r02 = q0*m2
	//   r10 = 0      r11 = q1*m1  r12 = q1*m2
	//   r20 = 0      r21 = 0      r22 = q2*m2
	//
	// so D = diag(r00,r11,r22) and U has entries u01 = r01/r00,
	// u02 = r02/r00, and u12 = r12/r11.

	// Q = rotation
	// D = scaling
	// U = shear

	// D stores the three diagonal entries r00, r11, r22
	// U stores the entries U[0] = u01, U[1] = u02, U[2] = u12

	// build orthogonal matrix Q
	PiMatrix4 k_q;
	PiVector3 k_d;
	PiMatrix4 k_r;
	float f_dot = 0.0f;
	float f_det = 0.0f;
	int i,j;

	float inv_length = 1.0f / pi_math_sqrt(mat->m[0][0]*mat->m[0][0]
									+ mat->m[1][0]*mat->m[1][0] +
									mat->m[2][0]*mat->m[2][0]);
	k_q.m[0][0] = mat->m[0][0]*inv_length;
	k_q.m[1][0] = mat->m[1][0]*inv_length;
	k_q.m[2][0] = mat->m[2][0]*inv_length;

	f_dot = k_q.m[0][0]*mat->m[0][1] + k_q.m[1][0]*mat->m[1][1] +
		k_q.m[2][0]*mat->m[2][1];
	k_q.m[0][1] = mat->m[0][1]-f_dot*k_q.m[0][0];
	k_q.m[1][1] = mat->m[1][1]-f_dot*k_q.m[1][0];
	k_q.m[2][1] = mat->m[2][1]-f_dot*k_q.m[2][0];
	inv_length = 1.0f / pi_math_sqrt(k_q.m[0][1]*k_q.m[0][1] + k_q.m[1][1]*k_q.m[1][1] +
		k_q.m[2][1]*k_q.m[2][1]);
	k_q.m[0][1] *= inv_length;
	k_q.m[1][1] *= inv_length;
	k_q.m[2][1] *= inv_length;

	f_dot = k_q.m[0][0]*mat->m[0][2] + k_q.m[1][0]*mat->m[1][2] +
		k_q.m[2][0]*mat->m[2][2];
	k_q.m[0][2] = mat->m[0][2]-f_dot*k_q.m[0][0];
	k_q.m[1][2] = mat->m[1][2]-f_dot*k_q.m[1][0];
	k_q.m[2][2] = mat->m[2][2]-f_dot*k_q.m[2][0];
	f_dot = k_q.m[0][1]*mat->m[0][2] + k_q.m[1][1]*mat->m[1][2] +
		k_q.m[2][1]*mat->m[2][2];
	k_q.m[0][2] -= f_dot*k_q.m[0][1];
	k_q.m[1][2] -= f_dot*k_q.m[1][1];
	k_q.m[2][2] -= f_dot*k_q.m[2][1];
	inv_length = 1.0f / pi_math_sqrt(k_q.m[0][2]*k_q.m[0][2] + k_q.m[1][2]*k_q.m[1][2] +
		k_q.m[2][2]*k_q.m[2][2]);
	k_q.m[0][2] *= inv_length;
	k_q.m[1][2] *= inv_length;
	k_q.m[2][2] *= inv_length;

	// guarantee that orthogonal matrix has determinant 1 (no reflections)
	f_det = k_q.m[0][0]*k_q.m[1][1]*k_q.m[2][2] + k_q.m[0][1]*k_q.m[1][2]*k_q.m[2][0] +
		k_q.m[0][2]*k_q.m[1][0]*k_q.m[2][1] - k_q.m[0][2]*k_q.m[1][1]*k_q.m[2][0] -
		k_q.m[0][1]*k_q.m[1][0]*k_q.m[2][2] - k_q.m[0][0]*k_q.m[1][2]*k_q.m[2][1];

	if ( f_det < 0.0 )
	{
		for ( i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
				k_q.m[i][j] = -k_q.m[i][j];
	}

	// build "right" matrix R
	k_r.m[0][0] = k_q.m[0][0]*mat->m[0][0] + k_q.m[1][0]*mat->m[1][0] +
		k_q.m[2][0]*mat->m[2][0];
	k_r.m[0][1] = k_q.m[0][0]*mat->m[0][1] + k_q.m[1][0]*mat->m[1][1] +
		k_q.m[2][0]*mat->m[2][1];
	k_r.m[1][1] = k_q.m[0][1]*mat->m[0][1] + k_q.m[1][1]*mat->m[1][1] +
		k_q.m[2][1]*mat->m[2][1];
	k_r.m[0][2] = k_q.m[0][0]*mat->m[0][2] + k_q.m[1][0]*mat->m[1][2] +
		k_q.m[2][0]*mat->m[2][2];
	k_r.m[1][2] = k_q.m[0][1]*mat->m[0][2] + k_q.m[1][1]*mat->m[1][2] +
		k_q.m[2][1]*mat->m[2][2];
	k_r.m[2][2] = k_q.m[0][2]*mat->m[0][2] + k_q.m[1][2]*mat->m[1][2] +
		k_q.m[2][2]*mat->m[2][2];

	// the scaling component
	k_d.x = k_r.m[0][0];
	k_d.y = k_r.m[1][1];
	k_d.z = k_r.m[2][2];

	// the shear component 这里不需要这个分量
	//int_d0 = 1.0f/k_d.x;
	//k_u.x = k_r.m[0][1]*int_d0;
	//k_u.y = k_r.m[0][2]*int_d0;
	//k_u.z = k_r.m[1][2]/k_d.y;

	pos->x = mat->m[0][3];
	pos->y = mat->m[1][3];
	pos->z = mat->m[2][3];
	
	scale->x = k_d.x;
	scale->y = k_d.y;
	scale->z = k_d.z;
	pi_quat_from_mat4(rotate, &k_q);

}

PiBool pi_mat4_lookat_rh(PiMatrix4 *dst, const PiVector3 *pos, const PiVector3 *target, const PiVector3 *up)
{
	PiVector3 xAxis, yAxis, zAxis;

	pi_mat4_set_identity(dst);

	/* 左右手矩阵推导的不同，仅在于这行，如果是左手: z = target - pos */
	pi_vec3_sub(&zAxis, pos, target);
	if(!pi_vec3_normalise(&zAxis, &zAxis))
		return FALSE;

	pi_vec3_cross(&xAxis, up, &zAxis);
	if(!pi_vec3_normalise(&xAxis, &xAxis))
		return FALSE;

	pi_vec3_cross(&yAxis, &zAxis, &xAxis);

	dst->m[0][0] = xAxis.x;
	dst->m[1][0] = yAxis.x;
	dst->m[2][0] = zAxis.x;

	dst->m[0][1] = xAxis.y;
	dst->m[1][1] = yAxis.y;
	dst->m[2][1] = zAxis.y;

	dst->m[0][2] = xAxis.z;
	dst->m[1][2] = yAxis.z;
	dst->m[2][2] = zAxis.z;

	dst->m[0][3] = -pi_vec3_dot(&xAxis, pos);
	dst->m[1][3] = -pi_vec3_dot(&yAxis, pos);
	dst->m[2][3] = -pi_vec3_dot(&zAxis, pos);

	return TRUE;
}

void pi_mat4_frustum_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf)
{
	float dx = 1.0f / (rt - lt);
	float dy = 1.0f / (tp - bm);
	float dz = 1.0f / (zf - zn);

	pi_memset_inline(dst, 0, sizeof(PiMatrix4));
	dst->m[0][0] = 2 * zn * dx;
	dst->m[1][1] = 2 * zn * dy;
	dst->m[0][2] = dx * (rt + lt);
	dst->m[1][2] = dy * (bm + tp);
	dst->m[2][2] = -zf * dz;
	dst->m[3][2] = -1.0f;
	dst->m[2][3] = -zf * zn * dz;
}

void pi_mat4_pers_fov_rh(PiMatrix4 *dst, float fovRad, float ratio, float zNear, float zFar)
{
	float h = zNear * pi_math_tan(0.5f * fovRad);
	float w = h * ratio;
	pi_mat4_frustum_rh(dst, -w, w, -h, h, zNear, zFar);
}

void pi_mat4_ortho_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf)
{
	float dx = 1.0f / (rt - lt);
	float dy = 1.0f / (tp - bm);
	float dz = 1.0f / (zf - zn);

	pi_memset_inline(dst, 0, sizeof(PiMatrix4));
	dst->m[0][0] = 2 * dx;
	dst->m[1][1] = 2 * dy;
	dst->m[2][2] = -dz;
	dst->m[0][3] = -dx * (lt + rt);
	dst->m[1][3] = -dy * (bm + tp);
	dst->m[2][3] = -zn * dz;
	dst->m[3][3] = 1.0f;
	
}
