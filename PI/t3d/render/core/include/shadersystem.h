#ifndef INCLUDE_SHADER_SYSTEM_H
#define INCLUDE_SHADER_SYSTEM_H

/**
 * shaderϵͳ������shaderԤ����ͷ��룬ת������
 */

#include <pi_lib.h>
#include <shader.h>
#include <rendersystem.h>

PI_BEGIN_DECLS

void PI_API pi_shadersystem_init(PiRenderSystem *system);

void PI_API pi_shadersystem_clear(PiRenderSystem *system);

void PI_API pi_shadersystem_reset(PiRenderSystem *system);

/* ������ɫ����Դ������ */
PiBool PI_API pi_shadersystem_add_shader_source(PiRenderSystem *system, const wchar *key, byte *data, uint size);

/* ���Ԥ����shader*/
PiBool PI_API pi_shadersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type);

/* �Ƴ���ɫ����Դ������ */
void PI_API pi_shadersystem_remove_shader_source(PiRenderSystem *system, const wchar *key);

PI_END_DECLS

#endif	/* INCLUDE_SHADER_SYSTEM_H */