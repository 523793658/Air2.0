#ifndef INCLUDE_GL_INTERFACE_H
#define INCLUDE_GL_INTERFACE_H

#include <pi_lib.h>

#include <rendersystem.h>

typedef enum
{
	RIT_NULL,	/* 空设备 */
	RIT_OPENGL,	/* OpenGL */
	RIT_GLES,	/* gles */
} RenderInterfaceType;

typedef PiBool (PI_API *GL_Self_IsLostContext)(void);

typedef void (PI_API *GL_PolygonMode)(unsigned int face, unsigned int mode);
typedef void (PI_API *GL_GetTexImage)(unsigned int target, int level, unsigned int format, unsigned int type, void* pixels);
typedef void (PI_API *GL_GetCompressedTexImage)(unsigned int target, int level, void* img);
typedef void (PI_API *GL_CopyImageSubData)(unsigned int srcName, unsigned int srcTarget, int srcLevel, int srcX, int srcY, int srcZ, unsigned int dstName, unsigned int dstTarget, int dstLevel, int dstX, int dstY, int dstZ, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth);
typedef void (PI_API *GL_FramebufferTexture3D)(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, sint level, sint zoffset);

typedef void (PI_API *GL2_ActiveTexture)(unsigned int texture);
typedef void (PI_API *GL2_AttachShader)(unsigned int program, unsigned int shader);
typedef void (PI_API *GL2_BindAttribLocation)(unsigned int program, unsigned int index, const char* name);
typedef void (PI_API *GL2_BindBuffer)(unsigned int target, unsigned int buffer);
typedef void (PI_API *GL2_BindFramebuffer)(unsigned int target, unsigned int framebuffer);
typedef void (PI_API *GL2_BindRenderbuffer)(unsigned int target, unsigned int renderbuffer);
typedef void (PI_API *GL2_BindTexture)(unsigned int target, unsigned int texture);
typedef void (PI_API *GL2_BlendColor)(float red, float green, float blue, float alpha);
typedef void (PI_API *GL2_BlendEquation)(unsigned int mode);
typedef void (PI_API *GL2_BlendEquationSeparate)(unsigned int modeRGB, unsigned int modeAlpha);
typedef void (PI_API *GL2_BlendFunc)(unsigned int sfactor, unsigned int dfactor);
typedef void (PI_API *GL2_BlendFuncSeparate)(unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha);
typedef void (PI_API *GL2_BufferData)(unsigned int target, unsigned int size, const void* data, unsigned int usage);
typedef void (PI_API *GL2_BufferSubData)(unsigned int target, int offset, unsigned int size, const void* data);
typedef unsigned int  (PI_API *GL2_CheckFramebufferStatus)(unsigned int target);
typedef void (PI_API *GL2_Clear)(unsigned int mask);
typedef void (PI_API *GL2_ClearColor)(float red, float green, float blue, float alpha);
typedef void (PI_API *GL2_ClearDepthf)(float depth);
typedef void (PI_API *GL2_ClearStencil)(int s);
typedef void (PI_API *GL2_ColorMask)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (PI_API *GL2_CompileShader)(unsigned int shader);
typedef void (PI_API *GL2_CompressedTexImage2D)(unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, int border, unsigned int imageSize, const void* data);
typedef void (PI_API *GL2_CompressedTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int imageSize, const void* data);
typedef void (PI_API *GL2_CopyTexImage2D)(unsigned int target, int level, unsigned int internalformat, int x, int y, unsigned int width, unsigned int height, int border);
typedef void (PI_API *GL2_CopyTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int x, int y, unsigned int width, unsigned int height);
typedef unsigned int  (PI_API *GL2_CreateProgram)(void);
typedef unsigned int  (PI_API *GL2_CreateShader)(unsigned int type);
typedef void (PI_API *GL2_CullFace)(unsigned int mode);
typedef void (PI_API *GL2_DeleteBuffers)(unsigned int n, const unsigned int* buffers);
typedef void (PI_API *GL2_DeleteFramebuffers)(unsigned int n, const unsigned int* framebuffers);
typedef void (PI_API *GL2_DeleteProgram)(unsigned int program);
typedef void (PI_API *GL2_DeleteRenderbuffers)(unsigned int n, const unsigned int* renderbuffers);
typedef void (PI_API *GL2_DeleteShader)(unsigned int shader);
typedef void (PI_API *GL2_DeleteTextures)(unsigned int n, const unsigned int* textures);
typedef void (PI_API *GL2_DepthFunc)(unsigned int func);
typedef void (PI_API *GL2_DepthMask)(unsigned char flag);
typedef void (PI_API *GL2_DepthRangef)(float n, float f);
typedef void (PI_API *GL2_DetachShader)(unsigned int program, unsigned int shader);
typedef void (PI_API *GL2_Disable)(unsigned int cap);
typedef void (PI_API *GL2_DisableVertexAttribArray)(unsigned int index);
typedef void (PI_API *GL2_DrawArrays)(unsigned int mode, int first, unsigned int count);
typedef void (PI_API *GL2_DrawElements)(unsigned int mode, unsigned int count, unsigned int type, const void* indices);
typedef void (PI_API *GL2_Enable)(unsigned int cap);
typedef void (PI_API *GL2_EnableVertexAttribArray)(unsigned int index);
typedef void (PI_API *GL2_Finish)(void);
typedef void (PI_API *GL2_Flush)(void);
typedef void (PI_API *GL2_FramebufferRenderbuffer)(unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer);
typedef void (PI_API *GL2_FramebufferTexture2D)(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
typedef void (PI_API *GL2_FrontFace)(unsigned int mode);
typedef void (PI_API *GL2_GenBuffers)(unsigned int n, unsigned int* buffers);
typedef void (PI_API *GL2_GenerateMipmap)(unsigned int target);
typedef void (PI_API *GL2_GenFramebuffers)(unsigned int n, unsigned int* framebuffers);
typedef void (PI_API *GL2_GenRenderbuffers)(unsigned int n, unsigned int* renderbuffers);
typedef void (PI_API *GL2_GenTextures)(unsigned int n, unsigned int* textures);
typedef void (PI_API *GL2_GetActiveAttrib)(unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name);
typedef void (PI_API *GL2_GetActiveUniform)(unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name);
typedef void (PI_API *GL2_GetAttachedShaders)(unsigned int program, unsigned int maxcount, unsigned int* count, unsigned int* shaders);
typedef int   (PI_API *GL2_GetAttribLocation)(unsigned int program, const char* name);
typedef void (PI_API *GL2_GetBooleanv)(unsigned int pname, unsigned char* params);
typedef void (PI_API *GL2_GetBufferParameteriv)(unsigned int target, unsigned int pname, int* params);
typedef unsigned int  (PI_API *GL2_GetError)(void);
typedef void (PI_API *GL2_GetFloatv)(unsigned int pname, float* params);
typedef void (PI_API *GL2_GetFramebufferAttachmentParameteriv)(unsigned int target, unsigned int attachment, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetIntegerv)(unsigned int pname, int* params);
typedef void (PI_API *GL2_GetProgramiv)(unsigned int program, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetProgramInfoLog)(unsigned int program, unsigned int bufsize, unsigned int* length, char* infolog);
typedef void (PI_API *GL2_GetRenderbufferParameteriv)(unsigned int target, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetShaderiv)(unsigned int shader, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetShaderInfoLog)(unsigned int shader, unsigned int bufsize, unsigned int* length, char* infolog);
typedef void (PI_API *GL2_GetShaderPrecisionFormat)(unsigned int shadertype, unsigned int precisiontype, int* range, int* precision);
typedef void (PI_API *GL2_GetShaderSource)(unsigned int shader, unsigned int bufsize, unsigned int* length, char* source);
typedef const unsigned char* (PI_API *GL2_GetString)(unsigned int name);
typedef void (PI_API *GL2_GetTexParameterfv)(unsigned int target, unsigned int pname, float* params);
typedef void (PI_API *GL2_GetTexParameteriv)(unsigned int target, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetUniformfv)(unsigned int program, int location, float* params);
typedef void (PI_API *GL2_GetUniformiv)(unsigned int program, int location, int* params);
typedef int   (PI_API *GL2_GetUniformLocation)(unsigned int program, const char* name);
typedef void (PI_API *GL2_GetVertexAttribfv)(unsigned int index, unsigned int pname, float* params);
typedef void (PI_API *GL2_GetVertexAttribiv)(unsigned int index, unsigned int pname, int* params);
typedef void (PI_API *GL2_GetVertexAttribPointerv)(unsigned int index, unsigned int pname, void** pointer);
typedef void (PI_API *GL2_Hint)(unsigned int target, unsigned int mode);
typedef unsigned char  (PI_API *GL2_IsBuffer)(unsigned int buffer);
typedef unsigned char  (PI_API *GL2_IsEnabled)(unsigned int cap);
typedef unsigned char  (PI_API *GL2_IsFramebuffer)(unsigned int framebuffer);
typedef unsigned char  (PI_API *GL2_IsProgram)(unsigned int program);
typedef unsigned char  (PI_API *GL2_IsRenderbuffer)(unsigned int renderbuffer);
typedef unsigned char  (PI_API *GL2_IsShader)(unsigned int shader);
typedef unsigned char  (PI_API *GL2_IsTexture)(unsigned int texture);
typedef void (PI_API *GL2_LineWidth)(float width);
typedef void (PI_API *GL2_LinkProgram)(unsigned int program);
typedef void (PI_API *GL2_PixelStorei)(unsigned int pname, int param);
typedef void (PI_API *GL2_PolygonOffset)(float factor, float units);
typedef void (PI_API *GL2_ReadPixels)(int x, int y, unsigned int width, unsigned int height, unsigned int format, unsigned int type, void* pixels);
typedef void (PI_API *GL2_ReleaseShaderCompiler)(void);
typedef void (PI_API *GL2_RenderbufferStorage)(unsigned int target, unsigned int internalformat, unsigned int width, unsigned int height);
typedef void (PI_API *GL2_SampleCoverage)(float value, unsigned char invert);
typedef void (PI_API *GL2_Scissor)(int x, int y, unsigned int width, unsigned int height);
typedef void (PI_API *GL2_ShaderBinary)(unsigned int n, const unsigned int* shaders, unsigned int binaryformat, const void* binary, unsigned int length);
typedef void (PI_API *GL2_ShaderSource)(unsigned int shader, unsigned int count, const char* const* string, const int* length);
typedef void (PI_API *GL2_StencilFunc)(unsigned int func, int ref, unsigned int mask);
typedef void (PI_API *GL2_StencilFuncSeparate)(unsigned int face, unsigned int func, int ref, unsigned int mask);
typedef void (PI_API *GL2_StencilMask)(unsigned int mask);
typedef void (PI_API *GL2_StencilMaskSeparate)(unsigned int face, unsigned int mask);
typedef void (PI_API *GL2_StencilOp)(unsigned int fail, unsigned int zfail, unsigned int zpass);
typedef void (PI_API *GL2_StencilOpSeparate)(unsigned int face, unsigned int fail, unsigned int zfail, unsigned int zpass);
typedef void (PI_API *GL2_TexImage2D)(unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (PI_API *GL2_TexParameterf)(unsigned int target, unsigned int pname, float param);
typedef void (PI_API *GL2_TexParameterfv)(unsigned int target, unsigned int pname, const float* params);
typedef void (PI_API *GL2_TexParameteri)(unsigned int target, unsigned int pname, int param);
typedef void (PI_API *GL2_TexParameteriv)(unsigned int target, unsigned int pname, const int* params);
typedef void (PI_API *GL2_TexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels);
typedef void (PI_API *GL2_Uniform1f)(int location, float x);
typedef void (PI_API *GL2_Uniform1fv)(int location, unsigned int count, const float* v);
typedef void (PI_API *GL2_Uniform1i)(int location, int x);
typedef void (PI_API *GL2_Uniform1iv)(int location, unsigned int count, const int* v);
typedef void (PI_API *GL2_Uniform2f)(int location, float x, float y);
typedef void (PI_API *GL2_Uniform2fv)(int location, unsigned int count, const float* v);
typedef void (PI_API *GL2_Uniform2i)(int location, int x, int y);
typedef void (PI_API *GL2_Uniform2iv)(int location, unsigned int count, const int* v);
typedef void (PI_API *GL2_Uniform3f)(int location, float x, float y, float z);
typedef void (PI_API *GL2_Uniform3fv)(int location, unsigned int count, const float* v);
typedef void (PI_API *GL2_Uniform3i)(int location, int x, int y, int z);
typedef void (PI_API *GL2_Uniform3iv)(int location, unsigned int count, const int* v);
typedef void (PI_API *GL2_Uniform4f)(int location, float x, float y, float z, float w);
typedef void (PI_API *GL2_Uniform4fv)(int location, unsigned int count, const float* v);
typedef void (PI_API *GL2_Uniform4i)(int location, int x, int y, int z, int w);
typedef void (PI_API *GL2_Uniform4iv)(int location, unsigned int count, const int* v);
typedef void (PI_API *GL2_UniformMatrix2fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL2_UniformMatrix3fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL2_UniformMatrix4fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL2_UseProgram)(unsigned int program);
typedef void (PI_API *GL2_ValidateProgram)(unsigned int program);
typedef void (PI_API *GL2_VertexAttrib1f)(unsigned int indx, float x);
typedef void (PI_API *GL2_VertexAttrib1fv)(unsigned int indx, const float* values);
typedef void (PI_API *GL2_VertexAttrib2f)(unsigned int indx, float x, float y);
typedef void (PI_API *GL2_VertexAttrib2fv)(unsigned int indx, const float* values);
typedef void (PI_API *GL2_VertexAttrib3f)(unsigned int indx, float x, float y, float z);
typedef void (PI_API *GL2_VertexAttrib3fv)(unsigned int indx, const float* values);
typedef void (PI_API *GL2_VertexAttrib4f)(unsigned int indx, float x, float y, float z, float w);
typedef void (PI_API *GL2_VertexAttrib4fv)(unsigned int indx, const float* values);
typedef void (PI_API *GL2_VertexAttribPointer)(unsigned int indx, int size, unsigned int type, unsigned char normalized, unsigned int stride, const void* ptr);
typedef void (PI_API *GL2_Viewport)(int x, int y, unsigned int width, unsigned int height);

/* OpenGL ES 3.0 */

typedef void (PI_API *GL3_ReadBuffer)(unsigned int mode);
typedef void (PI_API *GL3_DrawRangeElements)(unsigned int mode, unsigned int start, unsigned int end, unsigned int count, unsigned int type, const void* indices);
typedef void (PI_API *GL3_TexImage3D)(unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (PI_API *GL3_TexSubImage3D)(unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int type, const void* pixels);
typedef void (PI_API *GL3_CopyTexSubImage3D)(unsigned int target, int level, int xoffset, int yoffset, int zoffset, int x, int y, unsigned int width, unsigned int height);
typedef void (PI_API *GL3_CompressedTexImage3D)(unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int imageSize, const void* data);
typedef void (PI_API *GL3_CompressedTexSubImage3D)(unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int imageSize, const void* data);
typedef void (PI_API *GL3_GenQueries)(unsigned int n, unsigned int* ids);
typedef void (PI_API *GL3_DeleteQueries)(unsigned int n, const unsigned int* ids);
typedef unsigned char  (PI_API *GL3_IsQuery)(unsigned int id);
typedef void (PI_API *GL3_BeginQuery)(unsigned int target, unsigned int id);
typedef void (PI_API *GL3_EndQuery)(unsigned int target);
typedef void (PI_API *GL3_GetQueryiv)(unsigned int target, unsigned int pname, int* params);
typedef void (PI_API *GL3_GetQueryObjectuiv)(unsigned int id, unsigned int pname, unsigned int* params);
typedef unsigned char  (PI_API *GL3_UnmapBuffer)(unsigned int target);
typedef void (PI_API *GL3_GetBufferPointerv)(unsigned int target, unsigned int pname, void** params);
typedef void (PI_API *GL3_DrawBuffers)(unsigned int n, const unsigned int* bufs);
typedef void (PI_API *GL3_UniformMatrix2x3fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_UniformMatrix3x2fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_UniformMatrix2x4fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_UniformMatrix4x2fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_UniformMatrix3x4fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_UniformMatrix4x3fv)(int location, unsigned int count, unsigned char transpose, const float* value);
typedef void (PI_API *GL3_BlitFramebuffer)(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);
typedef void (PI_API *GL3_RenderbufferStorageMultisample)(unsigned int target, unsigned int samples, unsigned int internalformat, unsigned int width, unsigned int height);
typedef void (PI_API *GL3_FramebufferTextureLayer)(unsigned int target, unsigned int attachment, unsigned int texture, int level, int layer);
typedef void* (PI_API *GL3_MapBufferRange)(unsigned int target, int offset, unsigned int length, unsigned int access);
typedef void (PI_API *GL3_FlushMappedBufferRange)(unsigned int target, int offset, unsigned int length);
typedef void (PI_API *GL3_BindVertexArray)(unsigned int array);
typedef void (PI_API *GL3_DeleteVertexArrays)(unsigned int n, const unsigned int* arrays);
typedef void (PI_API *GL3_GenVertexArrays)(unsigned int n, unsigned int* arrays);
typedef unsigned char  (PI_API *GL3_IsVertexArray)(unsigned int array);
typedef void (PI_API *GL3_GetIntegeri_v)(unsigned int target, unsigned int index, int* data);
typedef void (PI_API *GL3_BeginTransformFeedback)(unsigned int primitiveMode);
typedef void (PI_API *GL3_EndTransformFeedback)(void);
typedef void (PI_API *GL3_BindBufferRange)(unsigned int target, unsigned int index, unsigned int buffer, int offset, unsigned int size);
typedef void (PI_API *GL3_BindBufferBase)(unsigned int target, unsigned int index, unsigned int buffer);
typedef void (PI_API *GL3_TransformFeedbackVaryings)(unsigned int program, unsigned int count, const char* const* varyings, unsigned int bufferMode);
typedef void (PI_API *GL3_GetTransformFeedbackVarying)(unsigned int program, unsigned int index, unsigned int bufSize, unsigned int* length, unsigned int* size, unsigned int* type, char* name);
typedef void (PI_API *GL3_VertexAttribIPointer)(unsigned int index, int size, unsigned int type, unsigned int stride, const void* pointer);
typedef void (PI_API *GL3_GetVertexAttribIiv)(unsigned int index, unsigned int pname, int* params);
typedef void (PI_API *GL3_GetVertexAttribIuiv)(unsigned int index, unsigned int pname, unsigned int* params);
typedef void (PI_API *GL3_VertexAttribI4i)(unsigned int index, int x, int y, int z, int w);
typedef void (PI_API *GL3_VertexAttribI4ui)(unsigned int index, unsigned int x, unsigned int y, unsigned int z, unsigned int w);
typedef void (PI_API *GL3_VertexAttribI4iv)(unsigned int index, const int* v);
typedef void (PI_API *GL3_VertexAttribI4uiv)(unsigned int index, const unsigned int* v);
typedef void (PI_API *GL3_GetUniformuiv)(unsigned int program, int location, unsigned int* params);
typedef int   (PI_API *GL3_GetFragDataLocation)(unsigned int program, const char *name);
typedef void (PI_API *GL3_Uniform1ui)(int location, unsigned int v0);
typedef void (PI_API *GL3_Uniform2ui)(int location, unsigned int v0, unsigned int v1);
typedef void (PI_API *GL3_Uniform3ui)(int location, unsigned int v0, unsigned int v1, unsigned int v2);
typedef void (PI_API *GL3_Uniform4ui)(int location, unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3);
typedef void (PI_API *GL3_Uniform1uiv)(int location, unsigned int count, const unsigned int* value);
typedef void (PI_API *GL3_Uniform2uiv)(int location, unsigned int count, const unsigned int* value);
typedef void (PI_API *GL3_Uniform3uiv)(int location, unsigned int count, const unsigned int* value);
typedef void (PI_API *GL3_Uniform4uiv)(int location, unsigned int count, const unsigned int* value);
typedef void (PI_API *GL3_ClearBufferiv)(unsigned int buffer, int drawbuffer, const int* value);
typedef void (PI_API *GL3_ClearBufferuiv)(unsigned int buffer, int drawbuffer, const unsigned int* value);
typedef void (PI_API *GL3_ClearBufferfv)(unsigned int buffer, int drawbuffer, const float* value);
typedef void (PI_API *GL3_ClearBufferfi)(unsigned int buffer, int drawbuffer, float depth, int stencil);
typedef const unsigned char* (PI_API *GL3_GetStringi)(unsigned int name, unsigned int index);
typedef void (PI_API *GL3_CopyBufferSubData)(unsigned int readTarget, unsigned int writeTarget, int readOffset, int writeOffset, unsigned int size);
typedef void (PI_API *GL3_GetUniformIndices)(unsigned int program, unsigned int uniformCount, const char* const* uniformNames, unsigned int* uniformIndices);
typedef void (PI_API *GL3_GetActiveUniformsiv)(unsigned int program, unsigned int uniformCount, const unsigned int* uniformIndices, unsigned int pname, int* params);
typedef unsigned int  (PI_API *GL3_GetUniformBlockIndex)(unsigned int program, const char* uniformBlockName);
typedef void (PI_API *GL3_GetActiveUniformBlockiv)(unsigned int program, unsigned int uniformBlockIndex, unsigned int pname, int* params);
typedef void (PI_API *GL3_GetActiveUniformBlockName)(unsigned int program, unsigned int uniformBlockIndex, unsigned int bufSize, unsigned int* length, char* uniformBlockName);
typedef void (PI_API *GL3_UniformBlockBinding)(unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding);
typedef void (PI_API *GL3_DrawArraysInstanced)(unsigned int mode, int first, unsigned int count, unsigned int instanceCount);
typedef void (PI_API *GL3_DrawElementsInstanced)(unsigned int mode, unsigned int count, unsigned int type, const void* indices, unsigned int instanceCount);
typedef void*  (PI_API *GL3_FenceSync)(unsigned int condition, unsigned int flags);
typedef unsigned char  (PI_API *GL3_IsSync)(void* sync);
typedef void (PI_API *GL3_DeleteSync)(void* sync);
typedef unsigned int  (PI_API *GL3_ClientWaitSync)(void* sync, unsigned int flags, unsigned long long timeout);
typedef void (PI_API *GL3_WaitSync)(void* sync, unsigned int flags, unsigned long long timeout);
typedef void (PI_API *GL3_GetInteger64v)(unsigned int pname, long long* params);
typedef void (PI_API *GL3_GetSynciv)(void* sync, unsigned int pname, unsigned int bufSize, unsigned int* length, int* values);
typedef void (PI_API *GL3_GetInteger64i_v)(unsigned int target, unsigned int index, long long* data);
typedef void (PI_API *GL3_GetBufferParameteri64v)(unsigned int target, unsigned int pname, long long* params);
typedef void (PI_API *GL3_GenSamplers)(unsigned int count, unsigned int* samplers);
typedef void (PI_API *GL3_DeleteSamplers)(unsigned int count, const unsigned int* samplers);
typedef unsigned char  (PI_API *GL3_IsSampler)(unsigned int sampler);
typedef void (PI_API *GL3_BindSampler)(unsigned int unit, unsigned int sampler);
typedef void (PI_API *GL3_SamplerParameteri)(unsigned int sampler, unsigned int pname, int param);
typedef void (PI_API *GL3_SamplerParameteriv)(unsigned int sampler, unsigned int pname, const int* param);
typedef void (PI_API *GL3_SamplerParameterf)(unsigned int sampler, unsigned int pname, float param);
typedef void (PI_API *GL3_SamplerParameterfv)(unsigned int sampler, unsigned int pname, const float* param);
typedef void (PI_API *GL3_GetSamplerParameteriv)(unsigned int sampler, unsigned int pname, int* params);
typedef void (PI_API *GL3_GetSamplerParameterfv)(unsigned int sampler, unsigned int pname, float* params);
typedef void (PI_API *GL3_VertexAttribDivisor)(unsigned int index, unsigned int divisor);
typedef void (PI_API *GL3_BindTransformFeedback)(unsigned int target, unsigned int id);
typedef void (PI_API *GL3_DeleteTransformFeedbacks)(unsigned int n, const unsigned int* ids);
typedef void (PI_API *GL3_GenTransformFeedbacks)(unsigned int n, unsigned int* ids);
typedef unsigned char  (PI_API *GL3_IsTransformFeedback)(unsigned int id);
typedef void (PI_API *GL3_PauseTransformFeedback)(void);
typedef void (PI_API *GL3_ResumeTransformFeedback)(void);
typedef void (PI_API *GL3_GetProgramBinary)(unsigned int program, unsigned int bufSize, unsigned int* length, unsigned int* binaryFormat, void* binary);
typedef void (PI_API *GL3_ProgramBinary)(unsigned int program, unsigned int binaryFormat, const void* binary, unsigned int length);
typedef void (PI_API *GL3_ProgramParameteri)(unsigned int program, unsigned int pname, int value);
typedef void (PI_API *GL3_InvalidateFramebuffer)(unsigned int target, unsigned int numAttachments, const unsigned int* attachments);
typedef void (PI_API *GL3_InvalidateSubFramebuffer)(unsigned int target, unsigned int numAttachments, const unsigned int* attachments, int x, int y, unsigned int width, unsigned int height);
typedef void (PI_API *GL3_TexStorage2D)(unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height);
typedef void (PI_API *GL3_TexStorage3D)(unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth);
typedef void (PI_API *GL3_GetInternalformativ)(unsigned int target, unsigned int internalformat, unsigned int pname, unsigned int bufSize, int* params);

typedef struct
{
	GL_Self_IsLostContext _Self_IsLostContext;

	GL_PolygonMode _PolygonMode;
	GL_GetTexImage _GetTexImage;
	GL_GetCompressedTexImage _GetCompressedTexImage;
	GL_CopyImageSubData _CopyImageSubData;
	GL_FramebufferTexture3D _FramebufferTexture3D;

	GL2_ActiveTexture _ActiveTexture;
	GL2_AttachShader _AttachShader;
	GL2_BindAttribLocation _BindAttribLocation;
	GL2_BindBuffer _BindBuffer;
	GL2_BindFramebuffer _BindFramebuffer;
	GL2_BindRenderbuffer _BindRenderbuffer;
	GL2_BindTexture _BindTexture;
	GL2_BlendColor _BlendColor;
	GL2_BlendEquation _BlendEquation;
	GL2_BlendEquationSeparate _BlendEquationSeparate;
	GL2_BlendFunc _BlendFunc;
	GL2_BlendFuncSeparate _BlendFuncSeparate;
	GL2_BufferData _BufferData;
	GL2_BufferSubData _BufferSubData;
	GL2_CheckFramebufferStatus _CheckFramebufferStatus;
	GL2_Clear _Clear;
	GL2_ClearColor _ClearColor;
	GL2_ClearDepthf _ClearDepthf;
	GL2_ClearStencil _ClearStencil;
	GL2_ColorMask _ColorMask;
	GL2_CompileShader _CompileShader;
	GL2_CompressedTexImage2D _CompressedTexImage2D;
	GL2_CompressedTexSubImage2D _CompressedTexSubImage2D;
	GL2_CopyTexImage2D _CopyTexImage2D;
	GL2_CopyTexSubImage2D _CopyTexSubImage2D;
	GL2_CreateProgram _CreateProgram;
	GL2_CreateShader _CreateShader;
	GL2_CullFace _CullFace;
	GL2_DeleteBuffers _DeleteBuffers;
	GL2_DeleteFramebuffers _DeleteFramebuffers;
	GL2_DeleteProgram _DeleteProgram;
	GL2_DeleteRenderbuffers _DeleteRenderbuffers;
	GL2_DeleteShader _DeleteShader;
	GL2_DeleteTextures _DeleteTextures;
	GL2_DepthFunc _DepthFunc;
	GL2_DepthMask _DepthMask;
	GL2_DepthRangef _DepthRangef;
	GL2_DetachShader _DetachShader;
	GL2_Disable _Disable;
	GL2_DisableVertexAttribArray _DisableVertexAttribArray;
	GL2_DrawArrays _DrawArrays;
	GL2_DrawElements _DrawElements;
	GL2_Enable _Enable;
	GL2_EnableVertexAttribArray _EnableVertexAttribArray;
	GL2_Finish _Finish;
	GL2_Flush _Flush;
	GL2_FramebufferRenderbuffer _FramebufferRenderbuffer;
	GL2_FramebufferTexture2D _FramebufferTexture2D;
	GL2_FrontFace _FrontFace;
	GL2_GenBuffers _GenBuffers;
	GL2_GenerateMipmap _GenerateMipmap;
	GL2_GenFramebuffers _GenFramebuffers;
	GL2_GenRenderbuffers _GenRenderbuffers;
	GL2_GenTextures _GenTextures;
	GL2_GetActiveAttrib _GetActiveAttrib;
	GL2_GetActiveUniform _GetActiveUniform;
	GL2_GetAttachedShaders _GetAttachedShaders;
	GL2_GetAttribLocation _GetAttribLocation;
	GL2_GetBooleanv _GetBooleanv;
	GL2_GetBufferParameteriv _GetBufferParameteriv;
	GL2_GetError _GetError;
	GL2_GetFloatv _GetFloatv;
	GL2_GetFramebufferAttachmentParameteriv _GetFramebufferAttachmentParameteriv;
	GL2_GetIntegerv _GetIntegerv;
	GL2_GetProgramiv _GetProgramiv;
	GL2_GetProgramInfoLog _GetProgramInfoLog;
	GL2_GetRenderbufferParameteriv _GetRenderbufferParameteriv;
	GL2_GetShaderiv _GetShaderiv;
	GL2_GetShaderInfoLog _GetShaderInfoLog;
	GL2_GetShaderPrecisionFormat _GetShaderPrecisionFormat;
	GL2_GetShaderSource _GetShaderSource;
	GL2_GetString _GetString;
	GL2_GetTexParameterfv _GetTexParameterfv;
	GL2_GetTexParameteriv _GetTexParameteriv;
	GL2_GetUniformfv _GetUniformfv;
	GL2_GetUniformiv _GetUniformiv;
	GL2_GetUniformLocation _GetUniformLocation;
	GL2_GetVertexAttribfv _GetVertexAttribfv;
	GL2_GetVertexAttribiv _GetVertexAttribiv;
	GL2_GetVertexAttribPointerv _GetVertexAttribPointerv;
	GL2_Hint _Hint;
	GL2_IsBuffer _IsBuffer;
	GL2_IsEnabled _IsEnabled;
	GL2_IsFramebuffer _IsFramebuffer;
	GL2_IsProgram _IsProgram;
	GL2_IsRenderbuffer _IsRenderbuffer;
	GL2_IsShader _IsShader;
	GL2_IsTexture _IsTexture;
	GL2_LineWidth _LineWidth;
	GL2_LinkProgram _LinkProgram;
	GL2_PixelStorei _PixelStorei;
	GL2_PolygonOffset _PolygonOffset;
	GL2_ReadPixels _ReadPixels;
	GL2_ReleaseShaderCompiler _ReleaseShaderCompiler;
	GL2_RenderbufferStorage _RenderbufferStorage;
	GL2_SampleCoverage _SampleCoverage;
	GL2_Scissor _Scissor;
	GL2_ShaderBinary _ShaderBinary;
	GL2_ShaderSource _ShaderSource;
	GL2_StencilFunc _StencilFunc;
	GL2_StencilFuncSeparate _StencilFuncSeparate;
	GL2_StencilMask _StencilMask;
	GL2_StencilMaskSeparate _StencilMaskSeparate;
	GL2_StencilOp _StencilOp;
	GL2_StencilOpSeparate _StencilOpSeparate;
	GL2_TexImage2D _TexImage2D;
	GL2_TexParameterf _TexParameterf;
	GL2_TexParameterfv _TexParameterfv;
	GL2_TexParameteri _TexParameteri;
	GL2_TexParameteriv _TexParameteriv;
	GL2_TexSubImage2D _TexSubImage2D;
	GL2_Uniform1f _Uniform1f;
	GL2_Uniform1fv _Uniform1fv;
	GL2_Uniform1i _Uniform1i;
	GL2_Uniform1iv _Uniform1iv;
	GL2_Uniform2f _Uniform2f;
	GL2_Uniform2fv _Uniform2fv;
	GL2_Uniform2i _Uniform2i;
	GL2_Uniform2iv _Uniform2iv;
	GL2_Uniform3f _Uniform3f;
	GL2_Uniform3fv _Uniform3fv;
	GL2_Uniform3i _Uniform3i;
	GL2_Uniform3iv _Uniform3iv;
	GL2_Uniform4f _Uniform4f;
	GL2_Uniform4fv _Uniform4fv;
	GL2_Uniform4i _Uniform4i;
	GL2_Uniform4iv _Uniform4iv;
	GL2_UniformMatrix2fv _UniformMatrix2fv;
	GL2_UniformMatrix3fv _UniformMatrix3fv;
	GL2_UniformMatrix4fv _UniformMatrix4fv;
	GL2_UseProgram _UseProgram;
	GL2_ValidateProgram _ValidateProgram;
	GL2_VertexAttrib1f _VertexAttrib1f;
	GL2_VertexAttrib1fv _VertexAttrib1fv;
	GL2_VertexAttrib2f _VertexAttrib2f;
	GL2_VertexAttrib2fv _VertexAttrib2fv;
	GL2_VertexAttrib3f _VertexAttrib3f;
	GL2_VertexAttrib3fv _VertexAttrib3fv;
	GL2_VertexAttrib4f _VertexAttrib4f;
	GL2_VertexAttrib4fv _VertexAttrib4fv;
	GL2_VertexAttribPointer _VertexAttribPointer;
	GL2_Viewport _Viewport;

	GL3_ReadBuffer _ReadBuffer;
	GL3_DrawRangeElements _DrawRangeElements;
	GL3_TexImage3D _TexImage3D;
	GL3_TexSubImage3D _TexSubImage3D;
	GL3_CopyTexSubImage3D _CopyTexSubImage3D;
	GL3_CompressedTexImage3D _CompressedTexImage3D;
	GL3_CompressedTexSubImage3D _CompressedTexSubImage3D;
	GL3_GenQueries _GenQueries;
	GL3_DeleteQueries _DeleteQueries;
	GL3_IsQuery _IsQuery;
	GL3_BeginQuery _BeginQuery;
	GL3_EndQuery _EndQuery;
	GL3_GetQueryiv _GetQueryiv;
	GL3_GetQueryObjectuiv _GetQueryObjectuiv;
	GL3_UnmapBuffer _UnmapBuffer;
	GL3_GetBufferPointerv _GetBufferPointerv;
	GL3_DrawBuffers _DrawBuffers;
	GL3_UniformMatrix2x3fv _UniformMatrix2x3fv;
	GL3_UniformMatrix3x2fv _UniformMatrix3x2fv;
	GL3_UniformMatrix2x4fv _UniformMatrix2x4fv;
	GL3_UniformMatrix4x2fv _UniformMatrix4x2fv;
	GL3_UniformMatrix3x4fv _UniformMatrix3x4fv;
	GL3_UniformMatrix4x3fv _UniformMatrix4x3fv;
	GL3_BlitFramebuffer _BlitFramebuffer;
	GL3_RenderbufferStorageMultisample _RenderbufferStorageMultisample;
	GL3_FramebufferTextureLayer _FramebufferTextureLayer;
	GL3_MapBufferRange _MapBufferRange;
	GL3_FlushMappedBufferRange _FlushMappedBufferRange;
	GL3_BindVertexArray _BindVertexArray;
	GL3_DeleteVertexArrays _DeleteVertexArrays;
	GL3_GenVertexArrays _GenVertexArrays;
	GL3_IsVertexArray _IsVertexArray;
	GL3_GetIntegeri_v _GetIntegeri_v;
	GL3_BeginTransformFeedback _BeginTransformFeedback;
	GL3_EndTransformFeedback _EndTransformFeedback;
	GL3_BindBufferRange _BindBufferRange;
	GL3_BindBufferBase _BindBufferBase;
	GL3_TransformFeedbackVaryings _TransformFeedbackVaryings;
	GL3_GetTransformFeedbackVarying _GetTransformFeedbackVarying;
	GL3_VertexAttribIPointer _VertexAttribIPointer;
	GL3_GetVertexAttribIiv _GetVertexAttribIiv;
	GL3_GetVertexAttribIuiv _GetVertexAttribIuiv;
	GL3_VertexAttribI4i _VertexAttribI4i;
	GL3_VertexAttribI4ui _VertexAttribI4ui;
	GL3_VertexAttribI4iv _VertexAttribI4iv;
	GL3_VertexAttribI4uiv _VertexAttribI4uiv;
	GL3_GetUniformuiv _GetUniformuiv;
	GL3_GetFragDataLocation _GetFragDataLocation;
	GL3_Uniform1ui _Uniform1ui;
	GL3_Uniform2ui _Uniform2ui;
	GL3_Uniform3ui _Uniform3ui;
	GL3_Uniform4ui _Uniform4ui;
	GL3_Uniform1uiv _Uniform1uiv;
	GL3_Uniform2uiv _Uniform2uiv;
	GL3_Uniform3uiv _Uniform3uiv;
	GL3_Uniform4uiv _Uniform4uiv;
	GL3_ClearBufferiv _ClearBufferiv;
	GL3_ClearBufferuiv _ClearBufferuiv;
	GL3_ClearBufferfv _ClearBufferfv;
	GL3_ClearBufferfi _ClearBufferfi;
	GL3_GetStringi _GetStringi;
	GL3_CopyBufferSubData _CopyBufferSubData;
	GL3_GetUniformIndices _GetUniformIndices;
	GL3_GetActiveUniformsiv _GetActiveUniformsiv;
	GL3_GetUniformBlockIndex _GetUniformBlockIndex;
	GL3_GetActiveUniformBlockiv _GetActiveUniformBlockiv;
	GL3_GetActiveUniformBlockName _GetActiveUniformBlockName;
	GL3_UniformBlockBinding _UniformBlockBinding;
	GL3_DrawArraysInstanced _DrawArraysInstanced;
	GL3_DrawElementsInstanced _DrawElementsInstanced;
	GL3_FenceSync _FenceSync;
	GL3_IsSync _IsSync;
	GL3_DeleteSync _DeleteSync;
	GL3_ClientWaitSync _ClientWaitSync;
	GL3_WaitSync _WaitSync;
	GL3_GetInteger64v _GetInteger64v;
	GL3_GetSynciv _GetSynciv;
	GL3_GetInteger64i_v _GetInteger64i_v;
	GL3_GetBufferParameteri64v _GetBufferParameteri64v;
	GL3_GenSamplers _GenSamplers;
	GL3_DeleteSamplers _DeleteSamplers;
	GL3_IsSampler _IsSampler;
	GL3_BindSampler _BindSampler;
	GL3_SamplerParameteri _SamplerParameteri;
	GL3_SamplerParameteriv _SamplerParameteriv;
	GL3_SamplerParameterf _SamplerParameterf;
	GL3_SamplerParameterfv _SamplerParameterfv;
	GL3_GetSamplerParameteriv _GetSamplerParameteriv;
	GL3_GetSamplerParameterfv _GetSamplerParameterfv;
	GL3_VertexAttribDivisor _VertexAttribDivisor;
	GL3_BindTransformFeedback _BindTransformFeedback;
	GL3_DeleteTransformFeedbacks _DeleteTransformFeedbacks;
	GL3_GenTransformFeedbacks _GenTransformFeedbacks;
	GL3_IsTransformFeedback _IsTransformFeedback;
	GL3_PauseTransformFeedback _PauseTransformFeedback;
	GL3_ResumeTransformFeedback _ResumeTransformFeedback;
	GL3_GetProgramBinary _GetProgramBinary;
	GL3_ProgramBinary _ProgramBinary;
	GL3_ProgramParameteri _ProgramParameteri;
	GL3_InvalidateFramebuffer _InvalidateFramebuffer;
	GL3_InvalidateSubFramebuffer _InvalidateSubFramebuffer;
	GL3_TexStorage2D _TexStorage2D;
	GL3_TexStorage3D _TexStorage3D;
	GL3_GetInternalformativ _GetInternalformativ;

	RenderInterfaceType type;
}GLFunc;

/* 特性 */
typedef struct 
{
	RenderContextLoadType type;

	uint gl_version;
	PiBool _IsEsVersion;

	PiBool _isFBO;
	PiBool _IsInstanced;
	PiBool _IsGpuShader4;
	PiBool _IsCopyImage;
	PiBool _IsFramebufferSRGB;
	PiBool _IsHalfFloatPixel;
	PiBool _IsPackedFloat;
	PiBool _IsPackedDepthStencil;
	PiBool _IsDrawBuffers;
	PiBool _IsFramebufferBlit;
	PiBool _IsTextureArray;
	PiBool _IsTextureRG;
	PiBool _IsTexture3D;
	PiBool _IsTextureLOD;
	PiBool _IsTextureFloat;
	PiBool _IsTextureInteger;
	PiBool _IsTextureSRGB;
	PiBool _IsTextureSnorm;
	PiBool _IsTextureCompressionLatc;
	PiBool _IsTextureCompressionS3tc;
	PiBool _IsTextureCompressionRgtc;
	PiBool _IsTextureCompressionBptc;
	PiBool _IsTextureFilterAnisotropic;
	PiBool _IsVertexType_2_10_10_10_rev;
	PiBool _IsVertexArrayObject;
	PiBool _IsDepthClamp;
}GLCap;

/**
 * 以GLES3.0为蓝本定义的OpenGL接口
 * 注：GL3的可以调用GL2的宏和接口，向下兼容
 */

/*-------------------------------------------------------------------------
 * Token definitions
 *-----------------------------------------------------------------------*/

/************************************************ OpenGL 特有 ************************************************/

#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F

// Depth Stencil Textures. GL version is 4.3
#define GL_DEPTH_STENCIL_TEXTURE_MODE 0x90EA

/* 纹理格式 */

#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_SIGNED_RGB8_NV 0x86FF

/* 压缩纹理格式 */
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1

#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B

#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB

#define GL_COMPRESSED_LUMINANCE_LATC1_EXT 0x8C70
#define GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT 0x8C71
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT 0x8C72
#define GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT 0x8C73

/* 多边形模式 */
#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02

/* 纹理寻址 */
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_BORDER_COLOR 0x1004

/* 各向异性过滤 */
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

/* 多重采样 */
#define GL_MAX_SAMPLES_EXT 0x8D57

#define GL_DEPTH_CLAMP 0x864F
#define GL_MULTISAMPLE 0x809D
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_BACK_LEFT 0x0402
#define GL_TEXTURE_LOD_BIAS 0x8501

/************************************************ OpenGL ES 2.0 ************************************************/

/* ClearBufferMask */
#define GL2_DEPTH_BUFFER_BIT                              0x00000100
#define GL2_STENCIL_BUFFER_BIT                            0x00000400
#define GL2_COLOR_BUFFER_BIT                              0x00004000

/* Boolean */
#define GL2_FALSE                                         0
#define GL2_TRUE                                          1

/* BeginMode */
#define GL2_POINTS                                        0x0000
#define GL2_LINES                                         0x0001
#define GL2_LINE_LOOP                                     0x0002
#define GL2_LINE_STRIP                                    0x0003
#define GL2_TRIANGLES                                     0x0004
#define GL2_TRIANGLE_STRIP                                0x0005
#define GL2_TRIANGLE_FAN                                  0x0006

/* BlendingFactorDest */
#define GL2_ZERO                                          0
#define GL2_ONE                                           1
#define GL2_SRC_COLOR                                     0x0300
#define GL2_ONE_MINUS_SRC_COLOR                           0x0301
#define GL2_SRC_ALPHA                                     0x0302
#define GL2_ONE_MINUS_SRC_ALPHA                           0x0303
#define GL2_DST_ALPHA                                     0x0304
#define GL2_ONE_MINUS_DST_ALPHA                           0x0305

#define GL2_DST_COLOR                                     0x0306
#define GL2_ONE_MINUS_DST_COLOR                           0x0307
#define GL2_SRC_ALPHA_SATURATE                            0x0308

/* BlendEquationSeparate */
#define GL2_FUNC_ADD                                      0x8006
#define GL2_BLEND_EQUATION                                0x8009
#define GL2_BLEND_EQUATION_RGB                            0x8009    /* same as BLEND_EQUATION */
#define GL2_BLEND_EQUATION_ALPHA                          0x883D

/* BlendSubtract */
#define GL2_FUNC_SUBTRACT                                 0x800A
#define GL2_FUNC_REVERSE_SUBTRACT                         0x800B

/* Separate Blend Functions */
#define GL2_BLEND_DST_RGB                                 0x80C8
#define GL2_BLEND_SRC_RGB                                 0x80C9
#define GL2_BLEND_DST_ALPHA                               0x80CA
#define GL2_BLEND_SRC_ALPHA                               0x80CB
#define GL2_CONSTANT_COLOR                                0x8001
#define GL2_ONE_MINUS_CONSTANT_COLOR                      0x8002
#define GL2_CONSTANT_ALPHA                                0x8003
#define GL2_ONE_MINUS_CONSTANT_ALPHA                      0x8004
#define GL2_BLEND_COLOR                                   0x8005

/* Buffer Objects */
#define GL2_ARRAY_BUFFER                                  0x8892
#define GL2_ELEMENT_ARRAY_BUFFER                          0x8893
#define GL2_ARRAY_BUFFER_BINDING                          0x8894
#define GL2_ELEMENT_ARRAY_BUFFER_BINDING                  0x8895

#define GL2_STREAM_DRAW                                   0x88E0
#define GL2_STATIC_DRAW                                   0x88E4
#define GL2_DYNAMIC_DRAW                                  0x88E8

#define GL2_BUFFER_SIZE                                   0x8764
#define GL2_BUFFER_USAGE                                  0x8765

#define GL2_CURRENT_VERTEX_ATTRIB                         0x8626

/* CullFaceMode */
#define GL2_FRONT                                         0x0404
#define GL2_BACK                                          0x0405
#define GL2_FRONT_AND_BACK                                0x0408

/* EnableCap */
#define GL2_TEXTURE_2D                                    0x0DE1
#define GL2_CULL_FACE                                     0x0B44
#define GL2_BLEND                                         0x0BE2
#define GL2_DITHER                                        0x0BD0
#define GL2_STENCIL_TEST                                  0x0B90
#define GL2_DEPTH_TEST                                    0x0B71
#define GL2_SCISSOR_TEST                                  0x0C11
#define GL2_POLYGON_OFFSET_FILL                           0x8037
#define GL2_SAMPLE_ALPHA_TO_COVERAGE                      0x809E
#define GL2_SAMPLE_COVERAGE                               0x80A0

/* ErrorCode */
#define GL2_NO_ERROR                                      0
#define GL2_INVALID_ENUM                                  0x0500
#define GL2_INVALID_VALUE                                 0x0501
#define GL2_INVALID_OPERATION                             0x0502
#define GL2_OUT_OF_MEMORY                                 0x0505

/* FrontFaceDirection */
#define GL2_CW                                            0x0900
#define GL2_CCW                                           0x0901

/* GetPName */
#define GL2_LINE_WIDTH                                    0x0B21
#define GL2_ALIASED_POINT_SIZE_RANGE                      0x846D
#define GL2_ALIASED_LINE_WIDTH_RANGE                      0x846E
#define GL2_CULL_FACE_MODE                                0x0B45
#define GL2_FRONT_FACE                                    0x0B46
#define GL2_DEPTH_RANGE                                   0x0B70
#define GL2_DEPTH_WRITEMASK                               0x0B72
#define GL2_DEPTH_CLEAR_VALUE                             0x0B73
#define GL2_DEPTH_FUNC                                    0x0B74
#define GL2_STENCIL_CLEAR_VALUE                           0x0B91
#define GL2_STENCIL_FUNC                                  0x0B92
#define GL2_STENCIL_FAIL                                  0x0B94
#define GL2_STENCIL_PASS_DEPTH_FAIL                       0x0B95
#define GL2_STENCIL_PASS_DEPTH_PASS                       0x0B96
#define GL2_STENCIL_REF                                   0x0B97
#define GL2_STENCIL_VALUE_MASK                            0x0B93
#define GL2_STENCIL_WRITEMASK                             0x0B98
#define GL2_STENCIL_BACK_FUNC                             0x8800
#define GL2_STENCIL_BACK_FAIL                             0x8801
#define GL2_STENCIL_BACK_PASS_DEPTH_FAIL                  0x8802
#define GL2_STENCIL_BACK_PASS_DEPTH_PASS                  0x8803
#define GL2_STENCIL_BACK_REF                              0x8CA3
#define GL2_STENCIL_BACK_VALUE_MASK                       0x8CA4
#define GL2_STENCIL_BACK_WRITEMASK                        0x8CA5
#define GL2_VIEWPORT                                      0x0BA2
#define GL2_SCISSOR_BOX                                   0x0C10
#define GL2_COLOR_CLEAR_VALUE                             0x0C22
#define GL2_COLOR_WRITEMASK                               0x0C23
#define GL2_UNPACK_ALIGNMENT                              0x0CF5
#define GL2_PACK_ALIGNMENT                                0x0D05
#define GL2_MAX_TEXTURE_SIZE                              0x0D33
#define GL2_MAX_VIEWPORT_DIMS                             0x0D3A
#define GL2_SUBPIXEL_BITS                                 0x0D50
#define GL2_RED_BITS                                      0x0D52
#define GL2_GREEN_BITS                                    0x0D53
#define GL2_BLUE_BITS                                     0x0D54
#define GL2_ALPHA_BITS                                    0x0D55
#define GL2_DEPTH_BITS                                    0x0D56
#define GL2_STENCIL_BITS                                  0x0D57
#define GL2_POLYGON_OFFSET_UNITS                          0x2A00
#define GL2_POLYGON_OFFSET_FACTOR                         0x8038
#define GL2_TEXTURE_BINDING_2D                            0x8069
#define GL2_SAMPLE_BUFFERS                                0x80A8
#define GL2_SAMPLES                                       0x80A9
#define GL2_SAMPLE_COVERAGE_VALUE                         0x80AA
#define GL2_SAMPLE_COVERAGE_INVERT                        0x80AB

#define GL2_NUM_COMPRESSED_TEXTURE_FORMATS                0x86A2
#define GL2_COMPRESSED_TEXTURE_FORMATS                    0x86A3

/* HintMode */
#define GL2_DONT_CARE                                     0x1100
#define GL2_FASTEST                                       0x1101
#define GL2_NICEST                                        0x1102

/* HintTarget */
#define GL2_GENERATE_MIPMAP_HINT                          0x8192

/* DataType */
#define GL2_BYTE                                          0x1400
#define GL2_UNSIGNED_BYTE                                 0x1401
#define GL2_SHORT                                         0x1402
#define GL2_UNSIGNED_SHORT                                0x1403
#define GL2_INT                                           0x1404
#define GL2_UNSIGNED_INT                                  0x1405
#define GL2_FLOAT                                         0x1406
#define GL2_FIXED                                         0x140C

/* PixelFormat */
#define GL2_STENCIL_INDEX                                 0x1901
#define GL2_DEPTH_COMPONENT                               0x1902
#define GL2_ALPHA                                         0x1906
#define GL2_RGB                                           0x1907
#define GL2_RGBA                                          0x1908
#define GL2_LUMINANCE                                     0x1909
#define GL2_LUMINANCE_ALPHA                               0x190A

/* PixelType */
#define GL2_UNSIGNED_SHORT_4_4_4_4                        0x8033
#define GL2_UNSIGNED_SHORT_5_5_5_1                        0x8034
#define GL2_UNSIGNED_SHORT_5_6_5                          0x8363

/* Shaders */
#define GL2_FRAGMENT_SHADER                               0x8B30
#define GL2_VERTEX_SHADER                                 0x8B31
#define GL2_MAX_VERTEX_ATTRIBS                            0x8869
#define GL2_MAX_VERTEX_UNIFORM_VECTORS                    0x8DFB
#define GL2_MAX_VARYING_VECTORS                           0x8DFC
#define GL2_MAX_COMBINED_TEXTURE_IMAGE_UNITS              0x8B4D
#define GL2_MAX_VERTEX_TEXTURE_IMAGE_UNITS                0x8B4C
#define GL2_MAX_TEXTURE_IMAGE_UNITS                       0x8872
#define GL2_MAX_FRAGMENT_UNIFORM_VECTORS                  0x8DFD
#define GL2_SHADER_TYPE                                   0x8B4F
#define GL2_DELETE_STATUS                                 0x8B80
#define GL2_LINK_STATUS                                   0x8B82
#define GL2_VALIDATE_STATUS                               0x8B83
#define GL2_ATTACHED_SHADERS                              0x8B85
#define GL2_ACTIVE_UNIFORMS                               0x8B86
#define GL2_ACTIVE_UNIFORM_MAX_LENGTH                     0x8B87
#define GL2_ACTIVE_ATTRIBUTES                             0x8B89
#define GL2_ACTIVE_ATTRIBUTE_MAX_LENGTH                   0x8B8A
#define GL2_SHADING_LANGUAGE_VERSION                      0x8B8C
#define GL2_CURRENT_PROGRAM                               0x8B8D

/* StencilFunction */
#define GL2_NEVER                                         0x0200
#define GL2_LESS                                          0x0201
#define GL2_EQUAL                                         0x0202
#define GL2_LEQUAL                                        0x0203
#define GL2_GREATER                                       0x0204
#define GL2_NOTEQUAL                                      0x0205
#define GL2_GEQUAL                                        0x0206
#define GL2_ALWAYS                                        0x0207

/* StencilOp */
#define GL2_KEEP                                          0x1E00
#define GL2_REPLACE                                       0x1E01
#define GL2_INCR                                          0x1E02
#define GL2_DECR                                          0x1E03
#define GL2_INVERT                                        0x150A
#define GL2_INCR_WRAP                                     0x8507
#define GL2_DECR_WRAP                                     0x8508

/* StringName */
#define GL2_VENDOR                                        0x1F00
#define GL2_RENDERER                                      0x1F01
#define GL2_VERSION                                       0x1F02
#define GL2_EXTENSIONS                                    0x1F03

/* TextureMagFilter */
#define GL2_NEAREST                                       0x2600
#define GL2_LINEAR                                        0x2601

/* TextureMinFilter */
#define GL2_NEAREST_MIPMAP_NEAREST                        0x2700
#define GL2_LINEAR_MIPMAP_NEAREST                         0x2701
#define GL2_NEAREST_MIPMAP_LINEAR                         0x2702
#define GL2_LINEAR_MIPMAP_LINEAR                          0x2703

/* TextureParameterName */
#define GL2_TEXTURE_MAG_FILTER                            0x2800
#define GL2_TEXTURE_MIN_FILTER                            0x2801
#define GL2_TEXTURE_WRAP_S                                0x2802
#define GL2_TEXTURE_WRAP_T                                0x2803

/* TextureTarget */
#define GL2_TEXTURE                                       0x1702

#define GL2_TEXTURE_CUBE_MAP                              0x8513
#define GL2_TEXTURE_BINDING_CUBE_MAP                      0x8514
#define GL2_TEXTURE_CUBE_MAP_POSITIVE_X                   0x8515
#define GL2_TEXTURE_CUBE_MAP_NEGATIVE_X                   0x8516
#define GL2_TEXTURE_CUBE_MAP_POSITIVE_Y                   0x8517
#define GL2_TEXTURE_CUBE_MAP_NEGATIVE_Y                   0x8518
#define GL2_TEXTURE_CUBE_MAP_POSITIVE_Z                   0x8519
#define GL2_TEXTURE_CUBE_MAP_NEGATIVE_Z                   0x851A
#define GL2_MAX_CUBE_MAP_TEXTURE_SIZE                     0x851C

/* TextureUnit */
#define GL2_TEXTURE0                                      0x84C0
#define GL2_TEXTURE1                                      0x84C1
#define GL2_TEXTURE2                                      0x84C2
#define GL2_TEXTURE3                                      0x84C3
#define GL2_TEXTURE4                                      0x84C4
#define GL2_TEXTURE5                                      0x84C5
#define GL2_TEXTURE6                                      0x84C6
#define GL2_TEXTURE7                                      0x84C7
#define GL2_TEXTURE8                                      0x84C8
#define GL2_TEXTURE9                                      0x84C9
#define GL2_TEXTURE10                                     0x84CA
#define GL2_TEXTURE11                                     0x84CB
#define GL2_TEXTURE12                                     0x84CC
#define GL2_TEXTURE13                                     0x84CD
#define GL2_TEXTURE14                                     0x84CE
#define GL2_TEXTURE15                                     0x84CF
#define GL2_TEXTURE16                                     0x84D0
#define GL2_TEXTURE17                                     0x84D1
#define GL2_TEXTURE18                                     0x84D2
#define GL2_TEXTURE19                                     0x84D3
#define GL2_TEXTURE20                                     0x84D4
#define GL2_TEXTURE21                                     0x84D5
#define GL2_TEXTURE22                                     0x84D6
#define GL2_TEXTURE23                                     0x84D7
#define GL2_TEXTURE24                                     0x84D8
#define GL2_TEXTURE25                                     0x84D9
#define GL2_TEXTURE26                                     0x84DA
#define GL2_TEXTURE27                                     0x84DB
#define GL2_TEXTURE28                                     0x84DC
#define GL2_TEXTURE29                                     0x84DD
#define GL2_TEXTURE30                                     0x84DE
#define GL2_TEXTURE31                                     0x84DF
#define GL2_ACTIVE_TEXTURE                                0x84E0

/* TextureWrapMode */
#define GL2_REPEAT                                        0x2901
#define GL2_CLAMP_TO_EDGE                                 0x812F
#define GL2_MIRRORED_REPEAT                               0x8370

/* Uniform Types */
#define GL2_FLOAT_VEC2                                    0x8B50
#define GL2_FLOAT_VEC3                                    0x8B51
#define GL2_FLOAT_VEC4                                    0x8B52
#define GL2_INT_VEC2                                      0x8B53
#define GL2_INT_VEC3                                      0x8B54
#define GL2_INT_VEC4                                      0x8B55
#define GL2_BOOL                                          0x8B56
#define GL2_BOOL_VEC2                                     0x8B57
#define GL2_BOOL_VEC3                                     0x8B58
#define GL2_BOOL_VEC4                                     0x8B59
#define GL2_FLOAT_MAT2                                    0x8B5A
#define GL2_FLOAT_MAT3                                    0x8B5B
#define GL2_FLOAT_MAT4                                    0x8B5C
#define GL2_SAMPLER_2D                                    0x8B5E
#define GL2_SAMPLER_CUBE                                  0x8B60

/* Vertex Arrays */
#define GL2_VERTEX_ATTRIB_ARRAY_ENABLED                   0x8622
#define GL2_VERTEX_ATTRIB_ARRAY_SIZE                      0x8623
#define GL2_VERTEX_ATTRIB_ARRAY_STRIDE                    0x8624
#define GL2_VERTEX_ATTRIB_ARRAY_TYPE                      0x8625
#define GL2_VERTEX_ATTRIB_ARRAY_NORMALIZED                0x886A
#define GL2_VERTEX_ATTRIB_ARRAY_POINTER                   0x8645
#define GL2_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING            0x889F

/* Read Format */
#define GL2_IMPLEMENTATION_COLOR_READ_TYPE                0x8B9A
#define GL2_IMPLEMENTATION_COLOR_READ_FORMAT              0x8B9B

/* Shader Source */
#define GL2_COMPILE_STATUS                                0x8B81
#define GL2_INFO_LOG_LENGTH                               0x8B84
#define GL2_SHADER_SOURCE_LENGTH                          0x8B88
#define GL2_SHADER_COMPILER                               0x8DFA

/* Shader Binary */
#define GL2_SHADER_BINARY_FORMATS                         0x8DF8
#define GL2_NUM_SHADER_BINARY_FORMATS                     0x8DF9

/* Shader Precision-Specified Types */
#define GL2_LOW_FLOAT                                     0x8DF0
#define GL2_MEDIUM_FLOAT                                  0x8DF1
#define GL2_HIGH_FLOAT                                    0x8DF2
#define GL2_LOW_INT                                       0x8DF3
#define GL2_MEDIUM_INT                                    0x8DF4
#define GL2_HIGH_INT                                      0x8DF5

/* Framebuffer Object. */
#define GL2_FRAMEBUFFER                                   0x8D40
#define GL2_RENDERBUFFER                                  0x8D41

#define GL2_RGBA4                                         0x8056
#define GL2_RGB5_A1                                       0x8057
#define GL2_RGB565                                        0x8D62
#define GL2_DEPTH_COMPONENT16                             0x81A5
#define GL2_STENCIL_INDEX8                                0x8D48

#define GL2_RENDERBUFFER_WIDTH                            0x8D42
#define GL2_RENDERBUFFER_HEIGHT                           0x8D43
#define GL2_RENDERBUFFER_INTERNAL_FORMAT                  0x8D44
#define GL2_RENDERBUFFER_RED_SIZE                         0x8D50
#define GL2_RENDERBUFFER_GREEN_SIZE                       0x8D51
#define GL2_RENDERBUFFER_BLUE_SIZE                        0x8D52
#define GL2_RENDERBUFFER_ALPHA_SIZE                       0x8D53
#define GL2_RENDERBUFFER_DEPTH_SIZE                       0x8D54
#define GL2_RENDERBUFFER_STENCIL_SIZE                     0x8D55

#define GL2_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE            0x8CD0
#define GL2_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME            0x8CD1
#define GL2_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL          0x8CD2
#define GL2_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE  0x8CD3

#define GL2_COLOR_ATTACHMENT0                             0x8CE0
#define GL2_DEPTH_ATTACHMENT                              0x8D00
#define GL2_STENCIL_ATTACHMENT                            0x8D20

#define GL2_NONE                                          0

#define GL2_FRAMEBUFFER_COMPLETE                          0x8CD5
#define GL2_FRAMEBUFFER_INCOMPLETE_ATTACHMENT             0x8CD6
#define GL2_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT     0x8CD7
#define GL2_FRAMEBUFFER_INCOMPLETE_DIMENSIONS             0x8CD9
#define GL2_FRAMEBUFFER_UNSUPPORTED                       0x8CDD

#define GL2_FRAMEBUFFER_BINDING                           0x8CA6
#define GL2_RENDERBUFFER_BINDING                          0x8CA7
#define GL2_MAX_RENDERBUFFER_SIZE                         0x84E8

#define GL2_INVALID_FRAMEBUFFER_OPERATION                 0x0506

/************************************************ OpenGL ES 3.0 ************************************************/

/* ------- 目前用到的宏 ------- */

/* 混合模式 */
#define GL3_MIN                                           0x8007
#define GL3_MAX                                           0x8008

/* 纹理类型 */
#define GL3_TEXTURE_2D_ARRAY                              0x8C1A
#define GL3_TEXTURE_3D                                    0x806F
#define GL3_SAMPLER_3D                                    0x8B5F
#define GL3_SAMPLER_2D_ARRAY                              0x8DC1
#define GL3_SAMPLER_2D_SHADOW                             0x8B62
#define GL3_COMPARE_REF_TO_TEXTURE						  0X884E
#define GL3_UNSIGNED_INT_SAMPLER_2D                       0x8DD2

/* 纹理格式 */
#define GL3_RGBA32F                                       0x8814
#define GL3_RGB32F                                        0x8815
#define GL3_R8                                            0x8229
#define GL3_RG8                                           0x822B
#define GL3_R16F                                          0x822D
#define GL3_R32F                                          0x822E
#define GL3_RG16F                                         0x822F
#define GL3_RG32F                                         0x8230
#define GL3_RGBA16F                                       0x881A
#define GL3_RGB16F                                        0x881B
#define GL3_R8I                                           0x8231
#define GL3_R8UI                                          0x8232
#define GL3_R16I                                          0x8233
#define GL3_R16UI                                         0x8234
#define GL3_R32I                                          0x8235
#define GL3_R32UI                                         0x8236
#define GL3_RG8I                                          0x8237
#define GL3_RG8UI                                         0x8238
#define GL3_RG16I                                         0x8239
#define GL3_RG16UI                                        0x823A
#define GL3_RG32I                                         0x823B
#define GL3_RG32UI                                        0x823C
#define GL3_RGB10_A2                                      0x8059

#define GL3_RGBA32UI                                      0x8D70
#define GL3_RGB32UI                                       0x8D71
#define GL3_RGBA16UI                                      0x8D76
#define GL3_RGB16UI                                       0x8D77
#define GL3_RGBA8UI                                       0x8D7C
#define GL3_RGB8UI                                        0x8D7D
#define GL3_RGBA32I                                       0x8D82
#define GL3_RGB32I                                        0x8D83
#define GL3_RGBA16I                                       0x8D88
#define GL3_RGB16I                                        0x8D89
#define GL3_RGBA8I                                        0x8D8E
#define GL3_RGB8I                                         0x8D8F
#define GL3_R11F_G11F_B10F                                0x8C3A

#define GL3_RED_INTEGER                                   0x8D94
#define GL3_RGB_INTEGER                                   0x8D98
#define GL3_RGBA_INTEGER                                  0x8D99

#define GL3_RG                                            0x8227
#define GL3_RED                                           0x1903
#define GL3_RG_INTEGER                                    0x8228

#define GL3_HALF_FLOAT                                    0x140B
#define GL3_INT_2_10_10_10_REV                            0x8D9F
#define GL3_UNSIGNED_INT_2_10_10_10_REV                   0x8368
#define GL3_UNSIGNED_INT_10F_11F_11F_REV                  0x8C3B

#define GL3_DEPTH_COMPONENT24                             0x81A6
#define GL3_DEPTH_STENCIL                                 0x84F9
#define GL3_UNSIGNED_INT_24_8                             0x84FA
#define GL3_DEPTH24_STENCIL8                              0x88F0
#define GL3_DEPTH_COMPONENT32F                            0x8CAC
#define GL3_DEPTH32F_STENCIL8                             0x8CAD
#define GL3_SRGB8_ALPHA8                                  0x8C43

/* 读写建议 */
#define GL3_STREAM_READ                                   0x88E1
#define GL3_STREAM_COPY                                   0x88E2
#define GL3_STATIC_READ                                   0x88E5
#define GL3_STATIC_COPY                                   0x88E6
#define GL3_DYNAMIC_READ                                  0x88E9
#define GL3_DYNAMIC_COPY                                  0x88EA

/* 特性 */
#define GL3_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL3_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF
#define GL3_MAX_DRAW_BUFFERS                              0x8824

/* FrameBuffer的读写 */
#define GL3_READ_FRAMEBUFFER                              0x8CA8
#define GL3_DRAW_FRAMEBUFFER                              0x8CA9

/* 纹理 */
#define GL3_TEXTURE_WRAP_R                                0x8072
#define GL3_TEXTURE_MIN_LOD                               0x813A
#define GL3_TEXTURE_MAX_LOD                               0x813B
#define GL3_TEXTURE_COMPARE_MODE                          0x884C
#define GL3_TEXTURE_COMPARE_FUNC                          0x884D
#define GL3_TEXTURE_MAX_LEVEL                             0x813D


/* ------- 没有到的宏 ------- */

//#define GL3_RGB8                                          0x8051
//#define GL3_RGBA8                                         0x8058
//#define GL3_READ_BUFFER                                   0x0C02
//#define GL3_UNPACK_ROW_LENGTH                             0x0CF2
//#define GL3_UNPACK_SKIP_ROWS                              0x0CF3
//#define GL3_UNPACK_SKIP_PIXELS                            0x0CF4
//#define GL3_PACK_ROW_LENGTH                               0x0D02
//#define GL3_PACK_SKIP_ROWS                                0x0D03
//#define GL3_PACK_SKIP_PIXELS                              0x0D04
//#define GL3_COLOR                                         0x1800
//#define GL3_DEPTH                                         0x1801
//#define GL3_STENCIL                                       0x1802
//#define GL3_TEXTURE_BINDING_3D                            0x806A
//#define GL3_UNPACK_SKIP_IMAGES                            0x806D
//#define GL3_UNPACK_IMAGE_HEIGHT                           0x806E
//#define GL3_MAX_ELEMENTS_VERTICES                         0x80E8
//#define GL3_MAX_ELEMENTS_INDICES                          0x80E9
//#define GL3_TEXTURE_BASE_LEVEL                            0x813C
//#define GL3_MAX_TEXTURE_LOD_BIAS                          0x84FD
//#define GL3_CURRENT_QUERY                                 0x8865
//#define GL3_QUERY_RESULT                                  0x8866
//#define GL3_QUERY_RESULT_AVAILABLE                        0x8867
//#define GL3_BUFFER_MAPPED                                 0x88BC
//#define GL3_BUFFER_MAP_POINTER                            0x88BD
//#define GL3_DRAW_BUFFER0                                  0x8825
//#define GL3_DRAW_BUFFER1                                  0x8826
//#define GL3_DRAW_BUFFER2                                  0x8827
//#define GL3_DRAW_BUFFER3                                  0x8828
//#define GL3_DRAW_BUFFER4                                  0x8829
//#define GL3_DRAW_BUFFER5                                  0x882A
//#define GL3_DRAW_BUFFER6                                  0x882B
//#define GL3_DRAW_BUFFER7                                  0x882C
//#define GL3_DRAW_BUFFER8                                  0x882D
//#define GL3_DRAW_BUFFER9                                  0x882E
//#define GL3_DRAW_BUFFER10                                 0x882F
//#define GL3_DRAW_BUFFER11                                 0x8830
//#define GL3_DRAW_BUFFER12                                 0x8831
//#define GL3_DRAW_BUFFER13                                 0x8832
//#define GL3_DRAW_BUFFER14                                 0x8833
//#define GL3_DRAW_BUFFER15                                 0x8834
//#define GL3_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
//#define GL3_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A
//#define GL3_FRAGMENT_SHADER_DERIVATIVE_HINT               0x8B8B
//#define GL3_PIXEL_PACK_BUFFER                             0x88EB
//#define GL3_PIXEL_UNPACK_BUFFER                           0x88EC
//#define GL3_PIXEL_PACK_BUFFER_BINDING                     0x88ED
//#define GL3_PIXEL_UNPACK_BUFFER_BINDING                   0x88EF
//#define GL3_FLOAT_MAT2x3                                  0x8B65
//#define GL3_FLOAT_MAT2x4                                  0x8B66
//#define GL3_FLOAT_MAT3x2                                  0x8B67
//#define GL3_FLOAT_MAT3x4                                  0x8B68
//#define GL3_FLOAT_MAT4x2                                  0x8B69
//#define GL3_FLOAT_MAT4x3                                  0x8B6A
//#define GL3_SRGB                                          0x8C40
//#define GL3_SRGB8                                         0x8C41
//#define GL3_MAJOR_VERSION                                 0x821B
//#define GL3_MINOR_VERSION                                 0x821C
//#define GL3_NUM_EXTENSIONS                                0x821D
//#define GL3_VERTEX_ATTRIB_ARRAY_INTEGER                   0x88FD
//#define GL3_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
//#define GL3_MAX_PROGRAM_TEXEL_OFFSET                      0x8905
//#define GL3_MAX_VARYING_COMPONENTS                        0x8B4B
//#define GL3_TEXTURE_BINDING_2D_ARRAY                      0x8C1D
//#define GL3_RGB9_E5                                       0x8C3D
//#define GL3_UNSIGNED_INT_5_9_9_9_REV                      0x8C3E
//#define GL3_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH         0x8C76
//#define GL3_TRANSFORM_FEEDBACK_BUFFER_MODE                0x8C7F
//#define GL3_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
//#define GL3_TRANSFORM_FEEDBACK_VARYINGS                   0x8C83
//#define GL3_TRANSFORM_FEEDBACK_BUFFER_START               0x8C84
//#define GL3_TRANSFORM_FEEDBACK_BUFFER_SIZE                0x8C85
//#define GL3_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN         0x8C88
//#define GL3_RASTERIZER_DISCARD                            0x8C89
//#define GL3_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
//#define GL3_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
//#define GL3_INTERLEAVED_ATTRIBS                           0x8C8C
//#define GL3_SEPARATE_ATTRIBS                              0x8C8D
//#define GL3_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
//#define GL3_TRANSFORM_FEEDBACK_BUFFER_BINDING             0x8C8F
//#define GL3_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
//#define GL3_SAMPLER_CUBE_SHADOW                           0x8DC5
//#define GL3_UNSIGNED_INT_VEC2                             0x8DC6
//#define GL3_UNSIGNED_INT_VEC3                             0x8DC7
//#define GL3_UNSIGNED_INT_VEC4                             0x8DC8
//#define GL3_INT_SAMPLER_2D                                0x8DCA
//#define GL3_INT_SAMPLER_3D                                0x8DCB
//#define GL3_INT_SAMPLER_CUBE                              0x8DCC
//#define GL3_INT_SAMPLER_2D_ARRAY                          0x8DCF
//#define GL3_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
//#define GL3_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
//#define GL3_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
//#define GL3_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7
//#define GL3_BUFFER_ACCESS_FLAGS                           0x911F
//#define GL3_BUFFER_MAP_LENGTH                             0x9120
//#define GL3_BUFFER_MAP_OFFSET                             0x9121
//#define GL3_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD
//#define GL3_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING         0x8210
//#define GL3_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE         0x8211
//#define GL3_FRAMEBUFFER_ATTACHMENT_RED_SIZE               0x8212
//#define GL3_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE             0x8213
//#define GL3_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE              0x8214
//#define GL3_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE             0x8215
//#define GL3_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE             0x8216
//#define GL3_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE           0x8217
//#define GL3_FRAMEBUFFER_DEFAULT                           0x8218
//#define GL3_FRAMEBUFFER_UNDEFINED                         0x8219
//#define GL3_DEPTH_STENCIL_ATTACHMENT                      0x821A
//#define GL3_UNSIGNED_NORMALIZED                           0x8C17
//#define GL3_DRAW_FRAMEBUFFER_BINDING                      GL3_FRAMEBUFFER_BINDING
//#define GL3_READ_FRAMEBUFFER_BINDING                      0x8CAA
//#define GL3_RENDERBUFFER_SAMPLES                          0x8CAB
//#define GL3_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER          0x8CD4
//#define GL3_MAX_COLOR_ATTACHMENTS                         0x8CDF
//#define GL3_COLOR_ATTACHMENT1                             0x8CE1
//#define GL3_COLOR_ATTACHMENT2                             0x8CE2
//#define GL3_COLOR_ATTACHMENT3                             0x8CE3
//#define GL3_COLOR_ATTACHMENT4                             0x8CE4
//#define GL3_COLOR_ATTACHMENT5                             0x8CE5
//#define GL3_COLOR_ATTACHMENT6                             0x8CE6
//#define GL3_COLOR_ATTACHMENT7                             0x8CE7
//#define GL3_COLOR_ATTACHMENT8                             0x8CE8
//#define GL3_COLOR_ATTACHMENT9                             0x8CE9
//#define GL3_COLOR_ATTACHMENT10                            0x8CEA
//#define GL3_COLOR_ATTACHMENT11                            0x8CEB
//#define GL3_COLOR_ATTACHMENT12                            0x8CEC
//#define GL3_COLOR_ATTACHMENT13                            0x8CED
//#define GL3_COLOR_ATTACHMENT14                            0x8CEE
//#define GL3_COLOR_ATTACHMENT15                            0x8CEF
//#define GL3_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE            0x8D56
//#define GL3_MAX_SAMPLES                                   0x8D57
#define GL3_MAP_READ_BIT                                  0x0001
#define GL3_MAP_WRITE_BIT                                 0x0002
#define GL3_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL3_MAP_INVALIDATE_BUFFER_BIT                     0x0008
#define GL3_MAP_FLUSH_EXPLICIT_BIT                        0x0010
#define GL3_MAP_UNSYNCHRONIZED_BIT                        0x0020
//#define GL3_VERTEX_ARRAY_BINDING                          0x85B5
//#define GL3_R8_SNORM                                      0x8F94
//#define GL3_RG8_SNORM                                     0x8F95
//#define GL3_RGB8_SNORM                                    0x8F96
//#define GL3_RGBA8_SNORM                                   0x8F97
//#define GL3_SIGNED_NORMALIZED                             0x8F9C
//#define GL3_PRIMITIVE_RESTART_FIXED_INDEX                 0x8D69
//#define GL3_COPY_READ_BUFFER                              0x8F36
//#define GL3_COPY_WRITE_BUFFER                             0x8F37
//#define GL3_COPY_READ_BUFFER_BINDING                      GL3_COPY_READ_BUFFER
//#define GL3_COPY_WRITE_BUFFER_BINDING                     GL3_COPY_WRITE_BUFFER
#define GL3_UNIFORM_BUFFER                                0x8A11
//#define GL3_UNIFORM_BUFFER_BINDING                        0x8A28
//#define GL3_UNIFORM_BUFFER_START                          0x8A29
//#define GL3_UNIFORM_BUFFER_SIZE                           0x8A2A
//#define GL3_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
//#define GL3_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
//#define GL3_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
//#define GL3_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
//#define GL3_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
//#define GL3_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
//#define GL3_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
//#define GL3_UNIFORM_BUFFER_OFFSET_ALIGNMENT               0x8A34
#define GL3_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH          0x8A35
#define GL3_ACTIVE_UNIFORM_BLOCKS                         0x8A36
#define GL3_UNIFORM_TYPE                                  0x8A37
#define GL3_UNIFORM_SIZE                                  0x8A38
//#define GL3_UNIFORM_NAME_LENGTH                           0x8A39
//#define GL3_UNIFORM_BLOCK_INDEX                           0x8A3A
#define GL3_UNIFORM_OFFSET                                0x8A3B
//#define GL3_UNIFORM_ARRAY_STRIDE                          0x8A3C
//#define GL3_UNIFORM_MATRIX_STRIDE                         0x8A3D
//#define GL3_UNIFORM_IS_ROW_MAJOR                          0x8A3E
#define GL3_UNIFORM_BLOCK_BINDING                         0x8A3F
#define GL3_UNIFORM_BLOCK_DATA_SIZE                       0x8A40
#define GL3_UNIFORM_BLOCK_NAME_LENGTH                     0x8A41
#define GL3_UNIFORM_BLOCK_ACTIVE_UNIFORMS                 0x8A42
#define GL3_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES          0x8A43
#define GL3_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER     0x8A44
#define GL3_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER   0x8A46
#define GL3_INVALID_INDEX                                 0xFFFFFFFFu
//#define GL3_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
//#define GL3_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
//#define GL3_MAX_SERVER_WAIT_TIMEOUT                       0x9111
//#define GL3_OBJECT_TYPE                                   0x9112
//#define GL3_SYNC_CONDITION                                0x9113
//#define GL3_SYNC_STATUS                                   0x9114
//#define GL3_SYNC_FLAGS                                    0x9115
//#define GL3_SYNC_FENCE                                    0x9116
//#define GL3_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
//#define GL3_UNSIGNALED                                    0x9118
//#define GL3_SIGNALED                                      0x9119
//#define GL3_ALREADY_SIGNALED                              0x911A
//#define GL3_TIMEOUT_EXPIRED                               0x911B
//#define GL3_CONDITION_SATISFIED                           0x911C
//#define GL3_WAIT_FAILED                                   0x911D
//#define GL3_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
//#define GL3_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFFull
//#define GL3_VERTEX_ATTRIB_ARRAY_DIVISOR                   0x88FE
//#define GL3_ANY_SAMPLES_PASSED                            0x8C2F
//#define GL3_ANY_SAMPLES_PASSED_CONSERVATIVE               0x8D6A
//#define GL3_SAMPLER_BINDING                               0x8919
//#define GL3_RGB10_A2UI                                    0x906F
//#define GL3_TEXTURE_SWIZZLE_R                             0x8E42
//#define GL3_TEXTURE_SWIZZLE_G                             0x8E43
//#define GL3_TEXTURE_SWIZZLE_B                             0x8E44
//#define GL3_TEXTURE_SWIZZLE_A                             0x8E45
//#define GL3_GREEN                                         0x1904
//#define GL3_BLUE                                          0x1905
//#define GL3_TRANSFORM_FEEDBACK                            0x8E22
//#define GL3_TRANSFORM_FEEDBACK_PAUSED                     0x8E23
//#define GL3_TRANSFORM_FEEDBACK_ACTIVE                     0x8E24
//#define GL3_TRANSFORM_FEEDBACK_BINDING                    0x8E25
//#define GL3_PROGRAM_BINARY_RETRIEVABLE_HINT               0x8257
//#define GL3_PROGRAM_BINARY_LENGTH                         0x8741
//#define GL3_NUM_PROGRAM_BINARY_FORMATS                    0x87FE
//#define GL3_PROGRAM_BINARY_FORMATS                        0x87FF
//#define GL3_COMPRESSED_R11_EAC                            0x9270
//#define GL3_COMPRESSED_SIGNED_R11_EAC                     0x9271
//#define GL3_COMPRESSED_RG11_EAC                           0x9272
//#define GL3_COMPRESSED_SIGNED_RG11_EAC                    0x9273
//#define GL3_COMPRESSED_RGB8_ETC2                          0x9274
//#define GL3_COMPRESSED_SRGB8_ETC2                         0x9275
//#define GL3_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
//#define GL3_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
//#define GL3_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
//#define GL3_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
//#define GL3_TEXTURE_IMMUTABLE_FORMAT                      0x912F
//#define GL3_MAX_ELEMENT_INDEX                             0x8D6B
//#define GL3_NUM_SAMPLE_COUNTS                             0x9380
//#define GL3_TEXTURE_IMMUTABLE_LEVELS                      0x82DF

/*-------------------------------------------------------------------------
 * Entrypoint definitions
 *-----------------------------------------------------------------------*/

/* 自行扩展的函数 */

void PI_API gl_Self_Init(RenderContextLoadType type, void *context);

RenderInterfaceType PI_API gl_Self_GetInterfaceType(void);

uint gl_Self_GetVersion(PiBool *is_es);
PiBool PI_API gl_Self_IsSupportExtension(const char *externsion);

PiBool PI_API gl_Self_IsLostContext(void);

PiBool PI_API gl_Self_IsGLES(void);

PiBool PI_API gl_Self_IsVersion3(void);

PiBool PI_API gl_Self_IsGpuShader4(void);

PiBool PI_API gl_Self_IsCopyImage(void);

PiBool PI_API gl_Self_IsFramebufferSRGB(void);
PiBool PI_API gl_Self_IsHalfFloatPixel(void);
PiBool PI_API gl_Self_IsPackedFloat(void);
PiBool PI_API gl_Self_IsPackedDepthStencil(void);

PiBool PI_API gl_Self_IsDrawBuffers(void);

PiBool PI_API gl_Self_IsDrawInstance(void);
PiBool PI_API gl_Self_IsTexture3D(void);
PiBool PI_API gl_Self_IsTextureLOD(void);
PiBool PI_API gl_Self_IsFramebufferBlit(void);
PiBool PI_API gl_Self_IsTextureArray(void);
PiBool PI_API gl_Self_IsTextureRG(void);
PiBool PI_API gl_Self_IsTextureFloat(void);
PiBool PI_API gl_Self_IsTextureInteger(void);
PiBool PI_API gl_Self_IsTextureSRGB(void);
PiBool PI_API gl_Self_IsTextureSnorm(void);
PiBool PI_API gl_Self_IsTextureCompressionLatc(void);
PiBool PI_API gl_Self_IsTextureCompressionS3tc(void);
PiBool PI_API gl_Self_IsTextureCompressionRgtc(void);
PiBool PI_API gl_Self_IsTextureCompressionBptc(void);
PiBool PI_API gl_Self_IsTextureFilterAnisotropic(void);
PiBool PI_API gl_Self_IsVertexType_2_10_10_10_rev(void);

PiBool PI_API gl_Self_IsVertexArrayObject(void);

PiBool PI_API gl_Self_IsDepthClamp(void);


/************************************************ OpenGL 特有 ************************************************/

void PI_API gl_PolygonMode(unsigned int face, unsigned int mode);

void PI_API gl_GetTexImage(unsigned int target, int level, unsigned int format, unsigned int type, void* pixels);

void PI_API gl_GetCompressedTexImage(unsigned int target, int level, void* img);

void PI_API gl_CopyImageSubData(unsigned int srcName, unsigned int srcTarget, int srcLevel, int srcX, int srcY, int srcZ, unsigned int dstName, unsigned int dstTarget, int dstLevel, int dstX, int dstY, int dstZ, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth);

void PI_API gl_FramebufferTexture3D(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, sint level, sint zoffset);

/************************************************ OpenGL ES 2.0 ************************************************/

void PI_API gl2_ActiveTexture (unsigned int texture);
void PI_API gl2_AttachShader (unsigned int program, unsigned int shader);
void PI_API gl2_BindAttribLocation (unsigned int program, unsigned int index, const char* name);
void PI_API gl2_BindBuffer (unsigned int target, unsigned int buffer);
void PI_API gl2_BindFramebuffer (unsigned int target, unsigned int framebuffer);
void PI_API gl2_BindRenderbuffer (unsigned int target, unsigned int renderbuffer);
void PI_API gl2_BindTexture (unsigned int target, unsigned int texture);
void PI_API gl2_BlendColor (float red, float green, float blue, float alpha);
void PI_API gl2_BlendEquation (unsigned int mode);
void PI_API gl2_BlendEquationSeparate (unsigned int modeRGB, unsigned int modeAlpha);
void PI_API gl2_BlendFunc (unsigned int sfactor, unsigned int dfactor);
void PI_API gl2_BlendFuncSeparate (unsigned int srcRGB, unsigned int dstRGB, unsigned int srcAlpha, unsigned int dstAlpha);
void PI_API gl2_BufferData (unsigned int target, unsigned int size, const void* data, unsigned int usage);
void PI_API gl2_BufferSubData (unsigned int target, int offset, unsigned int size, const void* data);
unsigned int  PI_API gl2_CheckFramebufferStatus (unsigned int target);
void PI_API gl2_Clear (unsigned int mask);
void PI_API gl2_ClearColor (float red, float green, float blue, float alpha);
void PI_API gl2_ClearDepthf (float depth);
void PI_API gl2_ClearStencil (int s);
void PI_API gl2_ColorMask (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void PI_API gl2_CompileShader (unsigned int shader);
void PI_API gl2_CompressedTexImage2D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, int border, unsigned int imageSize, const void* data);
void PI_API gl2_CompressedTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int imageSize, const void* data);
void PI_API gl2_CopyTexImage2D (unsigned int target, int level, unsigned int internalformat, int x, int y, unsigned int width, unsigned int height, int border);
void PI_API gl2_CopyTexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, int x, int y, unsigned int width, unsigned int height);
unsigned int  PI_API gl2_CreateProgram (void);
unsigned int  PI_API gl2_CreateShader (unsigned int type);
void PI_API gl2_CullFace (unsigned int mode);
void PI_API gl2_DeleteBuffers (unsigned int n, const unsigned int* buffers);
void PI_API gl2_DeleteFramebuffers (unsigned int n, const unsigned int* framebuffers);
void PI_API gl2_DeleteProgram (unsigned int program);
void PI_API gl2_DeleteRenderbuffers (unsigned int n, const unsigned int* renderbuffers);
void PI_API gl2_DeleteShader (unsigned int shader);
void PI_API gl2_DeleteTextures (unsigned int n, const unsigned int* textures);
void PI_API gl2_DepthFunc (unsigned int func);
void PI_API gl2_DepthMask (unsigned char flag);
void PI_API gl2_DepthRangef (float n, float f);
void PI_API gl2_DetachShader (unsigned int program, unsigned int shader);
void PI_API gl2_Disable (unsigned int cap);
void PI_API gl2_DisableVertexAttribArray (unsigned int index);
void PI_API gl2_DrawArrays (unsigned int mode, int first, unsigned int count);
void PI_API gl2_DrawElements (unsigned int mode, unsigned int count, unsigned int type, const void* indices);
void PI_API gl2_Enable (unsigned int cap);
void PI_API gl2_EnableVertexAttribArray (unsigned int index);
void PI_API gl2_Finish (void);
void PI_API gl2_Flush (void);
void PI_API gl2_FramebufferRenderbuffer (unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer);
void PI_API gl2_FramebufferTexture2D (unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
void PI_API gl2_FrontFace (unsigned int mode);
void PI_API gl2_GenBuffers (unsigned int n, unsigned int* buffers);
void PI_API gl2_GenerateMipmap (unsigned int target);
void PI_API gl2_GenFramebuffers (unsigned int n, unsigned int* framebuffers);
void PI_API gl2_GenRenderbuffers (unsigned int n, unsigned int* renderbuffers);
void PI_API gl2_GenTextures (unsigned int n, unsigned int* textures);
void PI_API gl2_GetActiveAttrib (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name);
void PI_API gl2_GetActiveUniform (unsigned int program, unsigned int index, unsigned int bufsize, unsigned int* length, int* size, unsigned int* type, char* name);
void PI_API gl2_GetAttachedShaders (unsigned int program, unsigned int maxcount, unsigned int* count, unsigned int* shaders);
int   PI_API gl2_GetAttribLocation (unsigned int program, const char* name);
void PI_API gl2_GetBooleanv (unsigned int pname, unsigned char* params);
void PI_API gl2_GetBufferParameteriv (unsigned int target, unsigned int pname, int* params);
unsigned int  PI_API gl2_GetError (void);
void PI_API gl2_GetFloatv (unsigned int pname, float* params);
void PI_API gl2_GetFramebufferAttachmentParameteriv (unsigned int target, unsigned int attachment, unsigned int pname, int* params);
void PI_API gl2_GetIntegerv (unsigned int pname, int* params);
void PI_API gl2_GetProgramiv (unsigned int program, unsigned int pname, int* params);
void PI_API gl2_GetProgramInfoLog (unsigned int program, unsigned int bufsize, unsigned int* length, char* infolog);
void PI_API gl2_GetRenderbufferParameteriv (unsigned int target, unsigned int pname, int* params);
void PI_API gl2_GetShaderiv (unsigned int shader, unsigned int pname, int* params);
void PI_API gl2_GetShaderInfoLog (unsigned int shader, unsigned int bufsize, unsigned int* length, char* infolog);
void PI_API gl2_GetShaderPrecisionFormat (unsigned int shadertype, unsigned int precisiontype, int* range, int* precision);
void PI_API gl2_GetShaderSource (unsigned int shader, unsigned int bufsize, unsigned int* length, char* source);
const unsigned char* PI_API gl2_GetString (unsigned int name);
void PI_API gl2_GetTexParameterfv (unsigned int target, unsigned int pname, float* params);
void PI_API gl2_GetTexParameteriv (unsigned int target, unsigned int pname, int* params);
void PI_API gl2_GetUniformfv (unsigned int program, int location, float* params);
void PI_API gl2_GetUniformiv (unsigned int program, int location, int* params);
int   PI_API gl2_GetUniformLocation (unsigned int program, const char* name);
void PI_API gl2_GetVertexAttribfv (unsigned int index, unsigned int pname, float* params);
void PI_API gl2_GetVertexAttribiv (unsigned int index, unsigned int pname, int* params);
void PI_API gl2_GetVertexAttribPointerv (unsigned int index, unsigned int pname, void** pointer);
void PI_API gl2_Hint (unsigned int target, unsigned int mode);
unsigned char  PI_API gl2_IsBuffer (unsigned int buffer);
unsigned char  PI_API gl2_IsEnabled (unsigned int cap);
unsigned char  PI_API gl2_IsFramebuffer (unsigned int framebuffer);
unsigned char  PI_API gl2_IsProgram (unsigned int program);
unsigned char  PI_API gl2_IsRenderbuffer (unsigned int renderbuffer);
unsigned char  PI_API gl2_IsShader (unsigned int shader);
unsigned char  PI_API gl2_IsTexture (unsigned int texture);
void PI_API gl2_LineWidth (float width);
void PI_API gl2_LinkProgram (unsigned int program);
void PI_API gl2_PixelStorei (unsigned int pname, int param);
void PI_API gl2_PolygonOffset (float factor, float units);
void PI_API gl2_ReadPixels (int x, int y, unsigned int width, unsigned int height, unsigned int format, unsigned int type, void* pixels);
void PI_API gl2_ReleaseShaderCompiler (void);
void PI_API gl2_RenderbufferStorage (unsigned int target, unsigned int internalformat, unsigned int width, unsigned int height);
void PI_API gl2_SampleCoverage (float value, unsigned char invert);
void PI_API gl2_Scissor (int x, int y, unsigned int width, unsigned int height);
void PI_API gl2_ShaderBinary (unsigned int n, const unsigned int* shaders, unsigned int binaryformat, const void* binary, unsigned int length);
void PI_API gl2_ShaderSource (unsigned int shader, unsigned int count, const char* const* string, const int* length);
void PI_API gl2_StencilFunc (unsigned int func, int ref, unsigned int mask);
void PI_API gl2_StencilFuncSeparate (unsigned int face, unsigned int func, int ref, unsigned int mask);
void PI_API gl2_StencilMask (unsigned int mask);
void PI_API gl2_StencilMaskSeparate (unsigned int face, unsigned int mask);
void PI_API gl2_StencilOp (unsigned int fail, unsigned int zfail, unsigned int zpass);
void PI_API gl2_StencilOpSeparate (unsigned int face, unsigned int fail, unsigned int zfail, unsigned int zpass);
void PI_API gl2_TexImage2D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, int border, unsigned int format, unsigned int type, const void* pixels);
void PI_API gl2_TexParameterf (unsigned int target, unsigned int pname, float param);
void PI_API gl2_TexParameterfv (unsigned int target, unsigned int pname, const float* params);
void PI_API gl2_TexParameteri (unsigned int target, unsigned int pname, int param);
void PI_API gl2_TexParameteriv (unsigned int target, unsigned int pname, const int* params);
void PI_API gl2_TexSubImage2D (unsigned int target, int level, int xoffset, int yoffset, unsigned int width, unsigned int height, unsigned int format, unsigned int type, const void* pixels);
void PI_API gl2_Uniform1f (int location, float x);
void PI_API gl2_Uniform1fv (int location, unsigned int count, const float* v);
void PI_API gl2_Uniform1i (int location, int x);
void PI_API gl2_Uniform1iv (int location, unsigned int count, const int* v);
void PI_API gl2_Uniform2f (int location, float x, float y);
void PI_API gl2_Uniform2fv (int location, unsigned int count, const float* v);
void PI_API gl2_Uniform2i (int location, int x, int y);
void PI_API gl2_Uniform2iv (int location, unsigned int count, const int* v);
void PI_API gl2_Uniform3f (int location, float x, float y, float z);
void PI_API gl2_Uniform3fv (int location, unsigned int count, const float* v);
void PI_API gl2_Uniform3i (int location, int x, int y, int z);
void PI_API gl2_Uniform3iv (int location, unsigned int count, const int* v);
void PI_API gl2_Uniform4f (int location, float x, float y, float z, float w);
void PI_API gl2_Uniform4fv (int location, unsigned int count, const float* v);
void PI_API gl2_Uniform4i (int location, int x, int y, int z, int w);
void PI_API gl2_Uniform4iv (int location, unsigned int count, const int* v);
void PI_API gl2_UniformMatrix2fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl2_UniformMatrix3fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl2_UniformMatrix4fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl2_UseProgram (unsigned int program);
void PI_API gl2_ValidateProgram (unsigned int program);
void PI_API gl2_VertexAttrib1f (unsigned int indx, float x);
void PI_API gl2_VertexAttrib1fv (unsigned int indx, const float* values);
void PI_API gl2_VertexAttrib2f (unsigned int indx, float x, float y);
void PI_API gl2_VertexAttrib2fv (unsigned int indx, const float* values);
void PI_API gl2_VertexAttrib3f (unsigned int indx, float x, float y, float z);
void PI_API gl2_VertexAttrib3fv (unsigned int indx, const float* values);
void PI_API gl2_VertexAttrib4f (unsigned int indx, float x, float y, float z, float w);
void PI_API gl2_VertexAttrib4fv (unsigned int indx, const float* values);
void PI_API gl2_VertexAttribPointer (unsigned int indx, int size, unsigned int type, unsigned char normalized, unsigned int stride, const void* ptr);
void PI_API gl2_Viewport (int x, int y, unsigned int width, unsigned int height);

/************************************************ OpenGL ES 3.0 ************************************************/

/* -------------- 目前用到的函数 --------------  */

/* VAO: WebGL支持 */
void PI_API gl3_GenVertexArrays (unsigned int n, unsigned int* arrays);
void PI_API gl3_DeleteVertexArrays (unsigned int n, const unsigned int* arrays);
unsigned char  PI_API gl3_IsVertexArray (unsigned int array);
void PI_API gl3_BindVertexArray (unsigned int array);

/* FBO：WebGL支持 */
void PI_API gl3_DrawBuffers (unsigned int n, const unsigned int* bufs);
void PI_API gl3_BlitFramebuffer (int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);

/*WebGL不支持 */
void PI_API gl3_FramebufferTextureLayer (unsigned int target, unsigned int attachment, unsigned int texture, int level, int layer);

/* 3D Texture: WebGL不支持 */
void PI_API gl3_TexImage3D (unsigned int target, int level, int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int format, unsigned int type, const void* pixels);
void PI_API gl3_TexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int type, const void* pixels);
void PI_API gl3_CopyTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, int x, int y, unsigned int width, unsigned int height);
void PI_API gl3_CompressedTexImage3D (unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth, int border, unsigned int imageSize, const void* data);
void PI_API gl3_CompressedTexSubImage3D (unsigned int target, int level, int xoffset, int yoffset, int zoffset, unsigned int width, unsigned int height, unsigned int depth, unsigned int format, unsigned int imageSize, const void* data);

/* 实例化 */
void PI_API gl3_DrawArraysInstanced(unsigned int mode, int first, unsigned int count, unsigned int instanceCount);
void PI_API gl3_DrawElementsInstanced(unsigned int mode, unsigned int count, unsigned int type, const void* indices, unsigned int instanceCount);

/* -------------- 没有用到的函数 --------------  */

void PI_API gl3_ReadBuffer (unsigned int mode);
void PI_API gl3_DrawRangeElements (unsigned int mode, unsigned int start, unsigned int end, unsigned int count, unsigned int type, const void* indices);
void PI_API gl3_GenQueries (unsigned int n, unsigned int* ids);
void PI_API gl3_DeleteQueries (unsigned int n, const unsigned int* ids);
unsigned char  PI_API gl3_IsQuery (unsigned int id);
void PI_API gl3_BeginQuery (unsigned int target, unsigned int id);
void PI_API gl3_EndQuery (unsigned int target);
void PI_API gl3_GetQueryiv (unsigned int target, unsigned int pname, int* params);
void PI_API gl3_GetQueryObjectuiv (unsigned int id, unsigned int pname, unsigned int* params);
unsigned char  PI_API gl3_UnmapBuffer (unsigned int target);
void PI_API gl3_GetBufferPointerv (unsigned int target, unsigned int pname, void** params);
void PI_API gl3_UniformMatrix2x3fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_UniformMatrix3x2fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_UniformMatrix2x4fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_UniformMatrix4x2fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_UniformMatrix3x4fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_UniformMatrix4x3fv (int location, unsigned int count, unsigned char transpose, const float* value);
void PI_API gl3_RenderbufferStorageMultisample (unsigned int target, unsigned int samples, unsigned int internalformat, unsigned int width, unsigned int height);
void* PI_API gl3_MapBufferRange (unsigned int target, int offset, unsigned int length, unsigned int access);
void PI_API gl3_FlushMappedBufferRange (unsigned int target, int offset, unsigned int length);
void PI_API gl3_GetIntegeri_v (unsigned int target, unsigned int index, int* data);
void PI_API gl3_BeginTransformFeedback (unsigned int primitiveMode);
void PI_API gl3_EndTransformFeedback (void);
void PI_API gl3_BindBufferRange (unsigned int target, unsigned int index, unsigned int buffer, int offset, unsigned int size);
void PI_API gl3_BindBufferBase (unsigned int target, unsigned int index, unsigned int buffer);
void PI_API gl3_TransformFeedbackVaryings (unsigned int program, unsigned int count, const char* const* varyings, unsigned int bufferMode);
void PI_API gl3_GetTransformFeedbackVarying (unsigned int program, unsigned int index, unsigned int bufSize, unsigned int* length, unsigned int* size, unsigned int* type, char* name);
void PI_API gl3_VertexAttribIPointer (unsigned int index, int size, unsigned int type, unsigned int stride, const void* pointer);
void PI_API gl3_GetVertexAttribIiv (unsigned int index, unsigned int pname, int* params);
void PI_API gl3_GetVertexAttribIuiv (unsigned int index, unsigned int pname, unsigned int* params);
void PI_API gl3_VertexAttribI4i (unsigned int index, int x, int y, int z, int w);
void PI_API gl3_VertexAttribI4ui (unsigned int index, unsigned int x, unsigned int y, unsigned int z, unsigned int w);
void PI_API gl3_VertexAttribI4iv (unsigned int index, const int* v);
void PI_API gl3_VertexAttribI4uiv (unsigned int index, const unsigned int* v);
void PI_API gl3_GetUniformuiv (unsigned int program, int location, unsigned int* params);
int   PI_API gl3_GetFragDataLocation (unsigned int program, const char *name);
void PI_API gl3_Uniform1ui (int location, unsigned int v0);
void PI_API gl3_Uniform2ui (int location, unsigned int v0, unsigned int v1);
void PI_API gl3_Uniform3ui (int location, unsigned int v0, unsigned int v1, unsigned int v2);
void PI_API gl3_Uniform4ui (int location, unsigned int v0, unsigned int v1, unsigned int v2, unsigned int v3);
void PI_API gl3_Uniform1uiv (int location, unsigned int count, const unsigned int* value);
void PI_API gl3_Uniform2uiv (int location, unsigned int count, const unsigned int* value);
void PI_API gl3_Uniform3uiv (int location, unsigned int count, const unsigned int* value);
void PI_API gl3_Uniform4uiv (int location, unsigned int count, const unsigned int* value);
void PI_API gl3_ClearBufferiv (unsigned int buffer, int drawbuffer, const int* value);
void PI_API gl3_ClearBufferuiv (unsigned int buffer, int drawbuffer, const unsigned int* value);
void PI_API gl3_ClearBufferfv (unsigned int buffer, int drawbuffer, const float* value);
void PI_API gl3_ClearBufferfi (unsigned int buffer, int drawbuffer, float depth, int stencil);
const unsigned char* PI_API gl3_GetStringi (unsigned int name, unsigned int index);
void PI_API gl3_CopyBufferSubData (unsigned int readTarget, unsigned int writeTarget, int readOffset, int writeOffset, unsigned int size);
void PI_API gl3_GetUniformIndices (unsigned int program, unsigned int uniformCount, const char* const* uniformNames, unsigned int* uniformIndices);
void PI_API gl3_GetActiveUniformsiv (unsigned int program, unsigned int uniformCount, const unsigned int* uniformIndices, unsigned int pname, int* params);
unsigned int  PI_API gl3_GetUniformBlockIndex (unsigned int program, const char* uniformBlockName);
void PI_API gl3_GetActiveUniformBlockiv (unsigned int program, unsigned int uniformBlockIndex, unsigned int pname, int* params);
void PI_API gl3_GetActiveUniformBlockName (unsigned int program, unsigned int uniformBlockIndex, unsigned int bufSize, unsigned int* length, char* uniformBlockName);
void PI_API gl3_UniformBlockBinding (unsigned int program, unsigned int uniformBlockIndex, unsigned int uniformBlockBinding);
void*  PI_API gl3_FenceSync (unsigned int condition, unsigned int flags);
unsigned char  PI_API gl3_IsSync (void* sync);
void PI_API gl3_DeleteSync (void* sync);
unsigned int  PI_API gl3_ClientWaitSync (void* sync, unsigned int flags, unsigned long long timeout);
void PI_API gl3_WaitSync (void* sync, unsigned int flags, unsigned long long timeout);
void PI_API gl3_GetInteger64v (unsigned int pname, long long* params);
void PI_API gl3_GetSynciv (void* sync, unsigned int pname, unsigned int bufSize, unsigned int* length, int* values);
void PI_API gl3_GetInteger64i_v (unsigned int target, unsigned int index, long long* data);
void PI_API gl3_GetBufferParameteri64v (unsigned int target, unsigned int pname, long long* params);
void PI_API gl3_GenSamplers (unsigned int count, unsigned int* samplers);
void PI_API gl3_DeleteSamplers (unsigned int count, const unsigned int* samplers);
unsigned char  PI_API gl3_IsSampler (unsigned int sampler);
void PI_API gl3_BindSampler (unsigned int unit, unsigned int sampler);
void PI_API gl3_SamplerParameteri (unsigned int sampler, unsigned int pname, int param);
void PI_API gl3_SamplerParameteriv (unsigned int sampler, unsigned int pname, const int* param);
void PI_API gl3_SamplerParameterf (unsigned int sampler, unsigned int pname, float param);
void PI_API gl3_SamplerParameterfv (unsigned int sampler, unsigned int pname, const float* param);
void PI_API gl3_GetSamplerParameteriv (unsigned int sampler, unsigned int pname, int* params);
void PI_API gl3_GetSamplerParameterfv (unsigned int sampler, unsigned int pname, float* params);
void PI_API gl3_VertexAttribDivisor (unsigned int index, unsigned int divisor);
void PI_API gl3_BindTransformFeedback (unsigned int target, unsigned int id);
void PI_API gl3_DeleteTransformFeedbacks (unsigned int n, const unsigned int* ids);
void PI_API gl3_GenTransformFeedbacks (unsigned int n, unsigned int* ids);
unsigned char  PI_API gl3_IsTransformFeedback (unsigned int id);
void PI_API gl3_PauseTransformFeedback (void);
void PI_API gl3_ResumeTransformFeedback (void);
void PI_API gl3_GetProgramBinary (unsigned int program, unsigned int bufSize, unsigned int* length, unsigned int* binaryFormat, void* binary);
void PI_API gl3_ProgramBinary (unsigned int program, unsigned int binaryFormat, const void* binary, unsigned int length);
void PI_API gl3_ProgramParameteri (unsigned int program, unsigned int pname, int value);
void PI_API gl3_InvalidateFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments);
void PI_API gl3_InvalidateSubFramebuffer (unsigned int target, unsigned int numAttachments, const unsigned int* attachments, int x, int y, unsigned int width, unsigned int height);
void PI_API gl3_TexStorage2D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height);
void PI_API gl3_TexStorage3D (unsigned int target, unsigned int levels, unsigned int internalformat, unsigned int width, unsigned int height, unsigned int depth);
void PI_API gl3_GetInternalformativ (unsigned int target, unsigned int internalformat, unsigned int pname, unsigned int bufSize, int* params);

#endif /* INCLUDE_GL_INTERFACE_H */
