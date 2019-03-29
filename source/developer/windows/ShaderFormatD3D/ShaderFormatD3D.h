#pragma once
#include "Windows/WindowsHWrapper.h"
#include <string>
namespace Air
{

	struct ShaderCompilerOutput;
	struct ShaderCompilerInput;

	void compileShader_windows_sm5(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory);

	void compileShader_windows_sm4(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory);

	void compileShader_windows_es2(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory);

	void compileShader_windows_es3_1(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory);
}