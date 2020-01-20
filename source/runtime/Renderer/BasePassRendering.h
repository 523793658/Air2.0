#pragma once
#include "Rendering/SceneRenderTargetParameters.h"
#include "SceneCore.h"
#include "HitProxies.h"
#include "MaterialShared.h"
#include "ShaderBaseClasses.h"
#include "RenderUtils.h"
#include "Classes/Materials/Material.h"
#include "RHIDefinitions.h"
#include "DebugViewModeHelpers.h"
#include "Classes/Materials/MeshMaterialShader.h"
#include "VertexFactory.h"
#include "ShaderParameterUtils.h"
#include "RenderResource.h"
#include "sceneRendering.h"
namespace Air
{
	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(SharedBasePassConstantParameters,)
		SHADER_PARAMETER_STRUCT(ForwardLightData, Forward)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()


	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(OpaqueBasePassConstantParameters, )
		SHADER_PARAMETER_STRUCT(SharedBasePassConstantParameters, Shared)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(TranslucentBasePassConstantParameters,)
		SHADER_PARAMETER_STRUCT(SharedBasePassConstantParameters, Shared)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()

	class ForwardLocalLightData
	{
	public:	 
		float4 mLightPositionAndInvRadius;
		float4 mLightColorAndFalloffExponent;
		float4 mLightDirectionAndShadowMapChannelMask;
		float4 mSpotAnglesAndSourceRadiusPacked;
		float4 mLightTangnetAndSoftSourceRadius;
		float4 mRectBarnDoor;
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
		

		void setInstancedEyeIndex(RHICommandList& RHICmdList, const uint32 eyeIndex)
		{
		}
	private:
	};



	inline void bindBasePassConstantBuffer(const ShaderParameterMap& parameterMap, ShaderConstantBufferParameter& basePassConstantBuffer)
	{
		TArray<const ShaderParametersMetadata*> nestedStructs;
		OpaqueBasePassConstantParameters::StaticStructMetadata.getNestedStructs(nestedStructs);
		TranslucentBasePassConstantParameters::StaticStructMetadata.getNestedStructs(nestedStructs);

		for (int32 structIndex = 0; structIndex < nestedStructs.size(); structIndex++)
		{
			const TCHAR* structVariableName = nestedStructs[structIndex]->getShaderVariableName();
			BOOST_ASSERT(!parameterMap.containsParameterAllocation(structVariableName));
		}

		const bool bNeedsOpaqueBasePass = parameterMap.containsParameterAllocation(OpaqueBasePassConstantParameters::StaticStructMetadata.getShaderVariableName());
		const bool bNeedsTranslucentBasePass = parameterMap.containsParameterAllocation(TranslucentBasePassConstantParameters::StaticStructMetadata.getShaderVariableName());

		BOOST_ASSERT(!(bNeedsOpaqueBasePass && bNeedsTranslucentBasePass));

		basePassConstantBuffer.bind(parameterMap, OpaqueBasePassConstantParameters::StaticStructMetadata.getShaderVariableName());
		if (!basePassConstantBuffer.isBound())
		{
			basePassConstantBuffer.bind(parameterMap, TranslucentBasePassConstantParameters::StaticStructMetadata.getShaderVariableName());
		}
	}


	template<typename LightMapPolicyType>
	class TBasePassPixelShaderPolicyParamType : public MeshMaterialShader, public LightMapPolicyType::PixelParametersType
	{
	public:
		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			MeshMaterialShader::modifyCompilationEnvironment(parameters, outEnvironment);
			const bool bOutputVelocity = false;
		}

		static bool validateCompileResult(EShaderPlatform platfom, const TArray<FMaterial*>& materials, const VertexFactoryType* vertexFactoryType, const ShaderParameterMap& parameterMap, TArray<wstring>& outError)
		{
			if (parameterMap.containsParameterAllocation(SceneTextureConstantParameters::StaticStructMetadata.getShaderVariableName()))
			{
				outError.add(TEXT("Base pass shaders cannot read from the SceneTexturesStruct."));
				return false;
			}
			return true;
		}


		TBasePassPixelShaderPolicyParamType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer)
			:MeshMaterialShader(initializer)
		{
			LightMapPolicyType::PixelParametersType::bind(initializer.mParameterMap);
			bindBasePassConstantBuffer(initializer.mParameterMap, mPassConstantBuffer);
			mReflectionCaptureBuffer.bind(initializer.mParameterMap, TEXT("ReflectionCapture"));
		}

		TBasePassPixelShaderPolicyParamType() {}

		virtual bool serialize(Archive& ar)
		{
			bool bShaderHasOutdatedParameters = MeshMaterialShader::serialize(ar);
			LightMapPolicyType::VertexParametersType::serialize(ar);
			ar << mReflectionCaptureBuffer;
			return bShaderHasOutdatedParameters;
		}
		
		void getShaderBindings(const Scene* scene, ERHIFeatureLevel::Type mFeatureLevel, const PrimitiveSceneProxy* primitiveSceneProxy, const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const MeshPassProcessorRenderState& drawRenderState, const MeshMaterialShaderElementData& shaderElementData, MeshDrawSingleShaderBindings& shaderBindings);

		void getElementShaderBindings(const Scene* scene, const SceneView* viewIfDynamicMeshCommand, const VertexFactory* vertexFactory, const EVertexInputStreamType inputStreamType, ERHIFeatureLevel::Type featureLevel, const PrimitiveSceneProxy* primitiveSceneProxy, const MeshBatch& meshBatch, const MeshBatchElement& batchElement, const MeshMaterialShaderElementData& shaderElementData, MeshDrawSingleShaderBindings& shaderBindings, VertexInputStreamArray& vertexStreams) const;

		ShaderConstantBufferParameter mReflectionCaptureBuffer;
	};

	template<typename LightMapPolicyType>
	class TBasePassVertexShaderBaseType : public TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType>
	{
		typedef TBasePassVertexShaderPolicyParamType<typename LightMapPolicyType::VertexParametersType> Super;
	protected:

		TBasePassVertexShaderBaseType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : Super(initializer) {}
		TBasePassVertexShaderBaseType() {}

	public:
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			return LightMapPolicyType::shouldCompilePermutation(parameters);
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			LightMapPolicyType::modifyCompilationEnvironment(parameters, outEnvironment);
			Super::modifyCompilationEnvironment(parameters, outEnvironment);
		}
	};

	template<typename LightMapPolicyType>
	class TBasePassPixelShaderBaseType : public TBasePassPixelShaderPolicyParamType<LightMapPolicyType>
	{
		typedef TBasePassPixelShaderPolicyParamType<LightMapPolicyType> Super;
	public:
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			return LightMapPolicyType::shouldCompilePermutation(parameters);
		}
		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			LightMapPolicyType::modifyCompilationEnvironment(parameters, outEnvironment);
			Super::modifyCompilationEnvironment(parameters, outEnvironment);
		}

		TBasePassPixelShaderBaseType(const MeshMaterialShaderType::CompiledShaderInitializerType& initializer) : Super(initializer) {}

		TBasePassPixelShaderBaseType(){}
	};



	template<typename LightMapPolicyType, bool bEnableSkyLight>
	class TBasePassPS : public TBasePassPixelShaderBaseType<LightMapPolicyType>
	{
		DECLARE_SHADER_TYPE(TBasePassPS, MeshMaterial);
	public:
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			static const auto supportStationarySkyLight = false;
			static const auto supportAllShaderPremutations = false;

			const bool bForceAllPermutations = supportAllShaderPremutations;
			const bool bProjectSupportsStationarySkyLight = true;

			const bool bCachedShaders = !bEnableSkyLight || (bProjectSupportsStationarySkyLight && (parameters.mMaterial->getShadingModel() != MSM_Unlit));

			return bCachedShaders && (isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM5)) && TBasePassPixelShaderBaseType<LightMapPolicyType>::shouldCompilePermutation(parameters);
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TEXT("SCENE_TEXTURES_DISABLED"), parameters.mMaterial->getMaterialDomain() == MD_DeferredDecal);
			outEnvironment.setDefine(TEXT("ENABLE_SKY_LIGHT"), bEnableSkyLight);
			TBasePassPixelShaderBaseType<LightMapPolicyType>::modifyCompilationEnvironment(parameters, outEnvironment);
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

		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			return (bEnableAtmosphericFog == false || isMetalPlatform(parameters.mPlatform)) && BaseHS::shouldCompilePermutation(parameters) && TBasePassVS<LightMapPolicyType, bEnableAtmosphericFog>::shouldCompilePermutation(parameters);
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
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			return BaseDS::shouldCompilePermutation(parameters) && TBasePassVS<LightMapPolicyType, false>::shouldCompilePermutation(parameters);
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			TBasePassVS<LightMapPolicyType, false>::modifyCompilationEnvironment(parameters, outEnvironment);
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
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			static const auto supportAtmosphericFog = false;
			static const auto supportAllShaderPermutations = false;
			const bool bForceAllPermutations = supportAllShaderPermutations && false;
			const bool bPrejectAllowsAtmosphericFog = false;

			bool bShouldCache = Super::shouldCompilePermutation(parameters);
			//bShouldCache &= false;
			return bShouldCache && (isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM4));
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			Super::modifyCompilationEnvironment(parameters, outEnvironment);
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
		TBasePassVertexShaderPolicyParamType<LightMapPolicyType>*& vertexShader,
		TBasePassPixelShaderPolicyParamType<LightMapPolicyType>*& pixelShader
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