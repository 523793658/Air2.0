#ifndef INCLUDE_BILLBOARD_H
#define INCLUDE_BILLBOARD_H

#include <controller.h>
#include <camera.h>
#include <entity.h>

/**
 * ����������
 */

typedef enum
{
	EFT_CAMERA,
	EFT_CAMERA_Z_AXIS,
	EFT_FREE,
} EFacingType;

PI_BEGIN_DECLS

/**
 * ����
 */
PiController* PI_API pi_billboard_new();

/**
 * �ͷ�
 */
void PI_API pi_billboard_free(PiController *c);

/**
 * ����Ӱ�쳯������
 */
void PI_API pi_billboard_set_camera(PiController *c, PiCamera* camera);

/**
 * ����Ҫ���в���廯��Entity
 */
void PI_API pi_billboard_set_facing(PiController *c, EFacingType type);

PI_END_DECLS

#endif /* INCLUDE_BILLBOARD_H */