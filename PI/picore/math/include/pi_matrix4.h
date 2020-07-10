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

#ifndef __PI_MATRIX4_H__
#define __PI_MATRIX4_H__

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_quaternion.h>

/**
 * 本文件定义了4*4的矩阵及其相关的处理函数
 * 注意：在这里是用列向量相乘的矩阵，
 * 矩阵m和向量v相乘：m * v，v是列向量:
 * 
 * [ m[0][0]  m[0][1]  m[0][2]  m[0][3] ]   {v[0]}
 * | m[1][0]  m[1][1]  m[1][2]  m[1][3] | * {v[1]}
 * | m[2][0]  m[2][1]  m[2][2]  m[2][3] |   {v[2]}
 * [ m[3][0]  m[3][1]  m[3][2]  m[3][3] ]   {1}
 *
 * 转OpenGL矩阵的时候，需要将m转置过来(OpenGL虽然也是和列向量相乘，不过OpenGL的矩阵也是列优先)
 * 转D3D矩阵的时候，也需要将m转置过来（D3D矩阵是行优先，不过是行向量和矩阵相乘）
 * 这里用的是右手坐标系，和OpenGL相同，和D3D相反
 *
 * 另，CVV用的是OpenGL的范围：-1 <= x <= 1, -1 <= y <= 1, -1 <= z <= 1
 */

/* 前置声明 */
struct PiVector3;
struct PiQuaternion;
				 
typedef struct PiMatrix4
{
	float m[4][4];
}PiMatrix4;

PI_BEGIN_DECLS

/* dst = src */
static void pi_mat4_copy(PiMatrix4 *dst, const PiMatrix4 *src2);

/* 判断相等 */
static PiBool pi_mat4_is_equal(const PiMatrix4 *src1, const PiMatrix4 *src2);

/* 矩阵加法 */
static void pi_mat4_add(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2);

/* 矩阵乘一个常数 */
static void pi_mat4_scale(PiMatrix4 *dst, const PiMatrix4 *src, float scale);

/* 转置矩阵：dst = Transpose(src) */
static void pi_mat4_transpose(PiMatrix4 *dst, const PiMatrix4 *src);

/* 矩阵作用于点：平移分量起作用 */
static void pi_mat4_apply_point(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat);

/* 矩阵作用于向量：平移分量不起作用 */
static void pi_mat4_apply_vector(PiVector3 *dst, const PiVector3 *src, const PiMatrix4 *mat);

/* 构造平移矩阵 */
static void pi_mat4_set_translate(PiMatrix4 *dst, const PiVector3 *translate);

/* 提取平移分量 */
static void pi_mat4_extract_translate( PiVector3 *dst, const PiMatrix4 *mat );

/* 把矩阵的平移缩放旋转分解出来 */
void pi_mat4_decompose(PiVector3 *pos, PiVector3 *scale, struct PiQuaternion *rotate, const PiMatrix4 *mat);

/* 构造缩放矩阵 */
static void pi_mat4_set_scale(PiMatrix4 *dst, const PiVector3 *scale);

/* 初始化dst为单位阵 */
void pi_mat4_set_identity(PiMatrix4 *dst);

/* 取单位矩阵 */
PiMatrix4* pi_mat4_get_identity(void);

/* 矩阵相乘，dst = src1 * src2 注意：这是在向量是咧向量的意义上的矩阵相乘 */
void pi_mat4_mul(PiMatrix4 *dst, const PiMatrix4 *src1, const PiMatrix4 *src2);

/* 逆矩阵：dst = Inverse(src) */
void pi_mat4_inverse(PiMatrix4 *dst, const PiMatrix4 *src);

/* 构造旋转矩阵 */
void pi_mat4_build_rotate(PiMatrix4 *dst, const struct PiQuaternion *rotate);

/**
 * 根据translate、rotate、scale构造出变换矩阵
 * 注意约定的顺序：先scale，后rotate，最后做translate
 */
void pi_mat4_build_transform(PiMatrix4 *dst, 
	const PiVector3 *translate, const PiVector3 *scale, const struct PiQuaternion *rotate);

/** 构造右手坐标系的视图矩阵 */
PiBool pi_mat4_lookat_rh(PiMatrix4 *dst, const PiVector3 *pos, const PiVector3 *target, const PiVector3 *up);

/** 
 * 构造右手系的透视投影矩阵
 * lt、rt、bm、tp为近裁剪面的矩阵的左、右、上、下边界的数值
 * 注意：由于透视矩阵的照相机位置不能位于裁剪体内部，所以zn和zf必须取正值，而且zn越少，近景的误差越大
 */
void pi_mat4_frustum_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf);

/** 
 * 构造一个右手系的透视投影矩阵
 * fovRad: yz方向的视野角度，单位弧度
 * zNear、zFar，近远裁剪面到相机的距离，均为正数，而且zNear不能等于zFar
 * ratio：宽高比 
 */
void pi_mat4_pers_fov_rh(PiMatrix4 *dst, float fovRad, float ratio, float zNear, float zFar);

/* 构造一个右手系的正交投影矩阵
 * lt到zf是相机空间中的长方体边界
 * zNear和zFar可正可负，但zNear和zfar不能相等 
 * zNear 表示z值的最小值
 * zFar 表示z值的最大值
 */
void pi_mat4_ortho_rh(PiMatrix4 *dst, float lt, float rt, float bm, float tp, float zn, float zf);

#include "matrix4.inl"

PI_END_DECLS

#endif /* __PI_MATRIX4_H__ */
