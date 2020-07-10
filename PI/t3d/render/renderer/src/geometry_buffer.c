
#include <geometry_buffer.h>
#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <environment.h>
#include <camera.h>

/**
 * 生成g-buffer的渲染器
 */
typedef struct
{
	uint width;
	uint height;

	PiRenderTarget *g_buffer_rt;

	PiTexture *g_buffer_tex0;
	PiTexture *g_buffer_tex1;
	PiTexture *g_buffer_tex2;
	PiTexture *g_buffer_tex3;
	PiTexture *g_buffer_depth;

	PiRenderView *g_buffer_tex0_view;
	PiRenderView *g_buffer_tex1_view;
	PiRenderView *g_buffer_tex2_view;
	PiRenderView *g_buffer_tex3_view;
	PiRenderView *g_buffer_depth_view;

	PiBool is_deploy;
	PiBool is_init;

	char *target_name;
	char *view_cam_name;
	char *entity_list_name;

	char *g_buffer_tex0_name;
	char *g_buffer_tex1_name;
	char *g_buffer_tex2_name;
	char *g_buffer_tex3_name;
	char *g_buffer_depth_name;
} GeometryBufferingRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_GEOMETRY_BUFFER, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	GeometryBufferingRenderer *impl = (GeometryBufferingRenderer *)renderer->impl;
	PiRenderTarget *target = NULL;
	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->width = target->width;
	impl->height = target->height;

	impl->g_buffer_rt = pi_rendertarget_new(TT_MRT, TRUE);

	impl->g_buffer_tex0 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->g_buffer_tex1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->g_buffer_tex2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->g_buffer_tex3 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->g_buffer_depth = pi_texture_2d_create(RF_D24S8, TU_DEPTH_STENCIL, 1, 1, impl->width, impl->height, TRUE);

	impl->g_buffer_tex0_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex0, 0, 0, TRUE);
	impl->g_buffer_tex1_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex1, 0, 0, TRUE);
	impl->g_buffer_tex2_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex2, 0, 0, TRUE);
	impl->g_buffer_tex3_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex3, 0, 0, TRUE);
	impl->g_buffer_depth_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->g_buffer_depth, 0, 0, TRUE);

	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR0, impl->g_buffer_tex0_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR1, impl->g_buffer_tex1_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR2, impl->g_buffer_tex2_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR3, impl->g_buffer_tex3_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_DEPTHSTENCIL, impl->g_buffer_depth_view);

	pi_rendertarget_set_viewport(impl->g_buffer_rt, 0, 0, impl->width, impl->height);

	impl->is_init = TRUE;

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	GeometryBufferingRenderer *impl = (GeometryBufferingRenderer *)renderer->impl;
	PiRenderTarget *target = impl->g_buffer_rt;
	PiCamera *view_camera;
	PiVector *entity_list;
	PiColor background;
	PI_USE_PARAM(tpf);
	_type_check(renderer);

	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);

	color_set(&background, 0.0f, 0.0f, 0.0f, 1.0f);
	pi_rendersystem_set_target(target);
	pi_rendersystem_clearview(TBM_ALL, &background, 1.0f, 0);
	pi_rendersystem_set_camera(view_camera);
	pi_entity_draw_list(entity_list);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	GeometryBufferingRenderer *impl;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (GeometryBufferingRenderer *)renderer->impl;

	pi_hash_enter(resources, impl->g_buffer_tex0_name, impl->g_buffer_tex0, NULL);
	pi_hash_enter(resources, impl->g_buffer_tex1_name, impl->g_buffer_tex1, NULL);
	pi_hash_enter(resources, impl->g_buffer_tex2_name, impl->g_buffer_tex2, NULL);
	pi_hash_enter(resources, impl->g_buffer_tex3_name, impl->g_buffer_tex3, NULL);
	pi_hash_enter(resources, impl->g_buffer_depth_name, impl->g_buffer_depth, NULL);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	GeometryBufferingRenderer *impl;
	_type_check(renderer);
	impl = (GeometryBufferingRenderer *)renderer->impl;

	impl->width = width;
	impl->height = height;

	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR0);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR1);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR2);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR3);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_DEPTHSTENCIL);

	pi_renderview_free(impl->g_buffer_tex0_view);
	pi_renderview_free(impl->g_buffer_tex1_view);
	pi_renderview_free(impl->g_buffer_tex2_view);
	pi_renderview_free(impl->g_buffer_tex3_view);
	pi_renderview_free(impl->g_buffer_depth_view);

	pi_texture_free(impl->g_buffer_tex0);
	pi_texture_free(impl->g_buffer_tex1);
	pi_texture_free(impl->g_buffer_tex2);
	pi_texture_free(impl->g_buffer_tex3);
	pi_texture_free(impl->g_buffer_depth);

	impl->g_buffer_tex0 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->g_buffer_tex1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->g_buffer_tex2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->g_buffer_tex3 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->g_buffer_depth = pi_texture_2d_create(RF_D24S8, TU_DEPTH_STENCIL, 1, 1, width, height, TRUE);

	impl->g_buffer_tex0_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex0, 0, 0, TRUE);
	impl->g_buffer_tex1_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex1, 0, 0, TRUE);
	impl->g_buffer_tex2_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex2, 0, 0, TRUE);
	impl->g_buffer_tex3_view = pi_renderview_new_tex2d(RVT_COLOR, impl->g_buffer_tex3, 0, 0, TRUE);
	impl->g_buffer_depth_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->g_buffer_depth, 0, 0, TRUE);

	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR0, impl->g_buffer_tex0_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR1, impl->g_buffer_tex1_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR2, impl->g_buffer_tex2_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_COLOR3, impl->g_buffer_tex3_view);
	pi_rendertarget_attach(impl->g_buffer_rt, ATT_DEPTHSTENCIL, impl->g_buffer_depth_view);

	pi_rendertarget_set_viewport(impl->g_buffer_rt, 0, 0, width, height);
}

PiRenderer *PI_API pi_geometry_buffer_new()
{
	PiRenderer *renderer;
	GeometryBufferingRenderer *impl = pi_new0(GeometryBufferingRenderer, 1);
	renderer = pi_renderer_create(ERT_GEOMETRY_BUFFER, "geometry buffer renderer", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_geometry_buffer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name)
{
	GeometryBufferingRenderer *impl;
	_type_check(renderer);
	impl = (GeometryBufferingRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);

	pi_free(impl->g_buffer_tex0_name);
	pi_free(impl->g_buffer_tex1_name);
	pi_free(impl->g_buffer_tex2_name);
	pi_free(impl->g_buffer_tex3_name);
	pi_free(impl->g_buffer_depth_name);

	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);

	impl->g_buffer_tex0_name = pi_str_dup(g_buffer_tex0_name);
	impl->g_buffer_tex1_name = pi_str_dup(g_buffer_tex1_name);
	impl->g_buffer_tex2_name = pi_str_dup(g_buffer_tex2_name);
	impl->g_buffer_tex3_name = pi_str_dup(g_buffer_tex3_name);
	impl->g_buffer_depth_name = pi_str_dup(g_buffer_depth_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_geometry_buffer_free(PiRenderer *renderer)
{
	GeometryBufferingRenderer *impl;
	_type_check(renderer);
	impl = (GeometryBufferingRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);

	pi_free(impl->g_buffer_tex0_name);
	pi_free(impl->g_buffer_tex1_name);
	pi_free(impl->g_buffer_tex2_name);
	pi_free(impl->g_buffer_tex3_name);
	pi_free(impl->g_buffer_depth_name);

	if (!impl->is_init)
	{
		return;
	}

	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR0);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR1);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR2);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_COLOR3);
	pi_rendertarget_detach(impl->g_buffer_rt, ATT_DEPTHSTENCIL);

	pi_renderview_free(impl->g_buffer_tex0_view);
	pi_renderview_free(impl->g_buffer_tex1_view);
	pi_renderview_free(impl->g_buffer_tex2_view);
	pi_renderview_free(impl->g_buffer_tex3_view);
	pi_renderview_free(impl->g_buffer_depth_view);

	pi_texture_free(impl->g_buffer_tex0);
	pi_texture_free(impl->g_buffer_tex1);
	pi_texture_free(impl->g_buffer_tex2);
	pi_texture_free(impl->g_buffer_tex3);
	pi_texture_free(impl->g_buffer_depth);

	pi_rendertarget_free(impl->g_buffer_rt);

	pi_free(impl);

	pi_renderer_destroy(renderer);
}
