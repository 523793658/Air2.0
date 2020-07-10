#ifndef INCLUDE_SHINY_H
#define INCLUDE_SHINY_H

#include <controller.h>
#include <pi_sequence.h>

PI_BEGIN_DECLS

PiController *PI_API pi_shiny_new();

void PI_API pi_shiny_free(PiController *c);
/*
 *the range of lightwidth is [0,1]
 */
void PI_API pi_shiny_set_width(PiController *c, float width);

void PI_API pi_shiny_show(PiController *c);

void PI_API pi_shiny_hide(PiController *c);

void PI_API pi_shiny_set_speed(PiController *c, float speed);

void PI_API pi_shiny_set_color_sequence(PiController *c, PiSequence *color_sequence);

void PI_API pi_shiny_set_color_time(PiController *c, float time);

PI_END_DECLS

#endif /*INCLUDE_SHINY_H*/
