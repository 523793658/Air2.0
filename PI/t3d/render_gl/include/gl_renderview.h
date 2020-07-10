#ifndef INCLUDE_GL_RENDERVIEW_H
#define INCLUDE_GL_RENDERVIEW_H

#include <renderview.h>
#include <rendersystem.h>

typedef PiBool (*GLRenderViewFunc)(PiRenderView *view, PiRenderTarget *target, TargetAttachment attachment);

typedef struct  
{
	uint32 gl_id;		/* 离线视图的OpenGL句柄 */
	uint32 gl_format;	/* 离线视图的OpenGL格式 */

	GLRenderViewFunc attach_func;
	GLRenderViewFunc detach_func;

	GLRenderViewFunc bind_func;		/* GL任何view都没有bind实现 */
	GLRenderViewFunc unbind_func;	/* 只有GraphicsBufferView有unbind实现 */
}GLRenderView;

#endif /* INCLUDE_GL_RENDERVIEW_H */