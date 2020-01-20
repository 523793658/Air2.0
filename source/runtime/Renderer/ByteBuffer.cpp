#include "ByteBuffer.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "RHIUtilities.h"
namespace Air
{
	class ScatterCopyCS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(ScatterCopyCS, Global)

		ScatterCopyCS() {}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return RHISupportsComputeShaders(parameters.mPlatform);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("THREADGROUP_SIZE"), ThreadGroupSize);
		}

	public:

		enum {ThreadGroupSize = 64};

		ShaderParameter	mNumScatters;
		ShaderResourceParameter mScatterBuffer;
		ShaderResourceParameter mUploadBuffer;
		ShaderResourceParameter mDstBuffer;

		ScatterCopyCS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mNumScatters.bind(initializer.mParameterMap, TEXT("NumScatters"));
			mScatterBuffer.bind(initializer.mParameterMap, TEXT("ScatterBuffer"));
			mUploadBuffer.bind(initializer.mParameterMap, TEXT("UploadBuffer"));
			mDstBuffer.bind(initializer.mParameterMap, TEXT("DstBuffer"));
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mNumScatters;
			ar << mScatterBuffer;
			ar << mUploadBuffer;
			ar << mDstBuffer;
			return bShaderHasOutdatedParameters;
		}
	};


	void ScatterUploadBuilder::uploadTo(RHICommandList& RHICmdList, RWBufferStructured& dstBuffer)
	{
		RHIUnlockVertexBuffer(mScatterBuffer.mBuffer);
		RHIUnlockVertexBuffer(mUploadBuffer.mBuffer);

		mScatterData = nullptr;
		mUploadData = nullptr;

		auto shaderMap = getGlobalShaderMap(GMaxRHIFeatureLevel);

		TShaderMapRef<ScatterCopyCS> computeShader(shaderMap);

		RHIComputeShader* shaderRHI = computeShader->getComputeShader();
		RHICmdList.setComputeShader(shaderRHI);

		setShaderValue(RHICmdList, shaderRHI, computeShader->mNumScatters, mNumScatters);
		setSRVParameter(RHICmdList, shaderRHI, computeShader->mScatterBuffer, mScatterBuffer.mSRV);
		setSRVParameter(RHICmdList, shaderRHI, computeShader->mUploadBuffer, mUploadBuffer.mSRV);
		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, dstBuffer.mUAV);

		RHICmdList.dispatchComputeShader(Math::divideAndRoundUp<uint32>(mNumScatters, ScatterCopyCS::ThreadGroupSize), 1, 1);

		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, UnorderedAccessViewRHIRef());
	}

	void ScatterUploadBuilder::uploadTo_Flush(RHICommandList& RHICmdList, RWBufferStructured& dstBuffer)
	{
		RHIUnlockVertexBuffer(mScatterBuffer.mBuffer);
		RHIUnlockVertexBuffer(mUploadBuffer.mBuffer);

		mScatterData = nullptr;
		mUploadData = nullptr;

		auto shaderMap = getGlobalShaderMap(GMaxRHIFeatureLevel);

		TShaderMapRef<ScatterCopyCS> computeShader(shaderMap);

		RHIComputeShader* shaderRHI = computeShader->getComputeShader();

		RHICmdList.setComputeShader(shaderRHI);
		setShaderValue(RHICmdList, shaderRHI, computeShader->mNumScatters, mNumScatters);
		setSRVParameter(RHICmdList, shaderRHI, computeShader->mScatterBuffer, mScatterBuffer.mSRV);
		setSRVParameter(RHICmdList, shaderRHI, computeShader->mUploadBuffer, mUploadBuffer.mSRV);
		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, dstBuffer.mUAV);

		RHICmdList.dispatchComputeShader(Math::divideAndRoundUp<int32>(mNumScatters, ScatterCopyCS::ThreadGroupSize), 1, 1);

		RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::DispatchToRHIThread);
		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, UnorderedAccessViewRHIRef());
	}

	class MemcpyBufferCS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(MemcpyBufferCS, Global)

		MemcpyBufferCS() {}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return RHISupportsComputeShaders(parameters.mPlatform);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("THREADGROUP_SIZE"), ThreadGroupSize);
		}
	public:
		enum { ThreadGroupSize = 64 };

		ShaderParameter			mSize;
		ShaderParameter			mSrcOffset;
		ShaderParameter			mDstOffset;
		ShaderResourceParameter	mSrcBuffer;
		ShaderResourceParameter	mDstBuffer;

		MemcpyBufferCS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mSize.bind(initializer.mParameterMap, TEXT("Size"));
			mSrcOffset.bind(initializer.mParameterMap, TEXT("SrcOffset"));
			mDstOffset.bind(initializer.mParameterMap, TEXT("DstOffset"));
			mSrcBuffer.bind(initializer.mParameterMap, TEXT("SrcBuffer"));
			mDstBuffer.bind(initializer.mParameterMap, TEXT("DstBuffer"));
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mSize;
			ar << mSrcOffset;
			ar << mDstOffset;
			ar << mSrcBuffer;
			ar << mDstBuffer;
			return b;

		}

	};

	IMPLEMENT_SHADER_TYPE(, MemcpyBufferCS, TEXT("ByteBuffer.hlsl"), TEXT("MemcpyBufferCS"), SF_Compute);




	void memcpyBuffer(RHICommandList& RHICmdList, const RWBufferStructured& srcBuffer, const RWBufferStructured& dstBuffer, uint32 numFloat4s, uint2 srcOffset /* = 0 */, uint2 dstOffset /* = 0 */)
	{
		auto shaderMap = getGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<MemcpyBufferCS> computeShader(shaderMap);

		RHIComputeShader* shaderRHI = computeShader->getComputeShader();
		RHICmdList.setComputeShader(shaderRHI);

		RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, EResourceTransitionPipeline::EGfxToCompute, dstBuffer.mUAV);

		setShaderValue(RHICmdList, shaderRHI, computeShader->mSrcOffset, srcOffset);
		setShaderValue(RHICmdList, shaderRHI, computeShader->mDstOffset, dstOffset);
		setShaderValue(RHICmdList, shaderRHI, computeShader->mSize, numFloat4s);
		setSRVParameter(RHICmdList, shaderRHI, computeShader->mSrcBuffer, srcBuffer.mSRV);
		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, dstBuffer.mUAV);

		RHICmdList.dispatchComputeShader(Math::divideAndRoundUp<uint32>(numFloat4s, MemcpyBufferCS::ThreadGroupSize), 1, 1);

		setUAVParameter(RHICmdList, shaderRHI, computeShader->mDstBuffer, UnorderedAccessViewRHIRef());

		RHICmdList.transitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, dstBuffer.mUAV);
	}
}