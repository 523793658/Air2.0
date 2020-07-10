
#include <renderview.h>
#include <renderwrap.h>

static PiRenderView* _init_view(PiRenderView *view, PiBool is_create_handle)
{
	if(is_create_handle)
	{
		if(!render_view_init(view))
		{
			pi_free(view);
			view = NULL;
		}
	}
	return view;
}

PiRenderView* PI_API pi_renderview_new(RenderViewType type, 
	uint width, uint height, RenderFormat format, PiBool is_create_handle)
{
	PiRenderView *view = pi_new0(PiRenderView, 1);
	view->type = type;
	view->source = RVS_OFFSCREEN;
	view->width = width;
	view->height = height;
	view->format = format;
	view = _init_view(view, is_create_handle);
	return view;
}

PiRenderView* PI_API pi_renderview_new_tex2d(RenderViewType type, 
	PiTexture *texture, uint array_index, uint level, PiBool is_create_handle)
{
	PiRenderView *view = NULL;

	if(texture->type != TT_2D)
	{
		pi_log_print(LOG_WARNING, "%s failed, texture's type isn't 2D", __FUNCTION__);
		return NULL;
	}
	if(array_index >= texture->array_size)
	{
		pi_log_print(LOG_WARNING, "%s failed, array_index is out of range", __FUNCTION__);
		return NULL;
	}
	if(level >= texture->num_mipmap)
	{
		pi_log_print(LOG_WARNING, "%s failed, level is out of range", __FUNCTION__);
		return NULL;
	}

	view = pi_new0(PiRenderView, 1);
	view->type = type;
	view->source = RVS_TEXTURE_2D;
	view->data.tex_2d.tex = texture;
	view->data.tex_2d.level = level;
	view->data.tex_2d.array_index = array_index;

	view->format = texture->format;
	view->width = texture->level_size[level].width;
	view->height = texture->level_size[level].height;
	
	view = _init_view(view, is_create_handle);
	return view;
}

PiRenderView* PI_API pi_renderview_new_texcube(RenderViewType type, 
	PiTexture *texture, TextureCubeFace face, uint level, PiBool is_create_handle)
{
	PiRenderView *view = NULL;
	if(texture->type != TT_CUBE)
	{
		pi_log_print(LOG_WARNING, "%s failed, texture's type isn't 2D", __FUNCTION__);
		return NULL;
	}
	if(level >= texture->num_mipmap)
	{
		pi_log_print(LOG_WARNING, "%s failed, level is out of range", __FUNCTION__);
		return NULL;
	}

	view = pi_new0(PiRenderView, 1);
	view->type = type;
	view->source = RVS_TEXTURE_CUBE;
	view->data.tex_cube.face = face;
	view->data.tex_cube.tex = texture;
	view->data.tex_cube.level = level;

	view->format = texture->format;
	view->width = texture->level_size[level].width;
	view->height = texture->level_size[level].height;
	view = _init_view(view, is_create_handle);
	return view;
}

uint PI_API pi_renderview_get_width(PiRenderView *view)
{
	return view->width;
}

uint PI_API pi_renderview_get_height(PiRenderView *view)
{
	return view->height;
}

RenderFormat PI_API pi_renderview_get_format(PiRenderView *view)
{
	return view->format;
}

PiBool PI_API pi_renderview_init(PiRenderView *view)
{
	return render_view_init(view);
}

PiBool PI_API pi_renderview_free(PiRenderView *view)
{
	if(view != NULL)
	{
		render_view_clear(view);
		pi_free(view);
	}
	return TRUE;
}