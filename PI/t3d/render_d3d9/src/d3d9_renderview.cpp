#include "d3d9_renderview.h"
#include "d3d9_convert.h"
#include "d3d9_rendersystem.h"
#include "d3d9_texture.h"
#include "renderinfo.h"
#include "renderwrap.h"
typedef struct
{
	IDirect3DSurface9 *surface;
} D3D9RenderView;

extern "C" {

	extern PiRenderSystem *g_rsystem;

	PiRenderView *d3d9_new_main_view(IDirect3DSurface9 *back_buffer, 
		uint width, uint height, RenderFormat format, RenderViewType type)
	{
		PiRenderView *view = pi_renderview_new(type, width, height, format, FALSE);
		D3D9RenderView *impl = pi_new0(D3D9RenderView, 1);
				
		impl->surface = back_buffer;
		view->impl = impl;
		return view;
	}

	void d3d9_free_main_view(PiRenderView *view)
	{
		pi_free(view->impl);
		view->impl = NULL;
		pi_renderview_free(view);
	}

	PiBool d3d9_view_init(PiRenderView *view)
	{
		PiBool r = FALSE;
		D3D9RenderView *impl;
		D3D9Texture *texture_impl;
		IDirect3DSurface9 *surface = NULL;

		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		switch (view->source)
		{
		case RVS_OFFSCREEN:
			r = TRUE;
			switch (view->type)
			{
			case RVT_COLOR:
				IDirect3DDevice9_CreateRenderTarget(context->device, view->width, view->height, d3d9_tex_format_get(view->format), D3DMULTISAMPLE_NONE, 0, FALSE, &surface, NULL);
				break;
			case RVT_DEPTH_STENCIL:
				IDirect3DDevice9_CreateDepthStencilSurface(context->device, view->width, view->height, d3d9_tex_format_get(view->format), D3DMULTISAMPLE_NONE, 0, TRUE, &surface, NULL);
				break;
			default:
				break;
			}
			break;
		case RVS_TEXTURE_2D:
			r = TRUE;
			texture_impl = (D3D9Texture *)view->data.tex_2d.tex->impl;
			IDirect3DTexture9_GetSurfaceLevel(texture_impl->handle.texture_2d, view->data.tex_2d.level, &surface);
			break;
		case RVS_TEXTURE_CUBE:
			texture_impl = (D3D9Texture *)view->data.tex_cube.tex->impl;
			IDirect3DCubeTexture9_GetCubeMapSurface(texture_impl->handle.texture_cube, d3d9_cube_map_face_get(view->data.tex_cube.face), view->data.tex_cube.level, &surface);
			break;
		default:
			PI_ASSERT(FALSE, "can't support view source, souce = %d", view->source);
			break;
		}

		if (r)
		{
			impl = pi_new0(D3D9RenderView, 1);
			impl->surface = surface;
			view->impl = impl;
		}

		return r;
	}

	PiBool d3d9_view_clear(PiRenderView *view)
	{
		if (view->impl != NULL)
		{
			D3D9RenderView *impl = (D3D9RenderView *)view->impl;

			IDirect3DSurface9_Release(impl->surface);

			pi_free(impl);

			view->impl = NULL;
		}
		return TRUE;
	}

	PiBool PI_API render_view_init(PiRenderView *view)
	{
		if (render_system_check(g_rsystem) != CHECK_LOST)
		{
			PiBool r = d3d9_view_init(view);
			if (r)
			{
				pi_renderinfo_add_view_num(1);

				d3d9_state_add_view(view);
			}
			return r;
		}
		else
		{
			pi_renderinfo_add_view_num(1);
			d3d9_state_add_view(view);
			return TRUE;
		}

	}

	PiBool PI_API render_view_clear(PiRenderView *view)
	{
		PiBool r = d3d9_view_clear(view);
		if (r)
		{
			pi_renderinfo_add_view_num(-1);

			d3d9_state_remove_view(view);
		}
		return r;
	}

	PiBool PI_API render_view_on_attach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
	{
		return TRUE;
	}

	PiBool PI_API render_view_on_detach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
	{
		return TRUE;
	}

	PiBool PI_API render_view_on_bind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
	{
		D3D9RenderView *impl = (D3D9RenderView *)view->impl;

		if (impl != NULL)
		{
			PiTexture *tex = NULL;
			D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
			D3D9Context *context = d3d9_system->context;

			switch (view->source)
			{
			case RVS_TEXTURE_2D:
				tex = view->data.tex_2d.tex;
				break;
			case RVS_TEXTURE_CUBE:
				tex = view->data.tex_cube.tex;
				break;
			}

			// TODO:解除纹理绑定,当一个纹理作为渲染目标的时候，是否需要
			if (tex != NULL)
			{
				d3d9_state_remove_texture(tex);
			}

			switch (attachment)
			{
			case ATT_COLOR0:
				IDirect3DDevice9_SetRenderTarget(context->device, 0, impl->surface);
				d3d9_system->state.left = 0;
				d3d9_system->state.bottom = 0;
				d3d9_system->state.width = view->width;
				d3d9_system->state.left = view->height;
				break;
			case ATT_COLOR1:
				IDirect3DDevice9_SetRenderTarget(context->device, 1, impl->surface);
				break;
			case ATT_COLOR2:
				IDirect3DDevice9_SetRenderTarget(context->device, 2, impl->surface);
				break;
			case ATT_COLOR3:
				IDirect3DDevice9_SetRenderTarget(context->device, 3, impl->surface);
				break;
			case ATT_DEPTHSTENCIL:
				IDirect3DDevice9_SetDepthStencilSurface(context->device, impl->surface);
				break;
			default:
				break;
			}
		}

		return TRUE;
	}

	PiBool PI_API render_view_on_unbind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
	{
		D3D9RenderView *impl = (D3D9RenderView *)view->impl;

		if (impl != NULL)
		{
			D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
			D3D9Context *context = d3d9_system->context;
			D3D9RenderView *defaultTarget = (D3D9RenderView *)g_rsystem->main_target.views[0]->impl;

			switch (attachment)
			{
			case ATT_COLOR0:
				IDirect3DDevice9_SetRenderTarget(context->device, 0, defaultTarget->surface);
				break;
			case ATT_COLOR1:
				IDirect3DDevice9_SetRenderTarget(context->device, 1, NULL);
				break;
			case ATT_COLOR2:
				IDirect3DDevice9_SetRenderTarget(context->device, 2, NULL);
				break;
			case ATT_COLOR3:
				IDirect3DDevice9_SetRenderTarget(context->device, 3, NULL);
				break;
			case ATT_DEPTHSTENCIL:
				IDirect3DDevice9_SetDepthStencilSurface(context->device, NULL);
				break;
			default:
				break;
			}
		}
		return TRUE;
	}

}
