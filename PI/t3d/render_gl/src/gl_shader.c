#include <gl_shader.h>

#include <gl_texture.h>
#include <gl_rendersystem.h>

#include <gl_interface.h>
#include <renderinfo.h>

/* 索引与枚举AttributeIndex一一对应 */
char *g_attrib_name[] = 
{
	"g_Position",
	"g_Normal",
	"g_Color",
	"g_Specular",
	"g_Binormal",
	"g_Tangent",
	"g_BlendWeights",
	"g_BlendIndices",
	"g_TexCoord0",
	"g_TexCoord1",
	"g_TexCoord2",
	"g_TexCoord3",
	"g_TexCoord4",
	"g_TexCoord5",
	"g_TexCoord6",
	"g_TexCoord7",
	"g_TexCoord8",
	"g_TexCoord9",
	"g_TexCoord10",
	"g_TexCoord11",
	"g_TexCoord12",
	"g_TexCoord13",
	"g_TexCoord14",
	"g_TexCoord15",
};

extern PiRenderSystem *g_rsystem;

/* 将自身的矩阵转为opengl接受的矩阵形式：转置 */
static void make_gl_mat4(float *dst, const PiMatrix4 *src, uint num)
{
	uint i;
	for(i = 0; i < num; ++i )
	{
		dst[ 0] = src[i].m[0][0];	dst[ 1] = src[i].m[1][0];	dst[ 2] = src[i].m[2][0];	dst[ 3] = src[i].m[3][0];
		dst[ 4] = src[i].m[0][1];	dst[ 5] = src[i].m[1][1];	dst[ 6] = src[i].m[2][1];	dst[ 7] = src[i].m[3][1];
		dst[ 8] = src[i].m[0][2];	dst[ 9] = src[i].m[1][2];	dst[10] = src[i].m[2][2];	dst[11] = src[i].m[3][2];
		dst[12] = src[i].m[0][3];	dst[13] = src[i].m[1][3];	dst[14] = src[i].m[2][3];	dst[15] = src[i].m[3][3];

		dst += 16;
	}
}

/* 将自身的矩阵转为opengl接受的矩阵形式：转置 */
static void make_gl_mat3(float *dst, const float *src, uint num)
{
	uint i;
	for(i = 0; i < num; ++i )
	{
		dst[0] = src[0 + 9 * i];	dst[1] = src[3 + 9 * i];	dst[2] = src[6 + 9 * i];
		dst[3] = src[1 + 9 * i];	dst[4] = src[4 + 9 * i];	dst[5] = src[7 + 9 * i];
		dst[6] = src[2 + 9 * i];	dst[7] = src[5 + 9 * i];	dst[8] = src[8 + 9 * i];

		dst += 9;
	}
}

static PiCompR PI_API _sort_uniform(void *user_data, const void *a, const void *b)
{
	Uniform *pa = (Uniform *)a;
	Uniform *pb = (Uniform *)b;
	return pi_str_compare(pa->name, pb->name);
}


/* 找到默认类型对应的枚举 */
static void get_default_type(Uniform *u, char *name)
{
	u->d_type = DUT_NONE;

	if(pi_str_compare(name, G_WORLD_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_WORLD_MATRIX;	
	} else if (pi_str_compare(name, G_VIEW_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_VIEW_MATRIX;
	} else if (pi_str_compare(name, G_PROJ_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_PROJ_MATRIX;
	} else if (pi_str_compare(name, G_WORLD_VIEW_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_WORLD_VIEW_MATRIX;
	} else if (pi_str_compare(name, G_VIEWPROJ_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_VIEW_PROJ_MATRIX;
	} else if (pi_str_compare(name, G_WORLD_VIEW_PROJ_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_WORLD_VIEW_PROJ_MATRIX;
	} else if (pi_str_compare(name, G_NORMAL_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_NORMAL_MATRIX;
	} else if (pi_str_compare(name, G_VIEWPORT_SIZE) == PI_COMP_EQUAL) {
		u->d_type = DUT_VIEWPORT_SIZE;
	} else if (pi_str_compare(name, G_TIME) == PI_COMP_EQUAL) {
		u->d_type = DUT_TIME;
	} else if (pi_str_compare(name, G_ALPHACULLOFF) == PI_COMP_EQUAL) {
		u->d_type = DUT_ALPHA_CULLOFF;
	} else if (pi_str_compare(name, G_VIEW_NORMAL_MATRIX) == PI_COMP_EQUAL) {
		u->d_type = DUT_VIEW_NORMAL_MATRIX;
	} else if (pi_str_compare(name, G_VIEW_POSITION) == PI_COMP_EQUAL) {
		u->d_type = DUT_VIEW_POSITION;
	}
}

static void get_uniform_type(Uniform *u, uint32 gl_type, uint count)
{
	u->count = count;
	u->type = UT_UNKOWN;
	switch(gl_type)
	{
	case GL2_FLOAT:
		u->type = UT_FLOAT;
		u->value = pi_malloc0(count * sizeof(float));
		break;
	case GL2_FLOAT_VEC2:
		u->type = UT_VEC2;
		u->value = pi_malloc0(2 * count * sizeof(float));
		break;
	case GL2_FLOAT_VEC3:		
		u->type = UT_VEC3;
		u->value = pi_malloc0(3 * count * sizeof(float));
		break;
	case GL2_FLOAT_VEC4:
		u->type = UT_VEC4;
		u->value = pi_malloc0(4 * count * sizeof(float));
		break;
	case GL2_INT:
		u->type = UT_INT;
		u->value = pi_malloc0(count * sizeof(int));
		break;
	case GL2_INT_VEC2:
		u->type = UT_IVEC2;
		u->value = pi_malloc0(2 * count * sizeof(int));
		break;
	case GL2_INT_VEC3:
		u->type = UT_IVEC3;
		u->value = pi_malloc0(3 * count * sizeof(int));
		break;
	case GL2_INT_VEC4:
		u->type = UT_IVEC4;
		u->value = pi_malloc0(4 * count * sizeof(int));
		break;
	case GL2_FLOAT_MAT2:
		u->type = UT_MATRIX2;
		u->value = pi_malloc0(2 * 2 * count * sizeof(float));
		break;
	case GL2_FLOAT_MAT3:
		u->type = UT_MATRIX3;
		u->value = pi_malloc0(3 * 3 * count * sizeof(float));
		break;
	case GL2_FLOAT_MAT4:	
		u->type = UT_MATRIX4;
		u->value = pi_malloc0(4 * 4 * count * sizeof(float));
		break;
	case GL2_SAMPLER_2D:	
		u->type = UT_SAMPLER_2D;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	case GL2_SAMPLER_CUBE:	
		u->type = UT_SAMPLER_CUBE;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	case GL3_SAMPLER_3D:	
		u->type = UT_SAMPLER_3D;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	case GL3_SAMPLER_2D_ARRAY:	
		u->type = UT_SAMPLER_2D_ARRAY;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	case GL3_SAMPLER_2D_SHADOW:	
		u->type = UT_SAMPLER_2D_SHADOW;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	case GL3_UNSIGNED_INT_SAMPLER_2D:
		u->type = UT_UNSIGNED_INT_SAMPLER_2D;
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	default:
		PI_ASSERT(FALSE, "gl_type isn't support u->type = %d", gl_type);
		break;
	}
}

PiBool PI_API render_shader_init(Shader *shader)
{
	GLShader *impl = pi_new0(GLShader, 1);
	switch(shader->type)
	{
	case ST_VS:
		pi_renderinfo_add_vs_num(1);
		break;
	case ST_PS:
		pi_renderinfo_add_fs_num(1);
		break;
	}
	shader->impl = impl;

	switch(shader->type)
	{
	case ST_VS:
		impl->gl_type = GL2_VERTEX_SHADER;
		break;
	case ST_PS:
		impl->gl_type = GL2_FRAGMENT_SHADER;
		break;
	default:
		pi_free(impl);
		shader->impl = NULL;
		PI_ASSERT(FALSE, "invalid type = %d", shader->type);
		break;
	}

	if(shader->impl != NULL)
	{
		impl->id = gl2_CreateShader(impl->gl_type);
	}
	return TRUE;
}

PiBool PI_API render_shader_clear(Shader *shader)
{
	GLShader *impl = (GLShader *)shader->impl;
	if(impl != NULL)
	{
		switch(shader->type)
		{
		case ST_VS:
			pi_renderinfo_add_vs_num(-1);
			break;
		case ST_PS:
			pi_renderinfo_add_fs_num(-1);
			break;
		}
		if(impl->id != 0)
		{
			gl2_DeleteShader(impl->id);
		}
		pi_free(shader->impl);	
	}
	return TRUE;
}

PiBool PI_API render_shader_compile(Shader *shader, const char *version, uint count, char **strs, uint32 *lens)
{
	PiBool is_ok = TRUE;
	GLShader *impl = (GLShader *)shader->impl;

	char* FS_PRECISION = NULL;

    uint count_final;
    char **strs_final;
    sint *lens_final;
	PiBool need_free = FALSE;

    count_final = count;
    strs_final = strs;
    lens_final = (sint*)lens;

	if(version != NULL)
	{
		++count_final;
	}

	if (gl_Self_GetInterfaceType() == RIT_GLES)
	{
		++count_final;
		FS_PRECISION = " precision mediump float;\n precision mediump int;\n";
	}

	if(count_final != count)
	{
		uint i, j;
		
		need_free = TRUE;
		strs_final = (char **)pi_malloc0(count_final * sizeof(char *));
		lens_final = (sint *)pi_malloc0(count_final * sizeof(sint));

		i = 0;
		if(version != NULL)
		{/* 版本必须写在最前面 */
			strs_final[i] = (char *)version;
			lens_final[i] = pi_strlen(version);
			++i;
		}

		if(FS_PRECISION != NULL)
		{/* 着色器精度声明 */
			strs_final[i] = FS_PRECISION;
			lens_final[i] = pi_strlen(FS_PRECISION);
			++i;
		}

		for (j = 0; j < count; j++)
		{
			strs_final[i + j] = strs[j];
			lens_final[i + j] = lens[j];
		}
	}
	
	gl2_ShaderSource(impl->id, count_final, strs_final, lens_final);
	gl2_CompileShader(impl->id);
	
	{
        char log[1024] = {0};
		uint real_length = 0;
		sint length = sizeof(log)/sizeof(log[0]) - 1;
        gl2_GetShaderInfoLog(impl->id, length, &real_length, log);
		if(log[0] != 0 && !pi_str_start_with(log, "No error"))
		{
			wchar *wlog = pi_utf8_to_wstr(log);
			pi_error_set(ERROR_TYPE_OPERATE_FORBIDDEN, 0, wlog, __FILE__, __LINE__);
			pi_free(wlog);
		}
	}

	gl2_GetShaderiv(impl->id, GL2_COMPILE_STATUS, &is_ok);

	if (need_free)
	{
		pi_free(strs_final);
		pi_free(lens_final);
	}
	return is_ok;
}

PiBool PI_API render_program_init(GpuProgram *program)
{
	GLProgram *impl = pi_new0(GLProgram, 1);
	pi_renderinfo_add_gpuprogram_num(1);
	program->impl = impl;
	pi_dvector_init(&impl->uniforms, sizeof(Uniform));
	pi_dvector_init(&impl->global_uniforms, sizeof(Uniform));
	impl->id = gl2_CreateProgram();
	return TRUE;
}

PiBool PI_API render_program_clear(GpuProgram *program)
{
	GLProgram *impl = program->impl;
	Uniform *us = pi_dvector_address(&impl->uniforms);
	uint i, len = pi_dvector_size(&impl->uniforms);
	
	pi_renderinfo_add_gpuprogram_num(-1);
	for(i = 0; i < len; ++i)
	{
		pi_free(us[i].impl);
		pi_free(us[i].value);
	}
	
	us = pi_dvector_address(&impl->global_uniforms);
	len = pi_dvector_size(&impl->global_uniforms);
	for(i = 0; i < len; ++i)
	{
		pi_free(us[i].impl);
		pi_free(us[i].value);
	}

	pi_dvector_clear(&impl->uniforms, TRUE);
	pi_dvector_clear(&impl->global_uniforms, TRUE);

	gl2_DeleteProgram(impl->id);
	pi_free(impl);
	return TRUE;
}

PiBool PI_API render_program_attach(GpuProgram *program, Shader *shader)
{
	GLShader *gl_shader = shader->impl;
	GLProgram *gl_program = program->impl;
	if(gl_shader != NULL)
	{
		gl_program->shaders[shader->type] = gl_shader;
		gl2_AttachShader(gl_program->id, gl_shader->id);
	}
	return TRUE;
}

PiBool PI_API render_program_dettach(GpuProgram *program, ShaderType type)
{
	GLProgram *gl_program = program->impl;
	if(gl_program->shaders[type] != NULL)
	{
		GLShader *gl_shader = gl_program->shaders[type];
		gl2_DetachShader(gl_program->id, gl_shader->id);
		gl_program->shaders[type] = NULL;
	}
	return TRUE;
}

PiBool PI_API render_program_link(GpuProgram *program)
{
	uint i;
	PiBool r = TRUE;
	GLProgram *impl = program->impl;
	GLRenderSystem *gl_system = g_rsystem->impl;
	GLRenderState *gl_state = &gl_system->state;

	if(impl->is_link)
	{
		return TRUE;
	}

	/* 绑定所有的流 */
	for(i = 0; i < gl_state->max_attrib_num; ++i)
	{
		gl2_BindAttribLocation(impl->id, i, g_attrib_name[i]);
	}

	gl2_LinkProgram(impl->id);
	
	{
		char log[1024] = {0};
		sint length = sizeof(log)/sizeof(log[0]) - 1;
		sint real_length = 0;
		gl2_GetProgramInfoLog(impl->id, length, (uint *)&real_length, log);
		if(log[0] != 0 && !pi_str_start_with(log, "No error"))
		{
			wchar *wlog = pi_utf8_to_wstr(log);
			pi_error_set(ERROR_TYPE_OPERATE_FORBIDDEN, 0, wlog, __FILE__, __LINE__);
			pi_free(wlog);
		}
	}

	gl2_GetProgramiv(impl->id, GL2_LINK_STATUS, &r);
	if(r && gl_Self_GetInterfaceType() != RIT_NULL)
	{
		sint num_uniforms = 0;
		sint max_name_size = 0;
		
		impl->is_link = TRUE;
		
		/* 取所有的流，判断流的id是否和绑定一样 */
		for(i = 0; i < EVS_NUM; ++i)
		{
			sint vid = gl2_GetAttribLocation(impl->id, g_attrib_name[i]);
			impl->is_attrib_enable[i] = (vid >= 0);

			if(vid >= 0 && vid != (sint)i)
			{/* 用到了超出硬件允许的流范围外的流id */
				PI_ASSERT(FALSE, "bind attribute failed, out of range");
			}
		}
	
		/* 取所有的uniform */
		gl2_GetProgramiv(impl->id, GL2_ACTIVE_UNIFORMS, &num_uniforms);
		gl2_GetProgramiv(impl->id, GL2_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_size);
		
		if(num_uniforms > 0)
		{
			sint i;
			char *name = pi_malloc0((1 + max_name_size) * sizeof(char));
			for(i = 0; i < num_uniforms; ++i)
			{
				Uniform u;
				uint32 type;
				GLUniform *u_impl;
				sint arr_index, name_len = -1, count = -1;
				pi_memset_inline(&u, 0, sizeof(Uniform));
				gl2_GetActiveUniform(impl->id, i, max_name_size, (uint *)&name_len, &count, &type, name);
				name[name_len] = '\0';

				/** 
				 * 由于N卡(GT510)上的const全局变量也会通过gl2_GetActiveUniform函数返回，经观察，返回的名字以下划线开始，所以要滤掉下划线开始的名字 
				 * 注：这意味着，我们的uniform名字不能以下划线开头
				 */
				if(pi_str_start_with(name, "_"))
				{
					continue;
				}

				u.is_dirty = TRUE;
				u.d_type = DUT_NONE;
				
				/* 去掉数组名字后面的[0] */
				arr_index = pi_str_char_index(name, '[');
				if(arr_index >= 0)
				{
					name[arr_index] = '\0';
				}

				u.name = pi_conststr(name);

				if(name_len > 2 && name[0] == 'g' && name[1] == '_')
				{/* 默认 全局 uniform */
					get_default_type(&u, name);
				}
								
				u_impl = pi_new0(GLUniform, 1);
				u_impl->location = gl2_GetUniformLocation(impl->id, u.name);
				u_impl->gl_type = type;
				u_impl->sampler_id = -1;

				u.impl = u_impl;
				
				/* 该函数内部会为u_impl的index赋值 */
				get_uniform_type(&u, type, count);
				u.size = pi_uniform_value_size(u.type, u.count);
				PI_ASSERT(u.size > 0, "uniform's size must positive");

				if(pi_str_start_with(u.name, "g_"))
				{
					pi_dvector_push(&impl->global_uniforms, &u);
				}
				else
				{
					pi_dvector_push(&impl->uniforms, &u);
				}
			}
			
			/* 所有的uniform按name从小到大排序 */
			{
				Uniform *us = pi_dvector_address(&impl->uniforms);
				uint i, len = pi_dvector_size(&impl->uniforms);
				if(len > 1)
				{
					pi_qsort(us, len, sizeof(Uniform), _sort_uniform, NULL);
				}
				
				for(i = 0; i < len; ++i)
				{
					switch(us[i].type)
					{
					case UT_SAMPLER_1D:
					case UT_SAMPLER_2D:
					case UT_SAMPLER_2D_SHADOW:
					case UT_SAMPLER_3D:
					case UT_SAMPLER_CUBE:
					case UT_SAMPLER_1D_ARRAY:
					case UT_SAMPLER_2D_ARRAY:
					case UT_UNSIGNED_INT_SAMPLER_2D:
						impl->samplers[impl->sampler_num++] = &us[i];
						break;
					}	
				}
			}
			pi_free(name);
		}
	}
	return r;
}

PiBool PI_API render_program_set_uniform(GpuProgram *program, Uniform *u)
{
	PI_USE_PARAM(program);
	u->is_dirty = TRUE;
	return TRUE;
}

static void _set_gluniform(Uniform *u)
{
	GLUniform *impl = u->impl;
	switch(u->type)
	{
	case UT_FLOAT:
		gl2_Uniform1fv(impl->location, u->count, u->value);
		break;
	case UT_VEC2:
		gl2_Uniform2fv(impl->location, u->count, u->value);
		break;
	case UT_VEC3:
		gl2_Uniform3fv(impl->location, u->count, u->value);
		break;
	case UT_VEC4:
		gl2_Uniform4fv(impl->location, u->count, u->value);
		break;
	case UT_INT:
		gl2_Uniform1iv(impl->location, u->count, u->value);
		break;
	case UT_IVEC2:
		gl2_Uniform2iv(impl->location, u->count, u->value);
		break;
	case UT_IVEC3:
		gl2_Uniform3iv(impl->location, u->count, u->value);
		break;
	case UT_IVEC4:
		gl2_Uniform4iv(impl->location, u->count, u->value);
		break;
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_SAMPLER_2D_SHADOW:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		if (!gl_texture_set_sampler(u->value))
		{
			pi_log_print(LOG_INFO, "warnning, gl_texture_set_sampler, uniform's texture is null, name = %s", u->name);
		}
		break;
	/* 高层矩阵全是用行主序存储，opengl默认使用列主序存储，所以需要转置 */
	case UT_MATRIX3:
		{
			float *gl_mats = pi_malloc0(u->count * sizeof(float) * 9);
			make_gl_mat3(gl_mats, u->value, u->count);
			gl2_UniformMatrix3fv(impl->location, u->count, FALSE, gl_mats);
			pi_free(gl_mats);
		}
		break;
	case UT_MATRIX4:
		{
         	float *gl_mats = pi_malloc0(u->count * sizeof(PiMatrix4));
			make_gl_mat4(gl_mats, u->value, u->count);
            gl2_UniformMatrix4fv(impl->location, u->count, FALSE, gl_mats);
			pi_free(gl_mats);
        }
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", u->type);
		break;
	}
}

PiBool PI_API gl_program_set_gluniforms(GpuProgram *program)
{
	GLProgram *gl_program = program->impl;
	uint i, len = pi_dvector_size(&gl_program->uniforms);
	for(i = 0; i < len; ++i)
	{
		Uniform *u = (Uniform *)pi_dvector_get(&gl_program->uniforms, i);
		if(u->is_dirty)
		{
			u->is_dirty = FALSE;
			_set_gluniform(u);
		}
	}
	
	len = pi_dvector_size(&gl_program->global_uniforms);
	for(i = 0; i < len; ++i)
	{
		Uniform *u = (Uniform *)pi_dvector_get(&gl_program->global_uniforms, i);
		if(u->is_dirty)
		{
			u->is_dirty = FALSE;
			_set_gluniform(u);
		}
	}
	return TRUE;
}

PiDvector* PI_API render_program_get_uniforms(GpuProgram *program, PiBool is_global)
{
	GLProgram *impl = program->impl;
	PiDvector *vec = &impl->uniforms;
	if(is_global)
	{
		vec = &impl->global_uniforms;
	}
	return vec;
}

PiBool PI_API render_program_clear_uniforms(GpuProgram *program)
{
	GLProgram *impl = program->impl;
	uint i, len = pi_dvector_size(&impl->uniforms);
	for(i = 0; i < len; ++i)
	{
		Uniform *u = pi_dvector_get(&impl->uniforms, i);
		u->is_dirty = TRUE;
		if(u->value && u->count > 0)
		{
			pi_memset_inline(u->value, 0, u->size);
		}		
	}

	len = pi_dvector_size(&impl->global_uniforms);
	for(i = 0; i < len; ++i)
	{
		Uniform *u = pi_dvector_get(&impl->global_uniforms, i);
		u->is_dirty = TRUE;
		if(u->value && u->count > 0)
		{
			pi_memset_inline(u->value, 0, u->size);
		}
	}

	return TRUE;
}