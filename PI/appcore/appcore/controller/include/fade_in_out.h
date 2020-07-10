#ifndef APP_FADE_IN_OUT_H
#define APP_FADE_IN_OUT_H
#include "app_controller.h"

typedef enum
{
	FD_IN,
	FD_OUT
}FadeType;

PiController* PI_API app_fade_in_out_new();

void PI_API app_fade_in_out_set_params(PiController* c, float time, FadeType type, PiBool iskeep, float keepProcess);

void PI_API app_fade_in_out_play(PiController*c);

void PI_API app_fade_in_out_free(PiController* c);

#endif