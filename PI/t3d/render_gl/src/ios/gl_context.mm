
#include <gl_context.h>
#include <gl_interface.h>


#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import <OpenGLES/ES3/gl.h>

#import <QuartzCore/CAEAGLLayer.h>

typedef struct
{
    // gles版本
    EAGLRenderingAPI gl_version;
    
    // fbo目标的宽高
    GLint width;
    GLint height;
    
    // fbo句柄
    GLuint framebuffer;
    
    GLuint color_buffer;
    GLuint depth_buffer;
    
    CAEAGLLayer *layer;
    EAGLContext *context;
}GLIOSContext;

static void _destroyFramebuffer(GLIOSContext *context)
{
    context->width = 0;
    context->height = 0;
    
    if (context->framebuffer != 0)
    {
        glDeleteFramebuffers(1, &context->framebuffer);
        context->framebuffer = 0;
    }
    
    if (context->color_buffer != 0)
    {
        glDeleteRenderbuffers(1, &context->color_buffer);
        context->color_buffer = 0;
    }
    
    if(context->depth_buffer != 0)
    {
        glDeleteRenderbuffers(1, &context->depth_buffer);
        context->depth_buffer = 0;
    }
}

static PiBool _createFramebuffer(GLIOSContext *context)
{
    _destroyFramebuffer(context);
    
    glGenFramebuffers(1, &context->framebuffer);
    glGenRenderbuffers(1, &context->color_buffer);
    
    glBindFramebuffer(GL_FRAMEBUFFER, context->framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, context->color_buffer);
    
    if(![context->context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id<EAGLDrawable>) context->layer])
    {
        glGetError();
        pi_log_print(LOG_ERROR, "%s, Failed to bind the drawable to a renderbuffer object", __FUNCTION__);
        return FALSE;
    }
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &context->width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &context->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, context->color_buffer);
    
    glGenRenderbuffers(1, &context->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, context->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, context->width, context->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, context->depth_buffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, context->color_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, context->framebuffer);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glGetError();
        pi_log_print(LOG_ERROR, "%s -- Failed to make a complete framebuffer object", __FUNCTION__);
        return FALSE;
    }
    
    return TRUE;
}

void gl_context_free(void *gl_context)
{
    GLIOSContext *r = (GLIOSContext *)gl_context;
    
    _destroyFramebuffer(r);
    
    if ([EAGLContext currentContext] == r->context)
    {
        [EAGLContext setCurrentContext:nil];
    }
    
    [r->context release];
    [r->layer release];
}

void gl_context_swapbuffer(void *gl_context)
{
    GLIOSContext *r = (GLIOSContext *)gl_context;
    GLuint attachments[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
    uint attachment_count = sizeof(attachments) / sizeof(attachments[0]);
    
    glBindFramebuffer(GL_FRAMEBUFFER, r->framebuffer);
    
    if(r->gl_version == kEAGLRenderingAPIOpenGLES2)
    {
       glDiscardFramebufferEXT(GL_FRAMEBUFFER, attachment_count, attachments);
    }
    else {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, attachment_count, attachments);
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, r->color_buffer);
    if ([r->context presentRenderbuffer:GL_RENDERBUFFER] == NO)
    {
        glGetError();
        pi_log_print(LOG_ERROR, "Failed to swap buffers in %s", __FUNCTION__);
    }
}

void gl_context_get_size(void *gl_context, sint *w, sint *h)
{
}

void* gl_context_new(RenderContextLoadType type, void *win)
{
    GLint w, h;
    GLIOSContext *r = pi_new0(GLIOSContext, 1);
    CAEAGLLayer *layer = (CAEAGLLayer *)win;
    r->layer = [layer retain];
    
    r->gl_version = kEAGLRenderingAPIOpenGLES3;
    r->context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!r->context)
    {
        r->gl_version = kEAGLRenderingAPIOpenGLES2;
        r->context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    
    if(!r->context)
    {
        pi_log_print(LOG_ERROR, "init gles context failed, EAGLContext.alloc");
        pi_free(r);
        return NULL;
    }
    
    if (![EAGLContext setCurrentContext:r->context])
    {
        pi_log_print(LOG_ERROR, "init gles context failed, EAGLContext.setCurrentContext");
        pi_free(r);
        return NULL;
    }
    
    if (!_createFramebuffer(r))
    {
        pi_log_print(LOG_ERROR, "init gles context failed, _createFramebuffer");
        pi_free(r);
        return NULL;
    }
    
	return r;
}