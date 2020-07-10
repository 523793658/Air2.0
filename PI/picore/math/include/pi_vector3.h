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

#ifndef __PI_VECTOR3_H__
#define __PI_VECTOR3_H__

#include <pi_lib.h>

/* 前置声明 */
struct PiMatrix4;
struct PiQuaternion;

/**
 * 本文件定义了3维向量及其相关的处理函数
 */

typedef struct PiVector3
{
	float x, y, z;
}PiVector3;

PI_BEGIN_DECLS

/* 赋值：dst={x,y,z} */
static void pi_vec3_set(PiVector3 *dst, float x, float y, float z);

/* 拷贝：dst = src */
static void pi_vec3_copy(PiVector3 *dst, const PiVector3 *src);

/* 返回两个向量是否相等 */
static PiBool pi_vec3_is_equal(const PiVector3 *src1, const PiVector3 *src2);

/* 向量相加 dst = src1 + src2 */
static void pi_vec3_add(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* 向量相减 dst = src1 - src2 */
static void pi_vec3_sub(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* 向量分量相乘 dst = src1 * src2 */
static void pi_vec3_mul(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* 向量分量相除 dst = src1 / src2 */
static void pi_vec3_div(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* 向量数乘 */
static void pi_vec3_scale(PiVector3 *dst, const PiVector3 *src, float scale);

/* 向量点乘 return dot(src1, src2) */
static float pi_vec3_dot(const PiVector3 *src1, const PiVector3 *src2);

/* 向量叉积 dst = Cross(src1, src2) */
static void pi_vec3_cross(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2);

/* 取得向量长度的平方 */
static float pi_vec3_len_square(const PiVector3 *src);

/* 取得向量长度 */
static float pi_vec3_len(const PiVector3 *src);

/* 取得两个向量距离的平方 */
static float pi_vec3_distance_square(const PiVector3 *src1, const PiVector3 *src2);

/* 取得两个向量距离 */
static float pi_vec3_distance(const PiVector3 *src1, const PiVector3 *src2);

/* 向量的线性插值 dst = src1 + frac * (src2 - src1) */
static void pi_vec3_lerp(PiVector3 *dst, const PiVector3 *src1, const PiVector3 *src2, float frac);

/* 取得0向量{0, 0, 0} */
const PiVector3* pi_vec3_get_zero(void);

/* 取得x轴单位向量 {1, 0, 0} */
const PiVector3* pi_vec3_get_xunit(void);

/* 取得y轴单位向量 {0, 1, 0} */
const PiVector3* pi_vec3_get_yunit(void);

/* 取得z轴单位向量  {0, 0, 1} */
const PiVector3* pi_vec3_get_zunit(void);

/* 取得缩放单位向量  {1, 1, 1} */
const PiVector3* pi_vec3_get_scale_unit(void);

/**
 * 单位化向量 dst = norm(src)
 * 如果src长度为0.0f，单位化失败，返回FALSE，此时dst被设为0向量
 */
PiBool pi_vec3_normalise(PiVector3 *dst, const PiVector3 *src);

/* 返回两个向量的夹角，单位：弧度 */
float pi_vec3_angle(const PiVector3 *src1, const PiVector3 *src2);

#include "vector3.inl"

PI_END_DECLS

#endif /* __PI_VECTOR3_H__ */
