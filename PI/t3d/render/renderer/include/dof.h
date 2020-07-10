#ifndef INCLUDE_DOF_H
#define INCLUDE_DOF_H

#include <renderer.h>

const static char *RS_DOF_VS = "default.vs";
const static char *RS_DOF_FS = "dof.fs";

PI_BEGIN_DECLS


/**
* 创建渲染器
* @returns 渲染器指针
*/
PiRenderer *PI_API pi_dof_new_with_name(char* name);

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer *PI_API pi_dof_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param target_name 输出变量名 渲染目标
 * @param view_cam_name 用于景深的物体列表名
 * @param color_tex_name 离屏渲染目标的颜色纹理
 * @param depth_tex_name 离屏渲染目标的深度纹理
 */
void PI_API pi_dof_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_dof_free(PiRenderer *renderer);

/**
* 设置焦距
* @param renderer 渲染器指针
* @param depth_tex_name 焦点距离
*/
void PI_API pi_dof_set_focal_distance(PiRenderer *renderer, float distance);



void PI_API pi_dof_set_focal_region(PiRenderer *renderer, float region);

void PI_API pi_dof_set_focal_range(PiRenderer *renderer, float region);

void PI_API pi_dof_set_focal_scale(PiRenderer *renderer, float region);

void PI_API pi_dof_set_point(PiRenderer* renderer, float x, float y);


PI_END_DECLS

#endif /* INCLUDE_DOF_H */
