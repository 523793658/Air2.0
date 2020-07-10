

#include <gl_context.h>
#include <gl_interface.h>

#include <Windows.h>
#include <glloader/glloader.h>

typedef WINGDIAPI BOOL  (WINAPI *SwapBuffersFunc)(HDC);


typedef struct  
{
	HWND hwnd;				/* 窗口句柄 */
	HDC hdc;				/* 窗口对应的设备句柄 */
	HGLRC hGLRC;			/* OpenGL渲染环境 */
	SwapBuffersFunc swapBuffers;
}GLWinContext;

static  RenderContextLoadType s_gl_context_type;

/* 初始化，创建gl环境 */
static void* _glloader_create_context(RenderContextLoadType type, void *win)
{
	GLWinContext *r = NULL;

	int pixelFormat = 0;
	PIXELFORMATDESCRIPTOR pfd; /* 像素格式描述符 */
	HDC curr_dc = wglGetCurrentDC();
	HGLRC curr_rc = wglGetCurrentContext();

	HDC gl_hdc;
	HGLRC gl_hrc;
	const byte *version_str;
	sint major_version = 0, minor_version = 0;

	pi_memset_inline(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	/* 渲染到窗口、支持OpenGL、支持双缓冲区 */
	pfd.dwFlags = 	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
	/* 颜色缓冲区是RGBA格式 */
	pfd.iPixelType = PFD_TYPE_RGBA;
	/* 颜色缓冲区每像素32位 */
	pfd.cColorBits = 32;
	/* 深度缓冲区每像素24位 */
	pfd.cDepthBits = 24;
	/* 模板缓冲区8位 */
	pfd.cStencilBits = 8;
	/* alpha为8位 */
	pfd.cDepthBits = 8;
	/* 渲染层是主平面，目前属于无效值 */
	pfd.iLayerType = PFD_MAIN_PLANE;

	gl_hdc = GetDC(win);
	if(NULL == gl_hdc)
	{
		pi_log_print(LOG_ERROR, "GetDC failed, hwnd = %d, error = %d\n", win	, GetLastError());
		return NULL;
	}

	/* 根据描述结构选择像素格式 */
	pixelFormat = ChoosePixelFormat(gl_hdc, &pfd);
	if(0 == pixelFormat)
	{
		pi_log_print(LOG_ERROR, "ChoosePixelFormat failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* 设置上面得到的像素格式 */
	if(!SetPixelFormat(gl_hdc, pixelFormat, &pfd))
	{
		pi_log_print(LOG_ERROR, "SetPixelFormat failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* 创建GL上下文 */
	gl_hrc = wglCreateContext(gl_hdc);
	if(NULL == gl_hrc)
	{
		pi_log_print(LOG_ERROR, "wglCreateContext failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* 将DC和RC与当前线程绑定起来 */
	if(!wglMakeCurrent(gl_hdc, gl_hrc))
	{
		wglDeleteContext(gl_hrc);
		pi_log_print(LOG_ERROR, "wglMakeCurrent failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* 共享和原环境一样的现实列表和纹理对象 */
	if(curr_rc != 0)
	{
		wglShareLists(curr_rc, gl_hrc);

		/* 还原现有的OpenGL环境 */
		if(!wglMakeCurrent(curr_dc, curr_rc))
		{
			pi_log_print(LOG_ERROR, "wglMakeCurrent 2 failed, error = %d\n", GetLastError());
			return NULL;
		}
	}

	/* 初始化 GL API*/
	gl_Self_Init(type, NULL);

	/* OpenGL版本必须是2.0以上 */
	version_str = gl2_GetString(GL2_VERSION);
	if(version_str != NULL)
	{
		uint len = pi_strlen((char *)version_str);
		if(len > 3)
		{
			major_version = version_str[0] - '0';
			minor_version = version_str[2] - '0';
		}
	}

	if(major_version < 2) 
	{
		wglDeleteContext(gl_hrc);
		wglMakeCurrent(NULL, NULL);
		PI_ASSERT(major_version >= 2, "OpenGL Version must >= 2.0, now is %d.%d", major_version, minor_version);
		return NULL;
	}

	r = pi_new0(GLWinContext, 1);

	r->hdc = gl_hdc;
	r->hGLRC = gl_hrc;
	r->hwnd = (HWND)win;
	
	{
		void *mod = pi_mod_open(L"opengl32.dll");
		if(mod != NULL)
		{
			r->swapBuffers = (SwapBuffersFunc)pi_mod_symbol(mod, "wglSwapBuffers");
		}
		if(r->swapBuffers == NULL)
		{
			r->swapBuffers = SwapBuffers;
		}
	}

	/* 关闭垂直同步 */
	wglSwapIntervalEXT(FALSE);

	return r;
}

void gl_context_free(void *context)
{
	if(s_gl_context_type == CONTEXT_LOAD_NATIVE)
	{
		GLWinContext *impl = (GLWinContext *)context;
		wglDeleteContext(impl->hGLRC);
		wglMakeCurrent(NULL, NULL);
	}
}

void gl_context_swapbuffer(void *gl_context)
{
	if(s_gl_context_type == CONTEXT_LOAD_NATIVE)
	{
		GLWinContext *impl = (GLWinContext *)gl_context;
		impl->swapBuffers(impl->hdc);
	}
}

void* gl_context_new(RenderContextLoadType type, void *data)
{
	void *context = NULL;
	s_gl_context_type = type;

	switch (type)
	{
	case CONTEXT_LOAD_NATIVE:
		context = _glloader_create_context(type, data);
		break;
	case CONTEXT_LOAD_EMPTY:
		context = (void *)0x1;	/* 给个非0的地址，用来供高层判断环境初始化成功 */
		gl_Self_Init(type, NULL);
		break;
	default:
		break;
	}

	/* 打印GL的扩展 */
	
	if(type != CONTEXT_LOAD_EMPTY && context != NULL)
	{
		const char *extension = (const char *)gl2_GetString(GL2_EXTENSIONS);
		pi_log_print(LOG_INFO, "\nGL Extensions: ");
		pi_log_print(LOG_INFO, "%s\n", extension);
	}

	return context;
}

void gl_context_get_size(void *gl_context, sint *w, sint *h)
{
	if(s_gl_context_type == CONTEXT_LOAD_NATIVE)
	{
		RECT rt;
		GLWinContext *impl = (GLWinContext *)gl_context;

		GetClientRect(impl->hwnd, &rt);
		*w = rt.right - rt.left;
		*h = rt.bottom - rt.top;
	}
}