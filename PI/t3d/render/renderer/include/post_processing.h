#ifndef INCLUDE_POST_PROCESSING_H
#define INCLUDE_POST_PROCESSING_H
#include "texture.h"
#include <renderer.h>


/* 雾的模式 */
typedef enum
{
	FM_NONE = 0,
	FM_EXP,
	FM_EXP2,
	FM_LINEAR
} FogMode;

PI_BEGIN_DECLS

/**
 * 创建后处理效果渲染器
 * @returns 后处理效果渲染器
 */
PiRenderer *PI_API pi_post_processing_new();

/**
 * 部署后处理效果渲染器
 * @param renderer 后处理效果渲染器指针
 * @param scene_color_name 待处理纹理的名字，比如上一个渲染器的rt的color纹理
 * @param target_name 输出rt的名字
 */
void PI_API pi_post_processing_deploy(PiRenderer *renderer, char *scene_color_name, char *scene_depth_name, char *target_name, char* view_camera_name);

/**
 * 释放后处理效果渲染器
 * @param renderer 后处理效果渲染器指针
 */
void PI_API pi_post_processing_free(PiRenderer *renderer);


/**
* 设置雾效的颜色
* @param renderer 渲染器指针
* @param r 雾效的颜色的r分量
* @param g 雾效的颜色的g分量
* @param b 雾效的颜色的b分量
*/
void PI_API pi_post_processing_set_fog_color(PiRenderer *renderer, float r, float g, float b);

/**
* 设置雾效的模式
* @param renderer 渲染器指针
* @param fog_mode 雾效的模式，默认FM_EXP
*/
void PI_API pi_post_processing_set_fog_mode(PiRenderer *renderer, FogMode mode);

/**
* 设置雾效的距离
* @param renderer 渲染器指针
* @param start 雾效的生效的开始距离
* @param end 雾效的完全生效的结束距离
*/
void PI_API pi_post_processing_set_fog_start_end(PiRenderer *renderer, float start, float end);

/**
* 设置雾效的浓度
* @param renderer 渲染器指针
* @param density 雾效的浓度数值
*/
void PI_API pi_post_processing_set_fog_density(PiRenderer *renderer, float density);

/**
* 设置基于范围的雾化
* @param renderer 渲染器指针
* @param density 雾效的浓度数值
*/
void PI_API pi_post_processing_set_fog_range_enable(PiRenderer *renderer, PiBool enable);

/**
* 设置雾流动
* @param renderer 渲染器指针
* @param density 是否流动
*/
void PI_API pi_post_processing_set_fog_flow(PiRenderer *renderer, PiBool isFlow, float flow_speed, float scale_ui);

/*
	设置抗锯齿开关
*/
void PI_API pi_post_processing_set_fxaa_enable(PiRenderer* renderer, PiBool enable);

void PI_API pi_post_processing_set_color_grading_enable(PiRenderer* renderer, PiBool enable);

void PI_API pi_post_processing_set_color_grading_clut(PiRenderer* renderer, PiTexture* texture);

void PI_API pi_post_processing_set_bloom_params(PiRenderer* renderer, float threshold, float scale);

void PI_API pi_post_processing_set_bloom_enable(PiRenderer* renderer, PiBool enable);

PI_END_DECLS

#endif /* INCLUDE_POST_PROCESSING_H */
