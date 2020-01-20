#include "D3D11DynamicRHI.h"
#include "RenderUtils.h"
#include "D3D11UnorderedAccessView.h"
#include "D3D11Texture.h"
namespace Air
{
	static void createD3D11ShaderResourceViewOnBuffer(ID3D11Device* device, ID3D11Buffer* buffer, uint32 stride, uint8 format, ID3D11ShaderResourceView** outSrv)
	{
		D3D11_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		Memory::memzero(viewDesc);
		viewDesc.Format = findShaderResourceDXGIFormat((DXGI_FORMAT)GPixelFormats[format].PlatformFormat, false);
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.NumElements = bufferDesc.ByteWidth / stride;

		HRESULT hr = device->CreateShaderResourceView(buffer, &viewDesc, outSrv);
		if (FAILED(hr))
		{
			if (hr == E_OUTOFMEMORY)
			{
				hr = device->CreateShaderResourceView(buffer, &viewDesc, outSrv);
			}
			if (FAILED(hr))
			{
				BOOST_ASSERT(false);
			}
		}
	}


	ShaderResourceViewRHIRef D3D11DynamicRHI::RHICreateShaderResourceView(RHIVertexBuffer* vertexBufferRHI, uint32 stride, uint8 format)
	{
		if (!vertexBufferRHI)
		{
			return new D3D11ShaderResourceView(nullptr, nullptr);
		}
		D3D11VertexBuffer* vertexBuffer = ResourceCast(vertexBufferRHI);
		BOOST_ASSERT(vertexBuffer);
		BOOST_ASSERT(vertexBuffer->mResource);

		TRefCountPtr<ID3D11ShaderResourceView> shaderResourceView;
		createD3D11ShaderResourceViewOnBuffer(mD3D11Device, vertexBuffer->mResource, stride, format, shaderResourceView.getInitReference());
		return new D3D11ShaderResourceView(shaderResourceView, vertexBuffer);
	}


	UnorderedAccessViewRHIRef D3D11DynamicRHI::RHICreateUnorderedAccessView(RHIIndexBuffer* indexBufferRHI, uint8 format)
	{
		D3D11IndexBuffer* indexBuffer = ResourceCast(indexBufferRHI);
		D3D11_BUFFER_DESC bufferDesc;
		indexBuffer->mResource->GetDesc(&bufferDesc);

		const bool bByteAccessBuffer = (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) != 0;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = findUnorderedAccessDXGIFormat((DXGI_FORMAT)GPixelFormats[format].PlatformFormat);
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / GPixelFormats[format].BlockBytes;

		if (bByteAccessBuffer)
		{
			uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}

		TRefCountPtr<ID3D11UnorderedAccessView> unorderedAccessView;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateUnorderedAccessView(indexBuffer->mResource, &uavDesc, unorderedAccessView.getInitReference()), mD3D11Device);
		return new D3D11UnorderedAccessView(unorderedAccessView, indexBuffer);
	}

	UnorderedAccessViewRHIRef D3D11DynamicRHI::RHICreateUnorderedAccessView(RHIStructuredBuffer* structuredBufferRHI, bool bUseUAVCounter, bool bAppendBuffer)
	{
		D3D11StructuredBuffer* structuredBuffer = ResourceCast(structuredBufferRHI);
		D3D11_BUFFER_DESC bufferDesc;
		structuredBuffer->mResource->GetDesc(&bufferDesc);
		const bool bByteAccessBuffer = (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) != 0;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;

		if (bufferDesc.BindFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS)
		{
			uavDesc.Format = DXGI_FORMAT_R32_UINT;
		}
		else if (bByteAccessBuffer)
		{
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}
		uavDesc.Buffer.FirstElement = 0;
		const uint32 effectiveStride = bufferDesc.StructureByteStride == 0 ? 4 : bufferDesc.StructureByteStride;
		uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / effectiveStride;
		uavDesc.Buffer.Flags = 0;
		if (bUseUAVCounter)
		{
			uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;
		}
		if (bAppendBuffer)
		{
			uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
		}
		if (bByteAccessBuffer)
		{
			uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
		}
		TRefCountPtr<ID3D11UnorderedAccessView> uav;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateUnorderedAccessView(structuredBuffer->mResource, &uavDesc, uav.getInitReference()), mD3D11Device);
		return new D3D11UnorderedAccessView(uav, structuredBuffer);
	}

	UnorderedAccessViewRHIRef D3D11DynamicRHI::RHICreateUnorderedAccessView(RHITexture* textureRHI, uint32 mipLevel)
	{
		D3D11TextureBase* texture = getD3D11TextureFromRHITexture(textureRHI);
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		if (textureRHI->getTexture3D() != nullptr)
		{
			
		}
		else if (textureRHI->getTexture2D() != nullptr)
		{
			D3D11TextureCube* textureCube = (D3D11TextureCube*)texture;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = 6;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.MipSlice = mipLevel;
		}
		else
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = mipLevel;
		}

		uavDesc.Format = findShaderResourceDXGIFormat((DXGI_FORMAT)GPixelFormats[textureRHI->getFormat()].PlatformFormat, false);
		TRefCountPtr<ID3D11UnorderedAccessView> uav;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateUnorderedAccessView(texture->getResource(), &uavDesc, uav.getInitReference()), mD3D11Device);
		return new D3D11UnorderedAccessView(uav, texture);
	}

	UnorderedAccessViewRHIRef D3D11DynamicRHI::RHICreateUnorderedAccessView(RHIVertexBuffer* vertexBufferRHI, uint8 format)
	{
		D3D11VertexBuffer* vertexBuffer = ResourceCast(vertexBufferRHI);
		D3D11_BUFFER_DESC bufferDesc;
		vertexBuffer->mResource->GetDesc(&bufferDesc);
		const bool bByteAccessBuffer = (bufferDesc.MiscFlags | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) != 0;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		
		uavDesc.Format = findUnorderedAccessDXGIFormat((DXGI_FORMAT)GPixelFormats[format].PlatformFormat);
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / GPixelFormats[format].BlockBytes;
		uavDesc.Buffer.Flags = 0;

		if (bByteAccessBuffer)
		{
			uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}
		TRefCountPtr<ID3D11UnorderedAccessView> uav;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateUnorderedAccessView(vertexBuffer->mResource, &uavDesc, uav.getInitReference()), mD3D11Device);
		return new D3D11UnorderedAccessView(uav, vertexBuffer);
	}

	ShaderResourceViewRHIRef D3D11DynamicRHI::RHICreateShaderResourceView(RHIStructuredBuffer* structuredBufferRHI)
	{
		D3D11StructuredBuffer* structuredBuffer = ResourceCast(structuredBufferRHI);
		D3D11_BUFFER_DESC bufferDesc;
		structuredBuffer->mResource->GetDesc(&bufferDesc);

		const bool bByteAccessBuffer = (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) != 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		if (bByteAccessBuffer)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / 4;
			srvDesc.BufferEx.FirstElement = 0;
			srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}
		else
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		}

		TRefCountPtr<ID3D11ShaderResourceView> srv;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateShaderResourceView(structuredBuffer->mResource, &srvDesc, srv.getInitReference()), mD3D11Device);
		return new D3D11ShaderResourceView(srv, structuredBuffer);
	}

}