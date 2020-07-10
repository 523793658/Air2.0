#include <gl_rendertarget.h>
#include <gl_rendersystem.h>

#include <gl_texture.h>
#include <gl_renderview.h>

#include <gl_interface.h>

#include <renderinfo.h>

#include <renderwrap.h>

extern PiRenderSystem *g_rsystem;

PiBool PI_API render_target_init(PiRenderTarget *target)
{
	GLRenderTarget *impl = pi_new0(GLRenderTarget, 1);
	target->impl = impl;
	
	pi_renderinfo_add_target_num(1);
	if(target->type == TT_MRT)
	{
		gl2_GenFramebuffers(1, &impl->gl_fbo);
	}
	return TRUE;
}

PiBool PI_API render_target_clear(PiRenderTarget *target)
{
	GLRenderTarget *impl = target->impl;
	if (impl != NULL)
	{
		pi_renderinfo_add_target_num(-1);
		glstate_delete_rendertarget(target);
		pi_free(impl);
		target->impl = NULL;
	}
	return TRUE;
}

static uint _get_gl_mask(TargetBufferMask mask)
{
	uint gl_mask = 0;
	if(mask & TBM_COLOR)
	{
		gl_mask |= GL2_COLOR_BUFFER_BIT;
	}
	if(mask & TBM_DEPTH)
	{
		gl_mask |= GL2_DEPTH_BUFFER_BIT;
	}
	if(mask & TBM_STENCIL)
	{
		gl_mask |= GL2_STENCIL_BUFFER_BIT;
	}
	return gl_mask;
}

PiBool PI_API render_target_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter)
{
	PiBool r = FALSE;
	GLRenderTarget *dst_impl = dst->impl;
	GLRenderTarget *src_impl = src->impl;
	uint gl_filter = (filter == FILTER_NEAREST) ? GL2_NEAREST : GL2_LINEAR;
	uint gl_buffer_mask = _get_gl_mask(mask);

	if(dst_impl != NULL && src_impl != NULL)
	{
		if(!gl_Self_IsFramebufferBlit())
		{
			PI_ASSERT(FALSE, "can't support blitframebuffer");
		}
		else
		{
			uint old_fbo = glstate_bind_fbo(0);

			r = TRUE;
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, dst_impl->gl_fbo);
			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, src_impl->gl_fbo);

			gl3_BlitFramebuffer(src->left, src->bottom, src->left + src->width, src->bottom + src->height, 
				dst->left, dst->bottom, dst->left + dst->width, dst->bottom + dst->height, 
				gl_buffer_mask, gl_filter);

			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, 0);
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
	}
	return r;	
}

PiBool PI_API render_target_set_viewport(PiRenderTarget *target, uint32 left, uint32 bottom, uint32 width, uint32 height)
{
	GLRenderSystem *gl_rs = g_rsystem->impl;
	
	target->left = left;
	target->bottom = bottom;
	target->width = width;
	target->height = height;
	
	/* 如果该目标已经绑在渲染环境中，还需要设置到OpenGL */
	if(target == gl_rs->state.target)
	{
		glstate_set_viewport(left, bottom, width, height);
	}
	return TRUE;
}

PiBool PI_API render_target_bind(PiRenderTarget *target)
{
	GLRenderTarget *impl = target->impl;
	if (impl != NULL)
	{
		uint i, num = 0;
		uint targets[ATT_NUM];
		
		glstate_bind_fbo(impl->gl_fbo);
		
		// Webgl和OpenGL都支持drawBuffer，gles3也支持
		if(gl_Self_IsDrawBuffers())
		{
			for (i = ATT_COLOR0; i <= ATT_COLOR7; ++i)
			{
				if(target->views[i] != NULL)
				{
					++num;
					if(num == 1)
					{
						glstate_enable_fbo_srgb(pi_renderformat_is_srgb(target->views[i]->format));
					}
					targets[i] = i - ATT_COLOR0 + GL2_COLOR_ATTACHMENT0;
				}
			}
			gl3_DrawBuffers(num, targets);
		}		
	}
	else
	{
		glstate_bind_fbo(0);
		if (gl_Self_IsDrawBuffers())
		{
			uint target = GL_BACK_LEFT;
			gl3_DrawBuffers(1, &target);
		}
		glstate_enable_fbo_srgb(FALSE);
	}
	return TRUE;
}

/* 无特殊实现 */
PiBool PI_API render_target_unbind(PiRenderTarget *target)
{
	return TRUE;
}