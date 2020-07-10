#ifndef INCLUDE_SWING_H
#define INCLUDE_SWING_H

#include <controller.h>
#include <texture.h>
#include <pi_sequence.h>

typedef enum
{
	ENUM_SWING_CUBE_MAP = 0,
	ENUM_SWING_PLANAR_MAP,
	ENUM_SWING_COUNT
} ENUM_SWING_MAP_MODE;

PI_BEGIN_DECLS

PiController *PI_API pi_swing_new();

void PI_API pi_swing_free(PiController *c);

void PI_API pi_swing_set_planar_map(PiController *c, PiTexture *planar_tex);

void PI_API pi_swing_set_cube_map(PiController *c, PiTexture *cube_tex);

void PI_API pi_swing_set_map_mode(PiController *c, ENUM_SWING_MAP_MODE mode);

void PI_API pi_swing_show(PiController *c);

void PI_API pi_swing_hide(PiController *c);

void PI_API pi_swing_set_speed(PiController *c, float speed);

void PI_API pi_swing_set_color_sequence(PiController *c, PiSequence *color_sequence);

void PI_API pi_swing_set_color_time(PiController *c, float time);

PI_END_DECLS

#endif /*INCLUDE_SWING_H*/
