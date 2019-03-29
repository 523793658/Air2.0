#include "ShaderBaseClasses.h"
#include "RHICommandList.h"
#include "ShaderParameterUtils.h"
namespace Air
{

	template<typename ShaderRHIParamRef>
	void MaterialShader::setParameters(RHICommandList& RHICmdList, const ShaderRHIParamRef shaderRHI, const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const SceneView& view, const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer, bool bDeferredPass, ESceneRenderTargetsMode::Type textureMode)
	{
		setViewParameters(RHICmdList, shaderRHI, view, viewConstantBuffer);
		const bool bOverrideSelection = false;
		ERHIFeatureLevel::Type featureLevel = view.getFeatureLevel();
		BOOST_ASSERT(material.getRenderingThreadShaderMap() && material.getRenderingThreadShaderMap()->isValidForRendering() && material.getFeatureLevel() == featureLevel);
		ConstantExpressionCache* constantExpressionCache = &materialRenderProxy->mConstantExpressionCache[featureLevel];
		bool bConstantExpressionCacheNeedsDelete = false;
		if (bAllowCachedConstantExpressions || !constantExpressionCache->bUpToData || bOverrideSelection)
		{
			MaterialRenderContext materialRenderContext(materialRenderProxy, material, &view);
			bConstantExpressionCacheNeedsDelete = true;
			constantExpressionCache = new ConstantExpressionCache();
			materialRenderProxy->evaluateConstantExpressions(*constantExpressionCache, materialRenderContext, &RHICmdList);
			setLocalConstantBufferParameter(RHICmdList, shaderRHI, mMaterialConstantBuffer, constantExpressionCache->mLocalConstantBuffer);
		}
		else
		{
			setConstantBufferParameter(RHICmdList, shaderRHI, mMaterialConstantBuffer, constantExpressionCache->mConstantBuffer);
		}
#if !(BUILD_TEST || BUILD_SHIPPING || !WITH_EDITOR)
		verifyExpressionAndShaderMaps(materialRenderProxy, material, constantExpressionCache);
#endif
		{
			const TArray<Guid>& parameterCollections = constantExpressionCache->mParameterCollections;
			const int32 parameterCollectionsNum = parameterCollections.size();
			if (mParameterCollectionConstantBuffers.size() < parameterCollectionsNum)
			{

			}
			BOOST_ASSERT(mParameterCollectionConstantBuffers.size() >= parameterCollectionsNum);
			int32 numToSet = Math::min(mParameterCollectionConstantBuffers.size(), parameterCollections.size());
			for (int32 collectionIndex = 0; collectionIndex < numToSet; collectionIndex++)
			{
				ConstantBufferRHIParamRef constantBuffer = getParameterCollectionBuffer(parameterCollections[collectionIndex], view.mFamily->mScene);
				setConstantBufferParameter(RHICmdList, shaderRHI, mParameterCollectionConstantBuffers[collectionIndex], constantBuffer);
			}
		}
		{
			const int32 numScalarExpressions = mPerFramePrevScalarExpressions.size();
			const int32 numVectorExpressions = mPerFrameVectorExpressions.size();
			if (numScalarExpressions > 0 || numVectorExpressions > 0)
			{
				const ConstantExpressionSet& materialConstantExpressionSet = material.getRenderingThreadShaderMap()->getConstantExpressionSet();
				MaterialRenderContext materialRenderContext(materialRenderProxy, material, &view);
				materialRenderContext.mTime = view.mFamily->mCurrentWorldTime;
				materialRenderContext.mRealTime = view.mFamily->mCurrentRealTime;
				for (int32 index = 0; index < numScalarExpressions; ++index)
				{
					auto& parameter = mPerFrameScalarExpressions[index];
					if (parameter.isBound())
					{
						LinearColor tempValue;
						materialConstantExpressionSet.mPerFrameConstantScalarExpressions[index]->getNumberValue(materialRenderContext, tempValue);
						setShaderValue(RHICmdList, shaderRHI, parameter, tempValue.R);
					}
				}
				for (int32 index = 0; index < numVectorExpressions; ++index)
				{
					auto & parameter = mPerFrameVectorExpressions[index];
					if (parameter.isBound())
					{
						LinearColor tempValue;
						materialConstantExpressionSet.mPerFrameConstantVectorExpressions[index]->getNumberValue(materialRenderContext, tempValue);
						setShaderValue(RHICmdList, shaderRHI, parameter, tempValue);
					}
				}
				const int32 numPrevScalarExpressions = mPerFramePrevScalarExpressions.size();
				const int32 numPrevVectorExpressions = mPerFramePrevVectorExpressions.size();
				if (numPrevScalarExpressions > 0 || numPrevVectorExpressions > 0)
				{
					materialRenderContext.mTime = view.mFamily->mCurrentWorldTime - view.mFamily->mDeltaWorldTime;
					materialRenderContext.mRealTime = view.mFamily->mCurrentRealTime - view.mFamily->mDeltaWorldTime;
					for (int32 index = 0; index < numPrevScalarExpressions; ++index)
					{
						auto& parameter = mPerFramePrevScalarExpressions[index];
						if (parameter.isBound())
						{
							LinearColor tempValue;
							materialConstantExpressionSet.mPerFramePrevConstantScalarExpressions[index]->getNumberValue(materialRenderContext, tempValue);
							setShaderValue(RHICmdList, shaderRHI, parameter, tempValue.R);
						}
					}
					for (int32 index = 0; index < numPrevVectorExpressions; ++index)
					{
						auto& parameter = mPerFramePrevVectorExpressions[index];
						if (parameter.isBound())
						{
							LinearColor tempValue;
							materialConstantExpressionSet.mPerFramePrevConstantVectorExpressions[index]->getNumberValue(materialRenderContext, tempValue);
							setShaderValue(RHICmdList, shaderRHI, parameter, tempValue.R);
						}
					}
				}
			}
		}

		if (featureLevel >= ERHIFeatureLevel::SM4)
		{
			if (mSceneColorCopyTexture.isBound())
			{

				setTextureParameter(RHICmdList, shaderRHI, mSceneColorCopyTexture, mSceneColorCopyTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(), SceneRenderTargets::get(RHICmdList).getLightAttenuationTexture());
			}
		}
		if (false)
		{
		}

		if (bConstantExpressionCacheNeedsDelete)
		{
			delete constantExpressionCache;
		}
	}

	

#define IMPLEMENT_MATERIAL_SHADER_SetParameters(ShaderRHIParamRef)	\
	template RENDERER_API void MaterialShader::setParameters<ShaderRHIParamRef>(\
		RHICommandList& RHICmdList,	\
		const ShaderRHIParamRef shaderRHI,	\
		const MaterialRenderProxy* materialRenderProxy,	\
		const FMaterial& material,	\
		const SceneView& view,	\
		const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer,	\
		bool bDeferredPass,	\
		ESceneRenderTargetsMode::Type	textureMode	\
	);

	IMPLEMENT_MATERIAL_SHADER_SetParameters(VertexShaderRHIParamRef);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(HullShaderRHIParamRef);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(DomainShaderRHIParamRef);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(GeometryShaderRHIParamRef);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(PixelShaderRHIParamRef);
	IMPLEMENT_MATERIAL_SHADER_SetParameters(ComputeShaderRHIParamRef);
}