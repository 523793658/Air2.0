#ifndef INCLUDE_SHADER_SYSTEM_H
#define INCLUDE_SHADER_SYSTEM_H

/**
 * shader系统，负责shader预处理和翻译，转换工作
 */

#include <pi_lib.h>
#include <shader.h>
#include <rendersystem.h>

PI_BEGIN_DECLS

void PI_API pi_shadersystem_init(PiRenderSystem *system);

void PI_API pi_shadersystem_clear(PiRenderSystem *system);

void PI_API pi_shadersystem_reset(PiRenderSystem *system);

/* 设置着色器的源码内容 */
PiBool PI_API pi_shadersystem_add_shader_source(PiRenderSystem *system, const wchar *key, byte *data, uint size);

/* 添加预编译shader*/
PiBool PI_API pi_shadersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type);

/* 移除着色器的源码内容 */
void PI_API pi_shadersystem_remove_shader_source(PiRenderSystem *system, const wchar *key);

PI_END_DECLS

#endif	/* INCLUDE_SHADER_SYSTEM_H */