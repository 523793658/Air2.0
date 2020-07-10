
#include <gl_context.h>
#include <gl_interface.h>
#include <glloader/glloader.h>
#include <android/native_window_jni.h>

typedef struct  
{
	void *hwnd;				/* 窗口句柄 */
	EGLSurface surface;
	EGLContext glRC;
	EGLDisplay display;
}GLAndroidContext;

/* 释放gl环境 */
void gl_context_free(void *gl_context)
{
	PI_USE_PARAM(gl_context);
}

/* 交换缓冲区 */
void gl_context_swapbuffer(void *gl_context)
{
	GLAndroidContext *impl = (GLAndroidContext *)gl_context;
	eglSwapBuffers(impl->display, impl->surface);
}

/* 取得当前环境的宽、高 */
void gl_context_get_size(void *gl_context, sint *w, sint *h)
{
	GLAndroidContext *impl = (GLAndroidContext *)gl_context;
	
	*w = ANativeWindow_getWidth(impl->hwnd);
	*h = ANativeWindow_getHeight(impl->hwnd);
}

/* 初始化，创建gl环境 */
void* gl_context_new(RenderContextLoadType type, void *win)
{
	GLAndroidContext *r = NULL;

	EGLConfig config;
	EGLint numConfigs;
	EGLSurface surface;
	EGLContext glRC;
	EGLDisplay display;
	EGLint w, h, dummy, format;

	const EGLint attribs[] = 
	{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

	int attribList[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	if (win == NULL)
	{
		pi_log_print(LOG_ERROR, "ANativeWindow_fromSurface failed");
		return NULL;
	}
	
	pi_log_print(LOG_INFO, "win = %d", win);
	
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);

	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(win, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, win, NULL);
	if (surface == NULL) {
		pi_log_print(LOG_ERROR, "eglCreateWindowSurface failed");
		return NULL;
	}

	glRC = eglCreateContext(display, config, EGL_NO_CONTEXT, attribList);
	if (glRC == NULL) {
		pi_log_print(LOG_ERROR, "eglCreateContext failed");
		return NULL;
	}

	if (eglMakeCurrent(display, surface, surface, glRC) == EGL_FALSE) {
		pi_log_print(LOG_ERROR, "eglMakeCurrent failed");
		return NULL;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	r = pi_new0(GLAndroidContext, 1);

	r->hwnd = win;
	r->surface = surface;
	r->glRC = glRC;
	r->display = display;
	
	pi_log_print(LOG_INFO, "surface = %d", surface);
	pi_log_print(LOG_INFO, "glRC = %d", glRC);
	pi_log_print(LOG_INFO, "display = %d", display);

    /* 初始化 GL API*/
	gl_Self_Init(CONTEXT_LOAD_NATIVE, NULL);

	return r;
}