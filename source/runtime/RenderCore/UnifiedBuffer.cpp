#include "UnifiedBuffer.h"
#include "GlobalShader.h"
#include "ShaderPermutation.h"
#include "ShaderParameterStruct.h"
namespace Air
{
	class ByteBufferShader : public GlobalShader
	{
	public:
		ByteBufferShader() {}

		ByteBufferShader(const ShaderMetaType::CompiledShaderInitializerType& initailizer)
			:GlobalShader(initailizer)
		{}

		class Float4BufferDim : SHADER_PERMUTATION_BOOL("FLOAT4_BUFFER");
		class Uint4AlignedDim : SHADER_PERMUTATION_BOOL("UNIT4_ALIGNED");
		using PermutationDomain = TShaderPermutationDomain<Float4BufferDim, Uint4AlignedDim>;

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			PermutationDomain permutationVector(parameters.mPermutationId);
			if (permutationVector.get<Float4BufferDim>())
			{
				return RHISupportsComputeShaders(parameters.mPlatform);
			}
			else
			{
				return FGenericDataDrivenShaderPlatformInfo::getSupportsByteBufferComputeShaders(
					parameters.mPlatform) || parameters.mPlatform == SP_PS4 || parameters.mPlatform == SP_PCD3D_SM5 || parameters.mPlatform == SP_XBOXONE_D3D12;
			}
		}

		ENGINE_API static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("FLOAT4_TEXTURE"), false);
		}

		BEGIN_SHADER_PARAMETER_STRUCT(Parameters, )
			SHADER_PARAMETER(uint32, Value)
			SHADER_PARAMETER(uint32, Size)
			SHADER_PARAMETER(uint32, SrcOffset)
			SHADER_PARAMETER(uint32, DstOffset)
			SHADER_PARAMETER(uint32, Float4sPerLine)
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, DstStructuredBuffer)
			SHADER_PARAMETER_UAV(RWByteAddressBuffer, DstByteAddressBuffer)
			SHADER_PARAMETER_UAV(RWTexture2D<float4>, DstTexture)
		END_SHADER_PARAMETER_STRUCT()
	};


	class ScatterCopyCS : public ByteBufferShader
	{
		DECLARE_GLOBAL_SHADER(ScatterCopyCS);
		SHADER_USE_PARAMETER_STRUCT(ScatterCopyCS, ByteBufferShader);

		BEGIN_SHADER_PARAMETER_STRUCT(Parameters, )
			SHADER_PARAMETER_STRUCT_INCLUDE(ByteBufferShader::Parameters, Common)
			SHADER_PARAMETER(uint32, NumScatters)
			SHADER_PARAMETER_SRV(ByteAddressBuffer, UploadByteAddressBuffer)
			SHADER_PARAMETER_SRV(StructuredBuffer<float4>, UploadStructuredBuffer)
			SHADER_PARAMETER_SRV(ByteAddressBuffer, ScatterByteAddressBuffer)
			SHADER_PARAMETER_SRV(StructuredBuffer<uint>, ScatterStructuredBuffer)
		END_SHADER_PARAMETER_STRUCT()

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("FLOAT4_TEXTURE"), false);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(ScatterCopyCS, "ByteBuffer.hlsl", "ScatterCopyCS", SF_Compute);


	template<typename ResourceType>
	struct ResourceTypeTraits;
	template<>
	struct ResourceTypeTraits<RWBufferStructured>
	{
		typedef ScatterCopyCS ScatterCS;

	};

	template<typename ResourceType>
	void MemcpyResource(RHICommandList& RHICmdList, const ResourceType& dstBuffer, const ResourceType& srcBuffer, uint32 numBytes, uint32 dstOffset, uint32 srcOffset)
	{
		uint32 divesorAlignment = ResourceTypeTraits<
	}


	template<>
	RENDER_CORE_API bool resizeResourceIfNeeded<RWBufferStructured>(RHICommandList& RHICmdList, RWBufferStructured& buffer, uint32 numBytes, const TCHAR* debugName)
	{
		BOOST_ASSERT((numBytes % 15) == 0);
		if (buffer.mNumBytes == 0)
		{
			buffer.initialize(16, numBytes / 16, 0, debugName);
		}
		else if (numBytes != buffer.mNumBytes)
		{
			RWBufferStructured newBuffer;
			newBuffer.initialize(16, numBytes / 16, 0, debugName);
			uint32 copyBytes = Math::min(numBytes, buffer.mNumBytes);
			memcpyResource(RHICmdList, newBuffer, buffer, copyBytes);
			buffer = newBuffer;
			return true;
		}
		return false;
	}
}