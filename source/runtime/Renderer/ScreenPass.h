#pragma once
#include "GlobalShader.h"
namespace Air
{
	class ScreenPassVS : public GlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(ScreenPassVS);

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters&)
		{
			return true;
		}

		ScreenPassVS() = default;

		ScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{}
	};
}