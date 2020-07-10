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

#ifndef __PI_LINE_H__
#define __PI_LINE_H__

#include <pi_lib.h>
#include <pi_vector3.h>

/**
 * 本文件定义了3维线段及其相关的处理函数
 */

typedef struct PiLine
{
	PiVector3 origin;	/* 直线上的点 */
	PiVector3 dir;		/* 直线的方向，必须是单位向量 */
}PiLine;


PI_BEGIN_DECLS

/**
 * 设置直线，dir不一定要是单位向量
 */
void PI_API pi_line_set(PiLine *line, PiVector3 *pt, PiVector3 *dir);

/**
 * 返回两条直线的距离的平方
 * pt1, pt2 用于返回两直线中最近的两个点
 */
float PI_API pi_line_dist_sq(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2);

/**
 * 返回两条直线的距离
 * pt1, pt2 用于返回两直线中最近的两个点
 */
float PI_API pi_line_dist(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2);

PI_END_DECLS

#endif /* __PI_LINE_H__ */
