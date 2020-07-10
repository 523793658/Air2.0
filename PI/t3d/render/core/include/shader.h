#ifndef INCLUDE_SHADER_H
#define INCLUDE_SHADER_H

#include <pi_lib.h>

/* 着色器宏数量的最大值 */
#define MAX_DEFINE_NUM 64

/* 着色器最多的uniform的值*/
#define MAX_UNIFORMS_NUM 60

/* 着色器import的最大数量  */
#define MAX_IMPORT_NUM 16

typedef enum
{
	ST_VS,
	ST_PS,
	ST_NUM
}ShaderType;

typedef enum
{
	RS_FLOAT,
	RS_INT
}RegisterSet;

typedef enum
{
	DUT_NONE,
	DUT_WORLD_MATRIX,
	DUT_PROJ_MATRIX,
	DUT_WORLD_VIEW_MATRIX,
	DUT_VIEW_PROJ_MATRIX,
	DUT_WORLD_VIEW_PROJ_MATRIX,
	DUT_NORMAL_MATRIX,
	DUT_VIEW_NORMAL_MATRIX,
	DUT_ALPHA_CULLOFF,
	DUT_VIEWPORT_SIZE,
	DUT_VIEW_MATRIX,
	DUT_VIEW_POSITION,
	DUT_ENVIRONMENT,
	DUT_TIME,
	DUT_SHADOW_DATA,
	DUT_SHADOW_MAP,
}DefaultUniformType;

typedef enum
{
	UT_UNKOWN = 0,
	UT_FLOAT = 1,
	UT_VEC2,
	UT_VEC3,
	UT_VEC4,

	UT_INT = 5,
	UT_IVEC2,
	UT_IVEC3,
	UT_IVEC4,

	UT_MATRIX2,
	UT_MATRIX3,
	UT_MATRIX4,
	UT_MATRIX4x3,

	UT_ISTRUCT,
	UT_STRUCT,
	
	UT_SAMPLER_1D,
	UT_SAMPLER_2D,
	UT_SAMPLER_3D,
	UT_SAMPLER_CUBE,
	UT_SAMPLER_1D_ARRAY,
	UT_SAMPLER_2D_ARRAY,
	UT_SAMPLER_2D_SHADOW,
	UT_UNSIGNED_INT_SAMPLER_2D,
}UniformType;

typedef struct
{
	char *name;
	
	UniformType type;
	
	void *value;
	uint size;			/* 大小*/
	uint32 count;		/* num大于1表示数组 */
	
	PiBool is_copy;
	void* reference_uniform;		/* 只有Material中的Uniform才有参考Uniform */

	DefaultUniformType d_type;

	void *impl;
	PiBool is_dirty;	/* 设置时候要更新 */
	PiBool is_packed;
}Uniform;

typedef struct
{
	char *version;		/* 版本号 */

	char *source_key;	/* 源码键 */
	
	char *source_value;

	uint define_num;
	
	/* 每个字符串带#define */
	char *defines[MAX_DEFINE_NUM];
	
	uint import_num;
	char *import_keys[MAX_IMPORT_NUM];
}ShaderSource;

typedef struct
{
	void *impl;
	ShaderType type;
}Shader;

#define MAX_BIND_NUM 16

typedef struct
{
	void *impl;
	void* bind_data[MAX_BIND_NUM];
}GpuProgram;

/* shader数据结构 */
typedef struct  
{
	char *key;		/* 键的字符串，其值为："source_key + define对应的索引 */
	
	sint ref_count;		/* 引用计数 */
	ShaderType type;	/* shader类型 */	
	Shader *shader;		/* shader指针 */
}ShaderData;

typedef struct
{
	void *program;
	ShaderData *vs;
	ShaderData *fs;
}ProgramData;

PI_BEGIN_DECLS

/* 从key和defines声明找到对应的shader指针，内部引用计数增1 */
ShaderData* PI_API shader_get(ShaderType type, const char *key, uint num_defines, const char **defines);

/* 释放shader对应的引用计数，引用为0则删除shader，以及链接该shader的着色器 */
void PI_API shader_release(ShaderData *shader);

/* 通过vs和fs找到对应的program */
void* PI_API program_get(ShaderData *vs, ShaderData *fs);

/* 在program获取和param同名的参数返回，如果找不到，创建一个返回 */
void PI_API program_set_uniform(void *program, Uniform *uniform, Uniform* material_uniform);

PiDvector* PI_API program_get_uniforms(void *program, PiBool is_global);

/* 清空所有的uniforms */
PiBool PI_API program_clear_uniforms(void *program);

/* 最多16个槽 */
void PI_API program_set_bind(void *program, uint index, void *bind);

void* PI_API program_get_bind(void *program, uint index);

/* 用户定义uniform的操作 */

void PI_API uniform_init(Uniform *u, char *name, UniformType type, uint count);

void PI_API uniform_clear(Uniform *u);

void PI_API uniform_update(Uniform *u, void *data);

char* PI_API create_shader_key(const char* key, uint num_defines, const char **defines, ShaderSource *value);

uint PI_API pi_shader_compile_offline(ShaderType type, const char *src_data, uint src_data_size, uint num_defines, const char *const *defines, void** buffer);

PiBool PI_API pi_shader_create_from_buffer(const char *key, byte *data, uint size, ShaderType type);

void PI_API pi_shader_save(PiVector* shader_list, wchar* base_path);

PI_END_DECLS

#endif /* INCLUDE_SHADER_H */