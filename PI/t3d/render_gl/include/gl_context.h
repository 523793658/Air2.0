#ifndef INCLUDE_GL_CONTEXT_H
#define INCLUDE_GL_CONTEXT_H

#include <pi_lib.h>

#include <rendersystem.h>

PI_BEGIN_DECLS

/* 初始化，创建gl环境 */
void* gl_context_new(RenderContextLoadType type, void *data);

/* 释放gl环境 */
void gl_context_free(void *context);

/* 交换缓冲区 */
void gl_context_swapbuffer(void *context);

void gl_context_get_size(void *gl_context, sint *w, sint *h);

PI_END_DECLS

#endif /* INCLUDE_GL_CONTEXT_H */