#include "DeferredShadingRenderer.h"
#include "DrawingPolicy.h"
#include "RHIStaticStates.h"
#include "BasePassRendering.h"
#include "SceneManagement.h"
#include "ScenePrivate.h"
#include "Materials/MaterialShader.h"
#include "PrimitiveSceneInfo.h"
#include "DebugViewModeHelpers.h"
#include "LightMapRendering.h"
namespace Air
{
	static void setupBasePassView(RHICommandList& cmdList, const ViewInfo& view, DrawingPolicyRenderState& drawRenderState, const bool bShaderComplexity, const bool bIsEditorPrimitivePass = false)
	{
		if (bShaderComplexity)
		{
			drawRenderState.setBlendState(cmdList, TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_One>::getRHI());
			drawRenderState.setDepthStencilState(cmdList, TStaticDepthStencilState<false, CF_DepthNearOrEqual>::getRHI());
		}
		else
		{
			drawRenderState.setBlendState(cmdList, TStaticBlendStateWriteMask<CW_RGBA, CW_RGBA, CW_RGBA, CW_RGBA>::getRHI());
		}
		drawRenderState.setDepthStencilState(cmdList, TStaticDepthStencilState<true, CF_DepthNearOrEqual>::getRHI());
		if (false)
		{

		}
		else
		{
			if (false)
			{

			}
			else
			{
				cmdList.setViewport(0, 0, 0, view.mFamily->mInstancedStereoWidth, view.mViewRect.max.y, 1);
			}
		}
	}



#define IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, bEnableSkyLight, SkyLightName)	\
	typedef TBasePassPS<LightMapPolicyType, bEnableSkyLight> TBasePassPS##LightMapPolicyName##SkyLightName;	   \
	IMPLEMENT_MATERIAL_SHADER_TYPE (template<>, TBasePassPS##LightMapPolicyName##SkyLightName, TEXT("BasePassPixelShader"), TEXT("MainPS"), SF_Pixel);

#define IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	   \
	typedef TBasePassVS<LightMapPolicyType, false> TBasePassVS##LightMapPolicyName; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassVS##LightMapPolicyName, TEXT("BasePassVertexShader"), TEXT("Main"), SF_Vertex); \
	typedef TBasePassHS<LightMapPolicyType, false> TBasePassHS##LightMapPolicyName;\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassHS##LightMapPolicyName, TEXT("BasePassTessellationShaders"), TEXT("MainHull"), SF_Hull);\
	typedef TBasePassDS<LightMapPolicyType> TBasePassDS##LightMapPolicyName;\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassDS##LightMapPolicyName, TEXT("BasePassTessellationShaders"), TEXT("MainDomain"), SF_Domain);

#define IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE(LightMapPolicyType, LightMapPolicyName, AtmosphericFogShaderName) \
	typedef TBasePassVS<LightMapPolicyType, true> TBasePassVS##LightMapPolicyName##AtmosphericFogShaderName;	  \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassVS##LightMapPolicyName##AtmosphericFogShaderName, TEXT("BasePassVertexShader"), TEXT("Main"), SF_Vertex)	 \
	typedef TBasePassHS<LightMapPolicyType, true>	TBasePassHS##LightMapPolicyName##AtmosphericFogShaderName;		\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassHS##LightMapPolicyName##AtmosphericFogShaderName, TEXT("BasePassTesselationShaders"), TEXT("MainHull"), SF_Hull);



#define IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	 \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	   \
	IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE(LightMapPolicyType, LightMapPolicyName, mAtmosphericFog)  \
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, true, Skylight)\
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, false,);


	IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(TConstantLightMapPolicy<LMP_NO_LIGHTMAP>, NoLightMapPolicy);

	template<ELightMapPolicyType Policy>
	void getConstantBasePassShaders(const FMaterial& material, VertexFactoryType* vertexFactoryType, bool bNeedsHSDS, bool bEnableAtosphericFog, bool bEnableSkyLight, BaseHS*& hullShader, BaseDS*& domainShader, TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicyShaderParametersType>*& vertexShader, TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicyShaderParametersType>*& pixelShader)
	{
		if (bNeedsHSDS)
		{
			domainShader = material.getShader<TBasePassDS<TConstantLightMapPolicy<Policy>>>(vertexFactoryType);
			if (bEnableAtosphericFog && domainShader && isMetalPlatform(EShaderPlatform(domainShader->getTarget().mPlatform)))
			{
				hullShader = material.getShader<TBasePassHS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
			}
			else
			{
				hullShader = material.getShader<TBasePassHS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
			}
		}
		if (bEnableAtosphericFog)
		{
			vertexShader = material.getShader<TBasePassVS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
		}
		else
		{
			vertexShader = material.getShader<TBasePassVS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
		}
		if (bEnableSkyLight)
		{
			pixelShader = material.getShader<TBasePassPS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
		}
		else
		{
			pixelShader = material.getShader<TBasePassPS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
		}
	}

	

	template<>
	void getBasePassShaders<ConstantLightMapPolicy>(const FMaterial& material, VertexFactoryType* vertexFactoryType, ConstantLightMapPolicy lightMapPolicy, bool bNeedsHSDS, bool bEnableAtmosphericFog, bool bEnableSkyLight, BaseHS*& hullShader, BaseDS*& domainShader, TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicyShaderParametersType>*& vertexShader, TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicyShaderParametersType>*& pixelShader)
	{
		switch (lightMapPolicy.getIndrectPolicy())
		{
		case LMP_NO_LIGHTMAP:
			getConstantBasePassShaders<LMP_NO_LIGHTMAP>(material, vertexFactoryType, bNeedsHSDS, bEnableAtmosphericFog, bEnableSkyLight, hullShader, domainShader, vertexShader, pixelShader);
			break;
		default:
			break;
		}
	}


	void DeferredShadingSceneRenderer::renderBasePassDynamicData(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState, bool &isDrity)
	{
		bool bDirty = false;
		BasePassOpaqueDrawingPolicyFactory::ContextType context(false, ESceneRenderTargetsMode::DontSet);
		for (int32 meshBatchIndex = 0; meshBatchIndex < view.mDynamicMeshElements.size(); meshBatchIndex++)
		{
			const MeshBatchAndRelevance& meshBatchAndRelevance = view.mDynamicMeshElements[meshBatchIndex];
			if ((meshBatchAndRelevance.getHasOpaqueOrMaskedMaterial() || mViewFamily.mEngineShowFlags.Wireframe) && meshBatchAndRelevance.getRenderInMainPass())
			{
				const MeshBatch& meshBatch = *meshBatchAndRelevance.Mesh;
				BasePassOpaqueDrawingPolicyFactory::drawDynamicMesh(RHICmdList, view, context, meshBatch, false, drawRenderState, meshBatchAndRelevance.mPrimitiveSceneProxy, meshBatch.mBatchHitPrixyId, view.isInstancedStereoPass());
			}
		}
		if (bDirty)
		{
			isDrity = true;
		}
	}

	bool DeferredShadingSceneRenderer::renderBasePassStaticData(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState)
	{
		bool bDirty = false;
		if (mEarlyZPassMode != DDM_None)
		{

		}
		else
		{
			bDirty |= renderBasePassStaticDataType(RHICmdList, view, drawRenderState, EBasePass_Default);
			bDirty |= renderBasePassStaticDataType(RHICmdList, view, drawRenderState, EBasePass_Masked);
		}
		return bDirty;
	}

	bool DeferredShadingSceneRenderer::renderBasePassView(RHICommandListImmediate& RHICmdList, ViewInfo& view)
	{
		bool bDirty = false;
		DrawingPolicyRenderState drawRenderState(&RHICmdList, view);
		setupBasePassView(RHICmdList, view, drawRenderState, mViewFamily.mEngineShowFlags.ShaderComplexity);
		bDirty |= renderBasePassStaticData(RHICmdList, view, drawRenderState);
		renderBasePassDynamicData(RHICmdList, view, drawRenderState, bDirty);
		return bDirty;
	}


	bool DeferredShadingSceneRenderer::renderBasePass(RHICommandListImmediate& RHICmdList)
	{
		bool bDirty = false;
		if (false)
		{

		}
		else
		{
			if (false)
			{
				//并行渲染分支 

			}
			else
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
				{
					ViewInfo& view = mViews[viewIndex];
					if (view.shouldRenderView())
					{
						bDirty |= renderBasePassView(RHICmdList, view);
					}

					//渲染编辑器辅助物件
				}
			}
		}
		return bDirty;
	}

	bool DeferredShadingSceneRenderer::renderBasePassStaticDataType(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState, const EBasePassDrawListType drawType)
	{
		bool bDirty = false;
		if (!view.isInstancedStereoPass())
		{
			bDirty |= mScene->mBasePassConstantLightMapPolicyDrawList[drawType].drawVisible(RHICmdList, view, drawRenderState, view.mStaticMeshVisibilityMap, view.mStaticMeshBatchVisibility);
		}
		else
		{

		}
		return bDirty;
	}

	class DrawBasePassDynamicMeshAction
	{
	public:
		const ViewInfo& mView;
		DrawingPolicyRenderState mDrawRenderState;
		HitProxyId mHitProxyId;
		DrawBasePassDynamicMeshAction(
			RHICommandList& inRHICmdList,
			const ViewInfo& inView,
			float inDitheredLODTransitionAlpha,
			const DrawingPolicyRenderState& inDrawRenderState,
			const HitProxyId inHitProxyId
		):
			mView(inView),
			mDrawRenderState(&inRHICmdList, inDrawRenderState),
			mHitProxyId(inHitProxyId)
		{
			mDrawRenderState.setDitheredLODTransitionAlpha(inDitheredLODTransitionAlpha);
		}

		bool useTranslucentSelfShadowing() const { return false; }

		bool allowIndirectLightingCache() const { return false; }

		bool allowIndirectLightingCacheVolumeTexture() const
		{
			return true;
		}

		template<typename LightMapPolicyType>
		void process(RHICommandList& RHICmdList,
			const ProcessBasePassMeshParameters& parameters,
			const LightMapPolicyType& lightMapPolicy,
			const typename LightMapPolicyType::ElementDataType& lightMapElementDatat)
		{
			const Scene* scene = parameters.mPrimitiveSceneProxy ? parameters.mPrimitiveSceneProxy->getPrimitiveSceneInfo()->mScene : nullptr;
			const bool bRenderSkylight = false;
			const bool bRenderAtmosphericFog = false;
			bool bEnableReceiveDecalOutput = false;
			TBasePassDrawingPolicy<LightMapPolicyType> drawingPolicy(
				parameters.mMesh.mVertexFactory,
				parameters.mMesh.mMaterialRenderProxy,
				*parameters.mMaterial,
				parameters.mFeatureLevel,
				lightMapPolicy,
				parameters.mBlendMode,
				parameters.mTextureMode,
				bRenderSkylight,
				bRenderAtmosphericFog,
				computeMeshOverrideSettings(parameters.mMesh),
				mView.mFamily->getDebugViewShaderMode(),
				parameters.bEditorCompositeDepthTest,
				bEnableReceiveDecalOutput);
			setDepthStencilStateForBasePass(RHICmdList, mDrawRenderState, mView, parameters.mMesh, parameters.mPrimitiveSceneProxy, bEnableReceiveDecalOutput, false, nullptr);
			RHICmdList.buildAndSetLocalBoundShaderState(drawingPolicy.getBoundShaderStateInput(mView.getFeatureLevel()));
			drawingPolicy.setSharedState(RHICmdList, &mView, typename TBasePassDrawingPolicy<LightMapPolicyType>::ContextDataType(parameters.bIsInstancedStereo), mDrawRenderState);
			for (int32 batchElementIndex = 0, num = parameters.mMesh.mElements.size(); batchElementIndex < num; batchElementIndex++)
			{
				const bool bIsInstancedMesh = parameters.mMesh.mElements[batchElementIndex].bIsInstancedMesh;
				const uint32 instantcedStereoDrawCount = (parameters.bIsInstancedStereo && bIsInstancedMesh) ? 2 : 1;
				for (uint32 drawCountIter = 0; drawCountIter < instantcedStereoDrawCount; ++drawCountIter)
				{
					drawingPolicy.setInstancedEyeIndex(RHICmdList, drawCountIter);
					TDrawEvent<RHICommandList> meshEvent;
					beginMeshDrawEvent(RHICmdList, parameters.mPrimitiveSceneProxy, parameters.mMesh, meshEvent);

					drawingPolicy.setMeshRenderState(RHICmdList, mView, parameters.mPrimitiveSceneProxy, parameters.mMesh, batchElementIndex, mDrawRenderState,
						typename TBasePassDrawingPolicy<LightMapPolicyType>::ElementDataType(lightMapElementDatat),
						typename TBasePassDrawingPolicy<LightMapPolicyType>::ContextDataType());
					drawingPolicy.drawMesh(RHICmdList, parameters.mMesh, batchElementIndex, parameters.bIsInstancedStereo);
				}
			}

#if !(BUILD_SHIPPING || BUILD_TEST)
			if (mView.mFamily->mEngineShowFlags.ShaderComplexity)
			{
				RHICmdList.setDepthStencilState(TStaticDepthStencilState<true, CF_DepthNearOrEqual>::getRHI());
			}
#endif

		}
	};

	class DrawBasePassStaticMeshAction
	{
	public:
		Scene* mScene;
		StaticMesh* mStaticMesh;
		DrawBasePassStaticMeshAction(Scene* inScene, StaticMesh* inStaticMesh)
			:mScene(inScene), mStaticMesh(inStaticMesh)
		{

		}

		bool useTranslucentSelfShadowing() const { return false; }
		const void* getTranslucentSelfShadow() const { return nullptr; }
		bool allowIndirectLightingCache() const { return false; }

		bool AllowIndirectLightingCacheVolumeTexture() const
		{
			return false;
		}

		template<typename LightMapPolicyType>
		void process(
			RHICommandList& RHICmdList,
			const ProcessBasePassMeshParameters& parameters,
			const LightMapPolicyType& lightMapPolicy,
			const typename LightMapPolicyType::ElementDataType& lightMapElementData
		)const
		{
			EBasePassDrawListType drawType = EBasePass_Default;
			if (mStaticMesh->isMasked(parameters.mFeatureLevel))
			{
				drawType = EBasePass_Masked;
			}
			if (mScene)
			{
				TStaticMeshDrawList<TBasePassDrawingPolicy<LightMapPolicyType>>& drawList = mScene->getBasePassDrawList<LightMapPolicyType>(drawType);
				const bool bRenderSkylight = false;
				const bool bRenderAtmosphericFog = false;
				drawList.addMesh(mStaticMesh, typename TBasePassDrawingPolicy<LightMapPolicyType>::ElementDataType(lightMapElementData), TBasePassDrawingPolicy<LightMapPolicyType>(mStaticMesh->mVertexFactory, mStaticMesh->mMaterialRenderProxy, *parameters.mMaterial, parameters.mFeatureLevel, lightMapPolicy, parameters.mBlendMode, parameters.mTextureMode, bRenderSkylight, bRenderAtmosphericFog, computeMeshOverrideSettings(*mStaticMesh), DVSM_None, false, false), mScene->getFeatureLevel());
			}
		}
	};


	static void setDepthStencilStateForBasePass(RHICommandList& RHICmdList, DrawingPolicyRenderState& drawRenderState, const SceneView& view, const MeshBatch& mesh, const PrimitiveSceneProxy* primitiveSceneProxy, bool bEnableReceiveDecalOutput, bool bUseDebugViewPS, DepthStencilStateRHIParamRef loadFadeOverrideDepthStencilState)
	{
		static bool bMaskInEarlyPass = (mesh.mMaterialRenderProxy->getMaterial(view.getFeatureLevel())->isMasked());
		if (bEnableReceiveDecalOutput && !bUseDebugViewPS)
		{

		}
		else if(bMaskInEarlyPass)
		{
			RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Equal>::getRHI());
		}
	}



	void BasePassDrawingPolicy::applyDitheredLODTransitionState(RHICommandList& RHICmdList, DrawingPolicyRenderState& drawRenderState, const ViewInfo& viewInfo, const StaticMesh& mesh, const bool inAllowStencilDither)
	{
		DepthStencilStateRHIParamRef depthStencilState = nullptr;
		drawRenderState.setDitheredLODTransitionAlpha(0.0f);
		if (mesh.bDitheredLODTransition)
		{

		}
		setDepthStencilStateForBasePass(RHICmdList, drawRenderState, viewInfo, mesh, mesh.mPrimitiveSceneInfo->mProxy, bEnableReceiveDecalOutput, useDebugViewPS(), depthStencilState);
	}
	

	void BasePassOpaqueDrawingPolicyFactory::addStaticMesh(RHICommandList& cmdList, Scene* scene, StaticMesh* staticMesh)
	{
		const FMaterial* material = staticMesh->mMaterialRenderProxy->getMaterial(scene->getFeatureLevel());
		const EBlendMode blendMode = material->getBlendMode();
		if (!isTranslucentBlendMode(blendMode))
		{
			processBasePassMesh(cmdList, ProcessBasePassMeshParameters(*staticMesh, material, staticMesh->mPrimitiveSceneInfo->mProxy, false, false, ESceneRenderTargetsMode::DontSet, scene->getFeatureLevel()), DrawBasePassStaticMeshAction(scene, staticMesh));
		}
	}

	bool BasePassOpaqueDrawingPolicyFactory::drawDynamicMesh(RHICommandList& RHICmdList, const ViewInfo& view, ContextType drawingContext, const MeshBatch& mesh, bool bPreFog, const DrawingPolicyRenderState& drawRenderState, const PrimitiveSceneProxy* primitiveSceneProxy, HitProxyId hitProxyId, const bool bIsInstancedStereo /* = false */)
	{
		const FMaterial* material = mesh.mMaterialRenderProxy->getMaterial(view.getFeatureLevel());
		const EBlendMode blendMode = material->getBlendMode();
		if (!isTranslucentBlendMode(blendMode))
		{
			processBasePassMesh(
				RHICmdList,
				ProcessBasePassMeshParameters(mesh, material, primitiveSceneProxy, !bPreFog, drawingContext.bEditorCompositeDepthTest, drawingContext.mTextureMode,
					view.getFeatureLevel(),
					bIsInstancedStereo),
				DrawBasePassDynamicMeshAction(RHICmdList, view, mesh.DitheredLODTransitionAlpha, drawRenderState, hitProxyId)
			);
			return true;
		}
		else
		{
			return false;
		}
	}

	template<typename PixelParametersType>
	void TBasePassPixelShaderPolicyParamType<PixelParametersType>::setMesh(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState, EBlendMode blendMode)
	{
		if (view.getFeatureLevel() >= ERHIFeatureLevel::SM4)
		{

		}
		MeshMaterialShader::setMesh(RHICmdList, getPixelShader(), vertexFactory, view, proxy, batchElement, drawRenderState);
	}

	void BasePassReflectionParameters::set(RHICommandList& RHICmdList, PixelShaderRHIParamRef pixelShaderRHI, const ViewInfo* view)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		mSkyLightReflectionParameters.setParameters(RHICmdList, pixelShaderRHI, (const Scene*)(view->mFamily->mScene), view->mFamily->mEngineShowFlags.SkyLighting);
	}

	void SkyLightReflectionParameters::getSkyParametersFromScene(const Scene* scene, bool bApplySkyLight, Texture*& outSkyLightTextureResource, Texture*& outSkyLightBlendDestinationTextureResource, float& outApplySkyLightMask, float& outSkyMipCount, bool& bSkyLightIsDynamic, float& outBlendFraction, float& outSkyAverageBrightness)
	{
		outSkyLightTextureResource = GBlackTextureCube;
		outSkyLightBlendDestinationTextureResource = GBlackTextureCube;
		outApplySkyLightMask = 0;
		bSkyLightIsDynamic = false;
		outBlendFraction = 0;
		outSkyAverageBrightness = 1.0f;
		if (scene && scene->mSkyLight && scene->mSkyLight->mProcessedTexture && bApplySkyLight)
		{
			const SkyLightSceneProxy& skyLight = *scene->mSkyLight;
			outSkyLightTextureResource = skyLight.mProcessedTexture;
			outBlendFraction = skyLight.mBlendFraction;
			if (skyLight.mBlendFraction > 0.0f && skyLight.mBlendDestinationProcessedTexture)
			{
				if (skyLight.mBlendFraction < 1.0f)
				{
					outSkyLightBlendDestinationTextureResource = skyLight.mBlendDestinationProcessedTexture;
				}
				else
				{
					outSkyLightTextureResource = skyLight.mBlendDestinationProcessedTexture;
					outBlendFraction = 0;
				}
			}
			outApplySkyLightMask = 1;
			bSkyLightIsDynamic = !skyLight.bHasStaticLighting && !skyLight.bWantsStaticShadowing;
			outSkyAverageBrightness = skyLight.mAverageBrightness;
		}
		outSkyMipCount = 1;
		if (outSkyLightTextureResource)
		{
			int32 cubemapWith = outSkyLightTextureResource->getWidth();
			outSkyMipCount = Math::log2(cubemapWith) + 1.0f;
		}
	}
}