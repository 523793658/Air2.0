#include "OneColorShader.h"
namespace Air
{
	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(ClearShaderUB, )
		SHADER_PARAMETER_ARRAY(float4, DrawColorMRT, [8])
	END_GLOBAL_SHADER_PARAMETER_STRUCT()

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(ClearShaderUB, "ClearShaderUB");



#define IMPLEMENT_ONECOLORVS(A, B)	typedef TOneColorVS<A, B> TOneColorVS##A##B;	  \
	IMPLEMENT_SHADER_TYPE2_WITH_TEMPLATE_PREFIX(template<> UTILITY_SHADER_API, TOneColorVS##A##B, SF_Vertex);

	IMPLEMENT_ONECOLORVS(false, false);
	IMPLEMENT_ONECOLORVS(false, true);
	IMPLEMENT_ONECOLORVS(true, true);
	IMPLEMENT_ONECOLORVS(true, false);
#undef IMPLEMENT_ONECOLORVS

	void OneColorPS::setColors(RHICommandList& RHICmdList, const LinearColor* colors, int32 numColors)
	{
		BOOST_ASSERT(numColors <= MaxSimultaneousRenderTargets);

		const ShaderConstantBufferParameter& clearUBParam = getConstantBufferParameter<ClearShaderUB>();
		if (clearUBParam.isBound())
		{
			ClearShaderUB clearData;
			Memory::memzero(clearData.DrawColorMRT);
			for (int32 i = 0; i < numColors; i++)
			{
				clearData.DrawColorMRT[i].x = colors[i].R;
				clearData.DrawColorMRT[i].y = colors[i].G;
				clearData.DrawColorMRT[i].z = colors[i].B;
				clearData.DrawColorMRT[i].w = colors[i].A;
			}
			LocalConstantBuffer localUB = TConstantBufferRef<ClearShaderUB>::createLocalConstantBuffer(RHICmdList, clearData, ConstantBuffer_SingleFrame);
			auto& parameter = getConstantBufferParameter<ClearShaderUB>();
			RHICmdList.setLocalShaderConstantBuffer(getPixelShader(), parameter.getBaseIndex(), localUB);
		}
	}

	IMPLEMENT_SHADER_TYPE(, OneColorPS, TEXT("OneColorShader"), TEXT("MainPixelShader"), SF_Pixel);


}