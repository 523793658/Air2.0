
#include "native_interface.h"

#include <gl_interface.h>
#include <glloader/glloader.h>

static void PI_API _glDepthRangef(float n, float f)
{
	glDepthRange(n, f);
}

static void PI_API _glClearDepthf(float depth)
{
	glClearDepth(depth);
}

static void _register_gl_func(GLCap *cap, GLFunc *gl_context)
{
	if (cap->_isFBO)
	{
		gl_context->_FramebufferTexture3D = (GL_FramebufferTexture3D)glFramebufferTexture3D;
		gl_context->_BindFramebuffer = (GL2_BindFramebuffer)glBindFramebuffer;
		gl_context->_BindRenderbuffer = (GL2_BindRenderbuffer)glBindRenderbuffer;
		gl_context->_CheckFramebufferStatus = (GL2_CheckFramebufferStatus)glCheckFramebufferStatus;
		gl_context->_DeleteFramebuffers = (GL2_DeleteFramebuffers)glDeleteFramebuffers;
		gl_context->_DeleteRenderbuffers = (GL2_DeleteRenderbuffers)glDeleteRenderbuffers;
		gl_context->_FramebufferRenderbuffer = (GL2_FramebufferRenderbuffer)glFramebufferRenderbuffer;
		gl_context->_FramebufferTexture2D = (GL2_FramebufferTexture2D)glFramebufferTexture2D;
		gl_context->_GenerateMipmap = (GL2_GenerateMipmap)glGenerateMipmap;
		gl_context->_GenFramebuffers = (GL2_GenFramebuffers)glGenFramebuffers;
		gl_context->_GenRenderbuffers = (GL2_GenRenderbuffers)glGenRenderbuffers;
		gl_context->_GetFramebufferAttachmentParameteriv = (GL2_GetFramebufferAttachmentParameteriv)glGetFramebufferAttachmentParameteriv;
		gl_context->_GetRenderbufferParameteriv = (GL2_GetRenderbufferParameteriv)glGetRenderbufferParameteriv;
		gl_context->_IsFramebuffer = (GL2_IsFramebuffer)glIsFramebuffer;
		gl_context->_IsRenderbuffer = (GL2_IsRenderbuffer)glIsRenderbuffer;
		gl_context->_RenderbufferStorage = (GL2_RenderbufferStorage)glRenderbufferStorage;
		gl_context->_FramebufferTextureLayer = (GL3_FramebufferTextureLayer)glFramebufferTextureLayer;
	}
	else
	{
		cap->_isFBO = glloader_GL_EXT_framebuffer_object();
		if (cap->_isFBO) 
		{
			gl_context->_FramebufferTexture3D = (GL_FramebufferTexture3D)glFramebufferTexture3DEXT;
			gl_context->_BindFramebuffer = (GL2_BindFramebuffer)glBindFramebufferEXT;
			gl_context->_BindRenderbuffer = (GL2_BindRenderbuffer)glBindRenderbufferEXT;
			gl_context->_CheckFramebufferStatus = (GL2_CheckFramebufferStatus)glCheckFramebufferStatusEXT;
			gl_context->_DeleteFramebuffers = (GL2_DeleteFramebuffers)glDeleteFramebuffersEXT;
			gl_context->_DeleteRenderbuffers = (GL2_DeleteRenderbuffers)glDeleteRenderbuffersEXT;
			gl_context->_FramebufferRenderbuffer = (GL2_FramebufferRenderbuffer)glFramebufferRenderbufferEXT;
			gl_context->_FramebufferTexture2D = (GL2_FramebufferTexture2D)glFramebufferTexture2DEXT;
			gl_context->_GenerateMipmap = (GL2_GenerateMipmap)glGenerateMipmapEXT;
			gl_context->_GenFramebuffers = (GL2_GenFramebuffers)glGenFramebuffersEXT;
			gl_context->_GenRenderbuffers = (GL2_GenRenderbuffers)glGenRenderbuffersEXT;
			gl_context->_GetFramebufferAttachmentParameteriv = (GL2_GetFramebufferAttachmentParameteriv)glGetFramebufferAttachmentParameterivEXT;
			gl_context->_GetRenderbufferParameteriv = (GL2_GetRenderbufferParameteriv)glGetRenderbufferParameterivEXT;
			gl_context->_IsFramebuffer = (GL2_IsFramebuffer)glIsFramebufferEXT;
			gl_context->_IsRenderbuffer = (GL2_IsRenderbuffer)glIsRenderbufferEXT;
			gl_context->_RenderbufferStorage = (GL2_RenderbufferStorage)glRenderbufferStorageEXT;
			gl_context->_FramebufferTextureLayer = (GL3_FramebufferTextureLayer)glFramebufferTextureLayerEXT;
		}		
	}

	gl_context->_PolygonMode = (GL_PolygonMode)glPolygonMode;
	gl_context->_GetTexImage = (GL_GetTexImage)glGetTexImage;
	gl_context->_GetCompressedTexImage = (GL_GetCompressedTexImage)glGetCompressedTexImage;
	gl_context->_CopyImageSubData = (GL_CopyImageSubData)glCopyImageSubData;
	gl_context->_ActiveTexture = (GL2_ActiveTexture)glActiveTexture;
	gl_context->_AttachShader = (GL2_AttachShader)glAttachShader;
	gl_context->_BindAttribLocation = (GL2_BindAttribLocation)glBindAttribLocation;
	gl_context->_BindBuffer = (GL2_BindBuffer)glBindBuffer;
	gl_context->_BindTexture = (GL2_BindTexture)glBindTexture;
	gl_context->_BlendColor = (GL2_BlendColor)glBlendColor;
	gl_context->_BlendEquation = (GL2_BlendEquation)glBlendEquation;
	gl_context->_BlendEquationSeparate = (GL2_BlendEquationSeparate)glBlendEquationSeparate;
	gl_context->_BlendFunc = (GL2_BlendFunc)glBlendFunc;
	gl_context->_BlendFuncSeparate = (GL2_BlendFuncSeparate)glBlendFuncSeparate;
	gl_context->_BufferData = (GL2_BufferData)glBufferData;
	gl_context->_BufferSubData = (GL2_BufferSubData)glBufferSubData;
	gl_context->_Clear = (GL2_Clear)glClear;
	gl_context->_ClearColor = (GL2_ClearColor)glClearColor;
	gl_context->_ClearDepthf = (GL2_ClearDepthf)_glClearDepthf;
	gl_context->_ClearStencil = (GL2_ClearStencil)glClearStencil;
	gl_context->_ColorMask = (GL2_ColorMask)glColorMask;
	gl_context->_CompileShader = (GL2_CompileShader)glCompileShader;
	gl_context->_CompressedTexImage2D = (GL2_CompressedTexImage2D)glCompressedTexImage2D;
	gl_context->_CompressedTexSubImage2D = (GL2_CompressedTexSubImage2D)glCompressedTexSubImage2D;
	gl_context->_CopyTexImage2D = (GL2_CopyTexImage2D)glCopyTexImage2D;
	gl_context->_CopyTexSubImage2D = (GL2_CopyTexSubImage2D)glCopyTexSubImage2D;
	gl_context->_CreateProgram = (GL2_CreateProgram)glCreateProgram;
	gl_context->_CreateShader = (GL2_CreateShader)glCreateShader;
	gl_context->_CullFace = (GL2_CullFace)glCullFace;
	gl_context->_DeleteBuffers = (GL2_DeleteBuffers)glDeleteBuffers;
	gl_context->_DeleteProgram = (GL2_DeleteProgram)glDeleteProgram;
	gl_context->_DeleteShader = (GL2_DeleteShader)glDeleteShader;
	gl_context->_DeleteTextures = (GL2_DeleteTextures)glDeleteTextures;
	gl_context->_DepthFunc = (GL2_DepthFunc)glDepthFunc;
	gl_context->_DepthMask = (GL2_DepthMask)glDepthMask;
	gl_context->_DepthRangef = (GL2_DepthRangef)_glDepthRangef;
	gl_context->_DetachShader = (GL2_DetachShader)glDetachShader;
	gl_context->_Disable = (GL2_Disable)glDisable;
	gl_context->_DisableVertexAttribArray = (GL2_DisableVertexAttribArray)glDisableVertexAttribArray;
	gl_context->_DrawArrays = (GL2_DrawArrays)glDrawArrays;
	gl_context->_DrawElements = (GL2_DrawElements)glDrawElements;
	gl_context->_Enable = (GL2_Enable)glEnable;
	gl_context->_EnableVertexAttribArray = (GL2_EnableVertexAttribArray)glEnableVertexAttribArray;
	gl_context->_Finish = (GL2_Finish)glFinish;
	gl_context->_Flush = (GL2_Flush)glFlush;
	gl_context->_FrontFace = (GL2_FrontFace)glFrontFace;
	gl_context->_GenBuffers = (GL2_GenBuffers)glGenBuffers;
	gl_context->_GenTextures = (GL2_GenTextures)glGenTextures;
	gl_context->_GetActiveAttrib = (GL2_GetActiveAttrib)glGetActiveAttrib;
	gl_context->_GetActiveUniform = (GL2_GetActiveUniform)glGetActiveUniform;
	gl_context->_GetAttachedShaders = (GL2_GetAttachedShaders)glGetAttachedShaders;
	gl_context->_GetAttribLocation = (GL2_GetAttribLocation)glGetAttribLocation;
	gl_context->_GetBooleanv = (GL2_GetBooleanv)glGetBooleanv;
	gl_context->_GetBufferParameteriv = (GL2_GetBufferParameteriv)glGetBufferParameteriv;
	gl_context->_GetError = (GL2_GetError)glGetError;
	gl_context->_GetFloatv = (GL2_GetFloatv)glGetFloatv;
	gl_context->_GetIntegerv = (GL2_GetIntegerv)glGetIntegerv;
	gl_context->_GetProgramiv = (GL2_GetProgramiv)glGetProgramiv;
	gl_context->_GetProgramInfoLog = (GL2_GetProgramInfoLog)glGetProgramInfoLog;
	gl_context->_GetShaderiv = (GL2_GetShaderiv)glGetShaderiv;
	gl_context->_GetShaderInfoLog = (GL2_GetShaderInfoLog)glGetShaderInfoLog;
	gl_context->_GetShaderPrecisionFormat = (GL2_GetShaderPrecisionFormat)glGetShaderPrecisionFormat;
	gl_context->_GetShaderSource = (GL2_GetShaderSource)glGetShaderSource;
	gl_context->_GetString = (GL2_GetString)glGetString;
	gl_context->_GetTexParameterfv = (GL2_GetTexParameterfv)glGetTexParameterfv;
	gl_context->_GetTexParameteriv = (GL2_GetTexParameteriv)glGetTexParameteriv;
	gl_context->_GetUniformfv = (GL2_GetUniformfv)glGetUniformfv;
	gl_context->_GetUniformiv = (GL2_GetUniformiv)glGetUniformiv;
	gl_context->_GetUniformLocation = (GL2_GetUniformLocation)glGetUniformLocation;
	gl_context->_GetVertexAttribfv = (GL2_GetVertexAttribfv)glGetVertexAttribfv;
	gl_context->_GetVertexAttribiv = (GL2_GetVertexAttribiv)glGetVertexAttribiv;
	gl_context->_GetVertexAttribPointerv = (GL2_GetVertexAttribPointerv)glGetVertexAttribPointerv;
	gl_context->_Hint = (GL2_Hint)glHint;
	gl_context->_IsBuffer = (GL2_IsBuffer)glIsBuffer;
	gl_context->_IsEnabled = (GL2_IsEnabled)glIsEnabled;
	gl_context->_IsProgram = (GL2_IsProgram)glIsProgram;
	gl_context->_IsShader = (GL2_IsShader)glIsShader;
	gl_context->_IsTexture = (GL2_IsTexture)glIsTexture;
	gl_context->_LineWidth = (GL2_LineWidth)glLineWidth;
	gl_context->_LinkProgram = (GL2_LinkProgram)glLinkProgram;
	gl_context->_PixelStorei = (GL2_PixelStorei)glPixelStorei;
	gl_context->_PolygonOffset = (GL2_PolygonOffset)glPolygonOffset;
	gl_context->_ReadPixels = (GL2_ReadPixels)glReadPixels;
	gl_context->_ReleaseShaderCompiler = (GL2_ReleaseShaderCompiler)glReleaseShaderCompiler;
	gl_context->_SampleCoverage = (GL2_SampleCoverage)glSampleCoverage;
	gl_context->_Scissor = (GL2_Scissor)glScissor;
	gl_context->_ShaderBinary = (GL2_ShaderBinary)glShaderBinary;
	gl_context->_ShaderSource = (GL2_ShaderSource)glShaderSource;
	gl_context->_StencilFunc = (GL2_StencilFunc)glStencilFunc;
	gl_context->_StencilFuncSeparate = (GL2_StencilFuncSeparate)glStencilFuncSeparate;
	gl_context->_StencilMask = (GL2_StencilMask)glStencilMask;
	gl_context->_StencilMaskSeparate = (GL2_StencilMaskSeparate)glStencilMaskSeparate;
	gl_context->_StencilOp = (GL2_StencilOp)glStencilOp;
	gl_context->_StencilOpSeparate = (GL2_StencilOpSeparate)glStencilOpSeparate;
	gl_context->_TexImage2D = (GL2_TexImage2D)glTexImage2D;
	gl_context->_TexParameterf = (GL2_TexParameterf)glTexParameterf;
	gl_context->_TexParameterfv = (GL2_TexParameterfv)glTexParameterfv;
	gl_context->_TexParameteri = (GL2_TexParameteri)glTexParameteri;
	gl_context->_TexParameteriv = (GL2_TexParameteriv)glTexParameteriv;
	gl_context->_TexSubImage2D = (GL2_TexSubImage2D)glTexSubImage2D;
	gl_context->_Uniform1f = (GL2_Uniform1f)glUniform1f;
	gl_context->_Uniform1fv = (GL2_Uniform1fv)glUniform1fv;
	gl_context->_Uniform1i = (GL2_Uniform1i)glUniform1i;
	gl_context->_Uniform1iv = (GL2_Uniform1iv)glUniform1iv;
	gl_context->_Uniform2f = (GL2_Uniform2f)glUniform2f;
	gl_context->_Uniform2fv = (GL2_Uniform2fv)glUniform2fv;
	gl_context->_Uniform2i = (GL2_Uniform2i)glUniform2i;
	gl_context->_Uniform2iv = (GL2_Uniform2iv)glUniform2iv;
	gl_context->_Uniform3f = (GL2_Uniform3f)glUniform3f;
	gl_context->_Uniform3fv = (GL2_Uniform3fv)glUniform3fv;
	gl_context->_Uniform3i = (GL2_Uniform3i)glUniform3i;
	gl_context->_Uniform3iv = (GL2_Uniform3iv)glUniform3iv;
	gl_context->_Uniform4f = (GL2_Uniform4f)glUniform4f;
	gl_context->_Uniform4fv = (GL2_Uniform4fv)glUniform4fv;
	gl_context->_Uniform4i = (GL2_Uniform4i)glUniform4i;
	gl_context->_Uniform4iv = (GL2_Uniform4iv)glUniform4iv;
	gl_context->_UniformMatrix2fv = (GL2_UniformMatrix2fv)glUniformMatrix2fv;
	gl_context->_UniformMatrix3fv = (GL2_UniformMatrix3fv)glUniformMatrix3fv;
	gl_context->_UniformMatrix4fv = (GL2_UniformMatrix4fv)glUniformMatrix4fv;
	gl_context->_UseProgram = (GL2_UseProgram)glUseProgram;
	gl_context->_ValidateProgram = (GL2_ValidateProgram)glValidateProgram;
	gl_context->_VertexAttrib1f = (GL2_VertexAttrib1f)glVertexAttrib1f;
	gl_context->_VertexAttrib1fv = (GL2_VertexAttrib1fv)glVertexAttrib1fv;
	gl_context->_VertexAttrib2f = (GL2_VertexAttrib2f)glVertexAttrib2f;
	gl_context->_VertexAttrib2fv = (GL2_VertexAttrib2fv)glVertexAttrib2fv;
	gl_context->_VertexAttrib3f = (GL2_VertexAttrib3f)glVertexAttrib3f;
	gl_context->_VertexAttrib3fv = (GL2_VertexAttrib3fv)glVertexAttrib3fv;
	gl_context->_VertexAttrib4f = (GL2_VertexAttrib4f)glVertexAttrib4f;
	gl_context->_VertexAttrib4fv = (GL2_VertexAttrib4fv)glVertexAttrib4fv;
	gl_context->_VertexAttribPointer = (GL2_VertexAttribPointer)glVertexAttribPointer;
	gl_context->_Viewport = (GL2_Viewport)glViewport;
	gl_context->_ReadBuffer = (GL3_ReadBuffer)glReadBuffer;
	gl_context->_DrawRangeElements = (GL3_DrawRangeElements)glDrawRangeElements;
	gl_context->_TexImage3D = (GL3_TexImage3D)glTexImage3D;
	gl_context->_TexSubImage3D = (GL3_TexSubImage3D)glTexSubImage3D;
	gl_context->_CopyTexSubImage3D = (GL3_CopyTexSubImage3D)glCopyTexSubImage3D;
	gl_context->_CompressedTexImage3D = (GL3_CompressedTexImage3D)glCompressedTexImage3D;
	gl_context->_CompressedTexSubImage3D = (GL3_CompressedTexSubImage3D)glCompressedTexSubImage3D;
	gl_context->_GenQueries = (GL3_GenQueries)glGenQueries;
	gl_context->_DeleteQueries = (GL3_DeleteQueries)glDeleteQueries;
	gl_context->_IsQuery = (GL3_IsQuery)glIsQuery;
	gl_context->_BeginQuery = (GL3_BeginQuery)glBeginQuery;
	gl_context->_EndQuery = (GL3_EndQuery)glEndQuery;
	gl_context->_GetQueryiv = (GL3_GetQueryiv)glGetQueryiv;
	gl_context->_GetQueryObjectuiv = (GL3_GetQueryObjectuiv)glGetQueryObjectuiv;
	gl_context->_UnmapBuffer = (GL3_UnmapBuffer)glUnmapBuffer;
	gl_context->_GetBufferPointerv = (GL3_GetBufferPointerv)glGetBufferPointerv;
	gl_context->_DrawBuffers = (GL3_DrawBuffers)glDrawBuffers;
	gl_context->_UniformMatrix2x3fv = (GL3_UniformMatrix2x3fv)glUniformMatrix2x3fv;
	gl_context->_UniformMatrix3x2fv = (GL3_UniformMatrix3x2fv)glUniformMatrix3x2fv;
	gl_context->_UniformMatrix2x4fv = (GL3_UniformMatrix2x4fv)glUniformMatrix2x4fv;
	gl_context->_UniformMatrix4x2fv = (GL3_UniformMatrix4x2fv)glUniformMatrix4x2fv;
	gl_context->_UniformMatrix3x4fv = (GL3_UniformMatrix3x4fv)glUniformMatrix3x4fv;
	gl_context->_UniformMatrix4x3fv = (GL3_UniformMatrix4x3fv)glUniformMatrix4x3fv;
	gl_context->_BlitFramebuffer = (GL3_BlitFramebuffer)glBlitFramebuffer;
	gl_context->_RenderbufferStorageMultisample = (GL3_RenderbufferStorageMultisample)glRenderbufferStorageMultisample;
	gl_context->_MapBufferRange = (GL3_MapBufferRange)glMapBufferRange;
	gl_context->_FlushMappedBufferRange = (GL3_FlushMappedBufferRange)glFlushMappedBufferRange;
	gl_context->_BindVertexArray = (GL3_BindVertexArray)glBindVertexArray;
	gl_context->_DeleteVertexArrays = (GL3_DeleteVertexArrays)glDeleteVertexArrays;
	gl_context->_GenVertexArrays = (GL3_GenVertexArrays)glGenVertexArrays;
	gl_context->_IsVertexArray = (GL3_IsVertexArray)glIsVertexArray;
	gl_context->_GetIntegeri_v = (GL3_GetIntegeri_v)glGetIntegeri_v;
	gl_context->_BeginTransformFeedback = (GL3_BeginTransformFeedback)glBeginTransformFeedback;
	gl_context->_EndTransformFeedback = (GL3_EndTransformFeedback)glEndTransformFeedback;
	gl_context->_BindBufferRange = (GL3_BindBufferRange)glBindBufferRange;
	gl_context->_BindBufferBase = (GL3_BindBufferBase)glBindBufferBase;
	gl_context->_TransformFeedbackVaryings = (GL3_TransformFeedbackVaryings)glTransformFeedbackVaryings;
	gl_context->_GetTransformFeedbackVarying = (GL3_GetTransformFeedbackVarying)glGetTransformFeedbackVarying;
	gl_context->_VertexAttribIPointer = (GL3_VertexAttribIPointer)glVertexAttribIPointer;
	gl_context->_GetVertexAttribIiv = (GL3_GetVertexAttribIiv)glGetVertexAttribIiv;
	gl_context->_GetVertexAttribIuiv = (GL3_GetVertexAttribIuiv)glGetVertexAttribIuiv;
	gl_context->_VertexAttribI4i = (GL3_VertexAttribI4i)glVertexAttribI4i;
	gl_context->_VertexAttribI4ui = (GL3_VertexAttribI4ui)glVertexAttribI4ui;
	gl_context->_VertexAttribI4iv = (GL3_VertexAttribI4iv)glVertexAttribI4iv;
	gl_context->_VertexAttribI4uiv = (GL3_VertexAttribI4uiv)glVertexAttribI4uiv;
	gl_context->_GetUniformuiv = (GL3_GetUniformuiv)glGetUniformuiv;
	gl_context->_GetFragDataLocation = (GL3_GetFragDataLocation)glGetFragDataLocation;
	gl_context->_Uniform1ui = (GL3_Uniform1ui)glUniform1ui;
	gl_context->_Uniform2ui = (GL3_Uniform2ui)glUniform2ui;
	gl_context->_Uniform3ui = (GL3_Uniform3ui)glUniform3ui;
	gl_context->_Uniform4ui = (GL3_Uniform4ui)glUniform4ui;
	gl_context->_Uniform1uiv = (GL3_Uniform1uiv)glUniform1uiv;
	gl_context->_Uniform2uiv = (GL3_Uniform2uiv)glUniform2uiv;
	gl_context->_Uniform3uiv = (GL3_Uniform3uiv)glUniform3uiv;
	gl_context->_Uniform4uiv = (GL3_Uniform4uiv)glUniform4uiv;
	gl_context->_ClearBufferiv = (GL3_ClearBufferiv)glClearBufferiv;
	gl_context->_ClearBufferuiv = (GL3_ClearBufferuiv)glClearBufferuiv;
	gl_context->_ClearBufferfv = (GL3_ClearBufferfv)glClearBufferfv;
	gl_context->_ClearBufferfi = (GL3_ClearBufferfi)glClearBufferfi;
	gl_context->_GetStringi = (GL3_GetStringi)glGetStringi;
	gl_context->_CopyBufferSubData = (GL3_CopyBufferSubData)glCopyBufferSubData;
	gl_context->_GetUniformIndices = (GL3_GetUniformIndices)glGetUniformIndices;
	gl_context->_GetActiveUniformsiv = (GL3_GetActiveUniformsiv)glGetActiveUniformsiv;
	gl_context->_GetUniformBlockIndex = (GL3_GetUniformBlockIndex)glGetUniformBlockIndex;
	gl_context->_GetActiveUniformBlockiv = (GL3_GetActiveUniformBlockiv)glGetActiveUniformBlockiv;
	gl_context->_GetActiveUniformBlockName = (GL3_GetActiveUniformBlockName)glGetActiveUniformBlockName;
	gl_context->_UniformBlockBinding = (GL3_UniformBlockBinding)glUniformBlockBinding;
	gl_context->_DrawArraysInstanced = (GL3_DrawArraysInstanced)glDrawArraysInstanced;
	gl_context->_DrawElementsInstanced = (GL3_DrawElementsInstanced)glDrawElementsInstanced;
	gl_context->_FenceSync = (GL3_FenceSync)glFenceSync;
	gl_context->_IsSync = (GL3_IsSync)glIsSync;
	gl_context->_DeleteSync = (GL3_DeleteSync)glDeleteSync;
	gl_context->_ClientWaitSync = (GL3_ClientWaitSync)glClientWaitSync;
	gl_context->_WaitSync = (GL3_WaitSync)glWaitSync;
	gl_context->_GetInteger64v = (GL3_GetInteger64v)glGetInteger64v;
	gl_context->_GetSynciv = (GL3_GetSynciv)glGetSynciv;
	gl_context->_GetInteger64i_v = (GL3_GetInteger64i_v)glGetInteger64i_v;
	gl_context->_GetBufferParameteri64v = (GL3_GetBufferParameteri64v)glGetBufferParameteri64v;
	gl_context->_GenSamplers = (GL3_GenSamplers)glGenSamplers;
	gl_context->_DeleteSamplers = (GL3_DeleteSamplers)glDeleteSamplers;
	gl_context->_IsSampler = (GL3_IsSampler)glIsSampler;
	gl_context->_BindSampler = (GL3_BindSampler)glBindSampler;
	gl_context->_SamplerParameteri = (GL3_SamplerParameteri)glSamplerParameteri;
	gl_context->_SamplerParameteriv = (GL3_SamplerParameteriv)glSamplerParameteriv;
	gl_context->_SamplerParameterf = (GL3_SamplerParameterf)glSamplerParameterf;
	gl_context->_SamplerParameterfv = (GL3_SamplerParameterfv)glSamplerParameterfv;
	gl_context->_GetSamplerParameteriv = (GL3_GetSamplerParameteriv)glGetSamplerParameteriv;
	gl_context->_GetSamplerParameterfv = (GL3_GetSamplerParameterfv)glGetSamplerParameterfv;
	gl_context->_VertexAttribDivisor = (GL3_VertexAttribDivisor)glVertexAttribDivisor;
	gl_context->_BindTransformFeedback = (GL3_BindTransformFeedback)glBindTransformFeedback;
	gl_context->_DeleteTransformFeedbacks = (GL3_DeleteTransformFeedbacks)glDeleteTransformFeedbacks;
	gl_context->_GenTransformFeedbacks = (GL3_GenTransformFeedbacks)glGenTransformFeedbacks;
	gl_context->_IsTransformFeedback = (GL3_IsTransformFeedback)glIsTransformFeedback;
	gl_context->_PauseTransformFeedback = (GL3_PauseTransformFeedback)glPauseTransformFeedback;
	gl_context->_ResumeTransformFeedback = (GL3_ResumeTransformFeedback)glResumeTransformFeedback;
	gl_context->_GetProgramBinary = (GL3_GetProgramBinary)glGetProgramBinary;
	gl_context->_ProgramBinary = (GL3_ProgramBinary)glProgramBinary;
	gl_context->_ProgramParameteri = (GL3_ProgramParameteri)glProgramParameteri;
	gl_context->_InvalidateFramebuffer = (GL3_InvalidateFramebuffer)glInvalidateFramebuffer;
	gl_context->_InvalidateSubFramebuffer = (GL3_InvalidateSubFramebuffer)glInvalidateSubFramebuffer;
	gl_context->_TexStorage2D = (GL3_TexStorage2D)glTexStorage2D;
	gl_context->_TexStorage3D = (GL3_TexStorage3D)glTexStorage3D;
	gl_context->_GetInternalformativ = (GL3_GetInternalformativ)glGetInternalformativ;

}

static void _init_cap(GLCap *cap)
{
	cap->gl_version = gl_Self_GetVersion(&cap->_IsEsVersion);

	cap->_isFBO = (cap->gl_version >= 300) || glloader_GL_ARB_framebuffer_object();
	cap->_IsTextureLOD = (cap->gl_version >= 300);
	cap->_IsInstanced = (cap->gl_version >= 300) || glloader_GL_EXT_draw_instanced();
	cap->_IsTexture3D = (cap->gl_version >= 300) || glloader_GL_EXT_texture3D();
	cap->_IsGpuShader4 = (cap->gl_version >= 300) || glloader_GL_EXT_gpu_shader4();
	cap->_IsCopyImage = glloader_GL_ARB_copy_image();
	cap->_IsFramebufferSRGB = (cap->gl_version >= 300) || glloader_GL_EXT_framebuffer_sRGB();
	cap->_IsHalfFloatPixel = (cap->gl_version >= 300) || glloader_GL_ARB_half_float_pixel();
	cap->_IsPackedFloat = (cap->gl_version >= 300) || glloader_GL_EXT_packed_float();
	cap->_IsPackedDepthStencil = (cap->gl_version >= 300) || glloader_GL_EXT_packed_depth_stencil();
	cap->_IsFramebufferBlit = (cap->gl_version >= 300) || glloader_GL_EXT_framebuffer_blit();
	cap->_IsTextureArray = (cap->gl_version >= 300) || glloader_GL_EXT_texture_array();
	cap->_IsTextureRG = (cap->gl_version >= 300) || glloader_GL_ARB_texture_rg();
	cap->_IsDrawBuffers = (cap->gl_version >= 300) || glloader_GL_EXT_draw_buffers2();
	cap->_IsTextureFloat = (cap->gl_version >= 300) || glloader_GL_ARB_texture_float();
	cap->_IsTextureInteger = (cap->gl_version >= 300) || glloader_GL_EXT_texture_integer();
	cap->_IsTextureSRGB = (cap->gl_version >= 300) || glloader_GL_EXT_texture_sRGB();
	cap->_IsTextureSnorm = (cap->gl_version >= 300) || glloader_GL_EXT_texture_snorm();
	cap->_IsTextureCompressionLatc = (cap->gl_version >= 300) || glloader_GL_EXT_texture_compression_latc();
	cap->_IsTextureCompressionS3tc = (cap->gl_version >= 300) || glloader_GL_EXT_texture_compression_s3tc();
	cap->_IsTextureCompressionRgtc = (cap->gl_version >= 300) || glloader_GL_EXT_texture_compression_rgtc();
	cap->_IsTextureCompressionBptc = (cap->gl_version >= 300) || glloader_GL_ARB_texture_compression_bptc();
	cap->_IsTextureFilterAnisotropic = (cap->gl_version >= 300) || glloader_GL_EXT_texture_filter_anisotropic();
	cap->_IsVertexType_2_10_10_10_rev = (cap->gl_version >= 300) || glloader_GL_ARB_vertex_type_2_10_10_10_rev();
	cap->_IsVertexArrayObject = (cap->gl_version >= 300) || glloader_GL_ARB_vertex_array_object();

	cap->_IsDepthClamp = (cap->gl_version >= 300) || glloader_GL_ARB_depth_clamp();
}

void PI_API native_Self_Init(GLFunc *gl_context, GLCap *cap)
{
	glloader_init();
	
	gl_context->_GetString = (GL2_GetString)glGetString;
	_init_cap(cap);
	
	_register_gl_func(cap, gl_context);
}

PiBool PI_API native_Self_IsLostContext(void)
{
	return FALSE;
}