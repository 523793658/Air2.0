#ifndef INCLUDE_GL_CONTEXT_H
#define INCLUDE_GL_CONTEXT_H

#include <pi_lib.h>

#include <rendersystem.h>

PI_BEGIN_DECLS

/* ��ʼ��������gl���� */
void* gl_context_new(RenderContextLoadType type, void *data);

/* �ͷ�gl���� */
void gl_context_free(void *context);

/* ���������� */
void gl_context_swapbuffer(void *context);

void gl_context_get_size(void *gl_context, sint *w, sint *h);

PI_END_DECLS

#endif /* INCLUDE_GL_CONTEXT_H */