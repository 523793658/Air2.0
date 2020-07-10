#ifndef INCLUDE_picture_switch_H
#define INCLUDE_picture_switch_H

#include <renderer.h>
#include <renderstate.h>

typedef enum
{
	PSS_BEFORE = 0,
	PSS_DURING,
	PSS_AFTER
}PictureSwitchState;

PI_BEGIN_DECLS

PiRenderer *PI_API pi_picture_switch_new_with_name(char* name);

PiRenderer *PI_API pi_picture_switch_new();

void PI_API pi_picture_switch_deploy(PiRenderer *renderer, char *src_name, char *dst_name, char* target_name, char* fs_shader_name);

void PI_API pi_picture_switch_free(PiRenderer *renderer);

void PI_API pi_picture_switch_set_filter(PiRenderer *renderer, TexFilterOp filter);

void PI_API pi_picture_switch_change_state(PiRenderer *renderer, PictureSwitchState state);

void PI_API pi_picture_switch_set_duration(PiRenderer *renderer, float duration);

void PI_API pi_picture_switch_set_params_texture_name(PiRenderer* renderer, char* name);

void PI_API pi_picture_switch_set_params(PiRenderer* renderer, float* data);

PI_END_DECLS

#endif /* INCLUDE_picture_switch_H */