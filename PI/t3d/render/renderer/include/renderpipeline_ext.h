#ifndef INCLUDE_RENDERPIPELINE_EXT_H
#define INCLUDE_RENDERPIPELINE_EXT_H

#include <pi_lib.h>
#include <renderer.h>
#include <rendertarget.h>

/**
 * ��Ⱦ����
 */
typedef struct 
 {
	 PiBool	is_resize;		/* �Ƿ���Ҫִ��resize���� */
	 uint width, height;	/* ִ��resize�����Ŀ��߲��� */
	 PiHash* global_resources;
	 PiHash* resources;
	 PiVector* renderers;
 } PiRenderPipelineExt;

PI_BEGIN_DECLS

PiRenderPipelineExt* PI_API pi_renderpipeline_ext_new();

void PI_API pi_renderpipeline_ext_free(PiRenderPipelineExt* pipeline);

void PI_API pi_renderpipeline_ext_resize(PiRenderPipelineExt* pipeline, uint width, uint height);

void PI_API pi_renderpipeline_ext_draw(PiRenderPipelineExt* pipeline, float tpf);

void PI_API pi_renderpipeline_ext_add(PiRenderPipelineExt* pipeline, PiRenderer *renderer);

void PI_API pi_renderpipeline_ext_remove(PiRenderPipelineExt* pipeline, PiRenderer *renderer);

void PI_API pi_renderpipeline_ext_add_global_resource(PiRenderPipelineExt* pipeline, char* name, void* resource);

void PI_API pi_renderpipeline_ext_remove_global_resource(PiRenderPipelineExt* pipeline, char* name);

PI_END_DECLS

#endif /* INCLUDE_RENDERPIPELINE_EXT_H */