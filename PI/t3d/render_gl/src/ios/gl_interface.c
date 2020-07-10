#include <gl_interface.h>
#import <OpenGLES/ES3/gl.h>

#define CHECK_GL_ERROR() 
// #define CHECK_GL_ERROR() do{ uint err; PI_ASSERT((err = gl2_GetError()) == GL2_NO_ERROR, "gl operation failed, error code = %x", err);}while(0)

RenderInterfaceType PI_API gl_Self_GetInterfaceType(void)
{
	return RIT_GLES;
}

PiBool PI_API gl_Self_IsLostContext(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsDrawBuffers(void)
{
	return FALSE;
}

void PI_API gl_Self_Init(RenderContextLoadType type, void *context)
{
}

void PI_API gl2_ActiveTexture (unsigned int texture) 
{
	glActiveTexture(texture);
	CHECK_GL_ERROR();
}

void PI_API gl2_AttachShader (unsigned int program, unsigned int shader) 
{
	glAttachShader(program, shader);
	CHECK_GL_ERROR();
}

void PI_API gl2_BindAttribLocation (unsigned int program, unsigned int index, const char* name) 
{
	glBindAttribLocation(program, index, name);
	CHECK_GL_ERROR();
}

void PI_API gl2_BindBuffer(unsigned int target, unsigned int buffer)
{
	glBindBuffer(target, buffer);
	CHECK_GL_ERROR();
}

void PI_API gl2_BindFramebuffer (unsigned int target, unsigned int framebuffer) 
{
	glBindFramebuffer(target, framebuffer);
	CHECK_GL_ERROR();
}

void PI_API gl2_BindRenderbuffer (unsigned int target, unsigned int renderbuffer) 
{
	glBindRenderbuffer(target, renderbuffer);
	CHECK_GL_ERROR();
}

void PI_API gl2_BindTexture (unsigned int target, unsigned int texture) 
{
	glBindTexture(target, texture);
	CHECK_GL_ERROR();
}

void PI_API gl2_BlendColor (float red, float green, float blue, float alpha) 
{
	glBlendColor(red, green, blue, alpha);
	CHECK_GL_ERROR();
}

void PI_API gl2_BlendEquation (unsigned int mode) 
{
	glBlendEquation(mode);
	CHECK_GL_ERROR();
}

void PI_API gl2_BlendEquationSeparate (unsigned int modeRGB, unsigned int modeAlpha) 
{
	glBlendEquationSeparate(modeRGB, modeAlpha);
	CHECK_GL_ERROR();
}

void PI_API gl2_BlendFunc (unsigned int sfactor, unsigned int dfactor) 
{
	glBlendFunc(sfactor, dfactor);
	CHECK_GL_ERROR();
}

void PI_API gl2_BlendFuncSeparate (unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha) 
{
	glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
	CHECK_GL_ERROR();
}

void PI_API gl2_BufferData (unsigned int target, unsigned int size, const void* data, unsigned int usage) 
{
	glBufferData(target, size, data, usage);
	CHECK_GL_ERROR();
}

void PI_API gl2_BufferSubData (unsigned int target, int offset, unsigned int size, const void* data) 
{
	glBufferSubData(target, offset, size, data);
	CHECK_GL_ERROR();
}

unsigned int  PI_API gl2_CheckFramebufferStatus (unsigned int target) 
{
	unsigned int r = glCheckFramebufferStatus(target);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl2_Clear (unsigned int mask) 
{
	glClear(mask);
	CHECK_GL_ERROR();
}

void PI_API gl2_ClearColor (float red, float green, float blue, float alpha) 
{
	glClearColor(red, green, blue, alpha);
	CHECK_GL_ERROR();
}

void PI_API gl2_ClearDepthf (float depth) 
{
	glClearDepthf(depth);
	CHECK_GL_ERROR();
}

void PI_API gl2_ClearStencil (int s) 
{
	glClearStencil(s);
	CHECK_GL_ERROR();
}

void PI_API gl2_ColorMask (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) 
{
	glColorMask(red, green, blue, alpha);
	CHECK_GL_ERROR();
}

void PI_API gl2_CompileShader (unsigned int shader) 
{
	glCompileShader(shader);
	CHECK_GL_ERROR();
}

void PI_API gl2_CompressedTexImage2D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, int border, unsigned int imageSize, const void* data) 
{
	glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
	CHECK_GL_ERROR();
}

void PI_API gl2_CompressedTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int imageSize, const void* data) 
{
	glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	CHECK_GL_ERROR();
}

void PI_API gl2_CopyTexImage2D (unsigned int target, int level, unsigned int internalformat, int x, int y, unsigned int width, unsigned int height, int border) 
{
	glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	CHECK_GL_ERROR();
}

void PI_API gl2_CopyTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, int x, int y, unsigned int width, unsigned int height) 
{
	glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	CHECK_GL_ERROR();
}

unsigned int  PI_API gl2_CreateProgram (void) 
{
	unsigned int r = glCreateProgram();
	CHECK_GL_ERROR();
	return r;
}

unsigned int  PI_API gl2_CreateShader (unsigned int type) 
{
	unsigned int r = glCreateShader(type);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl2_CullFace (unsigned int mode) 
{
	glCullFace(mode);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteBuffers (unsigned int n, const unsigned int* buffers) 
{
	glDeleteBuffers(n, buffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteFramebuffers (unsigned int n, const unsigned int* framebuffers) 
{
	glDeleteFramebuffers(n, framebuffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteProgram (unsigned int program) 
{
	glDeleteProgram(program);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteRenderbuffers (unsigned int n, const unsigned int* renderbuffers) 
{
	glDeleteRenderbuffers(n, renderbuffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteShader (unsigned int shader) 
{
	glDeleteShader(shader);
	CHECK_GL_ERROR();
}

void PI_API gl2_DeleteTextures (unsigned int n, const unsigned int* textures) 
{
	glDeleteTextures(n, textures);
	CHECK_GL_ERROR();
}

void PI_API gl2_DepthFunc (unsigned int func) 
{
	glDepthFunc(func);
	CHECK_GL_ERROR();
}

void PI_API gl2_DepthMask (unsigned char flag) 
{
	glDepthMask(flag);
	CHECK_GL_ERROR();
}

void PI_API gl2_DepthRangef (float n, float f) 
{
	glDepthRangef(n, f);
	CHECK_GL_ERROR();
}

void PI_API gl2_DetachShader (unsigned int program, unsigned int shader) 
{
	glDetachShader(program, shader);
	CHECK_GL_ERROR();
}

void PI_API gl2_Disable (unsigned int cap) 
{
	glDisable(cap);
	CHECK_GL_ERROR();
}

void PI_API gl2_DisableVertexAttribArray (unsigned int index) 
{
	glDisableVertexAttribArray(index);
	CHECK_GL_ERROR();
}

void PI_API gl2_DrawArrays (unsigned int mode, int first, unsigned int count) 
{
	glDrawArrays(mode, first, count);
	CHECK_GL_ERROR();
}

void PI_API gl2_DrawElements (unsigned int mode, unsigned int count, unsigned int type, const void* indices) 
{
	glDrawElements(mode, count, type, indices);
	CHECK_GL_ERROR();
}

void PI_API gl2_Enable (unsigned int cap) 
{
	glEnable(cap);
	CHECK_GL_ERROR();
}

void PI_API gl2_EnableVertexAttribArray (unsigned int index) 
{
	glEnableVertexAttribArray(index);
	CHECK_GL_ERROR();
}

void PI_API gl2_Finish (void) 
{
	glFinish();
	CHECK_GL_ERROR();
}

void PI_API gl2_Flush (void) 
{
	glFlush();
	CHECK_GL_ERROR();
}

void PI_API gl2_FramebufferRenderbuffer (unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer) 
{
	glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	CHECK_GL_ERROR();
}

void PI_API gl2_FramebufferTexture2D (unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level) 
{
	glFramebufferTexture2D(target, attachment, textarget, texture, level);
	CHECK_GL_ERROR();
}

void PI_API gl2_FrontFace (unsigned int mode) 
{
	glFrontFace(mode);
	CHECK_GL_ERROR();
}

void PI_API gl2_GenBuffers (unsigned int n, unsigned int* buffers) 
{
	glGenBuffers(n, buffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_GenerateMipmap (unsigned int target) 
{
	glGenerateMipmap(target);
	CHECK_GL_ERROR();
}

void PI_API gl2_GenFramebuffers (unsigned int n, unsigned int* framebuffers) 
{
	glGenFramebuffers(n, framebuffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_GenRenderbuffers (unsigned int n, unsigned int* renderbuffers) 
{
	glGenRenderbuffers(n, renderbuffers);
	CHECK_GL_ERROR();
}

void PI_API gl2_GenTextures (unsigned int n, unsigned int* textures) 
{
	glGenTextures(n, textures);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetActiveAttrib (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name) 
{
	glGetActiveAttrib(program, index, bufsize, length, size, type, name);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetActiveUniform (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name) 
{
	glGetActiveUniform(program, index, bufsize, length, size, type, name);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetAttachedShaders (unsigned int program, unsigned int maxcount, unsigned int* count, unsigned int* shaders) 
{
	glGetAttachedShaders(program, maxcount, count, shaders);
	CHECK_GL_ERROR();
}

int   PI_API gl2_GetAttribLocation (unsigned int program, const char* name) 
{
	int r = glGetAttribLocation(program, name);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl2_GetBooleanv (unsigned int pname, unsigned char* params) 
{
	glGetBooleanv(pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetBufferParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	glGetBufferParameteriv(target, pname, params);
	CHECK_GL_ERROR();
}

unsigned int  PI_API gl2_GetError (void) 
{
	return glGetError();
}

void PI_API gl2_GetFloatv (unsigned int pname, float* params) 
{
	glGetFloatv(pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetFramebufferAttachmentParameteriv (unsigned int target, unsigned int attachment, unsigned int pname, int* params) 
{
	glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetIntegerv (unsigned int pname, int* params) 
{
	glGetIntegerv(pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetProgramiv (unsigned int program, unsigned int pname, int* params) 
{
	glGetProgramiv(program, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetProgramInfoLog (unsigned int program, unsigned int bufsize, unsigned int* length, char* infolog) 
{
	glGetProgramInfoLog(program, bufsize, length, infolog);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetRenderbufferParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	glGetRenderbufferParameteriv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetShaderiv (unsigned int shader, unsigned int pname, int* params) 
{
	glGetShaderiv(shader, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetShaderInfoLog (unsigned int shader, unsigned int bufsize, unsigned int* length, char* infolog) 
{
	glGetShaderInfoLog(shader, bufsize, length, infolog);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetShaderPrecisionFormat (unsigned int shadertype, unsigned int precisiontype, int* range, int* precision) 
{
	glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetShaderSource (unsigned int shader, unsigned int bufsize, unsigned int* length, char* source) 
{
	glGetShaderSource(shader, bufsize, length, source);
	CHECK_GL_ERROR();
}

const unsigned char* PI_API gl2_GetString (unsigned int name) 
{
	return glGetString(name);
}

void PI_API gl2_GetTexParameterfv (unsigned int target, unsigned int pname, float* params) 
{
	glGetTexParameterfv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetTexParameteriv (unsigned int target, unsigned int pname, int* params) 
{
	glGetTexParameteriv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetUniformfv (unsigned int program, int location, float* params) 
{
	glGetUniformfv(program, location, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetUniformiv (unsigned int program, int location, int* params) 
{
	glGetUniformiv(program, location, params);
	CHECK_GL_ERROR();
}

int   PI_API gl2_GetUniformLocation (unsigned int program, const char* name) 
{
	int r = glGetUniformLocation(program, name);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl2_GetVertexAttribfv (unsigned int index, unsigned int pname, float* params) 
{
	glGetVertexAttribfv(index, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetVertexAttribiv (unsigned int index, unsigned int pname, int* params) 
{
	glGetVertexAttribiv(index, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_GetVertexAttribPointerv (unsigned int index, unsigned int pname, void** pointer) 
{
	glGetVertexAttribPointerv(index, pname, pointer);
	CHECK_GL_ERROR();
}

void PI_API gl2_Hint (unsigned int target, unsigned int mode) 
{
	glHint(target, mode);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl2_IsBuffer (unsigned int buffer) 
{
	unsigned char r = glIsBuffer(buffer);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsEnabled (unsigned int cap) 
{
	unsigned char r = glIsEnabled(cap);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsFramebuffer (unsigned int framebuffer) 
{
	unsigned char r = glIsFramebuffer(framebuffer);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsProgram (unsigned int program) 
{
	unsigned char r = glIsProgram(program);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsRenderbuffer (unsigned int renderbuffer) 
{
	unsigned char r = glIsRenderbuffer(renderbuffer);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsShader (unsigned int shader) 
{
	unsigned char r = glIsShader(shader);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl2_IsTexture (unsigned int texture) 
{
	unsigned char r = glIsTexture(texture);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl2_LineWidth (float width) 
{
	glLineWidth(width);
	CHECK_GL_ERROR();
}

void PI_API gl2_LinkProgram (unsigned int program) 
{
	glLinkProgram(program);
	CHECK_GL_ERROR();
}

void PI_API gl2_PixelStorei (unsigned int pname, int param) 
{
	glPixelStorei(pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl2_PolygonOffset (float factor, float units) 
{
	glPolygonOffset(factor, units);
	CHECK_GL_ERROR();
}

void PI_API gl2_ReadPixels (int x, int y, unsigned int width, unsigned int height, unsigned int format, unsigned int type, void* pixels) 
{
	glReadPixels(x, y, width, height, format, type, pixels);
	CHECK_GL_ERROR();
}

void PI_API gl2_ReleaseShaderCompiler (void) 
{
	glReleaseShaderCompiler();
	CHECK_GL_ERROR();
}

void PI_API gl2_RenderbufferStorage (unsigned int target, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	glRenderbufferStorage(target, internalformat, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl2_SampleCoverage (float value, unsigned char invert) 
{
	glSampleCoverage(value, invert);
	CHECK_GL_ERROR();
}

void PI_API gl2_Scissor (int x, int y, unsigned int width, unsigned int height) 
{
	glScissor(x, y, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl2_ShaderBinary (unsigned int n, const unsigned int* shaders, unsigned int binaryformat, const void* binary, unsigned int length) 
{
	glShaderBinary(n, shaders, binaryformat, binary, length);
	CHECK_GL_ERROR();
}

void PI_API gl2_ShaderSource (unsigned int shader, unsigned int count, const char* const* string, const int* length) 
{
	glShaderSource(shader, count, string, length);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilFunc (unsigned int func, int ref, unsigned int mask) 
{
	glStencilFunc(func, ref, mask);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilFuncSeparate (unsigned int face, unsigned int func, int ref, unsigned int mask) 
{
	glStencilFuncSeparate(face, func, ref, mask);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilMask (unsigned int mask) 
{
	glStencilMask(mask);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilMaskSeparate (unsigned int face, unsigned int mask) 
{
	glStencilMaskSeparate(face, mask);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilOp (unsigned int fail, unsigned int zfail, unsigned int zpass) 
{
	glStencilOp(fail, zfail, zpass);
	CHECK_GL_ERROR();
}

void PI_API gl2_StencilOpSeparate (unsigned int face, unsigned int fail, unsigned int zfail, unsigned int zpass) 
{
	glStencilOpSeparate(face, fail, zfail, zpass);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexImage2D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels) 
{
	glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexParameterf (unsigned int target, unsigned int pname, float param) 
{
	glTexParameterf(target, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexParameterfv (unsigned int target, unsigned int pname, const float* params) 
{
	glTexParameterfv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexParameteri (unsigned int target, unsigned int pname, int param) 
{
	glTexParameteri(target, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexParameteriv (unsigned int target, unsigned int pname, const int* params) 
{
	glTexParameteriv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl2_TexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels) 
{
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform1f (int location, float x) 
{
	glUniform1f(location, x);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform1fv (int location, unsigned int count, const float* v) 
{
	glUniform1fv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform1i (int location, int x) 
{
	glUniform1i(location, x);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform1iv (int location, unsigned int count, const int* v) 
{
	glUniform1iv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform2f (int location, float x, float y) 
{
	glUniform2f(location, x, y);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform2fv (int location, unsigned int count, const float* v) 
{
	glUniform2fv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform2i (int location, int x, int y) 
{
	glUniform2i(location, x, y);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform2iv (int location, unsigned int count, const int* v) 
{
	glUniform2iv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform3f (int location, float x, float y, float z) 
{
	glUniform3f(location, x, y, z);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform3fv (int location, unsigned int count, const float* v) 
{
	glUniform3fv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform3i (int location, int x, int y, int z) 
{
	glUniform3i(location, x, y, z);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform3iv (int location, unsigned int count, const int* v) 
{
	glUniform3iv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform4f (int location, float x, float y, float z, float w) 
{
	glUniform4f(location, x, y, z, w);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform4fv (int location, unsigned int count, const float* v) 
{
	glUniform4fv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform4i (int location, int x, int y, int z, int w) 
{
	glUniform4i(location, x, y, z, w);
	CHECK_GL_ERROR();
}

void PI_API gl2_Uniform4iv (int location, unsigned int count, const int* v) 
{
	glUniform4iv(location, count, v);
	CHECK_GL_ERROR();
}

void PI_API gl2_UniformMatrix2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix2fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl2_UniformMatrix3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix3fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl2_UniformMatrix4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix4fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl2_UseProgram (unsigned int program) 
{
	glUseProgram(program);
	CHECK_GL_ERROR();
}

void PI_API gl2_ValidateProgram (unsigned int program) 
{
	glValidateProgram(program);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib1f (unsigned int indx, float x) 
{
	glVertexAttrib1f(indx, x);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib1fv (unsigned int indx, const float* values) 
{
	glVertexAttrib1fv(indx, values);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib2f (unsigned int indx, float x, float y) 
{
	glVertexAttrib2f(indx, x, y);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib2fv (unsigned int indx, const float* values) 
{
	glVertexAttrib2fv(indx, values);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib3f (unsigned int indx, float x, float y, float z) 
{
	glVertexAttrib3f(indx, x, y, z);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib3fv (unsigned int indx, const float* values) 
{
	glVertexAttrib3fv(indx, values);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib4f (unsigned int indx, float x, float y, float z, float w) 
{
	glVertexAttrib4f(indx, x, y, z, w);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttrib4fv (unsigned int indx, const float* values) 
{
	glVertexAttrib4fv(indx, values);
	CHECK_GL_ERROR();
}

void PI_API gl2_VertexAttribPointer (unsigned int indx, int size, unsigned int type, unsigned char normalized, unsigned int stride, const void* ptr) 
{
	glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
	CHECK_GL_ERROR();
}

void PI_API gl2_Viewport (int x, int y, unsigned int width, unsigned int height) 
{
	glViewport(x, y, width, height);
	CHECK_GL_ERROR();
}


/* OpenGL ES 3.0 */

void PI_API gl3_ReadBuffer (unsigned int mode) 
{
	glReadBuffer(mode);
	CHECK_GL_ERROR();
}

void PI_API gl3_DrawRangeElements (unsigned int mode, unsigned int start, unsigned int end, unsigned int count, unsigned int type, const void* indices) 
{
	glDrawRangeElements(mode, start, end, count, type, indices);
	CHECK_GL_ERROR();
}

void PI_API gl3_TexImage3D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int format, unsigned int type, const void* pixels) 
{
	glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
	CHECK_GL_ERROR();
}

void PI_API gl3_TexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int type, const void* pixels) 
{
	glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
	CHECK_GL_ERROR();
}

void PI_API gl3_CopyTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, int x, int y, unsigned int width, unsigned int height) 
{
	glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl3_CompressedTexImage3D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int imageSize, const void* data) 
{
	glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
	CHECK_GL_ERROR();
}

void PI_API gl3_CompressedTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int imageSize, const void* data) 
{
	glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	CHECK_GL_ERROR();
}

void PI_API gl3_GenQueries (unsigned int n, unsigned int* ids) 
{
	glGenQueries(n, ids);
	CHECK_GL_ERROR();
}

void PI_API gl3_DeleteQueries (unsigned int n, const unsigned int* ids) 
{
	glDeleteQueries(n, ids);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl3_IsQuery (unsigned int id) 
{
	unsigned char r = glIsQuery(id);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_BeginQuery (unsigned int target, unsigned int id) 
{
	glBeginQuery(target, id);
	CHECK_GL_ERROR();
}

void PI_API gl3_EndQuery (unsigned int target) 
{
	glEndQuery(target);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetQueryiv (unsigned int target, unsigned int pname, int* params) 
{
	glGetQueryiv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetQueryObjectuiv (unsigned int id, unsigned int pname, unsigned int* params) 
{
	glGetQueryObjectuiv(id, pname, params);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl3_UnmapBuffer (unsigned int target) 
{
	unsigned char r = glUnmapBuffer(target);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_GetBufferPointerv (unsigned int target, unsigned int pname, void** params) 
{
	glGetBufferPointerv(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_DrawBuffers (unsigned int n, const unsigned int* bufs) 
{
	glDrawBuffers(n, bufs);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix2x3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix2x3fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix3x2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix3x2fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix2x4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix2x4fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix4x2fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix4x2fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix3x4fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix3x4fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformMatrix4x3fv (int location, unsigned int count, unsigned char transpose, const float* value) 
{
	glUniformMatrix4x3fv(location, count, transpose, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_BlitFramebuffer (int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter) 
{
	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
	CHECK_GL_ERROR();
}

void PI_API gl3_RenderbufferStorageMultisample (unsigned int target, unsigned int samples, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl3_FramebufferTextureLayer (unsigned int target, unsigned int attachment, unsigned int texture, int level, int layer) 
{
	glFramebufferTextureLayer(target, attachment, texture, level, layer);
	CHECK_GL_ERROR();
}

void* PI_API gl3_MapBufferRange (unsigned int target, int offset, unsigned int length, unsigned int access) 
{
	void* r = glMapBufferRange(target, offset, length, access);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_FlushMappedBufferRange (unsigned int target, int offset, unsigned int length) 
{
	glFlushMappedBufferRange(target, offset, length);
	CHECK_GL_ERROR();
}

void PI_API gl3_BindVertexArray (unsigned int array) 
{
	glBindVertexArray(array);
	CHECK_GL_ERROR();
}

void PI_API gl3_DeleteVertexArrays (unsigned int n, const unsigned int* arrays) 
{
	glDeleteVertexArrays(n, arrays);
	CHECK_GL_ERROR();
}

void PI_API gl3_GenVertexArrays (unsigned int n, unsigned int* arrays) 
{
	glGenVertexArrays(n, arrays);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl3_IsVertexArray (unsigned int array) 
{
	unsigned char r = glIsVertexArray(array);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_GetIntegeri_v (unsigned int target, unsigned int index, int* data) 
{
	glGetIntegeri_v(target, index, data);
	CHECK_GL_ERROR();
}

void PI_API gl3_BeginTransformFeedback (unsigned int primitiveMode) 
{
	glBeginTransformFeedback(primitiveMode);
	CHECK_GL_ERROR();
}

void PI_API gl3_EndTransformFeedback (void) 
{
	glEndTransformFeedback();
	CHECK_GL_ERROR();
}

void PI_API gl3_BindBufferRange (unsigned int target, unsigned int index, unsigned int buffer, int offset, unsigned int size) 
{
	glBindBufferRange(target, index, buffer, offset, size);
	CHECK_GL_ERROR();
}

void PI_API gl3_BindBufferBase (unsigned int target, unsigned int index, unsigned int buffer) 
{
	glBindBufferBase(target, index, buffer);
	CHECK_GL_ERROR();
}

void PI_API gl3_TransformFeedbackVaryings (unsigned int program, unsigned int count, const char* const* varyings, unsigned int bufferMode) 
{
	glTransformFeedbackVaryings(program, count, varyings, bufferMode);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetTransformFeedbackVarying (unsigned int program, unsigned int index, unsigned int bufSize, unsigned int* length, unsigned int* size, unsigned int* type, char* name) 
{
	glGetTransformFeedbackVarying(program, index, bufSize, length, size,type, name);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribIPointer (unsigned int index, int size, unsigned int type, unsigned int stride, const void* pointer) 
{
	glVertexAttribIPointer(index, size, type, stride, pointer);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetVertexAttribIiv (unsigned int index, unsigned int pname, int* params) 
{
	glGetVertexAttribIiv(index, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetVertexAttribIuiv (unsigned int index, unsigned int pname, unsigned int* params) 
{
	glGetVertexAttribIuiv(index, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribI4i (unsigned int index, int x, int y, int z, int w) 
{
	glVertexAttribI4i(index, x, y, z, w);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribI4ui (unsigned int index, unsigned int x, unsigned int y, unsigned int z, unsigned int w) 
{
	glVertexAttribI4ui(index, x, y, z, w);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribI4iv (unsigned int index, const int* v) 
{
	glVertexAttribI4iv(index, v);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribI4uiv (unsigned int index, const unsigned int* v) 
{
	glVertexAttribI4uiv(index, v);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetUniformuiv (unsigned int program, int location, unsigned int* params) 
{
	glGetUniformuiv(program, location, params);
	CHECK_GL_ERROR();
}

int   PI_API gl3_GetFragDataLocation (unsigned int program, const char *name) 
{
	unsigned char r = glGetFragDataLocation(program, name);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_Uniform1ui (int location, unsigned int v0) 
{
	glUniform1ui(location, v0);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform2ui (int location, unsigned int v0, unsigned int v1) 
{
	glUniform2ui(location, v0, v1);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform3ui (int location, unsigned int v0, unsigned int v1, unsigned int v2) 
{
	glUniform3ui(location, v0, v1, v2);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform4ui (int location, unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3) 
{
	glUniform4ui(location, v0, v1, v2, v3);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform1uiv (int location, unsigned int count, const unsigned int* value) 
{
	glUniform1uiv(location, count, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform2uiv (int location, unsigned int count, const unsigned int* value) 
{
	glUniform2uiv(location, count, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform3uiv (int location, unsigned int count, const unsigned int* value) 
{
	glUniform3uiv(location, count, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_Uniform4uiv (int location, unsigned int count, const unsigned int* value) 
{
	glUniform4uiv(location, count, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_ClearBufferiv (unsigned int buffer, int drawbuffer, const int* value) 
{
	glClearBufferiv(buffer, drawbuffer, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_ClearBufferuiv (unsigned int buffer, int drawbuffer, const unsigned int* value) 
{
	glClearBufferuiv(buffer, drawbuffer, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_ClearBufferfv (unsigned int buffer, int drawbuffer, const float* value) 
{
	glClearBufferfv(buffer, drawbuffer, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_ClearBufferfi (unsigned int buffer, int drawbuffer, float depth, int stencil) 
{
	glClearBufferfi(buffer, drawbuffer, depth, stencil);
	CHECK_GL_ERROR();
}

const unsigned char* PI_API gl3_GetStringi (unsigned int name, unsigned int index) 
{
	const unsigned char *r = glGetStringi(name, index);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_CopyBufferSubData (unsigned int readTarget, unsigned int writeTarget, int readOffset, int writeOffset, unsigned int size) 
{
	glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetUniformIndices (unsigned int program, unsigned int uniformCount, const char* const* uniformNames, unsigned int* uniformIndices) 
{
	glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetActiveUniformsiv (unsigned int program, unsigned int uniformCount, const unsigned int* uniformIndices, unsigned int pname, int* params) 
{
	glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
	CHECK_GL_ERROR();
}

unsigned int  PI_API gl3_GetUniformBlockIndex (unsigned int program, const char* uniformBlockName) 
{
	unsigned int r = glGetUniformBlockIndex(program, uniformBlockName);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_GetActiveUniformBlockiv (unsigned int program, unsigned int uniformBlockIndex, unsigned int pname, int* params) 
{
	glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetActiveUniformBlockName (unsigned int program, unsigned int uniformBlockIndex, unsigned int bufSize, unsigned int* length, char* uniformBlockName) 
{
	glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
	CHECK_GL_ERROR();
}

void PI_API gl3_UniformBlockBinding (unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding) 
{
	glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
	CHECK_GL_ERROR();
}

void PI_API gl3_DrawArraysInstanced (unsigned int mode, int first, unsigned int count, unsigned int instanceCount) 
{
	glDrawArraysInstanced(mode, first, count, instanceCount);
	CHECK_GL_ERROR();
}

void PI_API gl3_DrawElementsInstanced (unsigned int mode, unsigned int count, unsigned int type, const void* indices, unsigned int instanceCount) 
{
	glDrawElementsInstanced(mode, count, type, indices, instanceCount);
	CHECK_GL_ERROR();
}

void*  PI_API gl3_FenceSync (unsigned int condition, unsigned int flags) 
{
	void* r = glFenceSync(condition, flags);
	CHECK_GL_ERROR();
	return r;
}

unsigned char  PI_API gl3_IsSync (void* sync) 
{
	unsigned char r = glIsSync(sync);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_DeleteSync (void* sync) 
{
	glDeleteSync(sync);
	CHECK_GL_ERROR();
}

unsigned int  PI_API gl3_ClientWaitSync (void* sync, unsigned int flags, unsigned long long timeout) 
{
	unsigned int r = glClientWaitSync(sync, flags, timeout);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_WaitSync (void* sync, unsigned int flags, unsigned long long timeout) 
{
	glWaitSync(sync, flags, timeout);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetInteger64v (unsigned int pname, long long* params) 
{
	glGetInteger64v(pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetSynciv (void* sync, unsigned int pname, unsigned int bufSize, unsigned int* length, int* values) 
{
	glGetSynciv(sync, pname, bufSize, length, values);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetInteger64i_v (unsigned int target, unsigned int index, long long* data) 
{
	glGetInteger64i_v(target, index, data);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetBufferParameteri64v (unsigned int target, unsigned int pname, long long* params) 
{
	glGetBufferParameteri64v(target, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GenSamplers (unsigned int count, unsigned int* samplers) 
{
	glGenSamplers(count, samplers);
	CHECK_GL_ERROR();
}

void PI_API gl3_DeleteSamplers (unsigned int count, const unsigned int* samplers) 
{
	glDeleteSamplers(count, samplers);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl3_IsSampler (unsigned int sampler) 
{
	unsigned char r = glIsSampler(sampler);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_BindSampler (unsigned int unit, unsigned int sampler) 
{
	glBindSampler(unit, sampler);
	CHECK_GL_ERROR();
}

void PI_API gl3_SamplerParameteri (unsigned int sampler, unsigned int pname, int param) 
{
	glSamplerParameteri(sampler, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl3_SamplerParameteriv (unsigned int sampler, unsigned int pname, const int* param) 
{
	glSamplerParameteriv(sampler, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl3_SamplerParameterf (unsigned int sampler, unsigned int pname, float param) 
{
	glSamplerParameterf(sampler, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl3_SamplerParameterfv (unsigned int sampler, unsigned int pname, const float* param) 
{
	glSamplerParameterfv(sampler, pname, param);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetSamplerParameteriv (unsigned int sampler, unsigned int pname, int* params) 
{
	glGetSamplerParameteriv(sampler, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetSamplerParameterfv (unsigned int sampler, unsigned int pname, float* params) 
{
	glGetSamplerParameterfv(sampler, pname, params);
	CHECK_GL_ERROR();
}

void PI_API gl3_VertexAttribDivisor (unsigned int index, unsigned int divisor) 
{
	glVertexAttribDivisor(index, divisor);
	CHECK_GL_ERROR();
}

void PI_API gl3_BindTransformFeedback (unsigned int target, unsigned int id) 
{
	glBindTransformFeedback(target, id);
	CHECK_GL_ERROR();
}

void PI_API gl3_DeleteTransformFeedbacks (unsigned int n, const unsigned int* ids) 
{
	glDeleteTransformFeedbacks(n, ids);
	CHECK_GL_ERROR();
}

void PI_API gl3_GenTransformFeedbacks (unsigned int n, unsigned int* ids) 
{
	glGenTransformFeedbacks(n, ids);
	CHECK_GL_ERROR();
}

unsigned char  PI_API gl3_IsTransformFeedback (unsigned int id) 
{
	unsigned char r = glIsTransformFeedback(id);
	CHECK_GL_ERROR();
	return r;
}

void PI_API gl3_PauseTransformFeedback (void) 
{
	glPauseTransformFeedback();
	CHECK_GL_ERROR();
}

void PI_API gl3_ResumeTransformFeedback (void) 
{
	glResumeTransformFeedback();
	CHECK_GL_ERROR();
}

void PI_API gl3_GetProgramBinary (unsigned int program, unsigned int bufSize, unsigned int* length, unsigned int* binaryFormat, void* binary) 
{
	glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
	CHECK_GL_ERROR();
}

void PI_API gl3_ProgramBinary (unsigned int program, unsigned int binaryFormat, const void* binary, unsigned int length) 
{
	glProgramBinary(program, binaryFormat, binary, length);
	CHECK_GL_ERROR();
}

void PI_API gl3_ProgramParameteri (unsigned int program, unsigned int pname, int value) 
{
	glProgramParameteri(program, pname, value);
	CHECK_GL_ERROR();
}

void PI_API gl3_InvalidateFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments) 
{
	glInvalidateFramebuffer(target, numAttachments, attachments);
	CHECK_GL_ERROR();
}

void PI_API gl3_InvalidateSubFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments, int x, int y, unsigned int width, unsigned int height) 
{
	glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl3_TexStorage2D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height) 
{
	glTexStorage2D(target, levels, internalformat, width, height);
	CHECK_GL_ERROR();
}

void PI_API gl3_TexStorage3D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth) 
{
	glTexStorage3D(target, levels, internalformat, width, height, depth);
	CHECK_GL_ERROR();
}

void PI_API gl3_GetInternalformativ (unsigned int target, unsigned int internalformat, unsigned int pname, unsigned int bufSize, int* params) 
{
	glGetInternalformativ(target, internalformat, pname, bufSize, params);
	CHECK_GL_ERROR();
}

PiBool PI_API gl_Self_IsGLES(void)
{
	return TRUE;
}

PiBool PI_API gl_Self_IsTexture3D(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureLOD(void)
{
	return FALSE;
}

void PI_API gl_Self_SwapIntervalEXT(PiBool is_enable)
{
	PI_ASSERT(FALSE, "this operation can't support");
}

PiBool PI_API gl_Self_IsVersion3(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsVersion4(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsGpuShader4(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsCopyImage(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsFramebufferSRGB(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsHalfFloatPixel(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsPackedFloat(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsPackedDepthStencil(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsFramebufferBlit(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureArray(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureRG(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureFloat(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureInteger(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureSRGB(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureSnorm(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureCompressionLatc(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureCompressionS3tc(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsTextureCompressionRgtc(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureCompressionBptc(void)
{
	return FALSE;
}

PiBool PI_API gl_Self_IsTextureFilterAnisotropic(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsVertexType_2_10_10_10_rev(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsVertexArrayObject(void)
{
    return FALSE;
}

PiBool PI_API gl_Self_IsDepthClamp(void)
{
	return FALSE;
}

void PI_API gl_PolygonMode(unsigned int face, unsigned int mode)
{
	PI_ASSERT(FALSE, "this operation can't support");
}

void PI_API gl_GetTexImage(unsigned int target, int level, unsigned int format, unsigned int type, void* pixels)
{
	PI_ASSERT(FALSE, "this operation can't support");
}

void PI_API gl_GetCompressedTexImage(unsigned int target, int level, void* img)
{
	PI_ASSERT(FALSE, "this operation can't support");
}

void PI_API gl_CopyImageSubData(unsigned int srcName, unsigned int srcTarget, int srcLevel, int srcX, int srcY, int srcZ, unsigned int dstName, unsigned int dstTarget, int dstLevel, int dstX, int dstY, int dstZ, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth)
{
	PI_ASSERT(FALSE, "this operation can't support");
}

void PI_API gl_FramebufferTexture3D(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, sint level, sint zoffset)
{
	PI_ASSERT(FALSE, "this operation can't support");
}