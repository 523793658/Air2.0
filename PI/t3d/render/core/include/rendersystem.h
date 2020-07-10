#ifndef INCLUDE_RENDERSYSTEM_H
#define INCLUDE_RENDERSYSTEM_H

#include <pi_lib.h>

#include <renderview.h>
#include <rendercap.h>
#include <renderstate.h>
#include <rendertarget.h>
#include <renderlayout.h>

#include <shader.h>
#include <texture.h>
#include <entity.h>

/* ������Ⱦ����������*/
typedef enum 
{
	CONTEXT_LOAD_EMPTY,
	CONTEXT_LOAD_NATIVE	
}RenderContextLoadType;

/* ��Ⱦ���ķ���ֵ */
typedef enum
{
	CHECK_LOST,
	CHECK_RUNNING,
	CHECK_REST
}RenderCheckType;

typedef struct PiRenderContextParams
{
	MULTISAMPLE_TYPE multiSampleType;
	uint16 multiSampleQuality;
}PiRenderContextParams;

/**
 * ��Ⱦϵͳ����Ⱦ�߳�ֻ��һ��ʵ��
 */

typedef struct 
{
	/* ֡�� */
	uint frame;
	
	RenderContextLoadType type;

	/* ��ɫ��Ĭ�ϵ�uniform����, ��g_��ͷ */
	GDefaultVariable gdv;

	/* camera����Ϣ */
	PiVector3* view_position;
	PiMatrix4 view_mat;
	PiMatrix4 proj_mat;
	PiMatrix4 view_proj_mat;

	/* ��Ⱦ���ԣ��ɾ������Ⱦϵͳ�����ʼ�������� */
	PiRenderCap cap;
	
	BlendState bs;
	RasterizerState rs;
	DepthStencilState dss;
	
	PiHash source_map;				/* Ԫ��Ϊ ShaderSource���� source_key�� */
	PiHash shader_map[ST_NUM];		/* Ԫ��Ϊ ShaderData, �� key�� */
	PiHash program_map;				/* Ԫ��Ϊ Program���� <vs_id, fs_id> */

	PiRenderTarget main_target;

	PiRenderContextParams contextParams;

	void *impl;
}PiRenderSystem;

PI_BEGIN_DECLS

RenderContextLoadType PI_API pi_rendersystem_get_type(void);

PiRenderSystem* PI_API pi_rendersystem_get_instance(void);

PiRenderTarget* PI_API pi_rendersystem_init(RenderContextLoadType type, uint width, uint height, void *data, PiRenderContextParams* param);

void PI_API pi_rendersystem_clear(void);

RenderCheckType PI_API pi_rendersystem_check(void);

PiBool PI_API pi_rendersystem_begin_draw(void);

PiBool PI_API pi_rendersystem_end_draw(void);

/* ������Ⱦ���� */
PiBool PI_API pi_rendersystem_reset(PiBool is_window, uint width, uint height);

/* ������ɫ����Դ������ */
PiBool PI_API pi_rendersystem_add_shader_source(const wchar *key, byte *data, uint size);

/* ���Ԥ����shader*/
PiBool PI_API pi_rendersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type);


/* �Ƴ���ɫ����Դ������ */
void PI_API pi_rendersystem_remove_shader_source(const wchar *key);

/* ������ȾĿ����ӿ� */
PiBool PI_API pi_rendersystem_set_target(PiRenderTarget* rt);

/* ������ɫ������ */
PiBool PI_API pi_rendersystem_set_program(void *program);

/* ���������������C */
PiBool PI_API pi_rendersystem_set_camera(PiCamera *camera);

/* ����������ݣ�����JS */
PiBool PI_API pi_rendersystem_set_camera_data(PiVector3 *location, PiMatrix4 *view_mat, PiMatrix4 *proj_mat);

/* ������һ֡����Ⱦʱ�� */

PiBool PI_API pi_rendersystem_set_frame_time(float ms);

/* ���õ�ǰ��Ⱦ״̬��Ⱦ���� */
PiBool PI_API pi_rendersystem_draw(PiEntity *entity, uint num);

/* ���Ŀ��Ļ����� */
PiBool PI_API pi_rendersystem_clearview(uint32 flags, PiColor *clr, float depth, sint stencil);

/* ���������� */
void PI_API pi_rendersystem_swapbuffer();

/* ��õ�ǰ��֡�� */
uint PI_API pi_rendersystem_get_frame();

/* �����Ⱦ���Ե�ָ�� */
PiRenderCap* PI_API pi_rendersystem_get_cap(void);

uint PI_API pi_rendersystem_get_device_vendor();

PI_END_DECLS

#endif /* INCLUDE_RENDERSYSTEM_H */