#ifndef INCLUDE_PRESHADOW_PCF_H
#define INCLUDE_PRESHADOW_PCF_H

#include <renderer.h>
#include <camera.h>
#include <texture.h>

/**
 * PCF ShadowMap渲染器
 */

/**
 * PCF 光滑边缘采样质量
 */
typedef enum
{
	PCF_SS_HARD = 0,
	PCF_SS_8X,
	PCF_SS_16X,
	PCF_SS_32X,
	PCF_SS_48X,
	PCF_SS_64X,
} PCFShadowSamples;

/**
 * Lighting 光照渲染器部署数据
 */
typedef struct PCFShadowPipelineData
{
	PiMatrix4 shadow_matrix;
	PiTexture *depth_stencil;
	PiTexture *shadow_map;
	PCFShadowSamples shadow_samples;
	float shadow_z_far;
	float sizeInv;
} PCFShadowPipelineData;

PI_BEGIN_DECLS

PiRenderer *PI_API pi_preshadow_pcf_new_with_name(char *name);

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer *PI_API pi_preshadow_pcf_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param shadow_data_name 由阴影渲染器输出的阴影数据名,交由光照渲染器阴影部署接口处理
 * @param view_cam_name 场景相机名
 * @param shadow_cam_name 阴影相机名
 * @param entity_list_name 阴影投射体物体列表,材质Shader必须为model_pl.vs/preshadow_pcf.fs
 * @param env_name 环境数据名
 */
void PI_API pi_preshadow_pcf_deploy(PiRenderer *renderer, char *shadow_data_name, char *view_cam_name, char *shadow_cam_name, char *entity_list_name, char *env_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_preshadow_pcf_free(PiRenderer *renderer);

/**
 * 更新获得和指定场景相机匹配的阴影相机
 * 注意: 此函数可由任意线程调用以更新相机
 * @param renderer 渲染器指针
 * @param view_cam 场景相机指针
 * @param shadow_cam 阴影相机指针
 */
void PI_API pi_preshadow_pcf_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *shadow_cam);

/**
 * 设置阴影图尺寸,越大阴影精细度越高
 * @param renderer 渲染器指针
 * @param size 阴影图尺寸, 默认1024
 */
void PI_API pi_preshadow_pcf_set_shadow_mapsize(PiRenderer *renderer, uint size);

/**
 * 设置最远可产生阴影的范围(世界空间单位)
 * @param renderer 渲染器指针
 * @param z_far 阴影距离
 */
void PI_API pi_preshadow_pcf_set_zfar(PiRenderer *renderer, float z_far);

/**
 * 设置PCF阴影质量
 * @param renderer 渲染器指针
 * @param samples 可选的采样质量,质量越低渲染效率越高
 */
void PI_API pi_preshadow_pcf_set_quality(PiRenderer *renderer, PCFShadowSamples samples);

/**
 * 设置PCF阴影过滤范围, 越大阴影的半影范围越大
 * @param renderer 渲染器指针
 * @param size 多采样过滤范围, 一般建议0~0.01, 默认0.0055
 */
void PI_API pi_preshadow_pcf_set_filter_size(PiRenderer *renderer, float size);

PI_END_DECLS

#endif /* INCLUDE_PRESHADOW_PCF_H */
