#ifndef INCLUDE_TAIL_H
#define INCLUDE_TAIL_H

#include <controller.h>
#include <entity.h>
#include <pi_spatial.h>

/**
 * 拖尾网格控制器
 */

typedef enum
{
	ETT_LINE,
	ETT_RIBBON,
	ETT_RIBBON_FACING_CAMERA
} ETailType;

PI_BEGIN_DECLS

/**
 * 创建
 */
PiController* PI_API pi_tail_new(ETailType type, uint sample_step, float width, float life_time, PiBool head_follow);

/**
 * 释放
 */
void PI_API pi_tail_free(PiController *c);

void PI_API pi_tail_set_spatial(PiController *c, PiSpatial* spatial);

void PI_API pi_tail_reset(PiController *c);

PI_END_DECLS

#endif /* INCLUDE_TAIL_H */