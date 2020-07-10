#ifndef APP_INCLUDE_RADAR_MAP_H
#define APP_INCLUDE_RADAR_MAP_H

#include "component.h"

PI_BEGIN_DECLS

PiComponent *PI_API app_ui_radar_map_new(PiTexture* shape, PiTexture* border);

void PI_API app_ui_radar_map_set_background_texture(PiComponent* component, PiTexture* bg_texture);

void PI_API app_ui_radar_map_init(PiComponent* component, uint width, uint height, PiTexture*bg_texture, PiTexture** npc_Texture, uint typeSize);

void PI_API app_ui_radar_map_set_pos(PiComponent* component, float x, float y);

void PI_API app_ui_radar_map_set_npc_data(PiComponent* component, float* position_array, uint8* indexArray, uint count);

void PI_API app_ui_radar_map_free(PiComponent* component);

void PI_API app_ui_radar_map_set_line_points(PiComponent*, float*, uint);

PI_END_DECLS

#endif