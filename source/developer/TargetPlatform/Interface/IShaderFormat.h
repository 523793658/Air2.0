#pragma once
#include "CoreMinimal.h"

#define SHADERFORMAT_MODULE_WILDCARD	TEXT("*ShaderFormat*")

namespace Air
{
	class IShaderFormat
	{
	public:
		virtual void compileShader(wstring format, const struct ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const wstring & workingDirectory) const = 0;

		virtual uint16 getVersion(wstring format) const = 0;

		virtual void getSupportedFormats(TArray<wstring>& outFormats) const = 0;
	public:
		virtual ~IShaderFormat() {}
	};
}
