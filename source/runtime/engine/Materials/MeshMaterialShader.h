#pragma once
#include "CoreMinimal.h"
#include "RendererMininal.h"
#include "Materials/MaterialShader.h"
#include "MeshMaterialShaderType.h"
#include "VertexFactory.h"
#include "DrawingPolicy.h"
namespace Air
{
	class ENGINE_API MeshMaterialShader : public MaterialShader
	{
	public:

		MeshMaterialShader() {}
		MeshMaterialShader(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer)
			:MaterialShader(initializer)
			, mVertexFactoryParameters(initializer.mVertexFactoryType, initializer.mParameterMap, (EShaderFrequency)initializer.mTarget.mFrequency)
		{
			mNonInstancedDitherLODFactorParameter.bind(initializer.mParameterMap, TEXT("NonInstancedDitherLODFactor"));
		}

		template<typename ShaderRHIParamRef>
		void setParameters(
			RHICommandList& RHICmdList,
			const ShaderRHIParamRef shaderRHI,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& material,
			const SceneView& view,
			const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer,
			ESceneRenderTargetsMode::Type textureMode)
		{
			MaterialShader::setParameters(RHICmdList, shaderRHI, materialRenderProxy, material, view, viewConstantBuffer, false, textureMode);
		}

		void setVFParametersOnly(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const MeshBatchElement& batchElement)
		{
			mVertexFactoryParameters.setMesh(RHICmdList, this, vertexFactory, view, batchElement, 0);
		}

		template<typename ShaderRHIParamRef>
		void setMesh(RHICommandList& RHICmdList,
			const ShaderRHIParamRef shaderRHI,
			const VertexFactory* vertexFactory,
			const SceneView& view,
			const PrimitiveSceneProxy* proxy,
			const MeshBatchElement& batchElement,
			const DrawingPolicyRenderState& drawRenderState,
			uint32 dataFlags = 0);

		ConstantBufferRHIParamRef getPrimitiveFadeConstantBufferParameter(const SceneView& view, const PrimitiveSceneProxy* proxy);
	private:
		VertexFactoryParameterRef mVertexFactoryParameters;
		ShaderParameter mNonInstancedDitherLODFactorParameter;
	};

	
}