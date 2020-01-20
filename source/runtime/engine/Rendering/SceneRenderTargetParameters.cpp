#include "PostProcess/SceneRenderTargets.h"
#include "Rendering/SceneRenderTargetParameters.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
namespace Air
{
	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(SceneTextureConstantParameters, "SceneTexturesStruct");

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(MobileSceneTextureConstantParameters, "MobileSceneTextures");

	void bindSceneTextureConstantBufferDependentOnShadingPath(const Shader::CompiledShaderInitializerType& initializer, ShaderConstantBufferParameter& sceneTexturesConstantBuffer, ShaderConstantBufferParameter& mobileSceneTexturesConstantBuffer)
	{
		const ERHIFeatureLevel::Type featureLevel = getMaxSupportedFeatureLevel((EShaderPlatform)initializer.mTarget.mPlatform);
		if (SceneInterface::getShadingPath(featureLevel) == EShadingPath::Deferred)
		{
			sceneTexturesConstantBuffer.bind(initializer.mParameterMap, SceneTextureConstantParameters::StaticStructMetadata.getShaderVariableName());
			BOOST_ASSERT(!initializer.mParameterMap.containsParameterAllocation(MobileSceneTextureConstantParameters::StaticStructMetadata.getShaderVariableName()));
		}
	}


}