#ifndef INCLUDE_GL_RENDERVIEW_H
#define INCLUDE_GL_RENDERVIEW_H

#include <renderview.h>
#include <rendersystem.h>

typedef PiBool (*GLRenderViewFunc)(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

typedef struct  
{
	uint32 gl_id;		/* ������ͼ��OpenGL��� */
	uint32 gl_format;	/* ������ͼ��OpenGL��ʽ */

	GLRenderViewFunc attach_func;
	GLRenderViewFunc detach_func;

	GLRenderViewFunc bind_func;		/* GL�κ�view��û��bindʵ�� */
	GLRenderViewFunc unbind_func;	/* ֻ��GraphicsBufferView��unbindʵ�� */
}GLRenderView;

#endif /* INCLUDE_GL_RENDERVIEW_H */