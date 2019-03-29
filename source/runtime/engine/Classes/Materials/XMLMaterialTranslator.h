#pragma once
#include "MaterialCompiler.h"
#include "MaterialShared.h"
#include "HLSLMaterialTranslator.h"
#include "Containers/LazyPrintf.h"
namespace Air
{
	class XMLMaterialTranslator : public HLSLMaterialTranslator
	{
	private:
		wstring_view mShaderStr;
		wstring* chunkStrings[CompiledMP_Max];
	public:
		XMLMaterialTranslator(FMaterial* inMaterial, MaterialCompilationOutput& inMaterialCompilationOutput,
			const StaticParameterSet& inStaticParameters,
			EShaderPlatform inPlatform,
			EMaterialQualityLevel::Type inQualityLevel,
			ERHIFeatureLevel::Type inFeatureLevel)
			:
			HLSLMaterialTranslator(inMaterial, inMaterialCompilationOutput, inStaticParameters, inPlatform, inQualityLevel, inFeatureLevel)
		{

		}


		virtual void setShaderString(wstring_view shaderStr) override
		{
			mShaderStr = shaderStr;
		}

		

	

	};
}