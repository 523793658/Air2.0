#pragma once
#include "CoreMinimal.h"
#include "RendererMininal.h"
#include "MaterialShader.h"
#include "MeshMaterialShaderType.h"
#include "VertexFactory.h"
#include "MeshPassProcessor.h"
#include "MeshDrawShaderBindings.h"
namespace Air
{

	class MeshMaterialShaderElementData
	{
	public:
		RHIConstantBuffer* mFadeConstantBuffer = nullptr;
		RHIConstantBuffer* mDitherConstantBuffer = nullptr;
		RENDERER_API void initializerMeshMaterialdata(const SceneView* sceneView, const PrimitiveSceneProxy* RESTRICT primitiveSceneProxy, const MeshBatch& RESTRICT meshBatch, int32 staticMeshId, bool bAllowStencilDither);
	};

	class RENDERER_API MeshMaterialShader : public MaterialShader
	{
		DECLARE_SHADER_TYPE(MeshMaterialShader, MeshMaterial);
	public:

		MeshMaterialShader() {}
		MeshMaterialShader(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer)
			:MaterialShader(initializer)
			, mVertexFactoryParameters(initializer.mVertexFactoryType, initializer.mParameterMap, (EShaderFrequency)initializer.mTarget.mFrequency, initializer.mTarget.getPlatform())
		{
			
		}

		static bool validateCompiledResult(EShaderPlatform platform, const TArray<FMaterial*>& materials, const VertexFactoryType* vertexFactoryType, const ShaderParameterMap& parameterMap, TArray<wstring>& outError)
		{
			return true;
		}

		FORCEINLINE friend void validateAfterBind(MeshMaterialShader* shader)
		{
			BOOST_ASSERT(shader->mPassConstantBuffer.isInitialized());
		}

		void getShaderBindings(
			const Scene* scene,
			ERHIFeatureLevel::Type mFeatureLevel,
			const PrimitiveSceneProxy* primitiveSceneProxy,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& material,
			const MeshPassProcessorRenderState& drawRenderState,
			const MeshMaterialShaderElementData& shaderElementData,
			MeshDrawSingleShaderBindings& shaderBindings) const;

		void getElementShaderBindings(
			const Scene* scene,
			const SceneView* viewIfDynamicMeshCommand,
			const VertexFactory* vertexFactory,
			const EVertexInputStreamType inputStreamType,
			ERHIFeatureLevel::Type featureLevel,
			const PrimitiveSceneProxy* primitiveSceneProxy,
			const MeshBatch& meshBatch,
			const MeshBatchElement& batchElement,
			const MeshMaterialShaderElementData& shaderElementData,
			MeshDrawSingleShaderBindings& shaderBindings,
			VertexInputStreamArray& vertexStreams) const;

		virtual const VertexFactoryParameterRef* getVertexFactoryParameterRef() const override { return &mVertexFactoryParameters; }
		
		virtual bool serialize(Archive& ar) override;

		virtual uint32 getAllocatedSize() const override;

	protected:
		ShaderConstantBufferParameter mPassConstantBuffer;

	private:
		VertexFactoryParameterRef mVertexFactoryParameters;
	};

	
}