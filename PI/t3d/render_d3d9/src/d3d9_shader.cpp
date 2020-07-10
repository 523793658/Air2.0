#include "d3d9_shader.h"
#include "d3d9_rendersystem.h"
#include "renderinfo.h"
#include <d3dcompiler.h>
#include <d3dx9shader.h>

/* 找到默认类型对应的枚举 */
static DefaultUniformType _get_default_type(const char *name)
{
	if (pi_str_compare(name, G_WORLD_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_WORLD_MATRIX;
	}
	else if (pi_str_compare(name, G_PROJ_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_PROJ_MATRIX;
	}
	else if (pi_str_compare(name, G_WORLD_VIEW_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_WORLD_VIEW_MATRIX;
	}
	else if (pi_str_compare(name, G_VIEWPROJ_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_VIEW_PROJ_MATRIX;
	}
	else if (pi_str_compare(name, G_WORLD_VIEW_PROJ_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_WORLD_VIEW_PROJ_MATRIX;
	}
	else if (pi_str_compare(name, G_NORMAL_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_NORMAL_MATRIX;
	}
	else if (pi_str_compare(name, G_VIEWPORT_SIZE) == PI_COMP_EQUAL)
	{
		return DUT_VIEWPORT_SIZE;
	}
	else if (pi_str_compare(name, G_ALPHACULLOFF) == PI_COMP_EQUAL)
	{
		return DUT_ALPHA_CULLOFF;
	}
	else if (pi_str_compare(name, G_VIEW_NORMAL_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_VIEW_NORMAL_MATRIX;
	}
	else if (pi_str_compare(name, G_VIEW_MATRIX) == PI_COMP_EQUAL)
	{
		return DUT_VIEW_MATRIX;
	}
	else if (pi_str_compare(name, G_TIME) == PI_COMP_EQUAL)
	{
		return DUT_TIME;
	}
	else if (pi_str_compare(name, G_ShadowData) == PI_COMP_EQUAL)
	{
		return DUT_SHADOW_DATA;
	}
	else if (pi_str_compare(name, G_ShadowMap) == PI_COMP_EQUAL)
	{
		return DUT_SHADOW_MAP;
	}
	else if (pi_str_compare(name, G_EnvironmentData) == PI_COMP_EQUAL)
	{
		return DUT_ENVIRONMENT;
	}
	else if (pi_str_compare(name, G_ViewPosition) == PI_COMP_EQUAL)
	{
		return DUT_VIEW_POSITION;
	}
	else
	{
		return DUT_NONE;
	}
}

static UniformType _get_uniform_type(D3DXCONSTANT_DESC *const_desc, RegisterSet* register_set)
{
	UniformType type = UT_UNKOWN;
	switch (const_desc->Class)
	{
	case D3DXPC_MATRIX_COLUMNS:
		if (const_desc->Rows == 4 && const_desc->Columns == 4)
		{
			type = UT_MATRIX4;
			*register_set = RS_FLOAT;
		}
		else if (const_desc->Rows == 4 && const_desc->Columns == 3)
		{
			type = UT_MATRIX4x3;
			*register_set = RS_FLOAT;
		}
		else if (const_desc->Rows == 3 && const_desc->Columns == 3)
		{
			type = UT_MATRIX3;
			*register_set = RS_FLOAT;
		}
		else if (const_desc->Rows == 2 && const_desc->Columns == 2)
		{
			type = UT_MATRIX2;
			*register_set = RS_FLOAT;
		}
		break;
	case D3DXPC_SCALAR:
		if (const_desc->RegisterSet == D3DXRS_FLOAT4 && const_desc->Type == D3DXPT_FLOAT && const_desc->Columns == 1)
		{
			type = UT_FLOAT;
			*register_set = RS_FLOAT;
		}
		if (const_desc->RegisterSet == D3DXRS_INT4 && const_desc->Type == D3DXPT_INT && const_desc->Columns == 1)
		{
			type = UT_INT;
			*register_set = RS_INT;
		}
		break;
	case D3DXPC_VECTOR:
		if (const_desc->Type == D3DXPT_FLOAT)
		{
			if (const_desc->RegisterSet == D3DXRS_FLOAT4)
			{
				*register_set = RS_FLOAT;
			}
			else
			{
				*register_set = RS_INT;
			}
			if (const_desc->Columns == 4)
			{
				type = UT_VEC4;
			}
			else if (const_desc->Columns == 3)
			{
				type = UT_VEC3;
			}
			else if (const_desc->Columns == 2)
			{
				type = UT_VEC2;
			}
		}
		if (const_desc->Type == D3DXPT_INT)
		{
			if (const_desc->RegisterSet == D3DXRS_INT4)
			{
				*register_set = RS_INT;
			}
			else
			{
				*register_set = RS_FLOAT;
			}
			if (const_desc->Columns == 4)
			{
				type = UT_IVEC4;
			}
			else if (const_desc->Columns == 3)
			{
				type = UT_IVEC3;
			}
			else if (const_desc->Columns == 2)
			{
				type = UT_IVEC2;
			}
		}
		break;
	case D3DXPC_OBJECT:
		if (const_desc->RegisterSet == D3DXRS_SAMPLER)
		{
			if (const_desc->Type == D3DXPT_SAMPLER1D)
			{
				type = UT_SAMPLER_1D;
			}
			else if (const_desc->Type == D3DXPT_SAMPLER2D)
			{
				type = UT_SAMPLER_2D;
			}
			else if (const_desc->Type == D3DXPT_SAMPLER3D)
			{
				type = UT_SAMPLER_3D;
			}
			else if (const_desc->Type == D3DXPT_SAMPLERCUBE)
			{
				type = UT_SAMPLER_CUBE;
			}
		}
		break;
	case D3DXPC_STRUCT:
		if (const_desc->RegisterSet == D3DXRS_FLOAT4)
		{
			type = UT_STRUCT;
			*register_set = RS_FLOAT;
		}
		if (const_desc->RegisterSet == D3DXRS_INT4)
		{
			type = UT_ISTRUCT;
			*register_set = RS_INT;
		}
		break;
	default:
		PI_ASSERT(FALSE, "constant class isn't support: %d", const_desc->Class);
		break;
	}
	return type;
}

extern "C" {

	extern PiRenderSystem *g_rsystem;

	PiBool PI_API render_shader_init(Shader *shader)
	{
		D3D9Shader *shader_impl = pi_new0(D3D9Shader, 1);

		switch (shader->type)
		{
		case ST_VS:
			pi_renderinfo_add_vs_num(1);
			break;
		case ST_PS:
			pi_renderinfo_add_fs_num(1);
			break;
		}
		shader->impl = shader_impl;

		pi_dvector_init(&shader_impl->uniforms, sizeof(Uniform));

		return TRUE;
	}

	PiBool PI_API render_shader_clear(Shader *shader)
	{
		D3D9Shader *shader_impl = (D3D9Shader *)shader->impl;

		if (shader_impl != NULL)
		{
			uint len = pi_dvector_size(&shader_impl->uniforms);
			for (uint i = 0; i < len; ++i)
			{
				Uniform *u = (Uniform *)pi_dvector_get(&shader_impl->uniforms, i);
				pi_free(u->impl);
			}
			pi_dvector_clear(&shader_impl->uniforms, TRUE);

			switch (shader->type)
			{
			case ST_VS:
				pi_renderinfo_add_vs_num(-1);
				IDirect3DVertexShader9_Release(shader_impl->handle.vertex_shader);
				break;
			case ST_PS:
				pi_renderinfo_add_fs_num(-1);
				IDirect3DPixelShader9_Release(shader_impl->handle.pixel_shader);
				break;
			}

			pi_free(shader_impl);
		}
		return TRUE;
	}

	PiBool PI_API render_create_shader_from_buffer(Shader *shader, void* buffer)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3D9Shader *shader_impl = (D3D9Shader *)shader->impl;

		switch (shader->type)
		{
		case ST_VS:
			pi_renderinfo_add_vs_num(1);
			IDirect3DDevice9_CreateVertexShader(context->device, (DWORD *)buffer, &shader_impl->handle.vertex_shader);
			break;
		case ST_PS:
			pi_renderinfo_add_fs_num(1);
			IDirect3DDevice9_CreatePixelShader(context->device, (DWORD *)buffer, &shader_impl->handle.pixel_shader);
			break;
		}

		ID3DXConstantTable *const_table = NULL;
		//D3DXGetShaderConstantTable((DWORD *)buffer, &const_table);
		//如果开了启用大地址，不能使用D3DXGetShaderConstantTable，需要使用下面的函数
		D3DXGetShaderConstantTableEx((DWORD *)buffer, D3DXCONSTTABLE_LARGEADDRESSAWARE, &const_table);

		if (const_table)
		{
			D3DXCONSTANTTABLE_DESC const_table_desc;
			const_table->GetDesc(&const_table_desc);

			for (UINT i = 0; i < const_table_desc.Constants; i++)
			{
				D3DXHANDLE constant;
				constant = const_table->GetConstant(NULL, i);

				D3DXCONSTANT_DESC const_desc;
				UINT const_count = 1;
				const_table->GetConstantDesc(constant, &const_desc, &const_count);
				Uniform u;
				pi_memset_inline(&u, 0, sizeof(Uniform));
				u.name = pi_conststr(const_desc.Name);
				u.size = const_desc.Bytes;
				u.count = const_desc.Elements;
				RegisterSet register_set;
				u.type = _get_uniform_type(&const_desc, &register_set);
				if (u.type == UT_SAMPLER_1D ||
					u.type == UT_SAMPLER_2D ||
					u.type == UT_SAMPLER_3D ||
					u.type == UT_SAMPLER_CUBE ||
					u.type == UT_SAMPLER_1D_ARRAY ||
					u.type == UT_SAMPLER_2D_ARRAY ||
					u.type == UT_SAMPLER_2D_SHADOW ||
					u.type == UT_UNSIGNED_INT_SAMPLER_2D)
				{
					u.size = sizeof(SamplerState);
				}
				u.is_dirty = TRUE;
				u.d_type = DUT_NONE;

				if (pi_strlen(u.name) > 2 && u.name[0] == 'g' && u.name[1] == '_')
				{
					/* 默认 全局 uniform */
					u.d_type = _get_default_type(u.name);
				}

				D3D9Uniform *u_impl = pi_new0(D3D9Uniform, 1);
				switch (shader->type)
				{
				case ST_VS:
					u_impl->vs_register = TRUE;
					u_impl->vs_register_index = const_desc.RegisterIndex;
					u_impl->vs_register_count = const_desc.RegisterCount;
					u_impl->vs_columns = const_desc.Columns;
					u_impl->vs_register_set = register_set;
					break;
				case ST_PS:
					u_impl->ps_register = TRUE;
					u_impl->fs_register_index = const_desc.RegisterIndex;
					u_impl->fs_register_count = const_desc.RegisterCount;
					u_impl->fs_columns = const_desc.Columns;
					u_impl->fs_register_set = register_set;
					break;
				}

				u.impl = u_impl;

				pi_dvector_push(&shader_impl->uniforms, &u);
			}

			const_table->Release();
		}
		return TRUE;
	}


	PiBool PI_API render_shader_compile(Shader *shader, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void** output_buffer, uint32* size, PiBool save_buffer)
	{
		const char *macro_value = "1";
		D3D_SHADER_MACRO shader_macros[MAX_DEFINE_NUM + 1] = { NULL, NULL };
		for (uint i = 0; i < num_defines; i++)
		{
			shader_macros[i].Name = defines[i];
			shader_macros[i].Definition = macro_value;
		}

		// todo:默认的编译选项，根据需要可以进行优化
		UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL1;

		HRESULT hr = S_OK;
		ID3DBlob *error_message = NULL;
		ID3DBlob *binary = NULL;
		switch (shader->type)
		{
		case ST_VS:
			hr = D3DCompile(src_data, src_data_size, NULL, shader_macros, NULL, "main", "vs_3_0", flags, 0, &binary, &error_message);
			break;
		case ST_PS:
			hr = D3DCompile(src_data, src_data_size, NULL, shader_macros, NULL, "main", "ps_3_0", flags, 0, &binary, &error_message);
			break;
		}

		if (SUCCEEDED(hr))
		{
			render_create_shader_from_buffer(shader, binary->GetBufferPointer());
			if (save_buffer)
			{
				*size = binary->GetBufferSize();
				*output_buffer = pi_malloc0(*size);
				pi_memcpy_inline(*output_buffer, binary->GetBufferPointer(), *size);
			}
			binary->Release();
		}
		else
		{
			if (binary)
			{
				binary->Release();
			}

			if (error_message)
			{
				wchar *log = pi_utf8_to_wstr((char *)error_message->GetBufferPointer());
				pi_error_set(ERROR_TYPE_OPERATE_FORBIDDEN, 0, log, __FILE__, __LINE__);
				pi_free(log);
				error_message->Release();
			}
		}

		return TRUE;
	}

	uint PI_API render_shader_compile_offline(ShaderType type, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void** buffer)
	{
		const char *macro_value = "1";
		D3D_SHADER_MACRO shader_macros[MAX_DEFINE_NUM + 1] = { NULL, NULL };
		for (uint i = 0; i < num_defines; i++)
		{
			shader_macros[i].Name = defines[i];
			shader_macros[i].Definition = macro_value;
		}

		// todo:默认的编译选项，根据需要可以进行优化
		UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL1;

		HRESULT hr = S_OK;
		ID3DBlob *error_message = NULL;
		ID3DBlob *binary = NULL;
		switch (type)
		{
		case ST_VS:
			hr = D3DCompile(src_data, src_data_size, NULL, shader_macros, NULL, "main", "vs_3_0", flags, 0, &binary, &error_message);
			break;
		case ST_PS:
			hr = D3DCompile(src_data, src_data_size, NULL, shader_macros, NULL, "main", "ps_3_0", flags, 0, &binary, &error_message);
			break;
		}
		if (SUCCEEDED(hr))
		{
			DWORD* bufferPointer = (DWORD *)binary->GetBufferPointer();
			SIZE_T size = binary->GetBufferSize();
			*buffer = pi_malloc0(size);
			pi_memcpy_inline(*buffer, bufferPointer, size);
			binary->Release();
			return size;
		}
		else{
			if (error_message)
			{
				wchar *log = pi_utf8_to_wstr((char *)error_message->GetBufferPointer());
				pi_error_set(ERROR_TYPE_OPERATE_FORBIDDEN, 0, log, __FILE__, __LINE__);
				pi_free(log);
				error_message->Release();
			}
		}
		return 0;
	}


	PiBool PI_API render_program_init(GpuProgram *program)
	{
		D3D9Program *program_impl = pi_new0(D3D9Program, 1);
		pi_renderinfo_add_gpuprogram_num(1);
		program->impl = program_impl;
		pi_dvector_init(&program_impl->uniforms, sizeof(Uniform));
		pi_dvector_init(&program_impl->global_uniforms, sizeof(Uniform));
		return TRUE;
	}

	PiBool PI_API render_program_clear(GpuProgram *program)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;

		pi_renderinfo_add_gpuprogram_num(-1);

		uint i, len = pi_dvector_size(&program_impl->uniforms);
		for (i = 0; i < len; ++i)
		{
			Uniform *u = (Uniform *)pi_dvector_get(&program_impl->uniforms, i);
			pi_free(u->impl);
			pi_free(u->value);
		}
		pi_dvector_clear(&program_impl->uniforms, TRUE);

		len = pi_dvector_size(&program_impl->global_uniforms);
		for (i = 0; i < len; ++i)
		{
			Uniform *u = (Uniform *)pi_dvector_get(&program_impl->global_uniforms, i);
			pi_free(u->impl);
		}
		pi_dvector_clear(&program_impl->global_uniforms, TRUE);

		pi_free(program_impl);
		return TRUE;
	}

	PiBool PI_API render_program_attach(GpuProgram *program, Shader *shader)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;
		D3D9Shader *shader_impl = (D3D9Shader *)shader->impl;
		if (shader_impl != NULL)
		{
			program_impl->shaders[shader->type] = shader_impl;
		}
		return TRUE;
	}

	PiBool PI_API render_program_dettach(GpuProgram *program, ShaderType type)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;
		if (program_impl->shaders[type] != NULL)
		{
			program_impl->shaders[type] = NULL;
		}
		return TRUE;
	}

	PiBool PI_API render_program_link(GpuProgram *program)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;
		D3D9Shader *vs = program_impl->shaders[ST_VS];
		D3D9Shader *ps = program_impl->shaders[ST_PS];
		if (vs != NULL && ps != NULL)
		{
			uint vs_uniform_count = pi_dvector_size(&vs->uniforms);
			for (uint i = 0; i < vs_uniform_count; i++)
			{
				Uniform *vs_u = (Uniform *)pi_dvector_get(&vs->uniforms, i);
				if (vs_u->d_type != DUT_NONE)
				{
					Uniform global_u;
					pi_memcpy_inline(&global_u, vs_u, sizeof(Uniform));
					global_u.impl = pi_new0(D3D9Uniform, 1);
					pi_memcpy_inline(global_u.impl, vs_u->impl, sizeof(D3D9Uniform));
					pi_dvector_push(&program_impl->global_uniforms, &global_u);
				}
				else
				{
					Uniform prog_u;
					pi_memcpy_inline(&prog_u, vs_u, sizeof(Uniform));
					prog_u.impl = pi_new0(D3D9Uniform, 1);
					pi_memcpy_inline(prog_u.impl, vs_u->impl, sizeof(D3D9Uniform));
					pi_dvector_push(&program_impl->uniforms, &prog_u);
				}
			}

			uint ps_uniform_count = pi_dvector_size(&ps->uniforms);
			for (uint i = 0; i < ps_uniform_count; i++)
			{
				Uniform *ps_u = (Uniform *)pi_dvector_get(&ps->uniforms, i);

				if (ps_u->d_type != DUT_NONE)
				{
					uint j, global_count = pi_dvector_size(&program_impl->global_uniforms);
					for (j = 0; j < global_count; j++)
					{
						Uniform *global_u = (Uniform *)pi_dvector_get(&program_impl->global_uniforms, j);
						if (pi_str_compare(global_u->name, ps_u->name) == PI_COMP_EQUAL)
						{
							D3D9Uniform *u_impl = (D3D9Uniform *)global_u->impl;
							D3D9Uniform *fs_impl = (D3D9Uniform *)ps_u->impl;
							u_impl->ps_register = TRUE;
							u_impl->fs_register_index = fs_impl->fs_register_index;
							u_impl->fs_register_count = fs_impl->fs_register_count;
							u_impl->fs_columns = fs_impl->fs_columns;
							break;
						}
					}
					if (j == global_count)
					{
						Uniform global_u;
						pi_memcpy_inline(&global_u, ps_u, sizeof(Uniform));
						global_u.impl = pi_new0(D3D9Uniform, 1);
						pi_memcpy_inline(global_u.impl, ps_u->impl, sizeof(D3D9Uniform));
						pi_dvector_push(&program_impl->global_uniforms, &global_u);
					}
				}
				else
				{
					uint j, global_count = pi_dvector_size(&program_impl->uniforms);
					for (j = 0; j < global_count; j++)
					{
						Uniform *prog_u = (Uniform *)pi_dvector_get(&program_impl->uniforms, j);
						if (pi_str_compare(prog_u->name, ps_u->name) == PI_COMP_EQUAL)
						{
							D3D9Uniform *u_impl = (D3D9Uniform *)prog_u->impl;
							D3D9Uniform *ps_u_impl = (D3D9Uniform *)ps_u->impl;
							u_impl->ps_register = TRUE;
							u_impl->fs_register_index = ps_u_impl->fs_register_index;
							u_impl->fs_register_count = ps_u_impl->fs_register_count;
							u_impl->fs_register_set = ps_u_impl->fs_register_set;
							u_impl->fs_columns = ps_u_impl->fs_columns;
							break;
						}
					}
					if (j == global_count)
					{
						Uniform prog_u;
						pi_memcpy_inline(&prog_u, ps_u, sizeof(Uniform));
						prog_u.impl = pi_new0(D3D9Uniform, 1);
						pi_memcpy_inline(prog_u.impl, ps_u->impl, sizeof(D3D9Uniform));
						pi_dvector_push(&program_impl->uniforms, &prog_u);
					}
				}
			}
		}
		return TRUE;
	}

	void PI_API render_program_set_uniform(GpuProgram *program, Uniform *uniform, Uniform* material_uniform)
	{
		d3d9_state_set_uniform(program, uniform, material_uniform);
	}

	PiDvector *PI_API render_program_get_uniforms(GpuProgram *program, PiBool is_global)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;
		PiDvector *vec = &program_impl->uniforms;
		if (is_global)
		{
			vec = &program_impl->global_uniforms;
		}
		return vec;
	}

	PiBool PI_API render_program_clear_uniforms(GpuProgram *program)
	{
		D3D9Program *program_impl = (D3D9Program *)program->impl;
		uint i, len = pi_dvector_size(&program_impl->uniforms);
		for (i = 0; i < len; ++i)
		{
			Uniform *u = (Uniform *)pi_dvector_get(&program_impl->uniforms, i);
			u->is_dirty = TRUE;
			if (u->value && u->count > 0)
			{
				pi_memset_inline(u->value, 0, u->size);
			}
		}

		len = pi_dvector_size(&program_impl->global_uniforms);
		for (i = 0; i < len; ++i)
		{
			Uniform *u = (Uniform *)pi_dvector_get(&program_impl->global_uniforms, i);
			u->is_dirty = TRUE;
			if (u->value && u->count > 0)
			{
				pi_memset_inline(u->value, 0, u->size);
			}
		}
		return TRUE;
	}
}
