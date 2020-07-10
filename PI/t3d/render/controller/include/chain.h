#ifndef INCLUDE_CHAIN_H
#define INCLUDE_CHAIN_H

#include <controller.h>
#include <entity.h>
#include <pi_spatial.h>

/**
 * �������������
 */

typedef enum
{
	ECFT_HORIZONTAL,
	ECFT_VERTICAL,
	ECFT_CROSS,
	ECFT_FACING_CAMERA,
} EChainFacingType;

PI_BEGIN_DECLS

/**
 * ����
 */
PiController* PI_API pi_chain_new(EChainFacingType type);

/**
 * �ͷ�
 */
void PI_API pi_chain_free(PiController *c);

void PI_API pi_chain_set_step_points(PiController *c, PiVector* points, PiVector* offsets);

void PI_API pi_chain_reset(PiController *c, uint num_step, float step_time, float step_interval);

void PI_API pi_chain_set_width(PiController *c, float width);

PI_END_DECLS

#endif /* INCLUDE_CHAIN_H */