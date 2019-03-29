#include "D3D11DynamicRHI.h"
#include <limits>
namespace Air
{
	struct D3D11VertexDeclarationKey 
	{
		D3D11VertexElements mVertexElements;
		uint32 mHash;
		explicit D3D11VertexDeclarationKey(const VertexDeclarationElementList& inElements)
		{
			for (int32 elementIndex = 0; elementIndex < inElements.size(); elementIndex++)
			{
				const VertexElement& element = inElements[elementIndex];
				D3D11_INPUT_ELEMENT_DESC d3dElement = { 0 };
				d3dElement.InputSlot = element.mStreamIndex;
				d3dElement.SemanticName = "ATTRIBUTE";
				d3dElement.SemanticIndex = element.mAttributeIndex;
				d3dElement.AlignedByteOffset = element.mOffset;
				d3dElement.InputSlotClass = element.bUseInstanceIndex ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;

				d3dElement.InstanceDataStepRate = element.bUseInstanceIndex ? 1 : 0;

				switch (element.mType)
				{
				case VET_Float1: d3dElement.Format = DXGI_FORMAT_R32_FLOAT; break;
				case VET_Float2: d3dElement.Format = DXGI_FORMAT_R32G32_FLOAT; break;
				case VET_Float3: d3dElement.Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
				case VET_Float4: d3dElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				case VET_PackedNormal: d3dElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				case VET_UByte4: d3dElement.Format = DXGI_FORMAT_R8G8B8A8_UINT; break;
				case VET_UByte4N: d3dElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				case VET_Color: d3dElement.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				case VET_Short2: d3dElement.Format = DXGI_FORMAT_R16G16_SINT; break;
				case VET_Short2N: d3dElement.Format = DXGI_FORMAT_R16G16_SNORM; break;
				case VET_Short4: d3dElement.Format = DXGI_FORMAT_R16G16B16A16_SINT; break;
				case VET_Short4N: d3dElement.Format = DXGI_FORMAT_R16G16B16A16_SNORM; break;
				case VET_Half2: d3dElement.Format = DXGI_FORMAT_R16G16_FLOAT; break;
				case VET_Half4: d3dElement.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
				case VET_UShort2: d3dElement.Format = DXGI_FORMAT_R16G16_UINT; break;
				case VET_UShort2N: d3dElement.Format = DXGI_FORMAT_R16G16_UNORM; break;
				case VET_UShort4: d3dElement.Format = DXGI_FORMAT_R16G16B16A16_UINT; break;
				case VET_UShort4N: d3dElement.Format = DXGI_FORMAT_R16G16B16A16_UNORM; break;
				case VET_URGB10A2N: d3dElement.Format = DXGI_FORMAT_R10G10B10A2_UNORM; break;
				default:
					AIR_LOG(LogD3D11RHI, Fatal, TEXT("Unknown RHI vertex element type %u"), (uint8)inElements[elementIndex].mType);

					break;
				}
				mVertexElements.add(d3dElement);
			}
			struct CompareDesc
			{
				FORCEINLINE bool operator()(const D3D11_INPUT_ELEMENT_DESC& A, const D3D11_INPUT_ELEMENT_DESC& B) const
				{
					return ((int32)A.AlignedByteOffset + A.InputSlot * std::numeric_limits<uint16>::max()) < ((int32)B.AlignedByteOffset + B.InputSlot * std::numeric_limits<uint16>::max());
				}
			};
			Sorting::sort(mVertexElements.getData(), mVertexElements.size(), CompareDesc());
			mHash = Crc::memCrc_DEPRECATED(mVertexElements.getData(), mVertexElements.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
		}
	};

	uint32 getTypeHash(const D3D11VertexDeclarationKey& key)
	{
		return key.mHash;
	}

	bool operator == (const D3D11VertexDeclarationKey& A, const D3D11VertexDeclarationKey& B)
	{
		return A.mVertexElements == B.mVertexElements;
	}

	TMap<D3D11VertexDeclarationKey, VertexDeclarationRHIRef> GVertexDeclarationCache;





	VertexDeclarationRHIRef D3D11DynamicRHI::RHICreateVertexDeclaration(const VertexDeclarationElementList& elements)
	{
		D3D11VertexDeclarationKey key(elements);
		auto it = GVertexDeclarationCache.find(key);
		if (it == GVertexDeclarationCache.end())
		{
			it = GVertexDeclarationCache.emplace(key, new D3D11VertexDeclaration(key.mVertexElements)).first;
		}
		D3D11VertexDeclaration* d3d11VertexDeclaration = (D3D11VertexDeclaration*)it->second.getReference();
		BOOST_ASSERT(d3d11VertexDeclaration->mVertexElements == key.mVertexElements);
		return it->second;
	}
}