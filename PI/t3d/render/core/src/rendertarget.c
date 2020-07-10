
#include <rendertarget.h>
#include <renderwrap.h>

PiRenderTarget* PI_API pi_rendertarget_new(TargetType type, PiBool is_create_handle)
{
	PiRenderTarget *rt = pi_new0(PiRenderTarget, 1);
	rt->type = type;
	if(is_create_handle) 
	{
		if(!pi_rendertarget_init(rt))
		{
			pi_free(rt);
			rt = NULL;
		}
	}
	return rt;
}

PiBool PI_API pi_rendertarget_init(PiRenderTarget *target)
{
	return render_target_init(target);
}

PiBool PI_API pi_rendertarget_free(PiRenderTarget *target)
{
	PiBool r = TRUE;
	if(target != NULL)
	{
		r = render_target_clear(target);
		pi_free(target);
	}	
	return r;
}

uint PI_API pi_rendertarget_get_width(PiRenderTarget *target)
{
	return target->width;
}

uint PI_API pi_rendertarget_get_height(PiRenderTarget *target)
{
	return target->height;
}

PiBool PI_API pi_rendertarget_set_viewport(PiRenderTarget *target, uint32 left, uint32 top, uint32 width, uint32 height)
{
	return render_target_set_viewport(target, left, top, width, height);
}

PiBool PI_API pi_rendertarget_attach(PiRenderTarget *target, TargetAttachment attachment, PiRenderView *view)
{
	PiBool r = TRUE;
	PiRenderSystem *system = pi_rendersystem_get_instance();

	if (view == target->views[attachment])
	{
		return TRUE;
	}

	PI_ASSERT(view != NULL, "attach view is NULL");

	if(attachment == ATT_DEPTHSTENCIL)
	{
		if(view->type != RVT_DEPTH_STENCIL)
		{
			PI_ASSERT(FALSE, "view isn't depth stencil");
			return FALSE;
		}
	}
	else
	{
		if(view->type != RVT_COLOR)
		{
			PI_ASSERT(FALSE, "view isn't color");
			return FALSE;
		}
		if(attachment < ATT_COLOR0 || attachment > ATT_COLOR7)
		{
			PI_ASSERT(FALSE, "attachment is out of range");
			return FALSE;
		}

		if(attachment >= ATT_COLOR0 + system->cap.max_simultaneous_rts)
		{
			PI_ASSERT(FALSE, "attachment is too much to exceed hardware's limit, max_simultaneous_rts = %d", system->cap.max_simultaneous_rts);
			return FALSE;
		}

		if(attachment == ATT_COLOR0 
			&& (target->width == 0 || target->height == 0))
		{
			target->left = 0;
			target->bottom = 0;
			target->width = view->width;
			target->height = view->height;
		}
	}
	
	if(view != NULL)
	{
		r = render_view_on_attach(view, target, attachment);
	}
	
	target->views[attachment] = view;
	target->is_update = TRUE;
	return r;
}

PiBool PI_API pi_rendertarget_detach(PiRenderTarget *target, TargetAttachment attachment)
{
	if(target->views[attachment] != NULL)
	{
		render_view_on_detach(target->views[attachment], target, attachment);
		target->views[attachment] = NULL;
		target->is_update = TRUE;
	}
	return TRUE;
}

PiBool PI_API pi_rendertarget_bind(PiRenderTarget *target)
{
	uint i;
	target->is_bind = TRUE;
	for(i = 0; i < ATT_NUM; ++i)
	{
		if (target->views[i] != NULL)
		{
			render_view_on_bind(target->views[i], target, ATT_COLOR0 + i);
		}
	}
	render_target_bind(target);
	return TRUE;
}

PiBool PI_API pi_rendertarget_unbind(PiRenderTarget *target)
{
	uint i;
	target->is_bind = FALSE;
	for(i = 0; i < ATT_NUM; ++i)
	{
		if (target->views[i] != NULL)
		{
			render_view_on_unbind(target->views[i], target, ATT_COLOR0 + i);
		}
	}
	render_target_unbind(target);
	return TRUE;
}

PiBool PI_API pi_rendertarget_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter)
{
	return render_target_copy(dst, src, mask, filter);
}