#ifndef INCLUDE_GL_SHADER_H
#define INCLUDE_GL_SHADER_H

#include <shader.h>
#include <rendersystem.h>
#include <renderlayout.h>

/* 着色器的属性变量 */
extern char *g_attrib_name[];

typedef struct  
{
	uint id;
	uint gl_type;
}GLShader;

typedef struct  
{
	uint id;
	PiBool is_link;			/* 是否已经链接过了 */
	GLShader *shaders[ST_NUM];
	
	PiDvector uniforms;			/* 元素为Uniform */
	PiDvector global_uniforms;	/* 元素为Uniform */

	PiBool is_attrib_enable[EVS_NUM];	/* 每个流的索引 */
	
	uint sampler_num;
	Uniform *samplers[SAMPLER_STATE_NUM];	/* 采样器指针 */
}GLProgram;

typedef struct  
{
	uint gl_type;
	sint location;
	sint sampler_id;	/* 仅用于sampler，绑定的纹理单元，初始化为-1 */
}GLUniform;

PI_BEGIN_DECLS

/* 以下接口供opengl调用 */
PiBool PI_API gl_program_set_gluniforms(GpuProgram *program);

PI_END_DECLS

#endif /* INCLUDE_GL_SHADER_H */