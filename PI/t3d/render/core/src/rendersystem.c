#include <rendersystem.h>
#include <renderwrap.h>
#include <shadersystem.h>
#include "vector4.h"

/* 全局变量，渲染系统实例指针 */
PiRenderSystem *g_rsystem = NULL;

void PI_API pi_rendersystem_clear(void)
{
	render_system_clear(g_rsystem);

	pi_shadersystem_clear(g_rsystem);
	
	pi_free(g_rsystem);
	g_rsystem = NULL;
}

PiBool PI_API pi_rendersystem_reset(PiBool is_window, uint width, uint height)
{
	if (render_system_resize_check(g_rsystem, is_window, width, height))
	{
		render_system_reset(g_rsystem, is_window, width, height);
		return TRUE;
	}
	return FALSE;
}

RenderCheckType PI_API pi_rendersystem_check(void)
{
	return render_system_check(g_rsystem);
}

PiBool PI_API pi_rendersystem_begin_draw(void)
{
	return render_system_begin(g_rsystem);
}

PiBool PI_API pi_rendersystem_end_draw(void)
{
	return render_system_end(g_rsystem);
}

PiRenderTarget* PI_API pi_rendersystem_init(RenderContextLoadType type, uint width, uint height, void *data, PiRenderContextParams* params)
{
	PiRenderTarget *r = NULL;
	
	g_rsystem = pi_new0(PiRenderSystem, 1);
	if (params)
	{
		pi_memcpy_inline(&g_rsystem->contextParams, params, sizeof(PiRenderContextParams));
	}
	
	g_rsystem->type = type;

	pi_mat4_set_identity(&g_rsystem->view_mat);
	pi_mat4_set_identity(&g_rsystem->proj_mat);

	g_rsystem->gdv.g_view = &g_rsystem->view_mat;
	g_rsystem->gdv.g_proj = &g_rsystem->proj_mat;

	g_rsystem->view_position = &g_rsystem->gdv.g_viewPosition;

	pi_renderstate_set_default_sampler(&g_rsystem->gdv.g_ShadowData.texture);
	pi_sampler_set_addr_mode(&g_rsystem->gdv.g_ShadowData.texture, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_filter(&g_rsystem->gdv.g_ShadowData.texture, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_compare_func(&g_rsystem->gdv.g_ShadowData.texture, CF_LESSEQUAL);
	
	pi_shadersystem_init(g_rsystem);

	if(render_system_init(g_rsystem, type, width, height, data))
	{
		r = &g_rsystem->main_target;

		pi_renderstate_set_default_rasterizer(&g_rsystem->rs);
		pi_renderstate_set_default_blend(&g_rsystem->bs);
		pi_renderstate_set_default_depthstencil(&g_rsystem->dss);
	}
	g_rsystem->frame = 0;
	return r;
}

RenderContextLoadType PI_API pi_rendersystem_get_type(void)
{
	return g_rsystem->type;
}

PiRenderSystem* PI_API pi_rendersystem_get_instance(void)
{
	return g_rsystem;
}

PiBool PI_API pi_rendersystem_set_target(PiRenderTarget* rt)
{
	return render_system_set_target(g_rsystem, rt);
}

PiBool PI_API pi_rendersystem_set_program(void *program)
{
	return render_system_set_program(g_rsystem, program);
}

PiBool PI_API pi_rendersystem_draw(PiEntity *entity, uint num)
{
	return render_system_draw(g_rsystem, entity, num);
}

void PI_API pi_rendersystem_swapbuffer()
{
	if (g_rsystem == NULL)
	{
		return;
	}
	++g_rsystem->frame;
	render_system_swapbuffer(g_rsystem);
}

uint PI_API pi_rendersystem_get_frame()
{
	return g_rsystem->frame;
}

PiRenderCap* PI_API pi_rendersystem_get_cap()
{
	return &g_rsystem->cap;
}

PiBool PI_API pi_rendersystem_clearview(uint32 flags, PiColor *clr, float depth, sint stencil)
{
	return render_system_clearview(g_rsystem, flags, clr, depth, stencil);
}

PiBool PI_API pi_rendersystem_set_camera_data(PiVector3 *location, PiMatrix4 *view_mat, PiMatrix4 *proj_mat)
{
	if(g_rsystem != NULL)
	{
		pi_vec3_copy(g_rsystem->view_position, location);

		g_rsystem->gdv.wvp_mask &= ~WVP_WORLD_VIEW;
		g_rsystem->gdv.wvp_mask &= ~WVP_VIEW_PROJ;
		g_rsystem->gdv.wvp_mask &= ~WVP_VIEW_NORMAL;
		g_rsystem->gdv.wvp_mask &= ~WVP_WORLD_VIEW_PROJ;
		pi_mat4_copy(&g_rsystem->view_mat, view_mat);

		g_rsystem->gdv.wvp_mask &= ~WVP_VIEW_PROJ;
		g_rsystem->gdv.wvp_mask &= ~WVP_WORLD_VIEW_PROJ;
		pi_mat4_copy(&g_rsystem->proj_mat, proj_mat);
	}
	return TRUE;
}

PiBool PI_API pi_rendersystem_set_camera(PiCamera *camera)
{
	PiVector3 *location = pi_camera_get_location(camera);
	PiMatrix4 *view_mat = pi_camera_get_view_matrix(camera);
	PiMatrix4 *proj_mat = pi_camera_get_projection_matrix(camera);
	pi_mat4_mul(&g_rsystem->view_proj_mat, proj_mat, view_mat);
	return pi_rendersystem_set_camera_data(location, view_mat, proj_mat);
}

PiBool PI_API pi_rendersystem_set_frame_time(float ms)
{
	if(g_rsystem != NULL)
	{
		g_rsystem->gdv.g_Time = ms;
	}
	return TRUE;
}


/* 设置着色器的源码内容 */
PiBool PI_API pi_rendersystem_add_shader_source(const wchar *key, byte *data, uint size)
{
	return pi_shadersystem_add_shader_source(g_rsystem, key, data, size);
}

PiBool PI_API pi_rendersystem_add_compiled_shader(const char *key, byte *data, uint size, ShaderType type)
{
	return pi_shadersystem_add_compiled_shader(key, data, size, type);
}

/* 移除着色器的源码内容 */
void PI_API pi_rendersystem_remove_shader_source(const wchar *key)
{
	pi_shadersystem_remove_shader_source(g_rsystem, key);
}

uint PI_API pi_rendersystem_get_device_vendor()
{
	return rendersystem_get_device_vendor(g_rsystem);
}