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

#include "pi_line.h"

void PI_API pi_line_set(PiLine *line, PiVector3 *pt, PiVector3 *dir)
{
	pi_vec3_copy(&line->origin, pt);
	pi_vec3_normalise(&line->dir, dir);
}

/***************************************
 
 line1上任一点P1，p1 + s * dir1
 line2上任一点P2，p2 + t * dir2
 
 P1P2 = -p1p2 + s * dir1 - t * dir2
 
 距离就是最近的P1P2，令其为0, -p1p2 + s * dir1 - t * dir2 = 0
 矩阵：M = {{dir.x, -dir2.x}, {dir.y, -dir2.y}, {dir1.z, -dir2.z}}

 M * {s, t} = p2 - p1

 二个未知数，三个方程，可能得矛盾解；用最小二乘法，两边乘以M的转置M'

 M' * M * {s, t} = M' * (p2 - p1);

 Mat = M' * M = { {dir1^2, -dir1 . dir2}, {-dir1 . dir2, dir2^2} }
 
 注：dir1，dir2为单位向量 
 Mat = { {1, -dir1 . dir2}, {-dir1 . dir2, 1} } 
 
 diff = p2 - p1;
 b1 = diff . dir1
 b2 = - diff * dir2
 a12 = -dir1 . dir2
 则：
    Mat = { {1, a12}, {a12, 1} }
	
	Mat * {s, t} = {b1, b2}

 由克莱姆法则：
	
	det = || Mat || = 1 - a12^2
	
	当det不为0时，有唯一解：
		Mat1 = { {b1, b2}, {a12, 1} }
		Mat2 = { {1, a12}, {b1, b2} }
		s = || Mat1 || / det;
		t = || Mat2 || / det;
 ****************************************/

float PI_API pi_line_dist_sq(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2)
{
	PiVector3 diff;
	float a12, b1, c, det;
	float b2, s, t, sqrDist;
	
	pi_vec3_sub(&diff, &line1->origin, &line2->origin);
	a12 = - pi_vec3_dot(&line1->dir, &line2->dir);
	b1 = pi_vec3_dot(&diff, &line1->dir);
	c = pi_vec3_len_square(&diff);

	det = pi_math_abs(1.0f - a12 * a12);
	
	if (det >= 0.0f) /* 两条线不平行 */
	{
		float invDet = 1.0f / det;
	
		b2 = - pi_vec3_dot(&diff, &line2->dir);
		s = (a12 * b2 - b1) * invDet;
		t = (a12 * b1 - b2) * invDet;
		sqrDist = c + 
				  s * (s + a12 * t + 2.0f * b1) +
				  t * (a12 * s + t + 2.0f * b2) + c;
	}
	else
	{/* 平行，选择任何的最近点对都可 */
		
		s = -b1;
		t = 0.0f;
		sqrDist = c + b1 * s;
	}
	
	if(pt1 != NULL)
	{
		pi_vec3_scale(pt1, &line1->dir, s);
		pi_vec3_add(pt1, pt1, &line1->origin);
	}
	
	if(pt2 != NULL)
	{
		pi_vec3_scale(pt2, &line2->dir, t);
		pi_vec3_add(pt2, pt2, &line2->origin);
	}
	
	// 数值计算的舍入误差
	if (sqrDist < 0.0f)
		sqrDist = 0.0f;
	return sqrDist;
}

float PI_API pi_line_dist(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2)
{
	float sq = pi_line_dist_sq(line1, line2, pt1, pt2);
	return pi_math_sqrt(sq);
}

