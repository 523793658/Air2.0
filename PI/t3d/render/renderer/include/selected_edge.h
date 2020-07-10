#ifndef INCLUDE_SELECTED_EDGE_H
#define INCLUDE_SELECTED_EDGE_H

#include "renderer.h"

static const char *RS_SELECTED_EDGE_VS = "default.vs";
static const char *RS_BLUR_SIMPLE_FS = "edge_depict.fs";

PI_BEGIN_DECLS

PiRenderer *PI_API pi_selected_edge_new_with_name(char* name);

PiRenderer *PI_API pi_selected_edge_new();

void PI_API pi_selected_edge_deploy(PiRenderer *renderer, char *view_cam_name, char *scene_depth_name, char *output_name);

void PI_API pi_selected_edge_free(PiRenderer *renderer);

void PI_API pi_selected_edge_set_entity(PiRenderer *renderer, PiVector *entityList);

void PI_API pi_selected_edge_set_color(PiRenderer *renderer, float r, float g, float b);

void PI_API pi_selected_edge_enable_depth(PiRenderer *renderer, PiBool enable);

PI_END_DECLS

#endif /* INCLUDE_SELECTED_EDGE_H */
