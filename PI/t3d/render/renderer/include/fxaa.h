#ifndef INCLUDE_FXAA_H
#define INCLUDE_FXAA_H

#include <renderer.h>

const static char* RS_FXAA_VS = "default.vs";
const static char* RS_FXAA_FS = "fxaa.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_fxaa_new_with_name(char* name);

PiRenderer* PI_API pi_fxaa_new();

void PI_API pi_fxaa_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_fxaa_free(PiRenderer* renderer);

void PI_API pi_fxaa_set_preset(PiRenderer* renderer, uint n);

PI_END_DECLS

#endif /* INCLUDE_FXAA_H */