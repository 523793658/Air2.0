#ifndef INCLUDE_PRE_LIGHTING_H
#define INCLUDE_PRE_LIGHTING_H

#include <renderer.h>

/**
 * prelighting渲染器
 */
const static char *RS_PRE_LIGHTING_VS = "default.vs";
const static char *RS_PRE_LIGHTING_FS = "prelighting.fs";

const static char *RS_LIGHT_VS = "light.vs";
const static char *RS_POINT_LIGHT_FS = "point_light.fs";
const static char *RS_SPOT_LIGHT_FS = "spot_light.fs";

PI_BEGIN_DECLS

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer *PI_API pi_prelighting_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param target_name 渲染目标名
 * @param view_cam_name 场景相机名
 * @param point_light_list_name 点光源列表名
 * @param spot_light_list_name 聚光灯列表名
 * @param lighting_diffuse_name lighting-buffer的diffuse名
 * @param lighting_sepcular_name lighting-buffer的specular名
 * @param g_buffer_tex0_name g-buffer的color0纹理名
 * @param g_buffer_depth_name g-buffer的depth纹理名
 */
void PI_API pi_prelighting_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *point_light_list_name, char *spot_light_list_name,
                                   char *lighting_diffuse_name, char *lighting_sepcular_name, char *g_buffer_tex0_name, char *g_buffer_depth_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_prelighting_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_PRE_LIGHTING_H */