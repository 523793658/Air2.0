/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __PI_QUATERNION_H__
#define __PI_QUATERNION_H__

/**
 * 本文件定义了四元数及其相关的处理函数
 */

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_matrix4.h>

/* 前置声明 */
struct PiVector3;
struct PiMatrix4;

typedef struct PiQuaternion
{
	float w, x, y, z;
}PiQuaternion;

PI_BEGIN_DECLS

/* dst = {w, x, y, z} */
static void pi_quat_set(PiQuaternion *dst, float w, float x, float y, float z);

/* dst = src */
static void pi_quat_copy(PiQuaternion *dst, const PiQuaternion *src);

/* 返回TRUE表示src1和src2相等 */
static PiBool pi_quat_is_equal(const PiQuaternion *src1, const PiQuaternion *src2);

/* 四元数相加：dst = src1 + src2 */
static void pi_quat_add(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

/* 四元数相减：dst = src1 - src2 */
static void pi_quat_sub(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

static void pi_quat_scale(PiQuaternion *dst, const PiQuaternion *src, float scale);

/* 四元数的点积 */
static float pi_quat_dot(const PiQuaternion *src1, const PiQuaternion *src2);

/* 四元数模的平方 */
static float pi_quat_len_square(const PiQuaternion *src);

/* 求共轭四元数 dst = Conj(src) */
static void pi_quat_conjugate(PiQuaternion *dst, const PiQuaternion *src);

/* 四元数的模 */
static float pi_quat_len(const PiQuaternion *src);

/* 返回单位四元数的指针 */
PiQuaternion* pi_quat_get_unit(void);

/* 四元数相乘：dst = src1 * src2 */
void pi_quat_mul(PiQuaternion *dst, const PiQuaternion *src1, const PiQuaternion *src2);

/* 四元数的单位化 */
PiBool pi_quat_normalise(PiQuaternion *dst, const PiQuaternion *src);

/* 四元数的逆 */
PiBool pi_quat_inverse(PiQuaternion *dst, const PiQuaternion *src);

/* 从旋转矩阵转换为四元数 */
void pi_quat_from_mat4(PiQuaternion *dst, const struct PiMatrix4 *src);

/**
 * 从轴和弧度角构建四元数
 * 注意：axis必须是单位向量
 */
void pi_quat_from_angle_axis(PiQuaternion *dst, const PiVector3 *axis, float radAngle);

/* 从四元数变成轴和弧度角 */
void pi_quat_to_angle_axis(const PiQuaternion *src, PiVector3 *axis, float *radAngle);

/* 四元数旋转向量 */
void pi_quat_rotate_vec3(PiVector3 *dst, const PiVector3 *src, const PiQuaternion *rotate);

/* 四元数球面线性插值：isShortest表示：是否选择最短弧 */
void pi_quat_slerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest);

/* 四元数线性插值：isShortest表示：是否选择最短弧 */
void pi_quat_lerp(PiQuaternion *dst, const PiQuaternion *from, const PiQuaternion *to, float frac, PiBool isShortest);

/* 构造出src1转到src2的四元数，dst = Rotate(from, to) */
void pi_quat_rotate_to(PiQuaternion *dst, const PiVector3 *from, const PiVector3 *to);

/* 得到局部的x轴 */
void pi_quat_get_xaxis(PiQuaternion *q, PiVector3 *dst);

/* 得到局部的y轴 */
void pi_quat_get_yaxis(PiQuaternion *q, PiVector3 *dst);

/* 得到局部的z轴 */
void pi_quat_get_zaxis(PiQuaternion *q, PiVector3 *dst);

/* 得到绕z轴转动的弧度 */
float pi_quat_get_roll_rad(PiQuaternion *q);

/* 得到绕x轴转动的弧度 */
float pi_quate_get_pitch_rad(PiQuaternion *q);

/* 得到绕y轴转动的弧度 */
float pi_quat_get_yaw_rad(PiQuaternion *q);

/* 从 x-y-z 轴得到四元数：scene 模块的 set_direction 要用 */
void pi_quat_from_axes(PiQuaternion* quat, PiVector3* xaxis, const PiVector3* yaxis, const PiVector3* zaxis);

void pi_quat_from_euler_angle(PiQuaternion* quat, float pitch, float yaw, float roll);

#include "quaternion.inl"

PI_END_DECLS

#endif /* __PI_QUATERNION_H__ */
