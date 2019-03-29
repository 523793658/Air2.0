#pragma once
#include "CoreMinimal.h"
#include "DrawingPolicy.h"
namespace Air
{
	class IndirectLightingCacheAllocation;
	class IndirectLightingCache;
	class LightCacheInterface;

	enum ELightMapPolicyType
	{
		LMP_NO_LIGHTMAP,
	};

	BEGIN_CONSTANT_BUFFER_STRUCT(PrecomputedLightingParameters, )
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3, mIndirectLightingCachePrimitiveAdd)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3, mIndirectLightingCachePrimitiveScale)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3, mIndirectLightingCacheMinUV)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3, mIndirectLightingCacheMaxUV)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mPointSkyBentNormal)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float, mDirectinalLightShadowing, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mStaticShadowMapMask)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mInvConstantPenumbraSizes)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY(float4, mIndirectLightingSHCoefficients0, [3])
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY(float4, mIndirectLightingSHCoefficients1, [3])
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mIndirectLightingSHCOefficents2)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float4, mIndrectLightingSHSingleCoefficient, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mLightMapCoordinateScaleBias)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, mShadowMapCoordinateScaleBias)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY_EX(float4, mLightMapScale, [MAX_NUM_LIGHTMAP_COEF], EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY_EX(float4, mLightMapAdd, [MAX_NUM_LIGHTMAP_COEF], EShaderPrecisionModifier::Half)
	END_CONSTANT_BUFFER_STRUCT(PrecomputedLightingParameters)


	class EmptyPrecomputedLightingConstantBuffer : public TConstantBuffer<PrecomputedLightingParameters>
	{
		typedef TConstantBuffer<PrecomputedLightingParameters> Supper;
	public:
		virtual void initDynamicRHI() override;
	};

	void getPrecomputeLightingParameters(ERHIFeatureLevel::Type featureLevel, PrecomputedLightingParameters& parameters, const IndirectLightingCache* lightingCache = nullptr,
		const IndirectLightingCacheAllocation* lightingAllocation = nullptr, const LightCacheInterface* LCI = nullptr);

	extern TGlobalResource<EmptyPrecomputedLightingConstantBuffer> GEmptyPrecomputedLightingConstantBuffer;

	class ConstantLightMapPolicyShaderParametersType
	{
	public:
		void bind(const ShaderParameterMap& parameterMap)
		{
			mBufferParameter.bind(parameterMap, TEXT("PrecomputedLightingBuffer"));
		}

		void serialize(Archive& ar)
		{
			ar << mBufferParameter;
		}

		ShaderConstantBufferParameter mBufferParameter;
	};


	class ConstantLightMapPolicy
	{
	public:
		typedef const LightCacheInterface* ElementDataType;


		typedef ConstantLightMapPolicyShaderParametersType PixelParametersType;
		typedef ConstantLightMapPolicyShaderParametersType VertexParametersType;

		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			return false;
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{}

		ConstantLightMapPolicy(ELightMapPolicyType inIndirectPolicy) :
			mIndirectPolicy(inIndirectPolicy) {}

		void set(RHICommandList& RHICmdList, const VertexParametersType* vertexShaderParameters, const PixelParametersType* pixelShaderParameters, Shader* vertexShader,
			Shader* pixelShader, const VertexFactory* vertexFactory, const MaterialRenderProxy* materialRenderProxy, const SceneView* view) const;

		void setMesh(RHICommandList& RHICmdList, const SceneView& view, const PrimitiveSceneProxy* primitiveSceneProxy, const VertexParametersType* vertexShaderParameter, const PixelParametersType* pixelShaderParameters, Shader* vertexShader, Shader* pixelShader, const VertexFactory* vertexFactory, const MaterialRenderProxy* materialRenderProxy, const LightCacheInterface* LCI) const;

		friend bool operator == (const ConstantLightMapPolicy A, const ConstantLightMapPolicy B)
		{
			return A.mIndirectPolicy == B.mIndirectPolicy;
		}

		friend int32 compareDrawingPolicy(const ConstantLightMapPolicy& A, const ConstantLightMapPolicy& B)
		{
			//COMPAREDRAWINGPOLICYMEMBERS(mIndirectPolicy);
			return 0;
		}

		ELightMapPolicyType getIndrectPolicy() const { return mIndirectPolicy; }

	private:
		ELightMapPolicyType mIndirectPolicy;

	};

	struct NoLightMapPolicy
	{
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			return true;
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{}
	};

	template<ELightMapPolicyType Policy>
	class TConstantLightMapPolicy : public ConstantLightMapPolicy
	{
	public:
		TConstantLightMapPolicy(): ConstantLightMapPolicy(Policy) {}
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			switch (Policy)
			{
			case Air::LMP_NO_LIGHTMAP:
				return NoLightMapPolicy::shouldCache(platform, material, vertexFactoryType);
			default:
				return false;
				break;
			}
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TEXT("MAX_NUM_LIGHTMAP_COEF"), MAX_NUM_LIGHTMAP_COEF);
			switch (Policy)
			{
			case Air::LMP_NO_LIGHTMAP:
				NoLightMapPolicy::modifyCompilationEnvironment(platform, material, outEnvironment);
				break;
			default:
				break;
			}
		}
	};
}