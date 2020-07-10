#include "viewport_renderer.h"
#include "rendertarget.h"
#include "rendersystem.h"

typedef struct
{
	char* target_name;
	uint x,y;
	uint width, height;
	PiBool is_init;
	PiBool is_deploy;
}ViewportRenderer; 

static void _type_check(PiRenderer* renderer) 
{
	PI_ASSERT(renderer->type == ERT_VIEWPORT, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash* resources)
{
	ViewportRenderer* impl = (ViewportRenderer*)renderer->impl;

	PI_USE_PARAM(resources);

	if(!impl->is_deploy) {
		return FALSE;
	}

	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	ViewportRenderer* impl;
	PiRenderTarget* target;
	
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (ViewportRenderer*)renderer->impl;
	if(impl->is_init)
	{
		pi_hash_lookup(resources, impl->target_name, (void**)&target);
		pi_rendersystem_set_target(target);
		pi_rendertarget_set_viewport(target, impl->x, impl->y, impl->width, impl->height);
	}
}
static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(resources);
}

static void _resize(PiRenderer *r, uint width, uint height)
{
	PI_USE_PARAM(r);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer* PI_API app_viewport_renderer_new()
{
	PiRenderer* renderer;
	ViewportRenderer* impl = pi_new0(ViewportRenderer, 1);
	renderer = pi_renderer_create((RendererType)ERT_VIEWPORT, "viewport", _init, _resize, _update, _draw, impl);
	return renderer;
}
void PI_API app_viewport_renderer_deploy(PiRenderer* renderer, char* target_name)
{
	ViewportRenderer* impl;
	_type_check(renderer);
	impl = (ViewportRenderer*)renderer->impl;
	pi_free(impl->target_name);
	impl->target_name = pi_str_dup(target_name);
	impl->is_deploy = TRUE;
}
void PI_API app_viewport_renderer_free(PiRenderer* renderer)
{
	ViewportRenderer* impl;
	_type_check(renderer);
	impl = (ViewportRenderer*)renderer->impl;
	pi_free(impl->target_name);
	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}
void PI_API app_viewport_renderer_set_viewport(PiRenderer*renderer, uint x, uint y, uint width, uint height)
{
	ViewportRenderer* impl;
	_type_check(renderer);
	impl = (ViewportRenderer*)renderer->impl;
	impl->x = x;
	impl->y = y;
	impl->width = width;
	impl->height = height;
	impl->is_init = TRUE;
}