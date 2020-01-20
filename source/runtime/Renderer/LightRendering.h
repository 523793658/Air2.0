#pragma once
#include "CoreMinimal.h"
#include "sceneRendering.h"
#include "Math/Sphere.h"
#include "GlobalShader.h"
#include "ShadowRendering.h"
namespace Air
{

	template<bool bRadiaLight>
	class TDeferredLightVS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(TDeferredLightVS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return bRadiaLight ? isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4) : true;
		}


		TDeferredLightVS() {}
		TDeferredLightVS(const ShaderMetaType::CompiledShaderInitializerType& initializer) :
			GlobalShader(initializer)
		{

		}

		void setParameters(RHICommandList& RHICmdList, const ViewInfo& view, const LightSceneInfo* lightSceneInfo)
		{
			GlobalShader::setParameters<ViewConstantShaderParameters>(RHICmdList, getVertexShader(), view.mViewConstantBuffer);
			mStencilingGeometryParameters.set(RHICmdList, this, view, lightSceneInfo);
		}

		void setSimpleLightParameters(RHICommandList& RHICmdList, const ViewInfo& view, const Sphere& lightBounds)
		{
			GlobalShader::setParameters<ViewConstantShaderParameters>(RHICmdList, getVertexShader(), view.mViewConstantBuffer);
			float4 stencilingSpherePosAndScale;
			StencilingGeometry::GStencilSphereVertexBuffer.calcTransform(stencilingSpherePosAndScale, lightBounds, view.mViewMatrices.getPreviewTranslation());
			mStencilingGeometryParameters.set(RHICmdList, this, stencilingSpherePosAndScale);
		}
	private:
		StencilingGeometryShaderParameters mStencilingGeometryParameters;
	};
}