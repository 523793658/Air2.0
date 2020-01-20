#pragma once
#include "CoreType.h"
#include "Shader.h"
#include "RendererMininal.h"
#include "GlobalShader.h"
#include "MaterialShared.h"
#include "SceneView.h"
#include "Rendering/SceneRenderTargetParameters.h"
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
		MaterialShader()
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

		template<typename TRHIShader>
		void setParametersInner(
			RHICommandList& RHICmdList,
			TRHIShader* shaderRHI,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& material,
			const SceneView& view);

		template<typename TRHIShader>
		void setParameters(
			RHICommandList& RHICmdList,
			const TRHIShader* shaderRHI,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& material,
			const SceneView& view,
			const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer,
			 ESceneTextureSetupMode sceneTextureSetupMode);

		static int32 bAllowCachedConstantExpressions;

		RHIConstantBuffer* getParameterCollectionBuffer(const Guid& Id, const SceneInterface* sceneInterface) const;

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{}
	protected:
		SceneTextureShaderParameters mSceneTextureParameters;


	private:
		ShaderConstantBufferParameter mMaterialConstantBuffer;
		TArray<ShaderConstantBufferParameter> mParameterCollectionConstantBuffers;
		


#if !(BUILD_TEST || BUILD_SHIPPING || !WITH_EDITOR)
		void verifyExpressionAndShaderMaps(const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const ConstantExpressionCache* constantExpressionCache);
#endif

	};

	

	


}