#include "shader_compiler.h"
#include "rendersystem.h"
#include "shader.h"
#include "cJSON.h"
#include "load.h"
#include "upload.h"

void PI_API app_shader_save(char* ftp_url, char* ftp_user_name, char* ftp_password, char* shader_version)
{
	PiVector* shader_list = pi_vector_new();
	void* task = create_upload_task(ftp_url, ftp_user_name, ftp_password, TRUE);
	uint i, size;
	char romote_path[255];
	char file_path[255];
	wchar real_path[255];
	pi_strcpy(romote_path, "shader/", pi_strlen("shader/"));
	pi_str_cat(romote_path, 255, shader_version);

	pi_vfs_get_real_path(real_path, L"/$root/user/temp/shader");

	pi_file_create_path(real_path);

	pi_shader_save(shader_list, L"/$root/user/temp/shader");
	

	size = pi_vector_size(shader_list);

	if (size == 0)
	{
		pi_free(shader_list);
		free_upload_task(task);
		return;
	}


	for (i = 0; i < size; i++)
	{
		char* key = pi_vector_get(shader_list, i);
		pi_strcpy(file_path, "user/temp/shader/", pi_strlen("user/temp/shader/"));
		pi_str_cat(file_path, 255, key);
		pi_free(key);
		add_file_to_upload_task(task, file_path, romote_path, NULL);
	}
	pi_free(shader_list);
	upload_task_start(task);
	free_upload_task(task);

	
}


static void saveData(wchar_t* name, void* buffer, int size, wchar_t* dir)
{
	wchar_t* path;
	int length = pi_wstrlen(dir) + pi_wstrlen(name) + 2;
	path = pi_new0(wchar_t, length);
	pi_wstrcpy(path, dir, pi_wstrlen(dir));
	pi_wstr_cat(path, length, L"/");
	pi_wstr_cat(path, length, name);
	pi_vfs_file_delete(path);
	pi_vfs_create_path(dir);
	pi_vfs_file_write_data(path, buffer, size);
	pi_free(path);
}

void PI_API app_compile_shader(char* cfgPath, wchar* outputPath)
{
	int size;
	void* data;
	cJSON* json;
	int i, j;
	PiRenderSystem* system = pi_rendersystem_get_instance();
	data = pi_load_file(cfgPath, &size);
	json = cJSON_Parse(data);
	size = cJSON_GetArraySize(json);
	for (i = 0; i < size; i++)
	{
		char* key;
		cJSON* oneShader = cJSON_GetArrayItem(json, i);
		cJSON* nameJSON = cJSON_GetObjectItem(oneShader, "shader");
		char* name = nameJSON->valuestring;
		cJSON* defsJSON = cJSON_GetObjectItem(oneShader, "defs");
		cJSON* typeObj = cJSON_GetObjectItem(oneShader, "type");
		ShaderSource* source;
		int bufferSize;
		void* buffer;
		char* defs[MAX_DEFINE_NUM];
		int num_def;
		num_def = cJSON_GetArraySize(defsJSON);
		pi_hash_lookup(&system->source_map, name, &source);
		for (j = 0; j < num_def; j++)
		{
			defs[j] = cJSON_GetArrayItem(defsJSON, j)->valuestring;
		}
		bufferSize = pi_shader_compile_offline(typeObj->valueint, source->source_value, pi_strlen(source->source_value), num_def, defs, &buffer);
		key = create_shader_key(name, num_def, defs, source);
		if (bufferSize != 0){
			saveData(pi_str_to_wstr(key, PI_CP_UTF8), buffer, bufferSize, outputPath);
		}
	}
}

CompileResult* PI_API app_compiler_init(PiMaterial* material)
{
	CompileResult* result;
	ShaderSource* source;
	PiBool is_lookup;
	PiRenderSystem* system = pi_rendersystem_get_instance();
	result = pi_new0(CompileResult, 1);
	is_lookup = pi_hash_lookup(&system->source_map, material->vs_key, &source);
	if (is_lookup)
	{
		result->vsShader = TRUE;
		result->vsShaderKey = create_shader_key(material->vs_key, material->num_defs, material->def_names, source);
	}
	is_lookup = pi_hash_lookup(&system->source_map, material->fs_key, &source);
	if (is_lookup)
	{
		result->fsShader= TRUE;
		result->fsShaderKey = create_shader_key(material->fs_key, material->num_defs, material->def_names, source);
	}
	result->material = material;
	return result;
}

void PI_API app_compiler_set_params(CompileResult* result, PiBool createVs, PiBool createFs)
{
	result->vsShader = createVs;
	result->fsShader = createFs;
}

void PI_API app_compiler_start(CompileResult* result)
{
	ShaderSource* source;
	PiRenderSystem* system = pi_rendersystem_get_instance();
	
	if (result->fsShader)
	{
		//±àÒëÏñËØ×ÅÉ«Æ÷
		pi_hash_lookup(&system->source_map, result->material->fs_key, &source);
		void* buffer;
		result->fsBufferSize = pi_shader_compile_offline(ST_PS, source->source_value, pi_strlen(source->source_value), result->material->num_defs, result->material->defs, &buffer);
		result->fsBuffer = buffer;

	}
	if (result->vsShader)
	{
		//±àÒë¶¥µã×ÅÉ«Æ÷
		pi_hash_lookup(&system->source_map, result->material->vs_key, &source);
		void* buffer;
		result->vsBufferSize = pi_shader_compile_offline(ST_VS, source->source_value, pi_strlen(source->source_value), result->material->num_defs, result->material->defs, &buffer);
		result->vsBuffer = buffer;
	}
}

PiBool PI_API app_compiler_is_vs_enable(CompileResult* result)
{
	return result->vsShader;
}
char* PI_API app_compiler_get_vs_key(CompileResult* result)
{
	return pi_str_dup(result->vsShaderKey);
}
uint PI_API app_compiler_get_vs_buffer_size(CompileResult* result)
{
	return result->vsBufferSize;
}
void* PI_API app_compiler_get_vs_buffer_pointer(CompileResult* result)
{
	return result->vsBuffer;
}


PiBool PI_API app_compiler_is_fs_enable(CompileResult* result)
{
	return result->fsShader;
}

char* PI_API app_compiler_get_fs_key(CompileResult* result)
{
	return pi_str_dup(result->fsShaderKey);
}
uint PI_API app_compiler_get_fs_buffer_size(CompileResult* result)
{
	return result->fsBufferSize;
}
void* PI_API app_compiler_get_fs_buffer_pointer(CompileResult* result)
{
	return result->fsBuffer;
}

void PI_API app_compiler_free_result(CompileResult* result)
{
	if (result->vsShaderKey)
	{
		pi_free(result->vsShaderKey);
	}

	if (result->fsShaderKey)
	{
		pi_free(result->fsShaderKey);
	}
	if (result->vsShader)
	{
		pi_free(result->vsBuffer);
	}
	if (result->fsShader)
	{
		pi_free(result->fsBuffer);
	}
	pi_free(result);
}


static PiSelectR PI_API _findKey(void *user_data, PiKeyValue *kv)
{
	cJSON* temp = (cJSON*)user_data;
	cJSON* root = cJSON_GetObjectItem(temp, "root");
	cJSON* keys = cJSON_GetObjectItem(temp, "keys");
	cJSON* oneShader;
	cJSON* shaderNameobj;
	cJSON* defsObj;
	cJSON* typeObj;

	PiRenderSystem* system = pi_rendersystem_get_instance();
	ShaderSource* source;
	ShaderData* shader = (ShaderData*)kv->value;
	char *defines[MAX_DEFINE_NUM];
	PiBool is_lookup;
	int num_defs, i, def_count = 0;
	uint64 define_key;
	char* key = (char*)kv->key;
	char* shaderName = pi_str_dup(key + 20);
	char* number = pi_str_dup(key);
	number[20] = '\0';
	char* number1 = pi_malloc0(21);
	pi_str_cat(number1, 21, number);
	pi_free(number);
	pi_str_parse_number(number1, &define_key);
	pi_free(number1);
	is_lookup = pi_hash_lookup(&system->source_map, shaderName, &source);

	if (!is_lookup)
	{
		pi_free(shaderName);
		return SELECT_NEXT;
	}
	oneShader = cJSON_CreateObject();
	shaderNameobj = cJSON_CreateString(shaderName);
	typeObj = cJSON_CreateNumber(shader->type);


	pi_free(shaderName);
	num_defs = source->define_num;
	for (i = 0; i < num_defs; i++)
	{
		if ((define_key >> i) % 2)
		{
			defines[def_count] = source->defines[i];
			def_count++;
		}
	}
	key = create_shader_key(shaderNameobj->valuestring, def_count, defines, source);
	for (i = cJSON_GetArraySize(keys) - 1; i >= 0; i--)
	{
		if (pi_string_equal(key, cJSON_GetArrayItem(keys, i)->valuestring))
		{
			return SELECT_NEXT;
		}
	}


	defsObj = cJSON_CreateStringArray(defines, def_count);
	cJSON_AddItemToObject(oneShader, "shader", shaderNameobj);
	cJSON_AddItemToObject(oneShader, "defs", defsObj);
	cJSON_AddItemToObject(oneShader, "type", typeObj);
	cJSON_AddItemToArray(root, oneShader);
	cJSON_AddItemToArray(keys, cJSON_CreateString(key));
	pi_free(key);
	return SELECT_NEXT;
}


static PiSelectR PI_API _build_info(void *user_data, PiKeyValue *kv)
{
// 	cJSON* shader_info = (cJSON*)user_data;
// 	cJSON* item = cJSON_CreateObject();
// 	cJSON* key_obj;
// 	cJSON* defs_obj;
// 	cJSON* type_obj;
// 	int i;
// 	ShaderBuffer* buffer = kv->value;
// 	key_obj = cJSON_CreateString(buffer->key);
// 	defs_obj = cJSON_CreateStringArray(buffer->defs, buffer->num_defines);
// 	type_obj = cJSON_CreateNumber(buffer->type);
// 	cJSON_AddItemToObject(item, "shader", key_obj);
// 	cJSON_AddItemToObject(item, "defs", defs_obj);
// 	cJSON_AddItemToObject(item, "type", type_obj);
// 	cJSON_AddItemToArray(shader_info, item);
// 	pi_free(buffer->key);
// 	pi_free(buffer->shader_key);
// 	for (i = 0; i < buffer->num_defines; i++)
// 	{
// 		pi_free(buffer->defs[i]);
// 	}
// 	pi_free(buffer->defs);
// 	pi_free(buffer);
	return SELECT_NEXT;
}

void PI_API app_upload_shader_info()
{
// 	PiHash* shader_map = pi_shader_compile_buffer_save();
// 	cJSON* shader_info;
// 	if (pi_hash_size(shader_map) > 0)
// 	{
// 		char* str;
// 		shader_info = cJSON_CreateArray();
// 		pi_hash_foreach(shader_map, _build_info, shader_info);
// 		str = cJSON_Print(shader_info);
// 		saveData(L"compilerInfo.json", str, pi_strlen(str), L"log");
// 		pi_free(str);
// 		cJSON_free(shader_info);
// 		pi_hash_clear(shader_map, TRUE);
// 	}
}


void PI_API app_compiler_get_all_shader(wchar* output)
{
	int i;
	int length = pi_wstrlen(output) + 1 + pi_wstrlen(L"compilerInfo.json") + 1;
	int fileSize;
	int itemCount;
	wchar* outputPath = pi_new0(wchar, length);
	char* data;
	cJSON* temp;
	cJSON* json;
	cJSON* keys = cJSON_CreateArray();
	char* outputPaths;
	PiRenderSystem* system = pi_rendersystem_get_instance();
	temp = cJSON_CreateObject();
	cJSON_AddItemToObject(temp, "keys", keys);
	pi_wstr_cat(outputPath, length, output);
	pi_wstr_cat(outputPath, length, L"/compilerInfo.json");
	outputPaths = pi_wstr_to_str(outputPath, PI_CP_UTF8);
	data = pi_load_file(outputPaths, &fileSize);
	pi_free(outputPaths);
	pi_free(outputPath);
	if (fileSize > 0)
	{
		json = cJSON_Parse(data);
		itemCount = cJSON_GetArraySize(json);
		for (i = 0; i < itemCount; i++)
		{
			cJSON* item = cJSON_GetArrayItem(json, i);
			cJSON* defsObj = cJSON_GetObjectItem(item, "defs");
			char* shaderName = cJSON_GetObjectItem(item, "shader")->valuestring;
			ShaderSource* source;
			int num_def = cJSON_GetArraySize(defsObj);
			char** defs = pi_new0(char*, num_def);
			int j;
			for (j = 0; j < num_def; j++)
			{
				defs[j] = cJSON_GetArrayItem(defsObj, j)->valuestring;
			}
			pi_hash_lookup(&system->source_map, shaderName, &source);
			char* key = create_shader_key(cJSON_GetObjectItem(item, "shader")->valuestring, num_def, defs, source);
			cJSON_AddItemToArray(keys, cJSON_CreateString(key));
			pi_free(defs);
		}
	}
	else
	{
		json = cJSON_CreateArray();
	}
	cJSON_AddItemToObject(temp, "root", json);
	for (i = 0; i < ST_NUM; ++i)
	{
		pi_hash_foreach(&system->shader_map[i], _findKey, temp);
	}
	char* str = cJSON_Print(json);
	cJSON_Delete(temp);
	saveData(L"compilerInfo.json", str, pi_strlen(str), output);
}

