#ifndef INCLUDE_DEMO_H
#define INCLUDE_DEMO_H

#include "application.h"
#include "inputmanager.h"
#include "flycamera.h"
#include "camera.h"
#include "renderpipeline_ext.h"

#include "rendersystem.h"

#define FONT_FILE_NUM (3)

typedef struct PiDemo PiDemo;

typedef void (*DemoInitFunc)(PiDemo *demo, void *user_data);
typedef void (*DemoResizeFunc)(PiDemo *demo, void *user_data, uint width, uint height);
typedef void (*DemoUpdateFunc)(PiDemo *demo, void *user_data, float tpf);
typedef void (*DemoShutDownFunc)(PiDemo *demo, void *user_data);

struct PiDemo
{
	PiApplication *app;
	InputManager *input_mgr;
	void *user_data;

	uint width;
	uint height;
	PiBool is_window_mode;

	PiRenderTarget *screen_rt;
	PiCamera *screen_cam;
	FlyCamera *fly_cam;
	KeyboardListener reset_cam_listener;
	PiRenderPipelineExt *pipeline;

	PiBool esc_exit;

	DemoInitFunc init_func;
	DemoUpdateFunc update_func;
	DemoResizeFunc resize_func;
	DemoShutDownFunc shutdown_func;
};

PI_BEGIN_DECLS

PiApplication *PI_API pi_demo_create(char *title, uint width, uint height, DemoInitFunc init_func, DemoUpdateFunc update_func, DemoResizeFunc resize_func, DemoShutDownFunc shutdown_func, void *user_data);

InputManager *PI_API pi_demo_get_inputmanager(PiApplication *app);

PI_END_DECLS

#endif /* INCLUDE_DEMO_H */
