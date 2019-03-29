#include "d3d11shader.h"
#include "D3D11DynamicRHI.h"
#include "Serialization/MemoryReader.h"
#include "D3D11Util.h"
#include "D3D11Resource.h"
#include "BoundShaderStateCache.h"
#include "D3D11ConstantBuffer.h"
namespace Air
{
	template<typename TShaderType>
	static inline void readShaderOptionalData(ShaderCodeReader& inShaderCode, TShaderType& outShader)
	{
		auto packedResourceCounts = inShaderCode.findOptionalData<ShaderCodePackedResourceCounts>();
		BOOST_ASSERT(packedResourceCounts);
		outShader.bShaderNeedsGlobalConstantBuffer = packedResourceCounts->bGlobalConstantBufferUsed;
		const ANSICHAR* ansName = inShaderCode.findOptionalData('n');
		outShader.mShaderName = ansName ? ansName : "";
		int32 constantBufferTableSize = 0;
		auto * constantBufferData = inShaderCode.findOptionalDataAndSize('u', constantBufferTableSize);
		if (constantBufferData && constantBufferTableSize > 0)
		{
			BufferReader CBReader((void*)constantBufferData, constantBufferTableSize, false);
			TArray<wstring> names;
			CBReader << names;
			BOOST_ASSERT(outShader.mConstantBuffers.size() == 0);
			for (int32 index = 0; index < names.size(); ++index)
			{
				outShader.mConstantBuffers.push_back(names[index]);
			}
		}
	}


	VertexShaderRHIRef D3D11DynamicRHI::RHICreateVertexShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);

		D3D11VertexShader* shader = new D3D11VertexShader();
	
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data() + offset;
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreateVertexShader((void*)codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		shader->mCode = code;
		shader->mOffset = offset;
		return shader;
	}

	HullShaderRHIRef D3D11DynamicRHI::RHICreateHullShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);
		D3D11HullShader* shader = new D3D11HullShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data();
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreateHullShader(codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		return shader;
	}

	DomainShaderRHIRef D3D11DynamicRHI::RHICreateDomainShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);
		D3D11DomainShader* shader = new D3D11DomainShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data();
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreateDomainShader(codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		return shader;
	}

	PixelShaderRHIRef D3D11DynamicRHI::RHICreatePixelShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);
		D3D11PixelShader* shader = new D3D11PixelShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data() + offset;
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreatePixelShader(codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		return shader;
	}



	ComputeShaderRHIRef D3D11DynamicRHI::RHICreateComputeShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);
		D3D11ComputeShader* shader = new D3D11ComputeShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data();
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreateComputeShader(codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		return shader;
	}

	GeometryShaderRHIRef D3D11DynamicRHI::RHICreateGeometryShader(const TArray<uint8>& code)
	{
		ShaderCodeReader shaderCode(code);
		D3D11GeometryShader* shader = new D3D11GeometryShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data();
		const size_t codeSize = shaderCode.getActualShaderCodeSize() - offset;
		readShaderOptionalData(shaderCode, *shader);
		VERIFYD3d11RESULT(mD3D11Device->CreateGeometryShader(codePtr, codeSize, nullptr, shader->mResource.getInitReference()));
		return shader;
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer ? buffer->mResource : NULL;
			mStateCache.setConstantBuffer<SF_Vertex>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Vertex][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Vertex] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(PixelShaderRHIParamRef pixelShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer->mResource;
			mStateCache.setConstantBuffer<SF_Pixel>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Pixel][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Pixel] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(HullShaderRHIParamRef pixelShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer->mResource;
			mStateCache.setConstantBuffer<SF_Hull>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Hull][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Hull] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(DomainShaderRHIParamRef domainShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer->mResource;
			mStateCache.setConstantBuffer<SF_Domain>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Domain][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Domain] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(GeometryShaderRHIParamRef pixelShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer->mResource;
			mStateCache.setConstantBuffer<SF_Geometry>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Geometry][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Geometry] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderConstantBuffer(ComputeShaderRHIParamRef computeShader, uint32 bufferIndex, ConstantBufferRHIParamRef bufferRHI)
	{
		D3D11ConstantBuffer* buffer = ResourceCast(bufferRHI);
		{
			ID3D11Buffer* constantBuffer = buffer->mResource;
			mStateCache.setConstantBuffer<SF_Compute>(constantBuffer, bufferIndex);
		}
		mBoundConstantBuffers[SF_Compute][bufferIndex] = bufferRHI;
		mDirtyConstantBuffers[SF_Compute] |= (1 << bufferIndex);
	}

	void D3D11DynamicRHI::RHISetShaderParameter(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mVSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}

	void D3D11DynamicRHI::RHISetShaderParameter(HullShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mHSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}

	void D3D11DynamicRHI::RHISetShaderParameter(DomainShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mDSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}

	void D3D11DynamicRHI::RHISetShaderParameter(GeometryShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mGSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}

	void D3D11DynamicRHI::RHISetShaderParameter(PixelShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mPSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}

	static DXGI_FORMAT getPlatformTextureResourceFormat(DXGI_FORMAT inFormat, uint32 inFlags);

	void D3D11DynamicRHI::RHISetShaderParameter(ComputeShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue)
	{
		mCSUnifomBuffers[bufferIndex]->updateUniform((const uint8*)newValue, baseIndex, numBytes);
	}


	GeometryShaderRHIRef D3D11DynamicRHI::RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream)
	{
		ShaderCodeReader shaderCode(code);
		D3D11GeometryShader* shader = new D3D11GeometryShader();
		MemoryReader ar(code, true);
		ar << shader->mShaderResourceTable;
		int32 offset = ar.tell();
		const uint8* codePtr = code.data() + offset;
		const size_t codeSize = shaderCode.getActualShaderCodeSize();
		uint32 d3dRasterizedStream = rasterizedStream;
		if (rasterizedStream == -1)
		{
			d3dRasterizedStream = D3D11_SO_NO_RASTERIZED_STREAM;
		}
		D3D11_SO_DECLARATION_ENTRY streamOutEntries[D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT];
		for (int32 entryIndex = 0; entryIndex < elementList.size(); entryIndex++)
		{
			streamOutEntries[entryIndex].Stream = elementList[entryIndex].mStream;
			streamOutEntries[entryIndex].SemanticName = elementList[entryIndex].mSemanticName;
			streamOutEntries[entryIndex].SemanticIndex = elementList[entryIndex].mSementicIndex;
			streamOutEntries[entryIndex].StartComponent = elementList[entryIndex].mStartComponent;
			streamOutEntries[entryIndex].ComponentCount = elementList[entryIndex].mComponentCount;
			streamOutEntries[entryIndex].OutputSlot = elementList[entryIndex].mOutputSlot;
		}
		VERIFYD3d11RESULT(mD3D11Device->CreateGeometryShaderWithStreamOutput(codePtr, codeSize, streamOutEntries, elementList.size(), strides, numStrides, d3dRasterizedStream, NULL, shader->mResource.getInitReference()));
		auto packedResourceCounts = shaderCode.findOptionalData<ShaderCodePackedResourceCounts>();
		BOOST_ASSERT(packedResourceCounts);
		shader->bShaderNeedsGlobalConstantBuffer = packedResourceCounts->bGlobalConstantBufferUsed;
		shader->mShaderName = shaderCode.findOptionalData('n');
		return shader;
	}


	BoundShaderStateRHIRef D3D11DynamicRHI::RHICreateBoundShaderState(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShaderRHI, HullShaderRHIParamRef hullShaderRHI, DomainShaderRHIParamRef domainShaderRHI, GeometryShaderRHIParamRef geometryShaderRHI, PixelShaderRHIParamRef pixelShaderRHI)
	{
		BOOST_ASSERT(isInRenderingThread());
		BOOST_ASSERT(GIsRHIInitialized && mD3D11Context);
		CachedBoundShaderStateLink* cachedBoundShaderStateLink = getCachedBoundShaderState(vertexDeclaration, vertexShaderRHI, pixelShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI);
		if (cachedBoundShaderStateLink)
		{
			return cachedBoundShaderStateLink->mBoundShaderState;
		}
		else
		{
			return new D3D11BoundShaderState(vertexDeclaration, vertexShaderRHI, pixelShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, mD3D11Device);
		}
	}

#define TEST_HR(t) {hr = t;}

	D3D11BoundShaderState::D3D11BoundShaderState(VertexDeclarationRHIParamRef inVertexDeclarationRHI, VertexShaderRHIParamRef inVertexShaderRHI, PixelShaderRHIParamRef inPixelShaderRHI, HullShaderRHIParamRef inHullShaderRHI, DomainShaderRHIParamRef inDomainShaderRHI, GeometryShaderRHIParamRef inGeometryShaderRHI, ID3D11Device* direct3DDevice)
		:mCachedLink(inVertexDeclarationRHI, inVertexShaderRHI, inPixelShaderRHI,inHullShaderRHI, inDomainShaderRHI, inGeometryShaderRHI, this)
	{
		D3D11VertexDeclaration* inVertexDeclaration = D3D11DynamicRHI::ResourceCast(inVertexDeclarationRHI);

		D3D11VertexShader* inVertexShader = D3D11DynamicRHI::ResourceCast(inVertexShaderRHI);

		D3D11PixelShader* inPixelShader = D3D11DynamicRHI::ResourceCast(inPixelShaderRHI);

		D3D11HullShader* inHullShader = D3D11DynamicRHI::ResourceCast(inHullShaderRHI);

		D3D11DomainShader* inDomainShader = D3D11DynamicRHI::ResourceCast(inDomainShaderRHI);

		D3D11GeometryShader* inGeometryShader = D3D11DynamicRHI::ResourceCast(inGeometryShaderRHI);

		D3D11_INPUT_ELEMENT_DESC nullInputElement;
		Memory::memzero(&nullInputElement, sizeof(D3D11_INPUT_ELEMENT_DESC));

		ShaderCodeReader vertexShaderCode(inVertexShader->mCode);
		HRESULT hr;
		TEST_HR(direct3DDevice->CreateInputLayout(inVertexDeclaration ? inVertexDeclaration->mVertexElements.data() : &nullInputElement, inVertexDeclaration ? inVertexDeclaration->mVertexElements.size() : 0, &inVertexShader->mCode[inVertexShader->mOffset], vertexShaderCode.getActualShaderCodeSize() - inVertexShader->mOffset, mInputLayout.getInitReference()), direct3DDevice);

		mVertexShader = inVertexShader->mResource;
		mPixelShader = inPixelShader->mResource;
		mHullShader = inHullShader ? inHullShader->mResource : NULL;
		mDomainShader = inDomainShader ? inDomainShader->mResource : NULL;
		mGeometryShader = inGeometryShader ? inGeometryShader->mResource : NULL;
		Memory::memzero(&bShaderNeedsGlobalUniformBuffer, sizeof(bShaderNeedsGlobalUniformBuffer));

		bShaderNeedsGlobalUniformBuffer[SF_Vertex] = inVertexShader->bShaderNeedsGlobalConstantBuffer;

		bShaderNeedsGlobalUniformBuffer[SF_Pixel] = inPixelShader->bShaderNeedsGlobalConstantBuffer;

		bShaderNeedsGlobalUniformBuffer[SF_Hull] = inHullShader ? inHullShader->bShaderNeedsGlobalConstantBuffer : false;

		bShaderNeedsGlobalUniformBuffer[SF_Domain] = inDomainShader ? inDomainShader->bShaderNeedsGlobalConstantBuffer : false;

		bShaderNeedsGlobalUniformBuffer[SF_Geometry] = inGeometryShader ? inGeometryShader->bShaderNeedsGlobalConstantBuffer : false;

		static_assert(ARRAY_COUNT(bShaderNeedsGlobalUniformBuffer) == SF_NumFrequencies, "EShaderFrequency size should match with array count of bShaderNeedsGlobalConstantBuffer.");
	}

	D3D11BoundShaderState::~D3D11BoundShaderState()
	{

	}
}