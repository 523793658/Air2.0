#include "d3d9_rendersystem.h"
#include "d3d9_context.h"
#include "d3d9_convert.h"
#include "d3d9_rendercap.h"
#include "d3d9_renderlayout.h"
#include "d3d9_renderstate.h"
#include "d3d9_renderview.h"
#include "d3d9_texture.h"
#include "renderinfo.h"
#include "renderwrap.h"

extern "C" {

	extern PiRenderSystem *g_rsystem;

	static void _init_main_target(PiRenderSystem *rs, D3D9RenderSystem *d3d9_system, uint width, uint height)
	{
		rs->main_target.left = 0;
		rs->main_target.bottom = 0;
		rs->main_target.width = width;
		rs->main_target.height = height;

		rs->main_target.type = TT_WIN;

		pi_rendertarget_attach(&rs->main_target, ATT_COLOR0, d3d9_system->state.back_buffer_view);
		pi_rendertarget_attach(&rs->main_target, ATT_DEPTHSTENCIL, d3d9_system->state.back_depth_stencil_view);

		d3d9_system->state.target = &rs->main_target;

		pi_rendertarget_set_viewport(&rs->main_target, 0, 0, width, height);
	}

	PiBool PI_API render_system_init(PiRenderSystem *rs, RenderContextLoadType type, uint width, uint height, void *data)
	{
		PiBool r = TRUE;
		D3D9RenderSystem *d3d9_system = pi_new0(D3D9RenderSystem, 1);

		d3d9_system->context = d3d9_context_new((HWND)data, TRUE, width, height);

		if (d3d9_system->context != NULL)
		{
			rs->impl = d3d9_system;

			d3d9_rendercap_init(&g_rsystem->cap);

			d3d9_state_init(&d3d9_system->state);

			_init_main_target(rs, d3d9_system, width, height);
			
		}
		else
		{
			r = FALSE;
			pi_free(d3d9_system);
		}
		return r;
	}

	PiBool PI_API render_system_clear(PiRenderSystem *rs)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		d3d9_state_clear(&d3d9_system->state);

		d3d9_context_free(d3d9_system->context);
		pi_free(rs->impl);
		return TRUE;
	}

	RenderCheckType PI_API render_system_check(PiRenderSystem *rs)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		D3D9Context *context = d3d9_system->context;

		HRESULT hr = IDirect3DDevice9_TestCooperativeLevel(context->device);
		
		if (hr != D3DERR_DEVICENOTRESET)
		{
			if (SUCCEEDED(hr))
			{
				return CHECK_RUNNING;
			}
			else
			{
				return CHECK_LOST;
			}
		}
		
		if (render_system_reset(rs, context->is_window_mode, context->width, context->height))
		{
			return CHECK_REST;
		}
		return CHECK_LOST;
	}

	PiBool PI_API render_system_resize_check(PiRenderSystem *rs, PiBool is_window, uint width, uint height)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		D3D9Context *context = d3d9_system->context;
		HRESULT hr = IDirect3DDevice9_TestCooperativeLevel(context->device);
		if (!SUCCEEDED(hr))
		{
			context->width = width;
			context->height = height;
			context->is_window_mode = is_window;
			return FALSE;
		}
		return TRUE;
	}

	PiBool PI_API render_system_reset(PiRenderSystem *rs, PiBool is_window, uint width, uint height)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		
		d3d9_state_before_reset();
		PiBool r = d3d9_context_reset(d3d9_system->context, is_window, width, height);
		d3d9_state_after_reset();

		return r;
	}

	PiBool PI_API render_system_set_program(PiRenderSystem *rs, GpuProgram *program)
	{
		PI_USE_PARAM(rs);
		return d3d9_state_use_shader(program);
	}

	PiBool PI_API render_system_draw(PiRenderSystem *rs, PiEntity *entity, uint num)
	{
		PiBool r = FALSE;
		pi_renderinfo_add_entity_num(1);
		pi_renderinfo_add_face_num(pi_mesh_get_face_num(entity->mesh->mesh));
		pi_renderinfo_add_vertex_num(pi_mesh_get_vertex_num(entity->mesh->mesh));
		if (entity->skinedData){
			r = d3d9_renderlayout_draw(entity->mesh, entity->skinedData->renderMesh, num);
		}
		else{
			r = d3d9_renderlayout_draw(entity->mesh, NULL, num);
		}
		return r;
	}

	void PI_API render_system_set_default_uniform(uint register_index, void* data, uint register_count, PiBool is_vs, PiBool is_ps)
	{
		d3d9_state_set_default_uniform(register_index, data, register_count, is_vs, is_ps);
	}

	void PI_API render_system_swapbuffer(PiRenderSystem *rs)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		PiBool x = d3d9_context_swapbuffer(d3d9_system->context);
		x = x;
	}

	PiBool PI_API render_system_set_target(PiRenderSystem *rs, PiRenderTarget *rt)
	{
		PiBool r = TRUE;
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)rs->impl;
		D3D9RenderState *state = &d3d9_system->state;

		if (rt == NULL)
		{
			rt = &rs->main_target;
		}

		if (state->target != rt || rt->is_update)
		{
			pi_rendertarget_unbind(state->target);
			pi_rendertarget_bind(rt);
			d3d9_state_set_viewport(rt->left, rt->bottom, rt->width, rt->height);
			state->target = rt;
		}

		return r;
	}

	PiBool PI_API render_system_clearview(PiRenderSystem *rs, uint32 flags, PiColor *clr, float depth, uint stencil)
	{
		uint d3d9_flags = 0;
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		D3DCOLOR color = D3DCOLOR_COLORVALUE(clr->rgba[0], clr->rgba[1], clr->rgba[2], clr->rgba[3]);

		if (flags & TBM_COLOR)
		{
			d3d9_flags |= D3DCLEAR_TARGET;
		}

		if (flags & TBM_DEPTH)
		{
			d3d9_flags |= D3DCLEAR_ZBUFFER;
		}
		if (flags & TBM_STENCIL)
		{
			d3d9_flags |= D3DCLEAR_STENCIL;
		}

		// 注：如果flags设了深度或者模板，但是目标没有深度缓冲或者模板缓冲，Clear会失败。
		IDirect3DDevice9_Clear(context->device, 0, NULL, d3d9_flags, color, depth, stencil);

		return TRUE;
	}

	PiBool PI_API render_system_begin(PiRenderSystem *rs)
	{
		D3D9RenderSystem *system = (D3D9RenderSystem *)rs->impl;
		HRESULT hr = IDirect3DDevice9_BeginScene(system->context->device);
		if (FAILED(hr))
		{
			pi_log_print(LOG_WARNING, "Begin Scene failed, hr = %d", hr);
		}
		return SUCCEEDED(hr);
	}

	PiBool PI_API render_system_end(PiRenderSystem *rs)
	{
		D3D9RenderSystem *system = (D3D9RenderSystem *)rs->impl;
		HRESULT hr = IDirect3DDevice9_EndScene(system->context->device);
		if (FAILED(hr))
		{
			pi_log_print(LOG_WARNING, "End Scene failed, hr = %d", hr);
		}
		return SUCCEEDED(hr);
	}

	uint PI_API rendersystem_get_device_vendor(PiRenderSystem *rs)
	{
		D3D9RenderSystem* system = (D3D9RenderSystem*)rs->impl;
		return system->context->vender;
	}
}
