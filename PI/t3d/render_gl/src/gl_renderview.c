#include <gl_renderview.h>
#include <gl_convert.h>
#include <gl_rendertarget.h>
#include <gl_texture.h>
#include <gl_renderstate.h>
#include <gl_interface.h>
#include <renderinfo.h>

static PiBool _attach_color_tex2d(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_2d.tex;
	GLTexture *tex_impl = tex->impl;

	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;
	
	index += GL2_COLOR_ATTACHMENT0;
	if (tex->array_size == 1)
	{
		uint32 curr = glstate_bind_fbo(fbo);
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, index, GL2_TEXTURE_2D, tex_impl->gl_id, view->data.tex_2d.level);
		glstate_bind_fbo(curr);
	}
	else
	{
		uint32 curr = glstate_bind_fbo(fbo);
		gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, index, tex_impl->gl_id, view->data.tex_2d.level, view->data.tex_2d.array_index);
		glstate_bind_fbo(curr);
	}
	return TRUE;
}

static PiBool _detach_color_tex2d(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_2d.tex;

	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;

	index += GL2_COLOR_ATTACHMENT0;
	if (tex->array_size == 1)
	{
		uint32 curr = glstate_bind_fbo(fbo);
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, index, GL2_TEXTURE_2D, 0, 0);
		glstate_bind_fbo(curr);
	}
	else
	{
		uint32 curr = glstate_bind_fbo(fbo);
		gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, index, 0, 0, 0);
		glstate_bind_fbo(curr);
	}
	return TRUE;
}

static PiBool _attach_color_texcube(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_cube.tex;
	GLTexture *tex_impl = tex->impl;

	uint32 curr;
	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;
	uint face = view->data.tex_cube.face - CF_POSITIVE_X;
	
	index += GL2_COLOR_ATTACHMENT0;
	face += GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
	curr = glstate_bind_fbo(fbo);
	gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, index, face, tex_impl->gl_id, view->data.tex_cube.level);
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _detach_color_texcube(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	uint32 curr;
	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;
	uint face = view->data.tex_cube.face - CF_POSITIVE_X;

	index += GL2_COLOR_ATTACHMENT0;
	face += GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
	curr = glstate_bind_fbo(fbo);
	gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, index, face, 0, 0);
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _attach_color_offscreen(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	uint32 curr;
	GLRenderView *impl = view->impl;
	GLRenderTarget *target_impl = target->impl;
	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;

	index += GL2_COLOR_ATTACHMENT0;
	curr = glstate_bind_fbo(fbo);
	gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, index, GL2_RENDERBUFFER, impl->gl_id);
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _detach_color_offscreen(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	uint32 curr;
	GLRenderTarget *target_impl = target->impl;
	uint index = attachment - ATT_COLOR0;
	uint fbo = target_impl->gl_fbo;
	
	index += GL2_COLOR_ATTACHMENT0;
	curr = glstate_bind_fbo(fbo);
	gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, index, GL2_RENDERBUFFER, 0);
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _attach_depth_stencil_offscreen(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderView *impl = view->impl;
	GLRenderTarget *target_impl = target->impl;

	uint fbo = target_impl->gl_fbo;

	uint32 curr = glstate_bind_fbo(fbo);
	gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, GL2_RENDERBUFFER, impl->gl_id);
	if (pi_renderformat_is_stencil_format(view->format))
	{
		gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, GL2_RENDERBUFFER, impl->gl_id);
	}
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _detach_depth_stencil_offscreen(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;

	uint fbo = target_impl->gl_fbo;

	uint32 curr = glstate_bind_fbo(fbo);
	gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, GL2_RENDERBUFFER, 0);
	gl2_FramebufferRenderbuffer(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, GL2_RENDERBUFFER, 0);
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _attach_depth_stencil_tex2d(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_2d.tex;
	GLTexture *tex_impl = tex->impl;

	uint fbo = target_impl->gl_fbo;

	if (tex->array_size == 1)
	{
		uint32 curr = glstate_bind_fbo(fbo);
		if (pi_renderformat_is_depth_format(view->format))
		{
			gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, tex_impl->gl_target, tex_impl->gl_id, view->data.tex_2d.level);
		}
		if (pi_renderformat_is_stencil_format(view->format))
		{
			gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, tex_impl->gl_target, tex_impl->gl_id, view->data.tex_2d.level);
		}
		glstate_bind_fbo(curr);
	}
	else
	{
		Texture2DView *p = &view->data.tex_2d;
		
		uint curr = glstate_bind_fbo(fbo);
		if (pi_renderformat_is_depth_format(view->format))
		{
			gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, tex_impl->gl_id, p->level, p->array_index);
		}
		if (pi_renderformat_is_stencil_format(view->format))
		{
			gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, tex_impl->gl_id, p->level, p->array_index);
		}
		glstate_bind_fbo(curr);
	}
	return TRUE;
}

static PiBool _detach_depth_stencil_tex2d(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_2d.tex;
	
	uint fbo = target_impl->gl_fbo;

	if (tex->array_size == 1)
	{
		uint32 curr = glstate_bind_fbo(fbo);
		if (pi_renderformat_is_depth_format(view->format))
		{
			gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, GL2_TEXTURE_2D, 0, 0);
		}
		if (pi_renderformat_is_stencil_format(view->format))
		{
			gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, GL2_TEXTURE_2D, 0, 0);
		}
		glstate_bind_fbo(curr);
	}
	else
	{
		uint32 curr = glstate_bind_fbo(fbo);
		if (pi_renderformat_is_depth_format(view->format))
		{
			gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, 0, 0, 0);
		}
		if (pi_renderformat_is_stencil_format(view->format))
		{
			gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, 0, 0, 0);
		}
		glstate_bind_fbo(curr);
	}
	return TRUE;
}

static PiBool _attach_depth_stencil_texcube(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	GLRenderTarget *target_impl = target->impl;
	PiTexture *tex = view->data.tex_cube.tex;
	GLTexture *tex_impl = tex->impl;
	uint32 curr;
	uint fbo = target_impl->gl_fbo;
	uint face = view->data.tex_cube.face - CF_POSITIVE_X;
	face += GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
	
	curr = glstate_bind_fbo(fbo);
	if (pi_renderformat_is_depth_format(view->format))
	{
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, face, tex_impl->gl_id, view->data.tex_cube.level);
	}
	if (pi_renderformat_is_stencil_format(view->format))
	{
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, face, tex_impl->gl_id, view->data.tex_cube.level);
	}
	glstate_bind_fbo(curr);
	return TRUE;
}

static PiBool _detach_depth_stencil_texcube(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	uint32 curr;
	GLRenderTarget *target_impl = target->impl;
	uint fbo = target_impl->gl_fbo;
	uint face = view->data.tex_cube.face - CF_POSITIVE_X;
	
	face += GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
	curr = glstate_bind_fbo(fbo);
	if (pi_renderformat_is_depth_format(view->format))
	{
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, face, 0, 0);
	}
	if (pi_renderformat_is_stencil_format(view->format))
	{
		gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_STENCIL_ATTACHMENT, face, 0, 0);
	}
	glstate_bind_fbo(curr);
	return TRUE;
}

static void _init_offscene_view(PiRenderView *view, GLRenderView *impl)
{	
	gl2_GenRenderbuffers(1, &impl->gl_id);
	gl2_BindRenderbuffer(GL2_RENDERBUFFER, impl->gl_id);
	gl_tex_format_get(view->format, &impl->gl_format, NULL, NULL);
	if(impl->gl_format == GL2_DEPTH_COMPONENT) 
	{
		impl->gl_format = GL2_DEPTH_COMPONENT16;
	}
	gl2_RenderbufferStorage(GL2_RENDERBUFFER, impl->gl_format, view->width, view->height);
	gl2_BindRenderbuffer(GL2_RENDERBUFFER, 0);
}

PiBool PI_API render_view_init(PiRenderView *view)
{
	PiBool r = FALSE;
	GLRenderView *impl = pi_new0(GLRenderView, 1);
	view->impl = impl;
	
	pi_renderinfo_add_view_num(1);
	switch(view->source)
	{
	case RVS_OFFSCREEN:
		if(view->type == RVT_DEPTH_STENCIL)
		{
			r = TRUE;
			_init_offscene_view(view, impl);
			impl->attach_func = (view->type == RVT_COLOR) ? _attach_color_offscreen : _attach_depth_stencil_offscreen;
			impl->detach_func = (view->type == RVT_COLOR) ? _detach_color_offscreen : _detach_depth_stencil_offscreen;
		}
		break;
	case RVS_TEXTURE_2D:
		r = TRUE;
		impl->attach_func = (view->type == RVT_COLOR) ? _attach_color_tex2d : _attach_depth_stencil_tex2d;
		impl->detach_func = (view->type == RVT_COLOR) ? _detach_color_tex2d : _detach_depth_stencil_tex2d;
		break;
	case RVS_TEXTURE_CUBE:
		r = TRUE;
		impl->attach_func = (view->type == RVT_COLOR) ? _attach_color_texcube : _attach_depth_stencil_texcube;
		impl->detach_func = (view->type == RVT_COLOR) ? _detach_color_texcube : _detach_depth_stencil_texcube;
		break;
	default:
		PI_ASSERT(FALSE, "can't support view source, souce = %d", view->source);
		break;
	}
	if(!r)
	{
		if(impl != NULL)
		{
			pi_free(impl);
		}
	}
	return r;
}

PiBool PI_API render_view_clear(PiRenderView *view)
{
	if(view->impl != NULL)
	{
		GLRenderView *impl = view->impl;
		
		pi_renderinfo_add_view_num(-1);
		
		if(view->source == RVS_OFFSCREEN)
		{
			gl2_DeleteRenderbuffers(1, &impl->gl_id);
		}		
		pi_free(impl);
	}
	return TRUE;
}

PiBool PI_API render_view_on_attach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	PiBool r = TRUE;
	GLRenderView *impl = view->impl;
	
	if(impl != NULL && impl->attach_func != NULL)
	{
		r = impl->attach_func(view, target, attachment);
	}
	return r;
}

PiBool PI_API render_view_on_detach(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	PiBool r = TRUE;
	GLRenderView *impl = view->impl;
		
	if(impl != NULL && impl->detach_func != NULL)
	{
		r = impl->detach_func(view, target, attachment);
	}
	return r;
}

PiBool PI_API render_view_on_bind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	PiBool r = TRUE;
	GLRenderView *impl = view->impl;

	if(impl != NULL)
	{
		PiTexture *tex = NULL;
		switch(view->source)
		{
		case RVS_TEXTURE_2D:
			tex = view->data.tex_2d.tex;
			break;
		case RVS_TEXTURE_CUBE:
			tex = view->data.tex_cube.tex;
			break;
		}
		if(tex != NULL)
		{
			glstate_remove_texture(tex);
		}

		if(impl->bind_func != NULL)
		{
			r = impl->bind_func(view, target, attachment);
		}
	}
	return r;
}

PiBool PI_API render_view_on_unbind(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment)
{
	PiBool r = TRUE;
	GLRenderView *impl = view->impl;

	if(impl != NULL && impl->unbind_func != NULL)
	{
		r = impl->unbind_func(view, target, attachment);
	}
	return r;
}