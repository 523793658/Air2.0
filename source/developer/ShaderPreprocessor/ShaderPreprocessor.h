#pragma once
#include "CoreMinimal.h"
#include "ShaderCore.h"
namespace Air
{
#ifdef _SHADER_PREPROCESSOR_
#define SHADERPREPROCESSOR_API DLLEXPORT
#else
#define SHADERPREPROCESSOR_API DLLIMPORT
#endif
	extern SHADERPREPROCESSOR_API bool preprocessShader(
		wstring& outPreprocessedShader,
		ShaderCompilerOutput& shaderOutput,
		const ShaderCompilerInput& shaderInput,
		const ShaderCompilerDefinitions& additionalDefines
	);
}