#ifndef INCLUDE_VANIASH_H
#define INCLUDE_VANIASH_H

#include <controller.h>

PI_BEGIN_DECLS

/**
 * 创建
 */
PiController *PI_API pi_vanish_new();

/**
 * 设置相关参数
 * @param vanish_time 消散的时间
 * @param vanish_gap 消散边缘缝隙，也就是边缘区域alpha的间隔，间隔内的区域为消散边缘
 */
void PI_API pi_vanish_set_parameter(PiController *c, float vanish_time, float vanish_gap, PiBool end_visible);
/*
*重置效果
*/
void PI_API pi_vanish_reset(PiController *c, void *obj);
/*
 * 释放
 */
void PI_API pi_vanish_free(PiController *c);

PI_END_DECLS

#endif /* INCLUDE_VANIASH_H */
