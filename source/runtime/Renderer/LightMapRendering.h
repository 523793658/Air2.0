#pragma once
#include "CoreMinimal.h"
#include "MeshMaterialShaderType.h"
namespace Air
{
	class IndirectLightingCacheAllocation;
	class IndirectLightingCache;
	class LightCacheInterface;

	enum ELightMapPolicyType
	{
		LMP_NO_LIGHTMAP,
	};

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(PrecomputedLightingParameters, )
		SHADER_PARAMETER(float3, mIndirectLightingCachePrimitiveAdd)
		SHADER_PARAMETER(float3, mIndirectLightingCachePrimitiveScale)
		SHADER_PARAMETER(float3, mIndirectLightingCacheMinUV)
		SHADER_PARAMETER(float3, mIndirectLightingCacheMaxUV)
		SHADER_PARAMETER(float4, mPointSkyBentNormal)
		SHADER_PARAMETER_EX(float, mDirectinalLightShadowing, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, mStaticShadowMapMask)
		SHADER_PARAMETER(float4, mInvConstantPenumbraSizes)
		SHADER_PARAMETER_ARRAY(float4, mIndirectLightingSHCoefficients0, [3])
		SHADER_PARAMETER_ARRAY(float4, mIndirectLightingSHCoefficients1, [3])
		SHADER_PARAMETER(float4, mIndirectLightingSHCOefficents2)
		SHADER_PARAMETER_EX(float4, mIndrectLightingSHSingleCoefficient, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, mLightMapCoordinateScaleBias)
		SHADER_PARAMETER(float4, mShadowMapCoordinateScaleBias)
		SHADER_PARAMETER_ARRAY_EX(float4, mLightMapScale, [MAX_NUM_LIGHTMAP_COEF], EShaderPrecisionModifier::Half)
		SHADER_PARAMETER_ARRAY_EX(float4, mLightMapAdd, [MAX_NUM_LIGHTMAP_COEF], EShaderPrecisionModifier::Half)
	END_GLOBAL_SHADER_PARAMETER_STRUCT(PrecomputedLightingParameters)


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

		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameter)
		{
			return false;
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameter, ShaderCompilerEnvironment& outEnvironment)
		{}

		ConstantLightMapPolicy(ELightMapPolicyType inIndirectPolicy) :
			mIndirectPolicy(inIndirectPolicy) {}

		

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
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			return true;
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{}
	};

	template<ELightMapPolicyType Policy>
	class TConstantLightMapPolicy : public ConstantLightMapPolicy
	{
	
	public:
		TConstantLightMapPolicy(): ConstantLightMapPolicy(Policy) {}
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			switch (Policy)
			{
			case Air::LMP_NO_LIGHTMAP:
				return NoLightMapPolicy::shouldCompilePermutation(parameters);
			default:
				return false;
				break;
			}
		}

		static void modifyCompilationEnvironment(const MaterialShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TEXT("MAX_NUM_LIGHTMAP_COEF"), MAX_NUM_LIGHTMAP_COEF);
			switch (Policy)
			{
			case Air::LMP_NO_LIGHTMAP:
				NoLightMapPolicy::modifyCompilationEnvironment(parameters, outEnvironment);
				break;
			default:
				break;
			}
		}
	};
}