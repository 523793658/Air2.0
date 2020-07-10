#ifndef INCLUDE_VEGETATION_ANIM_H
#define INCLUDE_VEGETATION_ANIM_H

#include <controller.h>
#include <entity.h>
#include <environment.h>
#include <pi_sphere.h>

/**
 * 植被动画控制器
 */

PI_BEGIN_DECLS

/**
 * 创建一个植被动画控制器
 * @return 控制器指针
 */
PiController* PI_API pi_vegetation_anim_new();

/**
 * 释放指定的植被动画控制器
 * @param c 需要释放的控制器指针
 */
void PI_API pi_vegetation_anim_free(PiController *c);

/**
 * 将控制器的参数初始化指定的Entity，初始化或者修改动画参数后需调用此方法才能生效
 * @param c 动画控制器
 * @param entity 需要应用参数的Entity
 */
void PI_API pi_vegetation_anim_apply_params(PiController *c, PiEntity* entity);

/**
 * 设置影响此控制器的环境参数
 * @param c 动画控制器
 * @param env 环境参数
 */
void PI_API pi_vegetation_anim_set_environment(PiController *c, PiEnvironment* env);

/**
 * 设置影响此控制器的主干动画参数,更改后需要使用pi_vegetation_anim_apply_params使之生效
 * @param c 动画控制器
 * @param trunk_flexibility 主干强度，默认为0.1
 * @param wind_scale 风力缩放，默认为1
 * @param wind_frequency 受风力影响时的振动频率，默认为1
 */
void PI_API pi_vegetation_anim_set_trunk_param(PiController *c, float trunk_flexibility, float wind_scale, float wind_frequency);

/**
 * 是否启用叶子/枝干动画,更改后需要使用pi_vegetation_anim_apply_params使之生效
 * @param c 动画控制器
 * @param is_enable 是否启用，默认为FALSE
 */
void PI_API pi_vegetation_anim_set_leaf_anim_enable(PiController *c, PiBool is_enable);

/**
 * 设置影响此控制器的叶子/枝干动画参数,只有在开启叶子动画时有效,更改后需要使用pi_vegetation_anim_apply_params使之生效
 * @param c 动画控制器
 * @param leaf_amplitude 叶子振幅，默认为1
 * @param leaf_frequency 叶子振动频率，默认为1
 * @param branch_amplitude 枝干振幅，默认为1
 * @param branch_frequency 枝干振动频率，默认为1
 */
void PI_API pi_vegetation_anim_set_leaf_param(PiController *c, float leaf_amplitude, float leaf_frequency, float branch_amplitude, float branch_frequency);

/**
 * 设置影响此控制器的叶子/枝干动画衰减控制,只有在开启叶子动画时有效,更改后需要使用pi_vegetation_anim_apply_params使之生效
 * @param c 动画控制器
 * @param vertex_color 是否使用顶点色控制，启用时后2个参数设置将强制为NULL,默认为false
 * @param leaf_tex 叶子衰减控制图，NULL为使用自动计算结果控制，默认为NULL
 * @param branch_tex 枝干衰减控制图，NULL为使用自动计算结果控制， 默认为NULL
 */
void PI_API pi_vegetation_anim_set_leaf_attenuation(PiController *c, PiBool vertex_color, PiTexture* leaf_tex, PiTexture* branch_tex);

/**
 * 设置植被动画的独立相位，消除同参数下的植物动画的完全同步
 * @param c 动画控制器
 * @param phase 动画播放的相位，默认为0
 */
void PI_API pi_vegetation_anim_set_individual_phase(PiController *c, float phase);

/**
 * 设置踩踏效果产生者，使植被产生被踩踏的倒伏效果
 * @param c 动画控制器
 * @param generator 效果产生者
 * @param fall_scale 踩踏影响参数
 */
void PI_API pi_vegetation_anim_set_fall_generator(PiController *c, PiSphere *generator, float fall_scale);


/*
	设置植被动画广联的spatial
*/
void PI_API pi_vegetation_anim_set_bind_spatial(PiController *c, PiSpatial* spatial);

PI_END_DECLS

#endif /* INCLUDE_VEGETATION_ANIM_H */