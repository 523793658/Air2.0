#ifndef INCLUDE_UI_RENDERER_H
#define INCLUDE_UI_RENDERER_H

#include <renderer.h>

/**
 * 此文件定义了绘制UI的渲染器
 */

PI_BEGIN_DECLS

/**
 * 创建UI渲染器
 * @return 渲染器指针
 */
PiRenderer *PI_API pi_ui_renderer_new();

/**
 * 部署UI渲染器
 * @param renderer 渲染器指针
 * @param target_name 渲染目标名
 * @param component_list_name 组件列表名,注意:列表内必须是PiComponent,且其父组件必须为Null否则将直接被忽略
 */
void PI_API pi_ui_renderer_deploy(PiRenderer *renderer, char *target_name, char *component_list_name);

/**
 * 释放UI渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_ui_renderer_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_UI_RENDERER_H */