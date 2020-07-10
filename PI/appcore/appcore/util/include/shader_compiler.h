#ifndef _Shader_compiler_H_
#define _Shader_compiler_H_
#include "pi_lib.h"
#include "material.h"
typedef struct
{
	PiBool vsShader;
	PiBool fsShader;

	char* vsShaderKey;
	char* fsShaderKey;

	uint8_t* vsBuffer;
	uint8_t* fsBuffer;

	size_t vsBufferSize;
	size_t fsBufferSize;

	PiMaterial *material;
}CompileResult;


PI_BEGIN_DECLS

CompileResult* PI_API app_compiler_init(PiMaterial* material);
void PI_API app_compiler_set_params(CompileResult* result, PiBool createVs, PiBool createFs);
void PI_API app_compiler_start(CompileResult* result);

PiBool PI_API app_compiler_is_vs_enable(CompileResult* result);
char* PI_API app_compiler_get_vs_key(CompileResult* result);

uint PI_API app_compiler_get_vs_buffer_size(CompileResult* result);

void* PI_API app_compiler_get_vs_buffer_pointer(CompileResult* result);

PiBool PI_API app_compiler_is_fs_enable(CompileResult* result);

char* PI_API app_compiler_get_fs_key(CompileResult* result);

uint PI_API app_compiler_get_fs_buffer_size(CompileResult* result);
void* PI_API app_compiler_get_fs_buffer_pointer(CompileResult* result);

void PI_API app_compiler_free_result(CompileResult* result);

void PI_API app_compiler_get_all_shader(wchar* output);

void PI_API app_compile_shader(char* cfgPath, wchar* outputPath);

void PI_API app_upload_shader_info();

void PI_API app_shader_save(char* ftp_url, char* ftp_user_name, char* ftp_password, char* shader_version);

PI_END_DECLS
#endif