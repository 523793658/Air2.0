#include <shader.h>
#include <renderwrap.h>
#include <windows.h>
#include <time.h>
extern PiRenderSystem *g_rsystem;
static double spc;

typedef struct
{
	void* buffer;
	uint size;
	char* key;
}ShaderBuffer;

PiHash* shader_map = NULL;

typedef struct  
{
	PiVector* shader_list;
	wchar* base_path;
}UserData;

static PiSelectR PI_API _save_shader(void *user_data, PiKeyValue *kv)
{
	wchar file_path[256];
	UserData* data = (UserData*)user_data;
	ShaderBuffer* buffer = (ShaderBuffer*)kv->value;
	wchar* file_name = pi_str_to_wstr(buffer->key, PI_CP_UTF8);
	void* file;
	pi_wstrcpy(file_path, data->base_path, pi_wstrlen(data->base_path));
	pi_wstr_cat(file_path, 256, L"/");
	pi_wstr_cat(file_path, 256, file_name);
	pi_vfs_get_real_path(file_path, file_path);
	file = pi_file_open(file_path, FILE_OPEN_WRITE | FILE_OPEN_WRITE_CLEAR);
	pi_file_write(file, 0, FALSE, buffer->buffer, buffer->size);
	pi_file_close(file);
	pi_vector_push(data->shader_list, buffer->key);
	pi_free(file_name);
	pi_free(buffer->buffer);
	pi_free(buffer);
	return SELECT_NEXT;
}
void PI_API pi_shader_save(PiVector* shader_list, wchar* base_path)
{
	if (shader_map == NULL)
	{
		return;
	}
	UserData user_data = {
		shader_list,
		base_path
	};
	pi_hash_foreach(shader_map, (PiSelectFunc)_save_shader, &user_data);
	pi_hash_free(shader_map);
}

void PI_API program_set_uniform(void *program, Uniform *uniform, Uniform* material_uniform)
{
	/*if (uniform->type > UT_STRUCT)
	{
	if (((SamplerState *)material_uniform->value)->tex == NULL)
	{
	pi_log_print(LOG_WARNING, "sampler's texture must be valid, now texture is NULL!");
	return;
	}
	}*/
	render_program_set_uniform(program, uniform, material_uniform);
}

PiDvector* PI_API program_get_uniforms(void *program, PiBool is_global)
{
	return render_program_get_uniforms(program, is_global);
}

PiBool PI_API program_clear_uniforms(void *program)
{
	return render_program_clear_uniforms(program);
}

void PI_API program_set_bind(void *program, uint index, void *bind)
{
	if(index < MAX_BIND_NUM)
	{
		((GpuProgram*)program)->bind_data[index] = bind;
	}
}

void* PI_API program_get_bind(void *program, uint index)
{
	void *data = NULL;
	if(index < MAX_BIND_NUM)
	{
		data = ((GpuProgram*)program)->bind_data[index];
	}
	return data;
}

void PI_API uniform_init(Uniform *u, char *name, UniformType type, uint count)
{
	uint len = pi_strlen(name);
	pi_memset_inline(u, 0, sizeof(Uniform));
	u->type = type;
	u->count = count;
	u->name = pi_malloc(1 + len);
	pi_strcpy(u->name, name, len);
	
	switch(type)
	{
	case UT_FLOAT:
		u->value = pi_malloc0(sizeof(float)); 
		break;
	case UT_VEC2:
		u->value = pi_malloc0(2 * sizeof(float));
		break;
	case UT_VEC3:
		u->value = pi_malloc0(3 * sizeof(float));
		break;
	case UT_VEC4:
		u->value = pi_malloc0(4 * sizeof(float));
		break;
	case UT_INT:
		u->value = pi_malloc0(sizeof(sint));
		break;
	case UT_IVEC2:
		u->value = pi_malloc0(2 * sizeof(sint));
		break;
	case UT_IVEC3:
		u->value = pi_malloc0(3 * sizeof(sint));
		break;
	case UT_IVEC4:
		u->value = pi_malloc0(4 * sizeof(sint));
		break;
	case UT_MATRIX2:
		u->value = pi_malloc0(2 * 2 * sizeof(float));
		break;
	case UT_MATRIX3:
		u->value = pi_malloc0(3 * 3 * sizeof(float));
		break;
	case UT_MATRIX4:
		u->value = pi_malloc0(4 * 4 * sizeof(float));
		break;
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_2D_SHADOW:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		u->value = pi_malloc0(sizeof(SamplerState));
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", type);
		pi_free(u->name);
		pi_memset_inline(u, 0, sizeof(Uniform));
		break;
	}
}

void PI_API uniform_clear(Uniform *u)
{
	pi_free(u->name);
	pi_free(u->value);
	pi_memset_inline(u, 0, sizeof(Uniform));
}

void PI_API uniform_update(Uniform *u, void *data)
{
	uint len = 0;
	switch(u->type)
	{
	case UT_FLOAT:
		len = sizeof(float);
		break;
	case UT_VEC2:
		len = 2 * sizeof(float);
		break;
	case UT_VEC3:
		len = 3 * sizeof(float);
		break;
	case UT_VEC4:
		len = 4 * sizeof(float);
		break;
	case UT_INT:
		len = sizeof(sint);
		break;
	case UT_IVEC2:
		len = 2 * sizeof(sint);
		break;
	case UT_IVEC3:
		len = 3 * sizeof(sint);
		break;
	case UT_IVEC4:
		len = 4 * sizeof(sint);
		break;
	case UT_MATRIX2:
		len = 2 * 2 * sizeof(float);
		break;
	case UT_MATRIX3:
		len = 3 * 3 * sizeof(float);
		break;
	case UT_MATRIX4:
		len = 4 * 4 * sizeof(float);
		break;
	case UT_SAMPLER_1D:
	case UT_SAMPLER_2D:
	case UT_SAMPLER_2D_SHADOW:
	case UT_SAMPLER_3D:
	case UT_SAMPLER_CUBE:
	case UT_SAMPLER_1D_ARRAY:
	case UT_SAMPLER_2D_ARRAY:
	case UT_UNSIGNED_INT_SAMPLER_2D:
		len = sizeof(SamplerState);
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", u->type);
		break;
	}
	
	if(len > 0)
	{
		pi_memcpy_inline(u->value, data, len);
	}
}

static void* _create_shader(ShaderSource *source, ShaderType type, const char *version, uint num_defs, const char **defs, void** output_buffer, uint *size, PiBool save_buffer)
{
	PiBool r;
	Shader *shader;
	shader = pi_new0(Shader, 1);
	shader->type = type;
	render_shader_init(shader);
	
	pi_error_set(0, 0, NULL, __FILE__, __LINE__);
	//TODO:暂时先这样，后面应该把shader模块处理#include问题，t3d_gl处理宏和version问题
	r = render_shader_compile(shader, source->source_value, pi_strlen(source->source_value), num_defs, defs, output_buffer, size, save_buffer);
	{/* 打印着色器编译的警告或者错误信息 */
		PiError *er = pi_error_get();
		const char *type_str = type == ST_VS ? "vertex shader" : "pixel shader";

		if(!r)
		{
			pi_log_print(LOG_WARNING, "shader compile failed!");
		}
		if(er->message[0] != 0)
		{
			char *err_str = pi_wstr_to_utf8(er->message);
			pi_log_print(LOG_WARNING, "\ncompile shader info\ntype = %s, key = %s\n%s\n\n", type_str, source->source_key, err_str);
		}
	}

	if(!r)
	{/* 编译着色器失败，断言 */
		pi_free(shader);
		shader = NULL;
		PI_ASSERT(FALSE, "shader compile failed!");
	}
	//pi_free(src);
	//pi_free(lens);
	return shader;
}

PiBool PI_API pi_shader_create_from_buffer(const char *key, byte *data, uint size, ShaderType type)
{
	ShaderData *s = pi_new0(ShaderData, 1);
	Shader* shader;

	s->type = type;
	s->key = pi_str_dup(key);
	shader = pi_new0(Shader, 1);
	shader->type = type;
	render_shader_init(shader);
	render_create_shader_from_buffer(shader, data);
	s->shader = shader;
	pi_hash_insert(&g_rsystem->shader_map[type], s->key, s);
	return TRUE;
}



uint PI_API pi_shader_compile_offline(ShaderType type, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void** buffer)
{
	return render_shader_compile_offline(type, src_data, src_data_size, num_defines, defines, buffer);
}


char* PI_API create_shader_key(const char* key, uint num_defines, const char **defines, ShaderSource *value)
{
	char *shader_key;
	uint i, j, shader_key_len;
	uint64 define_key = 0, one = 1;

	for (i = 0; i < num_defines; ++i)
	{
		for (j = 0; j < value->define_num; ++j)
		{
			if (pi_str_equal(defines[i], value->defines[j], FALSE))
			{
				break;
			}
		}
		if (j < value->define_num)
		{
			define_key |= (one << j);
		}
	}

	// 20是uint64字符串形式的最大长度
	shader_key_len = 1 + pi_strlen(key) + 20;
	shader_key = (char *)pi_malloc0(shader_key_len);
	pi_memset_inline(shader_key, 0, shader_key_len);
	pi_str_convert_number(shader_key, define_key, 20);
	pi_str_cat(shader_key, shader_key_len - 1, key);
	return shader_key;
}

ShaderData* PI_API shader_get(ShaderType type, const char *key, uint num_defines, const char **defines)
{
	PiBool is_lookup;
	ShaderSource *value;
	ShaderData *shader_value = NULL;

	is_lookup = pi_hash_lookup(&g_rsystem->source_map, key, &value);
	PI_ASSERT(is_lookup, "shader's source isn't find, key = %s", key);
	if(is_lookup)
	{
		char* shader_key = create_shader_key(key, num_defines, defines, value);
		is_lookup = pi_hash_lookup(&g_rsystem->shader_map[type], shader_key, &shader_value);

		if(!is_lookup)
		{
			ShaderData *s = pi_new0(ShaderData, 1);
			ShaderBuffer* buffer = pi_new(ShaderBuffer, 1);
			s->type = type;
			s->key = pi_str_dup(shader_key);
			s->shader = _create_shader(value, type, value->version, num_defines, defines, &buffer->buffer, &buffer->size, TRUE);
			if (shader_map == NULL)
			{

				shader_map = pi_hash_new(1.75f, (PiHashFunc)pi_str_hash, pi_string_equal);
			}
			buffer->key = pi_str_dup(shader_key);
			pi_hash_insert(shader_map, buffer->key, buffer);
			pi_hash_insert(&g_rsystem->shader_map[type], s->key, s);
			is_lookup = pi_hash_lookup(&g_rsystem->shader_map[type], s->key, &shader_value);
		}
		++shader_value->ref_count;
		pi_free(shader_key);
	}
	return shader_value;
}


ShaderData* PI_API create_shader_from_file()
{
	return NULL;
}

void PI_API shader_add(ShaderData* shader_data)
{
	ShaderData *shader_value = NULL;
	PiBool is_lookup = pi_hash_lookup(&g_rsystem->source_map, shader_data->key, &shader_value);
	if (!is_lookup)
	{
		pi_hash_insert(&g_rsystem->shader_map[shader_data->type], shader_data->key, shader_data);
	}
}

static PiSelectR PI_API _destroy_program(void* user_data, void* value)
{
	PiSelectR r = SELECT_NEXT;
	ShaderData *shader = (ShaderData *)user_data;
	ProgramData	*program = (ProgramData *)value;
	if(program->vs == shader || program->fs == shader)
	{
		render_program_clear(program->program);
		pi_free(program->program);
		pi_free(program);
		r = SELECT_DELETE;
	}
	return r;
}

void PI_API shader_release(ShaderData *shader)
{
	--shader->ref_count;
	PI_ASSERT(shader->ref_count >= 0, "shader's ref_count must >= 0");
	
	/* 由于shader占用内存不多，所以暂时不需要删掉，缓存留着，节省后继的编译开销 */
	if(shader->ref_count == 0)
	{
		/*
		
		char *key = shader->key;
		
		pi_hash_foreach(&g_rsystem->program_map, _destroy_program, shader);

		render_shader_clear(shader->shader);
		
		pi_hash_delete(&g_rsystem->shader_map[shader->type], key, NULL);
		
		pi_free(key);
		pi_free(shader);
		
		*/
	}
}


void* PI_API program_get(ShaderData *vs, ShaderData *fs)
{
	PiBool r;
	ProgramData data;
	ProgramData *value = NULL;

	data.vs = vs;	
	data.fs = fs;

	if(!pi_hash_lookup(&g_rsystem->program_map, &data, &value))
	{
		
		ProgramData *program = pi_new0(ProgramData, 1);

		program->vs = vs;
		program->fs = fs;
		program->program = pi_new0(GpuProgram, 1);
		render_program_init(program->program);
		
		render_program_attach(program->program, vs->shader);
		render_program_attach(program->program, fs->shader);
		
		pi_error_set(0, 0, NULL, __FILE__, __LINE__);
		r = render_program_link(program->program);
		
		{/* 打印着色器链接的警告或者错误信息 */
			PiError *er = pi_error_get();
			if(!r)
			{
				pi_log_print(LOG_WARNING, "link shader failed!");
			}
			if(er->message[0] != 0)
			{
				char *err_str = pi_wstr_to_utf8(er->message);
				pi_log_print(LOG_WARNING, "\nlink shader info\n vs = %s, fs = %s\n%s\n\n", vs->key, fs->key, err_str);
			}
		}		
		
		if(r)
		{
			pi_hash_insert(&g_rsystem->program_map, program, program);
			pi_hash_lookup(&g_rsystem->program_map, &data, &value);
		}
		else 
		{
			value = NULL;
			pi_free(program->program);
			pi_free(program);
			PI_ASSERT(FALSE, "link shader failed!");
		}
	}
	
	return value != NULL ? value->program : NULL;
}