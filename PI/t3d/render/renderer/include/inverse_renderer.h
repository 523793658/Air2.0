#ifndef INCLUDE_INVERSE_RENDERER_H
#define INCLUDE_INVERSE_RENDERER_H

#include <renderer.h>
#include <renderstate.h>

const static char *RS_INVERSE_VS = "default.vs";
const static char *RS_INVERSE_PS = "inverse.fs";

PI_BEGIN_DECLS

PiRenderer *PI_API pi_inverse_new();

void PI_API pi_inverse_deploy(PiRenderer *renderer, char *src_name, char *dst_name);

void PI_API pi_inverse_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_FBO_COPY_H */