#ifndef INCLUDE_FOG_H
#define INCLUDE_FOG_H

#include <renderer.h>
#include "texture.h"
const static char *RS_FOG_VS = "default.vs";
const static char *RS_FOG_FS = "fog.fs";

/* 雾的模式 */
typedef enum
{
	FM_EXP,
	FM_EXP2,
	FM_LINEAR
} FogMode;

PI_BEGIN_DECLS


/**
* 创建雾效渲染器
* @returns 雾效渲染器指针
*/
PiRenderer *PI_API pi_fog_new_with_name(char* name);

/**
 * 创建雾效渲染器
 * @returns 雾效渲染器指针
 */
PiRenderer *PI_API pi_fog_new();

/**
 * 部署雾效渲染器
 * @param renderer 雾效渲染器指针
 * @param target_name 渲染目标的名称
 * @param view_cam_name 主渲染相机的名称
 * @param color_tex_name 离屏渲染目标的颜色纹理
 * @param depth_tex_name 离屏渲染目标的深度纹理
 */
void PI_API pi_fog_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name);

/************************************************************************/
/* 部署环境                                                                     */
/************************************************************************/
void PI_API pi_fog_deploy_env(PiRenderer* renderer, char *env_name);

void PI_API pi_fog_set_texture(PiRenderer* renderer, PiTexture* tex);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_fog_free(PiRenderer *renderer);

/**
 * 设置雾效的颜色
 * @param renderer 渲染器指针
 * @param r 雾效的颜色的r分量
 * @param g 雾效的颜色的g分量
 * @param b 雾效的颜色的b分量
 */
void PI_API pi_fog_set_color(PiRenderer *renderer, float r, float g, float b);

/**
 * 设置雾效的模式
 * @param renderer 渲染器指针
 * @param fog_mode 雾效的模式，默认FM_EXP
 */
void PI_API pi_fog_set_mode(PiRenderer *renderer, FogMode mode);

/**
 * 设置雾效的距离
 * @param renderer 渲染器指针
 * @param start 雾效的生效的开始距离
 * @param end 雾效的完全生效的结束距离
 */
void PI_API pi_fog_set_start_end(PiRenderer *renderer, float start, float end);

/**
 * 设置雾效的浓度
 * @param renderer 渲染器指针
 * @param density 雾效的浓度数值
 */
void PI_API pi_fog_set_density(PiRenderer *renderer, float density);

/**
 * 设置基于范围的雾化
 * @param renderer 渲染器指针
 * @param density 雾效的浓度数值
 */
void PI_API pi_fog_set_range_enable(PiRenderer *renderer, PiBool enable);


/**
* 设置雾流动
* @param renderer 渲染器指针
* @param density 是否流动
*/
void PI_API pi_fog_set_flow(PiRenderer *renderer, PiBool isFlow, float flow_speed, float scale_ui);

PI_END_DECLS

#endif /* INCLUDE_FOG_H */
