#include "dino_ui.h"
#include "rendertarget.h"
#include "rendersystem.h"
#include "DinoUIWrapper.h"
#include "load.h"

typedef struct
{
	char* target_name;
	uint x,y;
	uint width, height;
	PiBool is_init;
	PiBool is_deploy;
	PiCamera* camera;
}DinoUIRenderer; 

static void _type_check(PiRenderer* renderer) 
{
	PI_ASSERT(renderer->type == ERT_DINO_UI, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash* resources)
{
	DinoUIRenderer* impl = (DinoUIRenderer*)renderer->impl;
	PI_USE_PARAM(resources);
	if(!impl->is_deploy) {
		return FALSE;
	}
 	if ( DinoUI_Initialize() != 0)
	{
		return FALSE;
	}

	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 10.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, 0, 1440.0f, 0, 900.0f, -10.0f, 10.0f, TRUE);
	impl->is_init = TRUE;
	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	DinoUIRenderer* impl;
	PiRenderTarget* target;
	
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (DinoUIRenderer*)renderer->impl;

	if(impl->is_init)
	{
		pi_hash_lookup(resources, impl->target_name, (void**)&target);
		pi_rendersystem_set_target(target);
		pi_rendersystem_set_camera(impl->camera);
		DinoUI_Render();
	}
}
static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(resources);
	DinoUI_Update(tpf);
}

static void _resize(PiRenderer *r, uint width, uint height)
{
	PI_USE_PARAM(r);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer* PI_API app_dino_renderer_new()
{
	PiRenderer* renderer;
	DinoUIRenderer* impl = pi_new0(DinoUIRenderer, 1);
	renderer = pi_renderer_create(ERT_DINO_UI, "dino_ui", _init, _resize, _update, _draw, impl);
	return renderer;
}
void PI_API app_dino_renderer_deploy(PiRenderer* renderer, char* target_name)
{
	DinoUIRenderer* impl;
	_type_check(renderer);
	impl = (DinoUIRenderer*)renderer->impl;
	pi_free(impl->target_name);
	impl->target_name = pi_str_dup(target_name);
	impl->is_deploy = TRUE;
}
void PI_API app_dino_renderer_free(PiRenderer* renderer)
{
	DinoUIRenderer* impl;
	_type_check(renderer);
	impl = (DinoUIRenderer*)renderer->impl;
	pi_free(impl->target_name);
	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}