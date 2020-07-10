
#include "renderer.h"

static PiBool _default_init(PiRenderer *renderer, PiHash *resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(resources);
	return TRUE;
}

static void _default_resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

static void _default_update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(resources);
}

static void _default_draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(resources);
}

PiRenderer *PI_API pi_renderer_create(RendererType type, char *name,
                                      RendererInitFunc init, RendererResizeFunc resize, RendererUpdateFunc update, RendererDrawFunc draw, void *impl)
{
	PiRenderer *renderer = pi_new0(PiRenderer, 1);
	renderer->type = type;
	renderer->is_enable = TRUE;
	renderer->system_resize = TRUE;
	renderer->name = pi_str_dup(name);
	renderer->init_func = init ? init : _default_init;
	renderer->resize_func = resize ? resize : _default_resize;
	renderer->update_func = update ? update : _default_update;
	renderer->draw_func = draw ? draw : _default_draw;
	renderer->impl = impl;
	return renderer;
}

void PI_API pi_renderer_destroy(PiRenderer *renderer)
{
	pi_free(renderer->name);
	pi_free(renderer);
}

void PI_API pi_renderer_set_enable(PiRenderer *renderer, PiBool b)
{
	renderer->is_enable = b;
}

PiBool PI_API pi_renderer_is_enable(PiRenderer *renderer)
{
	return renderer->is_enable;
}

void PI_API pi_renderer_set_system_rendertarget_resize(PiRenderer *renderer, PiBool b)
{
	renderer->system_resize = b;
}

PiBool PI_API pi_renderer_is_system_rendertarget_resize(PiRenderer *renderer)
{
	return renderer->system_resize;
}
