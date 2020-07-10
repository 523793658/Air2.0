#ifndef INCLUDE_D3D9_SHADER_H
#define INCLUDE_D3D9_SHADER_H

#include "shader.h"
#include <d3d9.h>

typedef struct
{
	sint vs_register_index;
	sint fs_register_index;

	uint vs_register_count;
	uint fs_register_count;

	RegisterSet vs_register_set;
	RegisterSet fs_register_set;

	uint vs_columns;
	uint fs_columns;

	PiBool vs_register;
	PiBool ps_register;
} D3D9Uniform;

typedef struct
{
	union
	{
		IDirect3DVertexShader9 *vertex_shader;
		IDirect3DPixelShader9 *pixel_shader;
	} handle;
	PiDvector uniforms;				/* 元素为Uniform */
} D3D9Shader;

typedef struct
{
	D3D9Shader *shaders[ST_NUM];
	PiDvector uniforms;				/* 元素为Uniform */
	PiDvector global_uniforms;		/* 元素为Uniform */
} D3D9Program;

#endif /* INCLUDE_D3D9_SHADER_H */
