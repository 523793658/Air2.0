#include "D3D11DynamicRHI.h"
#include "D3D11DynamicRHI.h"
namespace Air
{
	StructuredBufferRHIRef D3D11DynamicRHI::RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		BOOST_ASSERT(size > 0);
		BOOST_ASSERT(size / stride > 0 && size % stride == 0);
		D3D11_BUFFER_DESC desc;
		::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.ByteWidth = size;
		desc.BindFlags = 0;
		desc.Usage = (inUsage & BUF_AnyDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

		if (inUsage & BUF_ShaderResource)
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (inUsage |= BUF_UnorderedAccess)
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		if (inUsage |= BUF_StreamOutput)
		{
			desc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		}
		desc.CPUAccessFlags = (inUsage & BUF_AnyDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.MiscFlags = 0;
		if (inUsage & BUF_DrawIndirect)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}
		else
		{
			if (inUsage & BUF_ByteAddressBuffer)
			{
				desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			}
			else
			{
				desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			}
		}
		desc.StructureByteStride = stride;

		if (PlatformMemory::supportsFastVRAMemory())
		{
			if (inUsage & BUF_FastVRAM)
			{
				FastVRAMAllocator::getFastVRAMAllocator()->allocUAVBuffer(desc);
			}
		}

		D3D11_SUBRESOURCE_DATA initData;
		D3D11_SUBRESOURCE_DATA* pInitData = NULL;

		if (createInfo.mResourceArray)
		{
			BOOST_ASSERT(size == createInfo.mResourceArray->getResourceDataSize());
			initData.pSysMem = createInfo.mResourceArray->getResourceData();
			initData.SysMemPitch = size;
			initData.SysMemSlicePitch = 0;
			pInitData = &initData;
		}

		TRefCountPtr<ID3D11Buffer> structuredBufferResource;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&desc, pInitData, structuredBufferResource.getInitReference()), mD3D11Device);
		if (createInfo.mDebugName)
		{
			structuredBufferResource->SetPrivateData(WKPDID_D3DDebugObjectName, CString::strlen(createInfo.mDebugName) + 1, TCHAR_TO_ANSI(createInfo.mDebugName));
		}

		updateBufferStats(structuredBufferResource, true);
		if (createInfo.mResourceArray)
		{
			createInfo.mResourceArray->discard();
		}
		return new D3D11StructuredBuffer(structuredBufferResource, stride, size, inUsage);

	}
}