#pragma once
#include "DeferredShadingRenderer.h"
#include "SceneRenderTargetParameters.h"
#include "SceneCore.h"
#include "HitProxies.h"
#include "DrawingPolicy.h"
#include "MaterialShared.h"
#include "ShaderBaseClasses.h"
#include "RenderUtils.h"
#include "Classes/Materials/Material.h"
#include "RHIDefinitions.h"
#include "DebugViewModeHelpers.h"
#include "Materials/MeshMaterialShader.h"
#include "VertexFactory.h"
#include "ShaderParameterUtils.h"
#include "RenderResource.h"
namespace Air
{
	class BasePassOpaqueDrawingPolicyFactory
	{
	public:
		enum { bAllowSimpleElements = true };
		struct ContextType
		{
			bool bEditorCompositeDepthTest;
			ESceneRenderTargetsMode::Type mTextureMode;
			ContextType(bool bInEditorCompositeDepthTest, ESceneRenderTargetsMode::Type inTextureMode)
				:bEditorCompositeDepthTest(bInEditorCompositeDepthTest)
				,mTextureMode(inTextureMode)
			{}
		};

		static void addStaticMesh(RHICommandList& cmdList, Scene* scene, StaticMesh* staticMesh);
		static bool drawDynamicMesh(
			RHICommandList& RHICmdList,
			const ViewInfo& view,
			ContextType drawingContext,
			const MeshBatch& mesh,
			bool bPreFog,
			const DrawingPolicyRenderState& drawRenderState,
			const PrimitiveSceneProxy* primitiveSceneProxy,
			HitProxyId hitProxyId,
			const bool bIsInstancedStereo = false
		);

	};

	class BasePassDrawingPolicy : public MeshDrawingPolicy
	{
	public:
		BasePassDrawingPolicy(
			const VertexFactory* inVertexFactory,
			const MaterialRenderProxy* inMaterialRenderProxy,
			const FMaterial& inMaterialResource,
			const MeshDrawingPolicyOverrideSettings& inOverrideSettings,
			EDebugViewShaderMode inDebugViewShaderMode,
			bool bInEnableReceiveDecalOutput) : MeshDrawingPolicy(inVertexFactory, inMaterialRenderProxy, inMaterialResource, inOverrideSettings, inDebugViewShaderMode),
			bEnableReceiveDecalOutput(bInEnableReceiveDecalOutput)
		{}

		

		void applyDitheredLODTransitionState(RHICommandList& RHICmdList, DrawingPolicyRenderState& drawRenderState, const ViewInfo& viewInfo, const StaticMesh& mesh, const bool inAllowStencilDither);
	protected:
		uint32 bEnableReceiveDecalOutput : 1;

	};

	template<typename VertexParametersType>
	class TBasePassVertexShaderPolicyParamType : public MeshMaterialShader, public VertexParametersType
	{
	protected:
		TBasePassVertexShaderPolicyParamType() {}
		TBasePassVertexShaderPolicyParamType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) :
			MeshMaterialShader(initializer)
		{
			VertexParametersType::bind(initializer.mParameterMap);
			
		}
	public:
		void setParameters(
			RHICommandList& RHICmdList,
			const MaterialRenderProxy* materialRenderProxy,
			const VertexFactory* vertexFactory,
			const FMaterial& inMaterialResource,
			const ViewInfo& view,
			ESceneRenderTargetsMode::Type textureMode,
			bool bIsInstancedStereo,
			bool bUseDownsampledTranslucencyViewConstantBuffer)
		{
			const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer = bUseDownsampledTranslucencyViewConstantBuffer ? view.mDownsampledTranslucencyViewConstantBuffer : view.mViewConstantBuffer;
			MeshMaterialShader::setParameters(RHICmdList, getVertexShader(), materialRenderProxy, inMaterialResource, view, viewConstantBuffer, textureMode);
			
		}
		void setMesh(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatch& mesh, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState)
		{
			VertexShaderRHIParamRef vertexShaderRHI = getVertexShader();
			MeshMaterialShader::setMesh(RHICmdList, vertexShaderRHI, vertexFactory, view, proxy, batchElement, drawRenderState);
			const bool bHashPrevousLocalToWorldParameter = false;
			const bool bHashSkipOutputVelocityParameter = false;
			if (bHashPrevousLocalToWorldParameter)
			{

			}
			if (bHashSkipOutputVelocityParameter)
			{

			}
		}

		void setInstancedEyeIndex(RHICommandList& RHICmdList, const uint32 eyeIndex)
		{
		}
	private:
	};

	class SkyLightReflectionParameters
	{
	public:
		void bind(const ShaderParameterMap& parameterMap)
		{
			mSkyLightCubemap.bind(parameterMap, TEXT("SkyLightCubemap"));
			mSkyLightCubemapSampler.bind(parameterMap, TEXT("SkyLightCubemapSampler"));
			mSkyLightBlendDestinationCubemap.bind(parameterMap, TEXT("SkyLightBlendDestinationCubemap"));
			mSkyLightBlendDestinationCubemapSampler.bind(parameterMap, TEXT("SkyLightBlendDestinationCubemapSampler"));
			mSkyLightParameters.bind(parameterMap, TEXT("SkyLightParameters"));
			mSkyLightCubemapBrightness.bind(parameterMap, TEXT("SkyLightCubemapBrightness"));
		}

		template<typename TParamRef, typename TRHICmdList>
		void setParameters(TRHICmdList& RHICmdList, const TParamRef& shaderRHI, const Scene* scene, bool bApplySkyLight)
		{
			if (mSkyLightCubemap.isBound() || mSkyLightBlendDestinationCubemap.isBound() || mSkyLightParameters.isBound())
			{
				Texture* skyLightTextureResource = GBlackTextureCube;
				Texture* skyLightBlendDistinationTextureResource = GBlackTextureCube;
				float applySkyLightMask = 0;
				float skyMipCount = 0;
				float blendFraction = 0;
				bool bSkyLightIsDynamic = false;
				float skyAverageBrightness = 1.0f;
				getSkyParametersFromScene(scene, bApplySkyLight, skyLightTextureResource, skyLightBlendDistinationTextureResource, applySkyLightMask, skyMipCount, bSkyLightIsDynamic, blendFraction, skyAverageBrightness);
				setTextureParameter(RHICmdList, shaderRHI, mSkyLightCubemap, mSkyLightCubemapSampler, skyLightTextureResource);
				setTextureParameter(RHICmdList, shaderRHI, mSkyLightBlendDestinationCubemap, mSkyLightBlendDestinationCubemapSampler, skyLightBlendDistinationTextureResource);
				const float4 skyParametersValue(skyMipCount - 1.0f, applySkyLightMask, bSkyLightIsDynamic ? 1.0f : 0.0f, blendFraction);
				setShaderValue(RHICmdList, shaderRHI, mSkyLightParameters, skyParametersValue);
				setShaderValue(RHICmdList, shaderRHI, mSkyLightCubemapBrightness, skyAverageBrightness);
			}
		}

	private:
		ShaderResourceParameter mSkyLightCubemap;
		ShaderResourceParameter mSkyLightCubemapSampler;
		ShaderResourceParameter mSkyLightBlendDestinationCubemap;
		ShaderResourceParameter mSkyLightBlendDestinationCubemapSampler;

		ShaderParameter mSkyLightParameters;
		ShaderParameter mSkyLightCubemapBrightness;


		void getSkyParametersFromScene(
			const Scene* scene,
			bool bApplySkyLight,
			Texture*& outSkyLightTextureResource,
			Texture*& outSkyLightBlendDestinationTextureResource,
			float& outApplySkyLightMask,
			float& outSkyMipCount,
			bool& bSkyLightIsDynamic,
			float& outBlendFraction,
			float& outSkyAverageBrightness
		);
	};

	class BasePassReflectionParameters
	{
	public:
		void bind(const ShaderParameterMap& parameterMap)
		{
			mSkyLightReflectionParameters.bind(parameterMap);
		}

		void set(RHICommandList& RHICmdList, PixelShaderRHIParamRef pixelShaderRHI, const ViewInfo* view);

	private:
		SkyLightReflectionParameters mSkyLightReflectionParameters;
	};


	template<typename PixelParametersType>
	class TBasePassPixelShaderPolicyParamType : public MeshMaterialShader, public PixelParametersType
	{
	public:
		TBasePassPixelShaderPolicyParamType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer)
			:MeshMaterialShader(initializer)
		{
			PixelParametersType::bind(initializer.mParameterMap);
			mReflectionParameters.bind(initializer.mParameterMap);
		}

		TBasePassPixelShaderPolicyParamType() {}


		void setMesh(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState, EBlendMode blendMode);

		void setParameters(
			RHICommandList& RHICmdList,
			const MaterialRenderProxy* materialRenderProxy,
			const FMaterial& materialResource,
			const ViewInfo* view,
			EBlendMode blendMode,
			bool bEnableEditorPrimitiveDepthTest,
			ESceneRenderTargetsMode::Type textureMode,
			bool bIsInstancedStereo,
			bool bUseDownsampledTranslucencyViewConstantBuffer)
		{
			const PixelShaderRHIParamRef shaderRHI = getPixelShader();
			BOOST_ASSERT(!bUseDownsampledTranslucencyViewConstantBuffer || view->mDownsampledTranslucencyViewConstantBuffer);
			const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer = bUseDownsampledTranslucencyViewConstantBuffer ? view->mDownsampledTranslucencyViewConstantBuffer : view->mViewConstantBuffer;
			MeshMaterialShader::setParameters(RHICmdList, shaderRHI, materialRenderProxy, materialResource, *view, viewConstantBuffer, textureMode);
			
			mReflectionParameters.set(RHICmdList, shaderRHI, view);
			//reflection parameter
			//transparent parameters
			//forwardlighting parameter
			

		}
	private:
		BasePassReflectionParameters mReflectionParameters;
	};

	template<typename LightMapPolicyType>
	class TBasePassVertexShaderBaseType : public TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType>
	{
		typedef TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType> Super;
	protected:

		TBasePassVertexShaderBaseType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : Super(initializer) {}
		TBasePassVertexShaderBaseType() {}

	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			return LightMapPolicyType::shouldCache(platform, material, vertexFactoryType);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			LightMapPolicyType::modifyCompilationEnvironment(platform, material, outEnvironment);
			Super::modifyCompilationEnvironment(platform, material, outEnvironment);
		}
	};

	template<typename LightMapPolicyType>
	class TBasePassPixelShaderBaseType : public TBasePassPixelShaderPolicyParamType<typename LightMapPolicyType::PixelParametersType>
	{
		typedef TBasePassPixelShaderPolicyParamType<typename LightMapPolicyType::PixelParametersType> Super;
	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			return LightMapPolicyType::shouldCache(platform, material, vertexFactoryType);
		}
		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			LightMapPolicyType::modifyCompilationEnvironment(platform, material, outEnvironment);
			Super::modifyCompilationEnvironment(platform, material, outEnvironment);
		}

		TBasePassPixelShaderBaseType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : Super(initializer) {}

		TBasePassPixelShaderBaseType(){}
	};



	template<typename LightMapPolicyType, bool bEnableSkyLight>
	class TBasePassPS : public TBasePassPixelShaderBaseType<LightMapPolicyType>
	{
		DECLARE_SHADER_TYPE(TBasePassPS, MeshMaterial);
	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			static const auto supportStationarySkyLight = false;
			static const auto supportAllShaderPremutations = false;

			const bool bForceAllPermutations = supportAllShaderPremutations;
			const bool bProjectSupportsStationarySkyLight = true;

			const bool bCachedShaders = !bEnableSkyLight || (bProjectSupportsStationarySkyLight && (material->getShadingModel() != MSM_Unlit));

			return bCachedShaders && (isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4)) && TBasePassPixelShaderBaseType<LightMapPolicyType>::shouldCache(platform, material, vertexFactoryType);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TEXT("SCENE_TEXTURES_DISABLED"), material->getMaterialDomain() == MD_DeferredDecal);
			outEnvironment.setDefine(TEXT("ENABLE_SKY_LIGHT"), bEnableSkyLight);
			TBasePassPixelShaderBaseType<LightMapPolicyType>::modifyCompilationEnvironment(platform, material, outEnvironment);
		}

		TBasePassPS(const ShaderMetaType::CompiledShaderInitializerType& initializer):
			TBasePassPixelShaderBaseType<LightMapPolicyType>(initializer) {}

		TBasePassPS() {}
	};


	template<typename LightMapPolicyType, bool bEnableAtmosphericFog>
	class TBasePassHS : public BaseHS
	{
		DECLARE_SHADER_TYPE(TBasePassHS, MeshMaterial);
	protected:
		TBasePassHS() {}
		TBasePassHS(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : BaseHS(initializer)
		{}

		static bool shouldCache(EShaderPlatform platform, const FMaterial* materia, const VertexFactoryType* vertexFactoryType)
		{
			return (bEnableAtmosphericFog == false || isMetalPlatform(platform)) && BaseHS::shouldCache(platform, materia, vertexFactoryType) && TBasePassVS<LightMapPolicyType, false>::shouldCache(platform, materia, vertexFactoryType);
		}
	};

	template<typename LightMapPolicyType>
	class TBasePassDS : public BaseDS
	{
		DECLARE_SHADER_TYPE(TBasePassDS, MeshMaterial);
	protected:
		TBasePassDS() {}
		TBasePassDS(const MeshMaterialShaderType::CompiledShaderInitializerType& inInitializer)
			:BaseDS(inInitializer)
		{}
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			return BaseDS::shouldCache(platform, material, vertexFactoryType) && TBasePassVS<LightMapPolicyType, false>::shouldCache(platform, material, vertexFactoryType);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			TBasePassVS<LightMapPolicyType, false>::modifyCompilationEnvironment(platform, material, outEnvironment);
		}
	public:	 
		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdataedParameters = BaseDS::serialize(ar);
			return bShaderHasOutdataedParameters;
		}
	};


	template<typename LightMapPolicyType, bool bEnableAtmosphericFog>
	class TBasePassVS : public TBasePassVertexShaderBaseType<LightMapPolicyType>
	{
		DECLARE_SHADER_TYPE(TBasePassVS, MeshMaterial);
		typedef TBasePassVertexShaderBaseType<LightMapPolicyType> Super;

	protected:
		TBasePassVS() {}
		TBasePassVS(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : Super(initializer)
		{}
	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			static const auto supportAtmosphericFog = false;
			static const auto supportAllShaderPermutations = false;
			const bool bForceAllPermutations = supportAllShaderPermutations && false;
			const bool bPrejectAllowsAtmosphericFog = false;

			bool bShouldCache = Super::shouldCache(platform, material, vertexFactoryType);
			//bShouldCache &= false;
			return bShouldCache && (isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4));
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			Super::modifyCompilationEnvironment(platform, material, outEnvironment);
			outEnvironment.setDefine(TEXT("BASEPASS_ATMOSPHERIC_FOG"), bEnableAtmosphericFog);
		}
	};
		

	template<typename LightMapPolicyType>
	void getBasePassShaders(
		const FMaterial& material, VertexFactoryType* vertexFactoryType,
		LightMapPolicyType lightMapPolicy,
		bool bNeedsHSDS,
		bool bEnableAtmosphericFog,
		bool bEnableSkyLight,
		BaseHS*& hullShader,
		BaseDS*& domainShader,
		TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType>*& vertexShader,
		TBasePassPixelShaderPolicyParamType<typename LightMapPolicyType::PixelParametersType>*& pixelShader
	)
	{
		if (bNeedsHSDS)
		{
		}
		if (bEnableAtmosphericFog)
		{

		}
		else
		{
			vertexShader = material.getShader < TBasePassVS<LightMapPolicyType, false>>(vertexFactoryType);
			auto a = &TBasePassVS<LightMapPolicyType, false>::mStaticType;
		}

		if (bEnableSkyLight)
		{
			pixelShader = material.getShader<TBasePassPS<LightMapPolicyType, false>>(vertexFactoryType);
		}
	}


	template<typename LightMapPolicyType>
	class TBasePassDrawingPolicy : public BasePassDrawingPolicy
	{

	public:
		class ElementDataType
		{
		public:	 
			typename LightMapPolicyType::ElementDataType LightMapElementData;
			ElementDataType()
			{}
			ElementDataType(const typename LightMapPolicyType::ElementDataType& inLightMapElementData)
				:LightMapElementData(inLightMapElementData)
			{}
		};


		TBasePassDrawingPolicy(const VertexFactory* inVertexFactory, const MaterialRenderProxy* inMaterialRenderProxy, const FMaterial& inMaterialResource, ERHIFeatureLevel::Type inFeatureLevel, LightMapPolicyType inLightMapPolicy, EBlendMode inBlendMode, ESceneRenderTargetsMode::Type inSceneTextureMode, bool bInEnableSkyLight, bool bInEnableAtmosphericFog, const MeshDrawingPolicyOverrideSettings& inOverrideSettings, EDebugViewShaderMode inDebugViewShaderMode = DVSM_None, bool bInEnableEditorPrimitiveDepthTest = false, bool bInEnableReciveDecalOutput = false) :
			BasePassDrawingPolicy(inVertexFactory, inMaterialRenderProxy, inMaterialResource, inOverrideSettings, inDebugViewShaderMode, bInEnableReciveDecalOutput),
			mLightMapPolicy(inLightMapPolicy),
			mBlendMode(inBlendMode),
			mSceneTextureMode(inSceneTextureMode),
			bEnableSkyLight(bInEnableSkyLight),
			bEnableEditorPrimitvieDepthTest(bInEnableEditorPrimitiveDepthTest),
			bEnableAtmosphericFog(bInEnableAtmosphericFog)
		{
			mHullShader = nullptr;
			mDomainShader = nullptr;
			const  EMaterialTessellationMode materialTessellationMode = inMaterialResource.getTessellationMode();

			const bool bNeedsHSDS = RHISupportsTessellation(GShaderPlatformForFeatureLevel[inFeatureLevel]) && inVertexFactory->getType()->supportsTessellationShaders() && materialTessellationMode != MTM_NOTessellation;

			getBasePassShaders<LightMapPolicyType>(inMaterialResource, mVertexFactory->getType(), inLightMapPolicy, bNeedsHSDS, bEnableAtmosphericFog, bEnableSkyLight, mHullShader, mDomainShader, mVertexShader, mPixelShader);
		}

		DrawingPolicyMatchResult matches(const TBasePassDrawingPolicy& other) const
		{
			DRAWING_POLICY_MATCH_BEGIN
				DRAWING_POLICY_MATCH(MeshDrawingPolicy::matches(other)) &&
				DRAWING_POLICY_MATCH(mVertexShader == other.mVertexShader) &&
				DRAWING_POLICY_MATCH(mPixelShader == other.mPixelShader) &&
				DRAWING_POLICY_MATCH(mHullShader == other.mHullShader) &&
				DRAWING_POLICY_MATCH(mDomainShader == other.mDomainShader) &&
				DRAWING_POLICY_MATCH(mSceneTextureMode == other.mSceneTextureMode) &&
				DRAWING_POLICY_MATCH(bEnableSkyLight == other.bEnableSkyLight) &&
				DRAWING_POLICY_MATCH(mLightMapPolicy == other.mLightMapPolicy);
			DRAWING_POLICY_MATCH_END
		}

		BoundShaderStateInput getBoundShaderStateInput(ERHIFeatureLevel::Type inFealtureLevel)
		{
			BoundShaderStateInput boundShaderStateInput(
				MeshDrawingPolicy::getVertexDeclaration(),
				mVertexShader->getVertexShader(),
				GETSAFERHISHADER_HULL(mHullShader),
				GETSAFERHISHADER_DOMAIN(mDomainShader),
				GeometryShaderRHIRef(),
				mPixelShader->getPixelShader()
			);
			if (useDebugViewPS())
			{
				//Debugview
			}
			return boundShaderStateInput;
		}

		void setMeshRenderState(RHICommandList& RHICmdList,
			const ViewInfo& view,
			const PrimitiveSceneProxy* primitiveSceneProxy,
			const MeshBatch& mesh,
			int32 batchElementIndex,
			DrawingPolicyRenderState& drawRenderState,
			const ElementDataType& elementData,
			const ContextDataType& policyContext)const
		{
			const MeshBatchElement& batchElement = mesh.mElements[batchElementIndex];
			if (view.mFamily->useDebugViewVSDSHS())
			{

			}
			else
			{
				mLightMapPolicy.setMesh(RHICmdList, view, primitiveSceneProxy, mVertexShader, !useDebugViewPS() ? mPixelShader : nullptr, mVertexShader, mPixelShader, mVertexFactory, mMaterialRenderProxy, elementData.LightMapElementData);
				mVertexShader->setMesh(RHICmdList, mVertexFactory, view, primitiveSceneProxy, mesh, batchElement, drawRenderState);
				if (mHullShader && mDomainShader)
				{
					mHullShader->setMesh(RHICmdList, mVertexFactory, view, primitiveSceneProxy, batchElement, drawRenderState);
					mDomainShader->setMesh(RHICmdList, mVertexFactory, view, primitiveSceneProxy, batchElement, drawRenderState);
				}
			}
			if (useDebugViewPS())
			{

			}
			else
			{
				mPixelShader->setMesh(RHICmdList, mVertexFactory, view, primitiveSceneProxy, batchElement, drawRenderState, mBlendMode);
			}
			MeshDrawingPolicy::setMeshRenderState(RHICmdList, view, primitiveSceneProxy, mesh, batchElementIndex, drawRenderState, MeshDrawingPolicy::ElementDataType(), policyContext);
		}

		void setInstancedEyeIndex(RHICommandList& RHICmdList, const uint32 eyeIndex) const
		{
			mVertexShader->setInstancedEyeIndex(RHICmdList, eyeIndex);
		}

	protected:
		TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType>* mVertexShader;

		BaseHS* mHullShader;
		BaseDS* mDomainShader;
		TBasePassPixelShaderPolicyParamType<typename LightMapPolicyType::PixelParametersType>* mPixelShader;

		LightMapPolicyType mLightMapPolicy;
		EBlendMode	mBlendMode;
		ESceneRenderTargetsMode::Type mSceneTextureMode;
		uint32	bEnableSkyLight : 1;
		uint32 bEnableEditorPrimitvieDepthTest : 1;
		uint32 bEnableAtmosphericFog : 1;

	public:

		void setSharedState(RHICommandList& RHICmdList, const ViewInfo* view, const ContextDataType policyContext, const DrawingPolicyRenderState& drawRenderState, bool bUseDownsampledTranslucencyViewConstantBuffer = false) const
		{
			if (false)
			{

			}
			else
			{
				mLightMapPolicy.set(RHICmdList, mVertexShader, mPixelShader, mVertexShader, mPixelShader, mVertexFactory, mMaterialRenderProxy, view);
				mVertexShader->setParameters(RHICmdList, mMaterialRenderProxy, mVertexFactory, *mMaterialResource, *view, mSceneTextureMode, policyContext.bIsInstancedStereo, bUseDownsampledTranslucencyViewConstantBuffer);
				if (mHullShader)
				{
					mHullShader->setParameters(RHICmdList, mMaterialRenderProxy, *view);
				}
				if (mDomainShader)
				{
					mDomainShader->setParameters(RHICmdList, mMaterialRenderProxy, *view);
				}
			}
			if (false)
			{

			}
			else
			{
				mPixelShader->setParameters(RHICmdList, mMaterialRenderProxy, *mMaterialResource, view, mBlendMode, bEnableEditorPrimitvieDepthTest, mSceneTextureMode, policyContext.bIsInstancedStereo, bUseDownsampledTranslucencyViewConstantBuffer);

				switch (mBlendMode)
				{
				default:
				case Air::BLEND_Opaque:

					break;
				case Air::BLEND_Masked:
					break;
				case Air::BLEND_Translucent:
					RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::getRHI());
					break;
				case Air::BLEND_Additive:
					RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_InverseSourceAlpha>::getRHI());
					break;
				case Air::BLEND_Modulate:
					RHICmdList.setBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_DestColor, BF_Zero>::getRHI());
					break;
				case Air::BLEND_AlphaComposite:
					RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::getRHI());
					break;
				}
			}
		}
	};

	class ProcessBasePassMeshParameters
	{
	public:
		const MeshBatch& mMesh;
		const uint64 mBatchElementMask;
		const FMaterial* mMaterial;
		const PrimitiveSceneProxy* mPrimitiveSceneProxy;
		EBlendMode mBlendMode;
		EMaterialShadingModel mShadingModel;
		const bool bAllowFog;
		const bool bEditorCompositeDepthTest;
		ESceneRenderTargetsMode::Type mTextureMode;
		ERHIFeatureLevel::Type mFeatureLevel;
		const bool bIsInstancedStereo;
		const bool bUseMobileMultiViewMask;
		ProcessBasePassMeshParameters(
			const MeshBatch& inMesh,
			const FMaterial* inMaterial,
			const PrimitiveSceneProxy* inPrimitiveSceneProxy,
			bool inbAllowFog,
			bool bInEditorCompositeDepthTest,
			ESceneRenderTargetsMode::Type inTextureMode,
			ERHIFeatureLevel::Type inFeatureLevel,
			const bool inbIsInstancedStereo = false,
			const bool inbUseMobileMultiViewMask = false
		):
			mMesh(inMesh),
			mBatchElementMask(mMesh.mElements.size() == 1? 1 : (1 << mMesh.mElements.size()) - 1),
			mMaterial(inMaterial),
			mPrimitiveSceneProxy(inPrimitiveSceneProxy),
			mBlendMode(inMaterial->getBlendMode()),
			mShadingModel(inMaterial->getShadingModel()),
			bAllowFog(inbAllowFog),
			bEditorCompositeDepthTest(bInEditorCompositeDepthTest),
			mTextureMode(inTextureMode),
			mFeatureLevel(inFeatureLevel),
			bIsInstancedStereo(inbIsInstancedStereo),
			bUseMobileMultiViewMask(inbUseMobileMultiViewMask)
		{}

		ProcessBasePassMeshParameters(
			const MeshBatch& inMesh,
			const uint64& inBatchElementMask,
			const FMaterial* inMaterial,
			const PrimitiveSceneProxy* inPrimitiveSceneProxy,
			bool inbAllowFog,
			bool bInEditorCompositeDepthTest,
			ESceneRenderTargetsMode::Type inTextureMode,
			ERHIFeatureLevel::Type inFeatureLevel,
			bool inbIsInstancedStereo = false,
			bool inbUseMobileMultiViewMask = false
		):
			mMesh(inMesh),
			mBatchElementMask(inBatchElementMask),
			mMaterial(inMaterial),
			mPrimitiveSceneProxy(inPrimitiveSceneProxy),
			mBlendMode(inMaterial->getBlendMode()),
			mShadingModel(inMaterial->getShadingModel()),
			bAllowFog(inbAllowFog),
			bEditorCompositeDepthTest(bInEditorCompositeDepthTest),
			mTextureMode(inTextureMode),
			mFeatureLevel(inFeatureLevel),
			bIsInstancedStereo(inbIsInstancedStereo),
			bUseMobileMultiViewMask(inbUseMobileMultiViewMask)
		{}
	};

	template<typename ProcessActionType>
	void processBasePassMesh(RHICommandList& RHICmdList, const ProcessBasePassMeshParameters& parameters, ProcessActionType && action)
	{
		const bool bIsLitMaterial = parameters.mShadingModel != MSM_Unlit;
		const bool bNeedsSceneTextures = parameters.mMaterial->needsSceneTextures() && !(parameters.mMaterial->getMaterialDomain() == MD_DeferredDecal);
		const bool bAllowStaticLighting = false;
		if (!(parameters.mTextureMode == ESceneRenderTargetsMode::DontSet && bNeedsSceneTextures))
		{
			if (bIsLitMaterial && action.useTranslucentSelfShadowing() && parameters.mFeatureLevel >= ERHIFeatureLevel::SM4)
			{

			}
			else
			{
				const LightMapInteraction lightMapInteraction = LightMapInteraction();

				if (isSimpleForwardShadingEnable(getFeatureLevelShaderPlatform(parameters.mFeatureLevel)))
				{

				}
				else
				{
					switch (lightMapInteraction.getType())
					{
					default:
					{
						if (false)
						{

						}
						else
						{
							action.template process<ConstantLightMapPolicy>(RHICmdList, parameters, ConstantLightMapPolicy(LMP_NO_LIGHTMAP), parameters.mMesh.LCI);
						}
					}
						break;
					}
				}
			}
		}
	}
}