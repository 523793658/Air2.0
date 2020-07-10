#include <shadersystem.h>
#include <renderwrap.h>

static uint	PI_API _program_hash(const ProgramData *key)
{
	return ((uint)key->vs >> 4) + ((uint)key->fs >> 4);
}

static PiBool PI_API _program_equal(const ProgramData *kv1, const ProgramData *kv2)
{
	return (kv1->vs == kv2->vs) && (kv1->fs == kv2->fs);
}

static PiSelectR PI_API _destroy_program(void *user_data, PiKeyValue *kv)
{
	ProgramData	*program = (ProgramData *)kv->value;
	render_program_clear(program->program);
	pi_free(program->program);
	pi_free(program);
	return SELECT_NEXT;
}

static PiSelectR PI_API _destroy_shader(void *user_data, PiKeyValue *kv)
{
	ShaderData *shader = (ShaderData *)kv->value;

	render_shader_clear(shader->shader);
	pi_free(shader->shader);

	pi_free(kv->key);
	pi_free(kv->value);

	return SELECT_NEXT;
}

void PI_API pi_shadersystem_init(PiRenderSystem *system)
{
	uint i;
	pi_hash_init(&system->source_map, 0.75f, (PiHashFunc)pi_str_hash, pi_string_equal);
	pi_hash_init(&system->program_map, 0.75f, _program_hash, _program_equal);

	for (i = 0; i < ST_NUM; ++i)
	{
		pi_hash_init(&system->shader_map[i], 0.75f, (PiHashFunc)pi_str_hash, pi_string_equal);
	}
}

void PI_API pi_shadersystem_clear(PiRenderSystem *system)
{
	uint i;
	pi_hash_clear(&system->source_map, TRUE);

	pi_hash_foreach(&system->program_map, _destroy_program, NULL);
	pi_hash_clear(&system->program_map, TRUE);

	for (i = 0; i < ST_NUM; ++i)
	{
		pi_hash_foreach(&system->shader_map[i], _destroy_shader, NULL);
		pi_hash_clear(&system->shader_map[i], TRUE);
	}
}

void PI_API pi_shadersystem_reset(PiRenderSystem *system)
{
	uint i;

	pi_hash_foreach(&system->program_map, _destroy_program, NULL);
	pi_hash_clear(&system->program_map, FALSE);

	for (i = 0; i < ST_NUM; ++i)
	{
		pi_hash_foreach(&system->shader_map[i], _destroy_shader, NULL);
		pi_hash_clear(&system->shader_map[i], FALSE);
	}
}

void PI_API pi_shadersystem_remove_shader_source(PiRenderSystem *system, const wchar *key)
{
	char *str_key;
	PiKeyValue old;
	PI_ASSERT(system != NULL, "render system isn't init");
	str_key = pi_wstr_to_utf8(key);

	if (pi_hash_delete(&system->source_map, str_key, &old))
	{
		uint i;
		ShaderSource *source = (ShaderSource *)old.value;

		pi_free(source->version);
		for (i = 0; i < source->define_num; ++i)
		{
			pi_free(source->defines[i]);
		}
		for (i = 0; i < source->import_num; ++i)
		{
			pi_free(source->import_keys[i]);
		}

		pi_free(old.key);
		pi_free(old.value);
	}
	pi_free(str_key);
}

/**
 * 跳过空白字符
 */
static char *_skip_white_spaces(char *str)
{
	char *curr = str;
	while (pi_is_white_char(*curr))
	{
		++curr;
	}
	return curr;
}

/**
 * 从str开始读一个单词： 直到不含空白字符为止
 * 返回的字符不用时候注意用pi_free释放
 */
static char *_read_word(char *str, uint *len)
{
	uint size;
	char ch, *r, *curr = str;

	curr = _skip_white_spaces(str);
	if (len != NULL)
	{
		*len = (curr - str);
	}

	str = curr;
	while (!pi_is_white_char(*curr))
	{
		++curr;
	}

	ch = *curr;
	*curr = '\0';
	size = curr - str;
	r = (char *)pi_malloc(sizeof(char) * (size + 1));
	pi_memcpy_inline(r, str, size);
	r[size] = '\0';
	*curr = ch;
	if (len != NULL)
	{
		*len += size;
	}
	return r;
}

/**
 * 从str开始读取一行字符串返回，遇到换行符位置
 */
static char *_read_line(char *str, uint *len)
{
	uint size;
	char *r, *curr = str;

	curr = _skip_white_spaces(str);
	if (len != NULL)
	{
		*len = (curr - str);
	}

	str = curr;
	while (*curr != '\n' && *curr != '\r' && *curr != '\0')
	{
		++curr;
	}

	if (*curr == '\r')
	{
		++curr;
	}
	if (*curr == '\n')
	{
		++curr;
	}

	size = curr - str;
	r = (char *)pi_malloc(sizeof(char) * (size + 1));
	pi_memcpy_inline(r, str, size);
	r[size] = '\0';

	if (len != NULL)
	{
		*len += size;
	}
	return r;
}

/**
 * 判断一个单词是否为预处理宏：#ifdef, #ifndef, #if, #elif
 */
static PiBool _is_ifdef_macro(char *str, sint *index)
{
	PiBool r = FALSE;
	char *macro[] = {"#ifdef", "#ifndef", "#if", "#elif"};
	uint i, len = sizeof(macro) / sizeof(macro[0]);

	for (i = 0; i < len; ++i)
	{
		if (pi_str_start_with(&str[*index], macro[i]))
		{
			r = TRUE;
			*index += pi_strlen(macro[i]);
			break;
		}
	}
	if (i == len)
	{
		*index = *index + 1;
	}
	return r;
}


PiBool PI_API pi_shadersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type)
{
	return pi_shader_create_from_buffer(key, data, size, type);
}


PiBool PI_API pi_shadersystem_add_shader_source(PiRenderSystem *system, const wchar *key, byte *data, uint size)
{
	sint curr = 0;

	PiKeyValue old;
	ShaderSource *source = pi_new0(ShaderSource, 1);
	PI_ASSERT(system != NULL, "render system isn't init");

	source->source_key = pi_wstr_to_utf8(key);
	source->source_value = pi_malloc0(size + 1);
	pi_memcpy_inline(source->source_value, data, size);
	source->source_value[size] = '\0';

	curr = pi_str_text_index(&source->source_value[curr], "#version ");
	if (curr >= 0)
	{
		/* 对version的处理 */
		uint len = 0;
		source->version = _read_line(&source->source_value[curr], &len);

		/* 将#version 单词 全部变成空格 */
		pi_memset_inline(&source->source_value[curr], ' ', len);
	}

	curr = 0;
	curr = pi_str_text_index(&source->source_value[curr], "#include ");
	while (curr >= 0)
	{
		/* 对include的处理 */
		sint tmp;
		uint len = 0, size = pi_strlen("#include ");
		source->import_keys[source->import_num++] = _read_word(&source->source_value[curr + size], &len);
		size += (len + 1);

		/* 将include 单词 全部变成空格 */
		pi_memset_inline(&source->source_value[curr], ' ', size);

		curr += size;
		tmp = pi_str_text_index(&source->source_value[curr], "#include ");
		if (tmp < 0)
		{
			curr = tmp;
		}
		else
		{
			curr += tmp;
		}
	}

	curr = 0;
	curr = pi_str_char_index(&source->source_value[curr], '#');
	while (curr >= 0)
	{
		/* 对预定义宏的处理：#ifdef, #ifndef, #if, #elif */
		sint s;
		if (_is_ifdef_macro(source->source_value, &curr))
		{
			uint i;
			char *p = _read_word(&source->source_value[curr], NULL);
			uint len_p = pi_strlen(p);
			uint len_s = sizeof(char) * (len_p + 1);
			char *s = (char *)pi_malloc0(len_s);
			pi_strcpy(s, p, len_p);

			for (i = 0; i < source->define_num; ++i)
			{
				if (pi_str_equal(s, source->defines[i], FALSE))
				{
					break;
				}
			}
			if (i == source->define_num)
			{
				source->defines[source->define_num++] = s;
			}
			else
			{
				pi_free(s);
			}

			pi_free(p);
		}
		s = pi_str_char_index(&source->source_value[curr], '#');
		if (s >= 0)
		{
			curr += s;
		}
		else
		{
			curr = -1;
		}
	}

	if (pi_hash_enter(&system->source_map, source->source_key, source, &old))
	{
		uint i;
		ShaderSource *old_source = (ShaderSource *)old.value;
		for (i = 0; i < old_source->define_num; ++i)
		{
			pi_free(old_source->defines[i]);
		}
		for (i = 0; i < old_source->import_num; ++i)
		{
			pi_free(old_source->import_keys[i]);
		}

		pi_free(old.key);
		pi_free(old.value);
	}
	return TRUE;
}