#include <gl_rendersystem.h>
#include <gl_renderlayout.h>
#include <gl_rendertarget.h>
#include <gl_renderview.h>

#include <gl_shader.h>
#include <gl_texture.h>
#include <gl_convert.h>
#include <gl_rendercap.h>
#include <gl_context.h>
#include <gl_interface.h>

#include <renderwrap.h>
#include <renderinfo.h>

extern PiRenderSystem *g_rsystem;

PiBool PI_API render_system_init(PiRenderSystem *rs, RenderContextLoadType type, uint width, uint height, void *data)
{
	PiBool r = TRUE;
	GLRenderSystem *impl = pi_new0(GLRenderSystem, 1);
	
	impl->context = gl_context_new(type, data);
	if(impl->context != NULL)
	{
		g_rsystem = rs;
		rs->impl = impl;
		
		render_force_def_state();

		gl_rendercap_init(&g_rsystem->cap);

		rs->main_target.left = rs->main_target.bottom = 0;
		rs->main_target.type = TT_WIN;
		impl->state.target = &rs->main_target;
		pi_rendertarget_set_viewport(&rs->main_target, 0, 0, width, height);

		if (gl_Self_GetInterfaceType() != RIT_GLES)
		{
			gl2_Enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}		
		
		if(!glstate_init(&impl->state))
		{
			pi_log_print(LOG_ERROR, "attribute bind texture failed, attribute num is too slow, num = %d", impl->state.max_attrib_num);
			gl_context_free(impl->context);
			pi_free(impl);
			rs->impl = NULL;
			return FALSE;
		}

		gl2_GenFramebuffers(1, &impl->blit_dst_fbo);
		gl2_GenFramebuffers(1, &impl->blit_src_fbo);
	}
	else
	{
		r = FALSE;
		pi_free(impl);
	}
	return r;
}

PiBool PI_API render_system_clear(PiRenderSystem *rs)
{
	GLRenderSystem *impl = rs->impl;
	glstate_clear(&impl->state);
	gl_rendercap_clear(&rs->cap);
	gl_context_free(impl->context);
	gl2_DeleteFramebuffers(1, &impl->blit_dst_fbo);
	gl2_DeleteFramebuffers(1, &impl->blit_src_fbo);
	pi_free(rs->impl);
	return TRUE;
}

PiBool PI_API render_system_is_lost_context(PiRenderSystem *rs)
{
	return gl_Self_IsLostContext();
}

PiBool PI_API render_system_reset(PiRenderSystem *rs)
{
	return TRUE;
}

PiBool PI_API render_system_set_program(PiRenderSystem *rs, void *program)
{
	PI_USE_PARAM(rs);
	return glstate_use_shader(program);
}

PiBool PI_API render_system_draw(PiRenderSystem *rs, PiEntity *entity, uint num)
{
	pi_renderinfo_add_entity_num(1);
	pi_renderinfo_add_face_num(pi_mesh_get_face_num(entity->mesh->mesh));
	pi_renderinfo_add_vertex_num(pi_mesh_get_vertex_num(entity->mesh->mesh));
	return gl_renderlayout_draw(entity->mesh, num);
}

void PI_API render_system_swapbuffer(PiRenderSystem *rs)
{
	//PiBool r = TRUE;
	GLRenderSystem *impl = rs->impl;
	gl_context_swapbuffer(impl->context);
	//if(is_force)
	//{
	//	gl2_Flush();
	//}

	//r = gl2_GetError();
	//if(r != GL2_NO_ERROR)
	//{
	//	pi_log_print(LOG_DEBUG, "gl error happen, error code = %d", r);
	//}	
	//return r;
}

PiBool PI_API render_system_set_target(PiRenderSystem *rs, PiRenderTarget* rt)
{
	PiBool r = TRUE;
	GLRenderSystem *impl = rs->impl;
	GLRenderState *state = &impl->state;

	if(rt == NULL)
	{
		rt = &rs->main_target;
	}

	if(state->target != rt)
	{
		pi_rendertarget_unbind(state->target);
		pi_rendertarget_bind(rt);
		state->target = rt;
	}
	
	glstate_set_viewport(rt->left, rt->bottom, rt->width, rt->height);
	
	return r;
}

PiBool PI_API render_system_clearview(PiRenderSystem *rs, uint32 flags, PiColor *clr, float depth, uint stencil)
{
	uint ogl_flags = 0;
	GLRenderSystem *impl = g_rsystem->impl;
	GLRenderState *state = &impl->state;
	
	if (flags & TBM_COLOR)
	{
		if (rs->bs.color_write_mask != CMASK_ALL)
		{
			gl2_ColorMask(TRUE, TRUE, TRUE, TRUE);
		}
	}

	if (flags & TBM_DEPTH)
	{
		if (!rs->dss.is_depth_write_mask)
		{
			gl2_DepthMask(TRUE);
		}
	}
	if (flags & TBM_STENCIL)
	{
		if (!rs->dss.stencil_write_mask)
		{
			gl2_StencilMask(0xFF);
		}
	}
	
	if (flags & TBM_COLOR)
	{
		ogl_flags |= GL2_COLOR_BUFFER_BIT;

		if(!color_is_equal(clr, &state->clear_color))
		{
			color_copy(&state->clear_color, clr);
			gl2_ClearColor(clr->rgba[0], clr->rgba[1], clr->rgba[2], clr->rgba[3]);
		}
	}
	
	if (flags & TBM_DEPTH)
	{
		ogl_flags |= GL2_DEPTH_BUFFER_BIT;
		if(!IS_FLOAT_EQUAL(depth, state->clear_depth))
		{
			state->clear_depth = depth;
			gl2_ClearDepthf(depth);
		}
	}

	if (flags & TBM_STENCIL)
	{
		ogl_flags |= GL2_STENCIL_BUFFER_BIT;
		if(stencil != state->clear_stencil)
		{
			state->clear_stencil = stencil;
			gl2_ClearStencil(stencil);
		}
	}

	gl2_Clear(ogl_flags);

	if (flags & TBM_COLOR)
	{
		if (rs->bs.color_write_mask != CMASK_ALL)
		{
			gl2_ColorMask((rs->bs.color_write_mask & CMASK_RED) != 0,
				(rs->bs.color_write_mask & CMASK_GREEN) != 0,
				(rs->bs.color_write_mask & CMASK_BLUE) != 0,
				(rs->bs.color_write_mask & CMASK_ALPHA) != 0);
		}
	}
	if (flags & TBM_DEPTH)
	{
		if (!rs->dss.is_depth_write_mask)
		{
			gl2_DepthMask(FALSE);
		}
	}
	if (flags & TBM_STENCIL)
	{
		if (!rs->dss.stencil_write_mask)
		{
			gl2_StencilMask(rs->dss.stencil_write_mask);
		}
	}	
	return TRUE;
}