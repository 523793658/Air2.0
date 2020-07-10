#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include <pi_lib.h>

typedef struct PiRenderer PiRenderer;

typedef PiBool(*RendererInitFunc)(PiRenderer *renderer, PiHash *resources);
typedef void(*RendererResizeFunc)(PiRenderer *renderer, uint width, uint height);
typedef void(*RendererUpdateFunc)(PiRenderer *renderer, float tpf, PiHash *resources);
typedef void(*RendererDrawFunc)(PiRenderer *renderer, float tpf, PiHash *resources);

typedef enum
{
	ERT_DEFAULT,
	ERT_FORWARD_LIGHTING,
	ERT_GEOMETRY_BUFFER,
	ERT_PRE_LIGHTING,
	ERT_DEFERRED_SHADING,
	ERT_PRE_SHADOW_VSM,
	ERT_PRE_CHARACTER_SHADOW,
	ERT_COLOR_GRADING,
	ERT_FXAA,
	ERT_BLOOM,
	ERT_DECAL,
	ERT_WATER,
	ERT_RADIALBLUR,
	ERT_REFRACTION,
	ERT_SKY,
	ERT_TERRAIN,
	ERT_BRUSH_RING,
	ERT_EDITOR_HELPER,
	ERT_SSAO,
	ERT_FBO_COPY,
	ERT_STRING,

	ERT_SELECTED_EDGE,
	ERT_PRE_SHADOW_PCF,
	ERT_HDR,
	ERT_DOF,
	ERT_FOG,
	ERT_UI,
	ERT_POST_PROCESSING,
	ERT_STROKE,
	ERT_BORDER_EXTENSION,
	ERT_GAUSSIAN_BLUR,

	ERT_INVERSE,
	ERT_PICTURE_SWITCH,

	ERT_TEXT_RENDERER,

	ERT_COUNT,

	ERT_USER = 5000,	/* 自定义渲染器的起始枚举范围 */
} RendererType;

/**
 * 渲染器
 */
struct PiRenderer
{
	void *impl;
	PiBool is_init;
	PiBool is_enable;
	PiBool system_resize;
	RendererType type;
	char *name;

	RendererInitFunc init_func;
	RendererResizeFunc resize_func;
	RendererUpdateFunc update_func;
	RendererDrawFunc draw_func;
};

PI_BEGIN_DECLS

PiRenderer *PI_API pi_renderer_create(RendererType type, char *name, RendererInitFunc init, RendererResizeFunc resize, RendererUpdateFunc update, RendererDrawFunc draw, void *impl);

void PI_API pi_renderer_destroy(PiRenderer *renderer);

void PI_API pi_renderer_set_enable(PiRenderer *renderer, PiBool b);

PiBool PI_API pi_renderer_is_enable(PiRenderer *renderer);

void PI_API pi_renderer_set_system_rendertarget_resize(PiRenderer *renderer, PiBool b);

PiBool PI_API pi_renderer_is_system_rendertarget_resize(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_RENDERER_H */
