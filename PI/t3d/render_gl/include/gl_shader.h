#ifndef INCLUDE_GL_SHADER_H
#define INCLUDE_GL_SHADER_H

#include <shader.h>
#include <rendersystem.h>
#include <renderlayout.h>

/* ��ɫ�������Ա��� */
extern char *g_attrib_name[];

typedef struct  
{
	uint id;
	uint gl_type;
}GLShader;

typedef struct  
{
	uint id;
	PiBool is_link;			/* �Ƿ��Ѿ����ӹ��� */
	GLShader *shaders[ST_NUM];
	
	PiDvector uniforms;			/* Ԫ��ΪUniform */
	PiDvector global_uniforms;	/* Ԫ��ΪUniform */

	PiBool is_attrib_enable[EVS_NUM];	/* ÿ���������� */
	
	uint sampler_num;
	Uniform *samplers[SAMPLER_STATE_NUM];	/* ������ָ�� */
}GLProgram;

typedef struct  
{
	uint gl_type;
	sint location;
	sint sampler_id;	/* ������sampler���󶨵�����Ԫ����ʼ��Ϊ-1 */
}GLUniform;

PI_BEGIN_DECLS

/* ���½ӿڹ�opengl���� */
PiBool PI_API gl_program_set_gluniforms(GpuProgram *program);

PI_END_DECLS

#endif /* INCLUDE_GL_SHADER_H */