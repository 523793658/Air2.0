#ifndef INCLUDE_SSAO_H
#define INCLUDE_SSAO_H

#include <renderer.h>

/**
 * SSAO(HDAO)渲染器
 */

const static char* RS_SSAO_VS = "default.vs";
const static char* RS_SSAO_FS = "ssao.fs";

PI_BEGIN_DECLS

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer* PI_API pi_ssao_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param depth_map_name 源场景深度纹理名
 * @param output_name 输出目标名，注意：当渲染器工作在independent_output模式下此目标由渲染器自己创建并输出同名纹理
 * @param scene_camera_name 场景相机名
 */
void PI_API pi_ssao_deploy(PiRenderer* renderer, char* depth_map_name, char* output_name, char* scene_camera_name);

/**
 * 部署渲染器法线功能，在有法线信息的情况下，法线也可以产生AO效果，但会带来额外的开销
 * @param renderer 渲染器指针
 * @param normal_map_name 场景法线纹理名
 */
void PI_API pi_ssao_deploy_normal(PiRenderer* renderer, char* normal_map_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_ssao_free(PiRenderer* renderer);

/**
 * 设置最大拒绝半径，影响AO产生范围
 * @param renderer 渲染器指针
 * @param radius 半径大小，默认为0.8
 */
void PI_API pi_ssao_set_reject_radius(PiRenderer* renderer, float radius);

/**
 * 设置最小接受半径，可以消除相对较平缓的面上产生不希望的AO
 * @param renderer 渲染器指针
 * @param radius 半径大小，默认为0.0003
 */
void PI_API pi_ssao_set_accept_radius(PiRenderer* renderer, float radius);

/**
 * 设置法线缩放，增强或减弱法线产生AO的能力，只有在启用法线的情况下起效
 * @param renderer 渲染器指针
 * @param scale 缩放值，默认为1
 */
void PI_API pi_ssao_set_normal_scale(PiRenderer* renderer, float scale);

/**
 * 设置AO强度
 * @param renderer 渲染器指针
 * @param intensity AO强度，默认为2
 */
void PI_API pi_ssao_set_intensity(PiRenderer* renderer, float intensity);

/**
 * 设置AO计算的采样半径缩放，一定程度上调整AO影响范围
 * 注意：过度调节此项可能会产生相当的瑕疵
 * @param renderer 渲染器指针
 * @param scale 采样半径缩放值，默认为1
 */
void PI_API pi_ssao_set_sample_radius_scale(PiRenderer* renderer, float scale);

/**
 * 设置AO品质，效果由低到高使用级别：1~4
 * @param renderer 渲染器指针
 * @param level 效果级别，默认为4
 */
void PI_API pi_ssao_set_quality(PiRenderer* renderer, uint level);

/**
 * 设置渲染器使用独立输出模式，开启后AO效果会被单独输出至渲染器自己创建的RT中并将此纹理使
 * 用部署时的output_name输出至流水线(Alpha8格式)，否则渲染器直接将AO结果使用ColorMult混合
 * 至output_name指定的RT中
 * 注意：此设置更改必须在流水线初始化之前有效
 * @param renderer 渲染器指针
 * @param is_enable 是否启用，默认为False
 */
void PI_API pi_ssao_set_independent_output(PiRenderer* renderer, PiBool is_enable);

/**
 * 设置渲染器使用半分辨率生成AO，可以在降低效果的同时极大地提升效率，只在启用独立输出模式时有效
 * 建议此模式下使用各向异性过滤对此AO纹理采样应用回场景画面
 * 纹理使用部署的output_name输出至流水线
 * 注意：此设置更改必须在流水线初始化之前有效
 * @param renderer 渲染器指针
 * @param is_enable 是否启用，默认为False
 */
void PI_API pi_ssao_set_half_resolution_mode(PiRenderer* renderer, PiBool is_enable);

PI_END_DECLS

#endif /* INCLUDE_SSAO_H */