

#include <gl_context.h>
#include <gl_interface.h>

#include <Windows.h>
#include <glloader/glloader.h>

typedef WINGDIAPI BOOL  (WINAPI *SwapBuffersFunc)(HDC);


typedef struct  
{
	HWND hwnd;				/* ���ھ�� */
	HDC hdc;				/* ���ڶ�Ӧ���豸��� */
	HGLRC hGLRC;			/* OpenGL��Ⱦ���� */
	SwapBuffersFunc swapBuffers;
}GLWinContext;

static  RenderContextLoadType s_gl_context_type;

/* ��ʼ��������gl���� */
static void* _glloader_create_context(RenderContextLoadType type, void *win)
{
	GLWinContext *r = NULL;

	int pixelFormat = 0;
	PIXELFORMATDESCRIPTOR pfd; /* ���ظ�ʽ������ */
	HDC curr_dc = wglGetCurrentDC();
	HGLRC curr_rc = wglGetCurrentContext();

	HDC gl_hdc;
	HGLRC gl_hrc;
	const byte *version_str;
	sint major_version = 0, minor_version = 0;

	pi_memset_inline(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	/* ��Ⱦ�����ڡ�֧��OpenGL��֧��˫������ */
	pfd.dwFlags = 	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
	/* ��ɫ��������RGBA��ʽ */
	pfd.iPixelType = PFD_TYPE_RGBA;
	/* ��ɫ������ÿ����32λ */
	pfd.cColorBits = 32;
	/* ��Ȼ�����ÿ����24λ */
	pfd.cDepthBits = 24;
	/* ģ�建����8λ */
	pfd.cStencilBits = 8;
	/* alphaΪ8λ */
	pfd.cDepthBits = 8;
	/* ��Ⱦ������ƽ�棬Ŀǰ������Чֵ */
	pfd.iLayerType = PFD_MAIN_PLANE;

	gl_hdc = GetDC(win);
	if(NULL == gl_hdc)
	{
		pi_log_print(LOG_ERROR, "GetDC failed, hwnd = %d, error = %d\n", win	, GetLastError());
		return NULL;
	}

	/* ���������ṹѡ�����ظ�ʽ */
	pixelFormat = ChoosePixelFormat(gl_hdc, &pfd);
	if(0 == pixelFormat)
	{
		pi_log_print(LOG_ERROR, "ChoosePixelFormat failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* ��������õ������ظ�ʽ */
	if(!SetPixelFormat(gl_hdc, pixelFormat, &pfd))
	{
		pi_log_print(LOG_ERROR, "SetPixelFormat failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* ����GL������ */
	gl_hrc = wglCreateContext(gl_hdc);
	if(NULL == gl_hrc)
	{
		pi_log_print(LOG_ERROR, "wglCreateContext failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* ��DC��RC�뵱ǰ�̰߳����� */
	if(!wglMakeCurrent(gl_hdc, gl_hrc))
	{
		wglDeleteContext(gl_hrc);
		pi_log_print(LOG_ERROR, "wglMakeCurrent failed, error = %d\n", GetLastError());
		return NULL;
	}

	/* �����ԭ����һ������ʵ�б��������� */
	if(curr_rc != 0)
	{
		wglShareLists(curr_rc, gl_hrc);

		/* ��ԭ���е�OpenGL���� */
		if(!wglMakeCurrent(curr_dc, curr_rc))
		{
			pi_log_print(LOG_ERROR, "wglMakeCurrent 2 failed, error = %d\n", GetLastError());
			return NULL;
		}
	}

	/* ��ʼ�� GL API*/
	gl_Self_Init(type, NULL);

	/* OpenGL�汾������2.0���� */
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

	/* �رմ�ֱͬ�� */
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
		context = (void *)0x1;	/* ������0�ĵ�ַ���������߲��жϻ�����ʼ���ɹ� */
		gl_Self_Init(type, NULL);
		break;
	default:
		break;
	}

	/* ��ӡGL����չ */
	
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