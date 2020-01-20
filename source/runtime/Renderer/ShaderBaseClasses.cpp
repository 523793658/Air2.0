#include "ShaderBaseClasses.h"
#include "RHICommandList.h"
#include "ShaderParameterUtils.h"
namespace Air
{

	

	template<typename TRHIShader>
	void MaterialShader::setParametersInner(
		RHICommandList& RHICmdList,
		TRHIShader* shaderRHI,
		const MaterialRenderProxy* materialRenderProxy,
		const FMaterial& material,
		const SceneView& view)
	{
		ERHIFeatureLevel::Type featureLevel = view.getFeatureLevel();
		BOOST_ASSERT(material.getRenderingThreadShaderMap());
		BOOST_ASSERT(material.getRenderingThreadShaderMap()->isValidForRendering() && material.getFeatureLevel() == featureLevel);

		ConstantExpressionCache* constantExpressionCache = &materialRenderProxy->mConstantExpressionCache[featureLevel];
		bool bCosntantExpressionCacheNeedsDelete = false;
		bool bForceExpressionEvaluation = false;
		if (!bAllowCachedConstantExpressions || !constantExpressionCache->bUpToData || bForceExpressionEvaluation)
		{
			MaterialRenderContext materialRenderContext(materialRenderProxy, material, &view);
			bCosntantExpressionCacheNeedsDelete = true;
			constantExpressionCache = new ConstantExpressionCache();
			materialRenderProxy->evaluateConstantExpressions(*constantExpressionCache, materialRenderContext, &RHICmdList);
			setLocalConstantBufferParameter(RHICmdList, shaderRHI, mMaterialConstantBuffer, constantExpressionCache->mLocalConstantBuffer);
		}
		else
		{
			setConstantBufferParameter(RHICmdList, shaderRHI, mMaterialConstantBuffer, constantExpressionCache->mConstantBuffer);
		}

		{
			const TArray<Guid>& parameterCollections = constantExpressionCache->mParameterCollections;
			const int32 parameterCollectionsNum = parameterCollections.size();

			BOOST_ASSERT(mParameterCollectionConstantBuffers.size() >= parameterCollectionsNum);

			uint32 numToSet = Math::min(mParameterCollectionConstantBuffers.size(), parameterCollections.size());

			for (int32 collectionIndex = 0; collectionIndex < numToSet; collectionIndex++)
			{
				RHIConstantBuffer* constantBuffer = getParameterCollectionBuffer(parameterCollections[collectionIndex], view.mFamily->mScene);
				
				setConstantBufferParameter(RHICmdList, shaderRHI, mParameterCollectionConstantBuffers[collectionIndex], constantBuffer);
			}
		}

		if (bCosntantExpressionCacheNeedsDelete)
		{
			delete constantExpressionCache;
		}
	}

	

#define IMPLEMENT_MATERIAL_SHADER_SetParameters(TRHIShader)	\
	template RENDERER_API void MaterialShader::setParametersInner<TRHIShader>(\
		RHICommandList& RHICmdList,	\
		TRHIShader* shaderRHI,	\
		const MaterialRenderProxy* materialRenderProxy,	\
		const FMaterial& material,	\
		const SceneView& view	\
	);

	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIVertexShader);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIHullShader);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIDomainShader);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIGeometryShader);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIPixelShader);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(RHIComputeShader);
}