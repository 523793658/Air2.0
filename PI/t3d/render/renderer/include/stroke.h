#ifndef _INCLUDE_STROKE_H_
#define _INCLUDE_STROKE_H_

#include <renderer.h>
#include "texture.h"

const static char* RS_STROKE_VS = "default.vs";
const static char* RS_STROKE_FS = "stroke.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_stroke_new();

void PI_API pi_stroke_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_stroke_free(PiRenderer* renderer);

void PI_API pi_stroke_set_params(PiRenderer* renderer, uint width, float r, float g, float b);

void PI_API pi_stroke_set_alpha_map(PiRenderer* renderer, PiTexture* texture);

PI_END_DECLS

#endif