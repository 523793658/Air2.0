#ifndef INCLUDE_FBO_COPY_H
#define INCLUDE_FBO_COPY_H

#include <renderer.h>
#include <renderstate.h>

const static char *RS_FBO_COPY_VS = "default.vs";
const static char *RS_FBO_COPY_FS = "fbo_copy.fs";

typedef enum
{
	FCBM_NONE,
	FTBM_COLOR_MULT,
	FCBM_ALPHA,
	FCBM_ALPHA_MULT,
	FCBM_DEPTH,
} FBOCopyBlendMode;

PI_BEGIN_DECLS

PiRenderer *PI_API pi_fbo_copy_new_with_name(char* name);

PiRenderer *PI_API pi_fbo_copy_new();

void PI_API pi_fbo_copy_deploy(PiRenderer *renderer, char *src_name, char *dst_name);

void PI_API pi_fbo_copy_light_map(PiRenderer* renderer, PiBool is_lightmap);

void PI_API pi_fbo_copy_free(PiRenderer *renderer);

void PI_API pi_fbo_copy_set_blend_mode(PiRenderer *renderer, FBOCopyBlendMode blend);

void PI_API pi_fbo_copy_set_filter(PiRenderer *renderer, TexFilterOp filter);

void PI_API pi_fob_copy_set_alpha_map(PiRenderer *renderer, PiTexture* texture);

PI_END_DECLS

#endif /* INCLUDE_FBO_COPY_H */