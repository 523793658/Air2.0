#pragma once
#include "CoreType.h"
#include "Shader.h"
#include "RendererMininal.h"
#include "GlobalShader.h"
#include "MaterialShared.h"
#include "SceneView.h"
#include "SceneRenderTargetParameters.h"
#include "MaterialShaderType.h"
namespace Air
{
	class MaterialRenderProxy;

	class PixelShaderParameter
	{
	public:
		virtual void bind(const ShaderParameterMap& parameterMap) = 0;
	};

	class ENGINE_API MaterialShader : public Shader
	{
	public:
		MaterialShader():mDebugConstantExpressionCBLayout(RHIConstantBufferLayout::Zero)
		{}
		MaterialShader(const MaterialShaderType::CompiledShaderInitializerType& initializer);

		~MaterialShader() {}



		template<typename ShaderRHIParamRef>
		FORCEINLINE_DEBUGGABLE void setViewParameters(RHICommandList& RHICmdList, const ShaderRHIParamRef shaderRHI, const SceneView& view, const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer)
		{
			const auto& viewConstantBufferParameter = getConstantBufferParameter<ViewConstantShaderParameters>();
			const auto& builtinSamplersCBParameter = getConstantBufferParameter<BuiltinSamplersParameters>();
			checkShaderIsValid();
			setConstantBufferParameter(RHICmdList, shaderRHI, viewConstantBufferParameter, viewConstantBuffer);
		}



		template<typename ShaderRHIParamRef>
		void setParameters(
			RHICommandList& RHICmdList,
			const ShaderRHIParamRef shaderRHI,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& material,
			const SceneView& view,
			const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer,
			bool bDeferredPass,
			ESceneRenderTargetsMode::Type textureMode);

		static int32 bAllowCachedConstantExpressions;

		ConstantBufferRHIParamRef getParameterCollectionBuffer(const Guid& Id, const SceneInterface* sceneInterface) const;

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{}



	private:
		ShaderConstantBufferParameter mMaterialConstantBuffer;
		TArray<ShaderConstantBufferParameter> mParameterCollectionConstantBuffers;
		TArray<ShaderParameter> mPerFrameScalarExpressions;
		TArray<ShaderParameter> mPerFrameVectorExpressions;
		TArray<ShaderParameter> mPerFramePrevScalarExpressions;
		TArray<ShaderParameter> mPerFramePrevVectorExpressions;
		ShaderResourceParameter	mSceneColorCopyTexture;
		ShaderResourceParameter mSceneColorCopyTextureSampler;
		RHIConstantBufferLayout mDebugConstantExpressionCBLayout;

#if !(BUILD_TEST || BUILD_SHIPPING || !WITH_EDITOR)
		void verifyExpressionAndShaderMaps(const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const ConstantExpressionCache* constantExpressionCache);
#endif

	};

	

	


}