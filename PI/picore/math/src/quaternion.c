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

#include "pi_quaternion.h"

/* 单位四元数 */
const PiQuaternion g_identity_quat = {1.0f, 0.0f, 0.0f, 0.0f};	

PiQuaternion* pi_quat_get_unit(void)
{
	return (PiQuaternion*)&g_identity_quat;
}


void pi_quat_mul(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2)
{
	float w = src1->w * src2->w - src1->x * src2->x - src1->y * src2->y - src1->z * src2->z;
	float x = src1->w * src2->x + src1->x * src2->w + src1->y * src2->z - src1->z * src2->y;
	float y = src1->w * src2->y + src1->y * src2->w + src1->z * src2->x - src1->x * src2->z;
	float z = src1->w * src2->z + src1->z * src2->w + src1->x * src2->y - src1->y * src2->x;
	dst->w = w;
	dst->x = x;
	dst->y = y;
	dst->z = z;
}

PiBool pi_quat_normalise(PiQuaternion *dst, const PiQuaternion *src)
{
	float len = pi_quat_len(src);
	if(IS_FLOAT_EQUAL(len, 0.0f))
	{
		pi_memset_inline(dst, 0, sizeof(PiQuaternion));
		return FALSE;
	}
	pi_quat_scale(dst, src, 1.0f / len);
	return TRUE;
}

PiBool pi_quat_inverse(PiQuaternion *dst, const PiQuaternion *src)
{
	float f = pi_quat_len_square(src);
	if(IS_FLOAT_EQUAL(f, 0.0f))
	{
		pi_memset_inline(dst, 0, sizeof(PiQuaternion));
		return FALSE;
	}
	
	f = 1.0f / f;
	pi_quat_scale(dst, src, f);
	pi_quat_conjugate(dst, dst);
	return TRUE;
}

void pi_quat_from_mat4(PiQuaternion *dst, const struct PiMatrix4 *src)
{
	float fRoot;
	float fTrace = src->m[0][0]+src->m[1][1]+src->m[2][2];
	
	if(0.0f < fTrace)
	{/* |dst->w| > 1/2, may as well choose dst->w > 1/2 */
		fRoot = pi_math_sqrt(1.0f + fTrace);  /* 2 * dst->w */
		dst->w = 0.5f * fRoot;
		fRoot = 0.5f / fRoot;  /* 1/(4 * dst->w) */
		dst->x = fRoot * (src->m[2][1] - src->m[1][2]);
		dst->y = fRoot * (src->m[0][2] - src->m[2][0]);
		dst->z = fRoot * (src->m[1][0] - src->m[0][1]);
	}
	else
	{/* |dst->w| <= 1/2 */
		int i = 0, j = 0, k = 0;
		int s_iNext[3] = {1, 2, 0};
		float *apkQuat[3];
		apkQuat[0] = &dst->x;
		apkQuat[1] = &dst->y;
		apkQuat[2] = &dst->z;
		
		if(src->m[1][1] > src->m[0][0] )
			i = 1;
		if(src->m[2][2] > src->m[i][i] )
			i = 2;
		j = s_iNext[i];
		k = s_iNext[j];

		fRoot = pi_math_sqrt(1.0f + src->m[i][i] - src->m[j][j] - src->m[k][k]);
		*apkQuat[i] = 0.5f * fRoot;
		fRoot = 0.5f / fRoot;
		dst->w = fRoot * (src->m[k][j] - src->m[j][k]);
		*apkQuat[j] = fRoot * (src->m[j][i] + src->m[i][j]);
		*apkQuat[k] = fRoot * (src->m[k][i] + src->m[i][k]);
	}
}

/* dst = cos(angle / 2) + sin(angle/2) * {x, y, z} */
void pi_quat_from_angle_axis(PiQuaternion *dst, const PiVector3 *axis, float radAngle)
{
	float halfAng = 0.5f * radAngle;
	float s = pi_math_sin(halfAng);
	
	dst->w = pi_math_cos(halfAng);
	dst->x = s * axis->x;
	dst->y = s * axis->y;
	dst->z = s * axis->z;
}

void pi_quat_to_angle_axis(const PiQuaternion *src, PiVector3 *axis, float *radAngle)
{
	float f = src->x * src->x + src->y * src->y + src->z * src->z;
	if(IS_FLOAT_EQUAL(0.0f, f))
	{/* 角度是2PI的整数倍，任何轴都可以 */
		*radAngle = 0.0f;
		pi_vec3_set(axis, 1.0f, 0.0f, 0.0f);
	}
	else
	{
		*radAngle = 2.0f * pi_math_acos(src->w);

		f = 1.0f / pi_math_sqrt(f);
		axis->x = f * src->x;
		axis->y = f * src->y;
		axis->z = f * src->z;
	}
}

void pi_quat_rotate_vec3(PiVector3 *dst, const PiVector3 *src, const PiQuaternion *rotate)
{
	PiVector3 qvec;
	PiVector3 uv, uuv;
	
	pi_vec3_set(&qvec, rotate->x, rotate->y, rotate->z);
	
	pi_vec3_cross(&uv, &qvec, src);
	pi_vec3_cross(&uuv, &qvec, &uv);
	pi_vec3_scale(&uv, &uv, 2.0f * rotate->w);
	pi_vec3_scale(&uuv, &uuv, 2.0f);

	pi_vec3_add(dst, src, &uv);
	pi_vec3_add(dst, dst, &uuv);
}

void pi_quat_slerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest)
{
	PiQuaternion temp1, temp2;
	float fCos = pi_quat_dot(from, to); 
	
	if(fCos < 0.0f && isShortest)
	{
		fCos = -fCos;
		pi_quat_set(&temp1, -to->w, -to->x, -to->y, -to->z);
	}
	else
		pi_quat_copy(&temp1, to);
	
	if(pi_math_abs(fCos) >= 0.99999)
	{
		pi_quat_scale(&temp2, from, 1.0f - frac);
		pi_quat_scale(&temp1, &temp1, frac);
		pi_quat_add(&temp2, &temp2, &temp1);
		pi_quat_normalise(dst, &temp2);
	}
	else
	{

		float fSin = pi_math_sqrt(1.0f - fCos * fCos);
		float fRadAngle = pi_math_atan2(fSin, fCos);
		float fInvSin = 1.0f / fSin;
		float fCoeff0 = fInvSin * pi_math_sin((1.0f - frac) * fRadAngle);
		float fCoeff1 = fInvSin * pi_math_sin(frac * fRadAngle);
		pi_quat_scale(&temp2, from, fCoeff0);
		pi_quat_scale(&temp1, &temp1, fCoeff1);
		pi_quat_add(dst, &temp2, &temp1);
	}
}

void pi_quat_lerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest)
{
	PiQuaternion temp;
	float fCos = pi_quat_dot(from, to);
	if(0.0f > fCos && isShortest)
	{/* dst = from + frac * ((-to) - from) */
		pi_quat_set(&temp, 0.0f, 0.0f, 0.0f, 0.0f);
		pi_quat_sub(&temp, &temp, to);
		pi_quat_sub(&temp, &temp, from);
	}
	else
	{/* dst = from + frac * (to - from) */
		pi_quat_sub(&temp, to, from);
	}
	
	pi_quat_scale(&temp, &temp, frac);
	pi_quat_add(&temp, from, &temp);
	pi_quat_normalise(dst, &temp);
}

void pi_quat_rotate_to(PiQuaternion *dst, const PiVector3 *from, const PiVector3 *to)
{
	float d = 0.0f;
	PiVector3 v0, v1;
	pi_vec3_normalise(&v0, from);
	pi_vec3_normalise(&v1, to);

	d = pi_vec3_dot(&v0, &v1);
	if(1.0f <= d)
		pi_quat_set(dst, 1.0f, 0.0f, 0.0f, 0.0f);
	else if(1e-6f - 1.0f > d)
	{
		PiVector3 axis;
		pi_vec3_cross(&axis, pi_vec3_get_xunit(), from);
		if(pi_vec3_is_equal(&axis, pi_vec3_get_zero()))
			pi_vec3_cross(&axis, pi_vec3_get_yunit(), from);
		pi_vec3_normalise(&axis, &axis);
		pi_quat_from_angle_axis(dst, &axis, (float)PI_PI);
	}
	else
	{
		float s = pi_math_sqrt(2.0f * (1 + d));
		float invs = 1 / s;
		PiVector3 c;
		pi_vec3_cross(&c, &v0, &v1);

		dst->w = s * 0.5f;
		dst->x = invs * c.x;
		dst->y = invs * c.y;
		dst->z = invs * c.z;
		pi_quat_normalise(dst, dst);	
	}
}

void pi_quat_get_xaxis(PiQuaternion *q, PiVector3 *dst)
{
	float fTy  = 2.0f * q->y;
	float fTz  = 2.0f * q->z;
	float fTwy = fTy * q->w;
	float fTwz = fTz * q->w;
	float fTxy = fTy * q->x;
	float fTxz = fTz * q->x;
	float fTyy = fTy * q->y;
	float fTzz = fTz * q->z;
	pi_vec3_set(dst, 1.0f - (fTyy + fTzz), fTxy + fTwz, fTxz - fTwy);
}

void pi_quat_get_yaxis(PiQuaternion *q, PiVector3 *dst)
{
	float fTx  = 2.0f * q->x;
	float fTy  = 2.0f * q->y;
	float fTz  = 2.0f * q->z;
	float fTwx = fTx * q->w;
	float fTwz = fTz * q->w;
	float fTxx = fTx * q->x;
	float fTxy = fTy * q->x;
	float fTyz = fTz * q->y;
	float fTzz = fTz * q->z;
	pi_vec3_set(dst, fTxy - fTwz, 1.0f - (fTxx + fTzz),  fTyz + fTwx);
}

void pi_quat_get_zaxis(PiQuaternion *q, PiVector3 *dst)
{
	float fTx  = 2.0f * q->x;
	float fTy  = 2.0f * q->y;
	float fTz  = 2.0f * q->z;
	float fTwx = fTx * q->w;
	float fTwy = fTy * q->w;
	float fTxx = fTx * q->x;
	float fTxz = fTz * q->x;
	float fTyy = fTy * q->y;
	float fTyz = fTz * q->y;
	pi_vec3_set(dst, fTxz + fTwy, fTyz - fTwx, 1.0f - (fTxx + fTyy));
}

float pi_quat_get_roll_rad(PiQuaternion *q)
{
	float fTy  = 2.0f * q->y;
	float fTz  = 2.0f * q->z;
	float fTwz = fTz * q->w;
	float fTxy = fTy * q->x;
	float fTyy = fTy * q->y;
	float fTzz = fTz * q->z;
	return pi_math_atan2(fTxy + fTwz, 1.0f - (fTyy + fTzz));
}

float pi_quate_get_pitch_rad(PiQuaternion *q)
{
	float fTx  = 2.0f * q->x;
	float fTz  = 2.0f * q->z;
	float fTwx = fTx * q->w;
	float fTxx = fTx * q->x;
	float fTyz = fTz * q->y;
	float fTzz = fTz * q->z;

	return pi_math_atan2(fTyz + fTwx, 1.0f - (fTxx + fTzz));
}

float pi_quat_get_yaw_rad(PiQuaternion *q)
{
	float fTx  = 2.0f * q->x;
	float fTy  = 2.0f * q->y;
	float fTz  = 2.0f * q->z;
	float fTwy = fTy * q->w;
	float fTxx = fTx * q->x;
	float fTxz = fTz * q->x;
	float fTyy = fTy * q->y;
	return pi_math_atan2(fTxz + fTwy, 1.0f - (fTxx + fTyy));
}

/* 从 x-y-z 轴得到四元数：scene 模块的 set_direction 要用 */
void pi_quat_from_axes(PiQuaternion* quat, PiVector3* xaxis, const PiVector3* yaxis, const PiVector3* zaxis)
{
	float kRot[3][3], *apkQuat[3];
	float fTrace, fRoot;
	uint32 s_iNext[3] = { 1, 2, 0 };
	uint32 i, j, k;

    kRot[0][0] = xaxis->x;
    kRot[1][0] = xaxis->y;
    kRot[2][0] = xaxis->z;

    kRot[0][1] = yaxis->x;
    kRot[1][1] = yaxis->y;
    kRot[2][1] = yaxis->z;

    kRot[0][2] = zaxis->x;
    kRot[1][2] = zaxis->y;
    kRot[2][2] = zaxis->z;

	fTrace = kRot[0][0] + kRot[1][1] + kRot[2][2];
    if (fTrace > 0.0)
    {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = pi_math_sqrt(fTrace + 1.0f);  // 2w
        quat->w = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;  // 1/(4w)
        quat->x = (kRot[2][1] - kRot[1][2]) * fRoot;
        quat->y = (kRot[0][2] - kRot[2][0]) * fRoot;
        quat->z = (kRot[1][0] - kRot[0][1]) * fRoot;
    }
    else
    {
        // |w| <= 1/2
        i = 0;
        if ( kRot[1][1] > kRot[0][0] )
            i = 1;
        if ( kRot[2][2] > kRot[i][i] )
            i = 2;

        j = s_iNext[i];
        k = s_iNext[j];

        fRoot = pi_math_sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
        apkQuat[0] = &quat->x;
		apkQuat[1] = &quat->y;
		apkQuat[2] = &quat->z;
        *apkQuat[i] = 0.5f*fRoot;
        fRoot = 0.5f/fRoot;
        quat->w = (kRot[k][j]-kRot[j][k])*fRoot;
        *apkQuat[j] = (kRot[j][i]+kRot[i][j])*fRoot;
        *apkQuat[k] = (kRot[k][i]+kRot[i][k])*fRoot;
    }
}

void pi_quat_from_euler_angle(PiQuaternion* quat, float pitch, float yaw, float roll)
{
	float cofHalfPitch = pi_math_cos(pitch * 0.5f);
	float sinHalfPitch = pi_math_sin(pitch * 0.5f);
	float cofHalfYaw = pi_math_cos(yaw * 0.5f);
	float sinHalfYaw = pi_math_sin(yaw * 0.5f);
	float cofHalfRoll = pi_math_cos(roll * 0.5f);
	float sinHalfRoll = pi_math_sin(roll * 0.5f);

	quat->w = cofHalfPitch * cofHalfYaw * cofHalfRoll + sinHalfPitch * sinHalfYaw * sinHalfRoll;
	quat->x = sinHalfPitch * cofHalfYaw * cofHalfRoll - cofHalfPitch * sinHalfYaw * sinHalfRoll;
	quat->y = cofHalfPitch * sinHalfYaw * cofHalfRoll + sinHalfPitch * cofHalfYaw * sinHalfRoll;
	quat->z = cofHalfPitch * cofHalfYaw * sinHalfRoll - sinHalfPitch * sinHalfYaw * cofHalfRoll;
}

