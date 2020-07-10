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
 * ���ļ�������3ά�߶μ�����صĴ�����
 */

typedef struct PiLine
{
	PiVector3 origin;	/* ֱ���ϵĵ� */
	PiVector3 dir;		/* ֱ�ߵķ��򣬱����ǵ�λ���� */
}PiLine;


PI_BEGIN_DECLS

/**
 * ����ֱ�ߣ�dir��һ��Ҫ�ǵ�λ����
 */
void PI_API pi_line_set(PiLine *line, PiVector3 *pt, PiVector3 *dir);

/**
 * ��������ֱ�ߵľ����ƽ��
 * pt1, pt2 ���ڷ�����ֱ���������������
 */
float PI_API pi_line_dist_sq(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2);

/**
 * ��������ֱ�ߵľ���
 * pt1, pt2 ���ڷ�����ֱ���������������
 */
float PI_API pi_line_dist(PiLine *line1, PiLine *line2, PiVector3 *pt1, PiVector3 *pt2);

PI_END_DECLS

#endif /* __PI_LINE_H__ */
