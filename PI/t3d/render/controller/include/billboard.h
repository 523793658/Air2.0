#ifndef INCLUDE_BILLBOARD_H
#define INCLUDE_BILLBOARD_H

#include <controller.h>
#include <camera.h>
#include <entity.h>

/**
 * 布告板控制器
 */

typedef enum
{
	EFT_CAMERA,
	EFT_CAMERA_Z_AXIS,
	EFT_FREE,
} EFacingType;

PI_BEGIN_DECLS

/**
 * 创建
 */
PiController* PI_API pi_billboard_new();

/**
 * 释放
 */
void PI_API pi_billboard_free(PiController *c);

/**
 * 设置影响朝向的相机
 */
void PI_API pi_billboard_set_camera(PiController *c, PiCamera* camera);

/**
 * 设置要进行布告板化的Entity
 */
void PI_API pi_billboard_set_facing(PiController *c, EFacingType type);

PI_END_DECLS

#endif /* INCLUDE_BILLBOARD_H */