#include "ResolveShader.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	IMPLEMENT_SHADER_TYPE(, ResolveDepthPS, TEXT("ResolvePixelShader"), TEXT("MainDepth"), SF_Pixel);

	IMPLEMENT_SHADER_TYPE(, ResolveDepthNonMSPS, TEXT("ResolvePixelShader"), TEXT("MainDepthNonMS"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(, ResolveSingleSamplePS, TEXT("ResolvePixelShader"), TEXT("MainSingleSample"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(, ResolveVS, TEXT("ResolveVertexShader"), TEXT("Main"), SF_Vertex);

	void ResolveSingleSamplePS::setParameters(RHICommandList& RHICmdList, uint32 singleSampleIndexValue)
	{
		setShaderValue(RHICmdList, getPixelShader(), mSingleSampleIndex, singleSampleIndexValue);
	}
}