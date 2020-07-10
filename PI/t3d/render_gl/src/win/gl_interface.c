#include <gl_interface.h>

#include "native_interface.h"

#define CHECK_GL_ERROR() 
// #define CHECK_GL_ERROR() do{ uint err; PI_ASSERT((err = gl2_GetError()) == GL2_NO_ERROR, "gl operation failed, error code = %x", err);}while(0)

static GLCap s_gl_cap;
static GLFunc *gl_context;

RenderInterfaceType PI_API gl_Self_GetInterfaceType(void)
{
	return gl_context->type;
}

void PI_API gl_Self_Init(RenderContextLoadType type, void *context)
{
	gl_context = pi_new0(GLFunc, 1);
	pi_memset_inline(&s_gl_cap, 0, sizeof(s_gl_cap));

	s_gl_cap.type = type;
	gl_context->type = RIT_NULL;
	switch (type)
	{
	case CONTEXT_LOAD_NATIVE:
		gl_context->type = RIT_OPENGL;
		native_Self_Init(gl_context, &s_gl_cap);
		break;
	default:
		break;
	}
}

uint gl_Self_GetVersion(PiBool *is_es)
{
	uint version = 210;
	if(gl_context->type != RIT_NULL)
	{
		uint major = 0, mijor = 0;
		byte *version_str = (byte *)gl2_GetString(GL2_VERSION);
		if(is_es) 
		{
			*is_es = FALSE;
		}

		if(version_str != NULL)
		{
			uint len;

			pi_log_print(LOG_INFO, "GL Version = %s", version_str);

			if(pi_str_start_with((const char *)version_str, "OpenGL ES "))
			{
				if(is_es) 
				{
					*is_es = TRUE;
				}
				version_str += pi_strlen("OpenGL ES ");
			}
			else if(pi_str_start_with((const char *)version_str, "OpenGL "))
			{
				version_str += pi_strlen("OpenGL ");
			}

			len = pi_strlen((char *)version_str);
			if(len > 3)
			{
				major = version_str[0] - '0';
			 	mijor = version_str[2] - '0';
			}
		}
		version = major * 100 + mijor * 10;
	}
	return version;
}

PiBool PI_API gl_Self_IsSupportExtension(const char *externsion)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		sint index = 0;
		uint length = pi_strlen(externsion);
		char *str = str = (char *)gl2_GetString(GL2_EXTENSIONS);

		do {
			index = pi_str_text_index(str, externsion);
			if(index < 0) {
				break;
			}

			if(index > 0 && str[index - 1] != ' ')
			{/* 前面不为空格 */
				str += index + length;
				continue;
			}

			if(str[index + length] != ' ' || str[index + length] != '\0') 
			{/* 后面不为空格或者结尾 */
				break;
			}

			str += index + length;
		} while (TRUE);

		r = (index >= 0);
	}
	return r;
}

PiBool PI_API gl_Self_IsLostContext(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL && gl_context->_Self_IsLostContext)
	{
		r = gl_context->_Self_IsLostContext();
	}
	return r;
}

void PI_API gl2_ActiveTexture (unsigned int texture) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ActiveTexture(texture);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_AttachShader (unsigned int program, unsigned int shader) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_AttachShader(program, shader);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BindAttribLocation (unsigned int program, unsigned int index, const char* name) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindAttribLocation(program, index, name);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BindBuffer (unsigned int target, unsigned int buffer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindBuffer(target, buffer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BindFramebuffer (unsigned int target, unsigned int framebuffer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindFramebuffer(target, framebuffer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BindRenderbuffer (unsigned int target, unsigned int renderbuffer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindRenderbuffer(target, renderbuffer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BindTexture (unsigned int target, unsigned int texture) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindTexture(target, texture);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BlendColor (float red, float green, float blue, float alpha) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlendColor(red, green, blue, alpha);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BlendEquation (unsigned int mode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlendEquation(mode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BlendEquationSeparate (unsigned int modeRGB, unsigned int modeAlpha) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlendEquationSeparate(modeRGB, modeAlpha);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BlendFunc (unsigned int sfactor, unsigned int dfactor) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlendFunc(sfactor, dfactor);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BlendFuncSeparate (unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BufferData (unsigned int target, unsigned int size, const void* data, unsigned int usage) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BufferData(target, size, data, usage);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_BufferSubData (unsigned int target, int offset, unsigned int size, const void* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BufferSubData(target, offset, size, data);
		CHECK_GL_ERROR();
	}
}

unsigned int  PI_API gl2_CheckFramebufferStatus (unsigned int target) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_CheckFramebufferStatus(target);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_Clear (unsigned int mask) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Clear(mask);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ClearColor (float red, float green, float blue, float alpha) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearColor(red, green, blue, alpha);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ClearDepthf (float depth) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearDepthf(depth);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ClearStencil (int s) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearStencil(s);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ColorMask (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ColorMask(red, green, blue, alpha);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_CompileShader (unsigned int shader) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CompileShader(shader);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_CompressedTexImage2D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, int border, unsigned int imageSize, const void* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_CompressedTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int imageSize, const void* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_CopyTexImage2D (unsigned int target, int level, unsigned int internalformat, int x, int y, unsigned int width, unsigned int height, int border) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_CopyTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, int x, int y, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
		CHECK_GL_ERROR();
	}
}

unsigned int  PI_API gl2_CreateProgram (void) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_CreateProgram();
		CHECK_GL_ERROR();
	}
	return r;
}
unsigned int  PI_API gl2_CreateShader (unsigned int type) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_CreateShader(type);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_CullFace (unsigned int mode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CullFace(mode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DeleteBuffers (unsigned int n, const unsigned int* buffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteBuffers(n, buffers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DeleteFramebuffers (unsigned int n, const unsigned int* framebuffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteFramebuffers(n, framebuffers);
		//CHECK_GL_ERROR();
	}
}

void PI_API gl2_DeleteProgram (unsigned int program) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteProgram(program);
		//CHECK_GL_ERROR();
	}
}

void PI_API gl2_DeleteRenderbuffers (unsigned int n, const unsigned int* renderbuffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteRenderbuffers(n, renderbuffers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DeleteShader (unsigned int shader) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteShader(shader);
	}
}

void PI_API gl2_DeleteTextures (unsigned int n, const unsigned int* textures) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteTextures(n, textures);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DepthFunc (unsigned int func) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DepthFunc(func);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DepthMask (unsigned char flag) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DepthMask(flag);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DepthRangef (float n, float f) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DepthRangef(n, f);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DetachShader (unsigned int program, unsigned int shader) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DetachShader(program, shader);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Disable (unsigned int cap) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Disable(cap);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DisableVertexAttribArray (unsigned int index) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DisableVertexAttribArray(index);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DrawArrays (unsigned int mode, int first, unsigned int count) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawArrays(mode, first, count);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_DrawElements (unsigned int mode, unsigned int count, unsigned int type, const void* indices) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawElements(mode, count, type, indices);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Enable (unsigned int cap) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Enable(cap);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_EnableVertexAttribArray (unsigned int index) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_EnableVertexAttribArray(index);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Finish (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Finish();
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Flush (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Flush();
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_FramebufferRenderbuffer (unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_FramebufferTexture2D (unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FramebufferTexture2D(target, attachment, textarget, texture, level);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_FrontFace (unsigned int mode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FrontFace(mode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GenBuffers (unsigned int n, unsigned int* buffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenBuffers(n, buffers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GenerateMipmap (unsigned int target) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenerateMipmap(target);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GenFramebuffers (unsigned int n, unsigned int* framebuffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenFramebuffers(n, framebuffers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GenRenderbuffers (unsigned int n, unsigned int* renderbuffers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenRenderbuffers(n, renderbuffers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GenTextures (unsigned int n, unsigned int* textures) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenTextures(n, textures);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetActiveAttrib (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetActiveAttrib(program, index, bufsize, length, size, type, name);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetActiveUniform (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetActiveUniform(program, index, bufsize, length, size, type, name);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetAttachedShaders (unsigned int program, unsigned int maxcount, unsigned int* count, unsigned int* shaders) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetAttachedShaders(program, maxcount, count, shaders);
		CHECK_GL_ERROR();
	}
}

int   PI_API gl2_GetAttribLocation (unsigned int program, const char* name) 
{
	int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetAttribLocation(program, name);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_GetBooleanv (unsigned int pname, unsigned char* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetBooleanv(pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetBufferParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetBufferParameteriv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

unsigned int  PI_API gl2_GetError (void) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetError();
	}
	return r;
}

void PI_API gl2_GetFloatv (unsigned int pname, float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetFloatv(pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetFramebufferAttachmentParameteriv (unsigned int target, unsigned int attachment, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetIntegerv (unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetIntegerv(pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetProgramiv (unsigned int program, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetProgramiv(program, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetProgramInfoLog (unsigned int program, unsigned int bufsize, unsigned int* length, char* infolog) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetProgramInfoLog(program, bufsize, length, infolog);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetRenderbufferParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetRenderbufferParameteriv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetShaderiv (unsigned int shader, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetShaderiv(shader, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetShaderInfoLog (unsigned int shader, unsigned int bufsize, unsigned int* length, char* infolog) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetShaderInfoLog(shader, bufsize, length, infolog);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetShaderPrecisionFormat (unsigned int shadertype, unsigned int precisiontype, int* range, int* precision) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetShaderSource (unsigned int shader, unsigned int bufsize, unsigned int* length, char* source) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetShaderSource(shader, bufsize, length, source);
		CHECK_GL_ERROR();
	}
}

const unsigned char* PI_API gl2_GetString (unsigned int name) 
{
	const unsigned char *r = NULL;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetString(name);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_GetTexParameterfv (unsigned int target, unsigned int pname, float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetTexParameterfv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetTexParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetTexParameteriv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetUniformfv (unsigned int program, int location, float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetUniformfv(program, location, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetUniformiv (unsigned int program, int location, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetUniformiv(program, location, params);
		CHECK_GL_ERROR();
	}
}

int   PI_API gl2_GetUniformLocation (unsigned int program, const char* name) 
{
	int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetUniformLocation(program, name);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_GetVertexAttribfv (unsigned int index, unsigned int pname, float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetVertexAttribfv(index, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetVertexAttribiv (unsigned int index, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetVertexAttribiv(index, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_GetVertexAttribPointerv (unsigned int index, unsigned int pname, void** pointer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetVertexAttribPointerv(index, pname, pointer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Hint (unsigned int target, unsigned int mode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Hint(target, mode);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl2_IsBuffer (unsigned int buffer) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsBuffer(buffer);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsEnabled (unsigned int cap) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsEnabled(cap);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsFramebuffer (unsigned int framebuffer) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsFramebuffer(framebuffer);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsProgram (unsigned int program) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsProgram(program);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsRenderbuffer (unsigned int renderbuffer) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsRenderbuffer(renderbuffer);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsShader (unsigned int shader) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsShader(shader);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl2_IsTexture (unsigned int texture) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsTexture(texture);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl2_LineWidth (float width) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_LineWidth(width);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_LinkProgram (unsigned int program) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_LinkProgram(program);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_PixelStorei (unsigned int pname, int param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_PixelStorei(pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_PolygonOffset (float factor, float units) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_PolygonOffset(factor, units);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ReadPixels (int x, int y, unsigned int width, unsigned int height, unsigned int format, unsigned int type, void* pixels) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ReadPixels(x, y, width, height, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ReleaseShaderCompiler (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ReleaseShaderCompiler();
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_RenderbufferStorage (unsigned int target, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_RenderbufferStorage(target, internalformat, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_SampleCoverage (float value, unsigned char invert) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_SampleCoverage(value, invert);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Scissor (int x, int y, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Scissor(x, y, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ShaderBinary (unsigned int n, const unsigned int* shaders, unsigned int binaryformat, const void* binary, unsigned int length) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ShaderBinary(n, shaders, binaryformat, binary, length);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ShaderSource (unsigned int shader, unsigned int count, const char* const* string, const int* length) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ShaderSource(shader, count, string, length);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilFunc (unsigned int func, int ref, unsigned int mask) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilFunc(func, ref, mask);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilFuncSeparate (unsigned int face, unsigned int func, int ref, unsigned int mask) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilFuncSeparate(face, func, ref, mask);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilMask (unsigned int mask) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilMask(mask);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilMaskSeparate (unsigned int face, unsigned int mask) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilMaskSeparate(face, mask);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilOp (unsigned int fail, unsigned int zfail, unsigned int zpass) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilOp(fail, zfail, zpass);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_StencilOpSeparate (unsigned int face, unsigned int fail, unsigned int zfail, unsigned int zpass) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_StencilOpSeparate(face, fail, zfail, zpass);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexImage2D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexParameterf (unsigned int target, unsigned int pname, float param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexParameterf(target, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexParameterfv (unsigned int target, unsigned int pname, const float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexParameterfv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexParameteri (unsigned int target, unsigned int pname, int param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexParameteri(target, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexParameteriv (unsigned int target, unsigned int pname, const int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexParameteriv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_TexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform1f (int location, float x) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1f(location, x);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform1fv (int location, unsigned int count, const float* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1fv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform1i (int location, int x) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1i(location, x);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform1iv (int location, unsigned int count, const int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1iv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform2f (int location, float x, float y) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2f(location, x, y);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform2fv (int location, unsigned int count, const float* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2fv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform2i (int location, int x, int y) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2i(location, x, y);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform2iv (int location, unsigned int count, const int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2iv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform3f (int location, float x, float y, float z) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3f(location, x, y, z);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform3fv (int location, unsigned int count, const float* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3fv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform3i (int location, int x, int y, int z) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3i(location, x, y, z);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform3iv (int location, unsigned int count, const int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3iv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform4f (int location, float x, float y, float z, float w) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4f(location, x, y, z, w);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform4fv (int location, unsigned int count, const float* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4fv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform4i (int location, int x, int y, int z, int w) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4i(location, x, y, z, w);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Uniform4iv (int location, unsigned int count, const int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4iv(location, count, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_UniformMatrix2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix2fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_UniformMatrix3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix3fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_UniformMatrix4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix4fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_UseProgram (unsigned int program) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UseProgram(program);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_ValidateProgram (unsigned int program) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ValidateProgram(program);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib1f (unsigned int indx, float x) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib1f(indx, x);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib1fv (unsigned int indx, const float* values) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib1fv(indx, values);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib2f (unsigned int indx, float x, float y) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib2f(indx, x, y);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib2fv (unsigned int indx, const float* values) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib2fv(indx, values);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib3f (unsigned int indx, float x, float y, float z) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib3f(indx, x, y, z);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib3fv (unsigned int indx, const float* values) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib3fv(indx, values);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib4f (unsigned int indx, float x, float y, float z, float w) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib4f(indx, x, y, z, w);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttrib4fv (unsigned int indx, const float* values) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttrib4fv(indx, values);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_VertexAttribPointer (unsigned int indx, int size, unsigned int type, unsigned char normalized, unsigned int stride, const void* ptr) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribPointer(indx, size, type, normalized, stride, ptr);
		CHECK_GL_ERROR();
	}
}

void PI_API gl2_Viewport (int x, int y, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Viewport(x, y, width, height);
		CHECK_GL_ERROR();
	}
}


/* OpenGL ES 3.0 */

void PI_API gl3_ReadBuffer (unsigned int mode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ReadBuffer(mode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DrawRangeElements (unsigned int mode, unsigned int start, unsigned int end, unsigned int count, unsigned int type, const void* indices) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawRangeElements(mode, start, end, count, type, indices);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_TexImage3D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int format, unsigned int type, const void* pixels) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_TexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int type, const void* pixels) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_CopyTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, int x, int y, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_CompressedTexImage3D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int imageSize, const void* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_CompressedTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int imageSize, const void* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GenQueries (unsigned int n, unsigned int* ids) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenQueries(n, ids);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DeleteQueries (unsigned int n, const unsigned int* ids) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteQueries(n, ids);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl3_IsQuery (unsigned int id) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsQuery(id);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_BeginQuery (unsigned int target, unsigned int id) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BeginQuery(target, id);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_EndQuery (unsigned int target) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_EndQuery(target);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetQueryiv (unsigned int target, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetQueryiv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetQueryObjectuiv (unsigned int id, unsigned int pname, unsigned int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetQueryObjectuiv(id, pname, params);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl3_UnmapBuffer (unsigned int target) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_UnmapBuffer(target);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_GetBufferPointerv (unsigned int target, unsigned int pname, void** params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetBufferPointerv(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DrawBuffers (unsigned int n, const unsigned int* bufs) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawBuffers(n, bufs);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix2x3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix2x3fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix3x2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix3x2fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix2x4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix2x4fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix4x2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix4x2fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix3x4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix3x4fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformMatrix4x3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformMatrix4x3fv(location, count, transpose, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BlitFramebuffer (int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_RenderbufferStorageMultisample (unsigned int target, unsigned int samples, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_RenderbufferStorageMultisample(target, samples, internalformat, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_FramebufferTextureLayer (unsigned int target, unsigned int attachment, unsigned int texture, int level, int layer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FramebufferTextureLayer(target, attachment, texture, level, layer);
		CHECK_GL_ERROR();
	}
}

void* PI_API gl3_MapBufferRange (unsigned int target, int offset, unsigned int length, unsigned int access) 
{
	void *r = NULL;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_MapBufferRange(target, offset, length, access);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_FlushMappedBufferRange (unsigned int target, int offset, unsigned int length) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FlushMappedBufferRange(target, offset, length);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BindVertexArray (unsigned int array) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindVertexArray(array);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DeleteVertexArrays (unsigned int n, const unsigned int* arrays) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteVertexArrays(n, arrays);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GenVertexArrays (unsigned int n, unsigned int* arrays) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenVertexArrays(n, arrays);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl3_IsVertexArray (unsigned int array) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsVertexArray(array);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_GetIntegeri_v (unsigned int target, unsigned int index, int* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetIntegeri_v(target, index, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BeginTransformFeedback (unsigned int primitiveMode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BeginTransformFeedback(primitiveMode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_EndTransformFeedback (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_EndTransformFeedback();
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BindBufferRange (unsigned int target, unsigned int index, unsigned int buffer, int offset, unsigned int size) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindBufferRange(target, index, buffer, offset, size);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BindBufferBase (unsigned int target, unsigned int index, unsigned int buffer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindBufferBase(target, index, buffer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_TransformFeedbackVaryings (unsigned int program, unsigned int count, const char* const* varyings, unsigned int bufferMode) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TransformFeedbackVaryings(program, count, varyings, bufferMode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetTransformFeedbackVarying (unsigned int program, unsigned int index, unsigned int bufSize, unsigned int* length, unsigned int* size, unsigned int* type, char* name) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetTransformFeedbackVarying(program, index, bufSize, length, size,type, name);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribIPointer (unsigned int index, int size, unsigned int type, unsigned int stride, const void* pointer) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribIPointer(index, size, type, stride, pointer);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetVertexAttribIiv (unsigned int index, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetVertexAttribIiv(index, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetVertexAttribIuiv (unsigned int index, unsigned int pname, unsigned int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetVertexAttribIuiv(index, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribI4i (unsigned int index, int x, int y, int z, int w) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribI4i(index, x, y, z, w);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribI4ui (unsigned int index, unsigned int x, unsigned int y, unsigned int z, unsigned int w) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribI4ui(index, x, y, z, w);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribI4iv (unsigned int index, const int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribI4iv(index, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribI4uiv (unsigned int index, const unsigned int* v) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribI4uiv(index, v);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetUniformuiv (unsigned int program, int location, unsigned int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetUniformuiv(program, location, params);
		CHECK_GL_ERROR();
	}
}

int   PI_API gl3_GetFragDataLocation (unsigned int program, const char *name) 
{
	int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetFragDataLocation(program, name);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_Uniform1ui (int location, unsigned int v0) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1ui(location, v0);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform2ui (int location, unsigned int v0, unsigned int v1) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2ui(location, v0, v1);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform3ui (int location, unsigned int v0, unsigned int v1, unsigned int v2) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3ui(location, v0, v1, v2);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform4ui (int location, unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4ui(location, v0, v1, v2, v3);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform1uiv (int location, unsigned int count, const unsigned int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform1uiv(location, count, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform2uiv (int location, unsigned int count, const unsigned int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform2uiv(location, count, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform3uiv (int location, unsigned int count, const unsigned int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform3uiv(location, count, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_Uniform4uiv (int location, unsigned int count, const unsigned int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_Uniform4uiv(location, count, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ClearBufferiv (unsigned int buffer, int drawbuffer, const int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearBufferiv(buffer, drawbuffer, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ClearBufferuiv (unsigned int buffer, int drawbuffer, const unsigned int* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearBufferuiv(buffer, drawbuffer, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ClearBufferfv (unsigned int buffer, int drawbuffer, const float* value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearBufferfv(buffer, drawbuffer, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ClearBufferfi (unsigned int buffer, int drawbuffer, float depth, int stencil) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ClearBufferfi(buffer, drawbuffer, depth, stencil);
		CHECK_GL_ERROR();
	}
}

const unsigned char* PI_API gl3_GetStringi (unsigned int name, unsigned int index) 
{
	const unsigned char *r = NULL;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetStringi(name, index);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_CopyBufferSubData (unsigned int readTarget, unsigned int writeTarget, int readOffset, int writeOffset, unsigned int size) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetUniformIndices (unsigned int program, unsigned int uniformCount, const char* const* uniformNames, unsigned int* uniformIndices) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetActiveUniformsiv (unsigned int program, unsigned int uniformCount, const unsigned int* uniformIndices, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
		CHECK_GL_ERROR();
	}
}

unsigned int  PI_API gl3_GetUniformBlockIndex (unsigned int program, const char* uniformBlockName) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_GetUniformBlockIndex(program, uniformBlockName);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_GetActiveUniformBlockiv (unsigned int program, unsigned int uniformBlockIndex, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetActiveUniformBlockName (unsigned int program, unsigned int uniformBlockIndex, unsigned int bufSize, unsigned int* length, char* uniformBlockName) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_UniformBlockBinding (unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DrawArraysInstanced (unsigned int mode, int first, unsigned int count, unsigned int instanceCount) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawArraysInstanced(mode, first, count, instanceCount);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DrawElementsInstanced (unsigned int mode, unsigned int count, unsigned int type, const void* indices, unsigned int instanceCount) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DrawElementsInstanced(mode, count, type, indices, instanceCount);
		CHECK_GL_ERROR();
	}
}

void*  PI_API gl3_FenceSync (unsigned int condition, unsigned int flags) 
{
	void *r = NULL;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_FenceSync(condition, flags);
		CHECK_GL_ERROR();
	}
	return r;
}

unsigned char  PI_API gl3_IsSync (void* sync) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsSync(sync);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_DeleteSync (void* sync) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteSync(sync);
		CHECK_GL_ERROR();
	}
}

unsigned int  PI_API gl3_ClientWaitSync (void* sync, unsigned int flags, unsigned long long timeout) 
{
	unsigned int r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_ClientWaitSync(sync, flags, timeout);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_WaitSync (void* sync, unsigned int flags, unsigned long long timeout) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_WaitSync(sync, flags, timeout);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetInteger64v (unsigned int pname, long long* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetInteger64v(pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetSynciv (void* sync, unsigned int pname, unsigned int bufSize, unsigned int* length, int* values) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetSynciv(sync, pname, bufSize, length, values);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetInteger64i_v (unsigned int target, unsigned int index, long long* data) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetInteger64i_v(target, index, data);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetBufferParameteri64v (unsigned int target, unsigned int pname, long long* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetBufferParameteri64v(target, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GenSamplers (unsigned int count, unsigned int* samplers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenSamplers(count, samplers);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DeleteSamplers (unsigned int count, const unsigned int* samplers) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteSamplers(count, samplers);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl3_IsSampler (unsigned int sampler) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsSampler(sampler);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_BindSampler (unsigned int unit, unsigned int sampler) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindSampler(unit, sampler);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_SamplerParameteri (unsigned int sampler, unsigned int pname, int param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_SamplerParameteri(sampler, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_SamplerParameteriv (unsigned int sampler, unsigned int pname, const int* param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_SamplerParameteriv(sampler, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_SamplerParameterf (unsigned int sampler, unsigned int pname, float param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_SamplerParameterf(sampler, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_SamplerParameterfv (unsigned int sampler, unsigned int pname, const float* param) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_SamplerParameterfv(sampler, pname, param);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetSamplerParameteriv (unsigned int sampler, unsigned int pname, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetSamplerParameteriv(sampler, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetSamplerParameterfv (unsigned int sampler, unsigned int pname, float* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetSamplerParameterfv(sampler, pname, params);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_VertexAttribDivisor (unsigned int index, unsigned int divisor) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_VertexAttribDivisor(index, divisor);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_BindTransformFeedback (unsigned int target, unsigned int id) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_BindTransformFeedback(target, id);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_DeleteTransformFeedbacks (unsigned int n, const unsigned int* ids) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_DeleteTransformFeedbacks(n, ids);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GenTransformFeedbacks (unsigned int n, unsigned int* ids) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GenTransformFeedbacks(n, ids);
		CHECK_GL_ERROR();
	}
}

unsigned char  PI_API gl3_IsTransformFeedback (unsigned int id) 
{
	unsigned char r = 0;
	if(gl_context->type != RIT_NULL)
	{
		r = gl_context->_IsTransformFeedback(id);
		CHECK_GL_ERROR();
	}
	return r;
}

void PI_API gl3_PauseTransformFeedback (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_PauseTransformFeedback();
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ResumeTransformFeedback (void) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ResumeTransformFeedback();
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetProgramBinary (unsigned int program, unsigned int bufSize, unsigned int* length, unsigned int* binaryFormat, void* binary) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetProgramBinary(program, bufSize, length, binaryFormat, binary);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ProgramBinary (unsigned int program, unsigned int binaryFormat, const void* binary, unsigned int length) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ProgramBinary(program, binaryFormat, binary, length);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_ProgramParameteri (unsigned int program, unsigned int pname, int value) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_ProgramParameteri(program, pname, value);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_InvalidateFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_InvalidateFramebuffer(target, numAttachments, attachments);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_InvalidateSubFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments, int x, int y, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_TexStorage2D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexStorage2D(target, levels, internalformat, width, height);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_TexStorage3D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_TexStorage3D(target, levels, internalformat, width, height, depth);
		CHECK_GL_ERROR();
	}
}

void PI_API gl3_GetInternalformativ (unsigned int target, unsigned int internalformat, unsigned int pname, unsigned int bufSize, int* params) 
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetInternalformativ(target, internalformat, pname, bufSize, params);
		CHECK_GL_ERROR();
	}
}

PiBool PI_API gl_Self_IsGLES(void)
{
	PiBool r = FALSE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsEsVersion;
	}
	return r;
}

PiBool PI_API gl_Self_IsVersion3(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap.gl_version >= 300;
	}
	return r;
}

PiBool PI_API gl_Self_IsGpuShader4(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsGpuShader4;
	}
	return r;
}

PiBool PI_API gl_Self_IsCopyImage(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsCopyImage;
	}
	return r;
}

PiBool PI_API gl_Self_IsFramebufferSRGB(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsFramebufferSRGB;
	}
	return r;
}

PiBool PI_API gl_Self_IsHalfFloatPixel(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsHalfFloatPixel;
	}
	return r;
}

PiBool PI_API gl_Self_IsPackedFloat(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsPackedFloat;
	}
	return r;
}

PiBool PI_API gl_Self_IsPackedDepthStencil(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsPackedDepthStencil;
	}
	return r;
}

PiBool PI_API gl_Self_IsTexture3D(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsTexture3D;
	}
	return r;
}

PiBool PI_API gl_Self_IsDrawInstance(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsInstanced;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureLOD(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureLOD;
	}
	return r;
}

PiBool PI_API gl_Self_IsFramebufferBlit(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsFramebufferBlit;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureArray(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureArray;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureRG(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureRG;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureFloat(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureFloat;
	}
	return r;
}

PiBool PI_API gl_Self_IsDrawBuffers(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsDrawBuffers;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureInteger(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureInteger;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureSRGB(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureSRGB;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureSnorm(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureSnorm;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureCompressionLatc(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureCompressionLatc;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureCompressionS3tc(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureCompressionS3tc;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureCompressionRgtc(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureCompressionRgtc;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureCompressionBptc(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsTextureCompressionBptc;
	}
	return r;
}

PiBool PI_API gl_Self_IsTextureFilterAnisotropic(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r =  s_gl_cap._IsTextureFilterAnisotropic;
	}
	return r;
}

PiBool PI_API gl_Self_IsVertexType_2_10_10_10_rev(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsVertexType_2_10_10_10_rev;
	}
	return r;
}

PiBool PI_API gl_Self_IsVertexArrayObject(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsVertexArrayObject;
	}
	return r;
}

PiBool PI_API gl_Self_IsDepthClamp(void)
{
	PiBool r = TRUE;
	if(gl_context->type != RIT_NULL)
	{
		r = s_gl_cap._IsDepthClamp;
	}
	return r;
}

void PI_API gl_PolygonMode(unsigned int face, unsigned int mode)
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_PolygonMode(face, mode);
		CHECK_GL_ERROR();
	}
}

void PI_API gl_GetTexImage(unsigned int target, int level, unsigned int format, unsigned int type, void* pixels)
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetTexImage(target, level, format, type, pixels);
		CHECK_GL_ERROR();
	}
}

void PI_API gl_GetCompressedTexImage(unsigned int target, int level, void* img)
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_GetCompressedTexImage(target, level, img);
		CHECK_GL_ERROR();
	}
}

void PI_API gl_CopyImageSubData(unsigned int srcName, unsigned int srcTarget, int srcLevel, int srcX, int srcY, int srcZ, unsigned int dstName, unsigned int dstTarget, int dstLevel, int dstX, int dstY, int dstZ, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth)
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_CopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
		CHECK_GL_ERROR();
	}
}

void PI_API gl_FramebufferTexture3D(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, sint level, sint zoffset)
{
	if(gl_context->type != RIT_NULL)
	{
		gl_context->_FramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
		CHECK_GL_ERROR();
	}
}
