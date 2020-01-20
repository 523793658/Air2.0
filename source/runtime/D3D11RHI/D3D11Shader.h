#pragma once
#include "ShaderCore.h"
#include "D3D11Resource.h"
#include "D3D11Typedefs.h"
#include "ShaderParameterMap.h"
namespace Air
{
	struct D3D11ShaderResourceTable : public BaseShaderResourceTable
	{
		TArray<uint32> mTextureMap;
		friend bool operator == (const D3D11ShaderResourceTable& lhs, const D3D11ShaderResourceTable& rhs)
		{
			const BaseShaderResourceTable& baseA = lhs;
			const BaseShaderResourceTable& baseB = rhs;
			return baseB == baseA && (Memory::memcmp(lhs.mTextureMap.data(), rhs.mTextureMap.data(), lhs.mTextureMap.getTypeSize() * lhs.mTextureMap.size()) == 0);
 		}

	};

	inline Archive& operator << (Archive & ar, D3D11ShaderResourceTable& srt)
	{
		BaseShaderResourceTable& baseSRT = srt;
		ar << baseSRT;
		ar << srt.mTextureMap;
		return ar;
	}

	struct D3D11ShaderData
	{
		D3D11ShaderResourceTable mShaderResourceTable;
		TArray<wstring>			mConstantBuffers;
		bool					bShaderNeedsGlobalConstantBuffer;
	};



	class D3D11VertexShader : public RHIVertexShader, public D3D11ShaderData
	{
	public:
		enum { StaticFrequency = SF_Vertex };
		ID3D11VertexShaderPtr	mResource;
		TArray<uint8> mCode;

		int32 mOffset;
	};

	class D3D11HullShader : public RHIHullShader, public D3D11ShaderData
	{
	public:
		enum { StaticFrequency = SF_Hull };
		ID3D11HullShaderPtr	mResource;
	};

	class D3D11DomainShader : public RHIDomainShader, public D3D11ShaderData
	{
	public:
		enum { StaticFrequency = SF_Domain };
		ID3D11DomainShaderPtr mResource;
	};

	class D3D11GeometryShader : public RHIGeometryShader, public D3D11ShaderData
	{
	public:
		enum { StaticFrequency = SF_Geometry };
		ID3D11GeometryShaderPtr mResource;
	};

	class D3D11PixelShader : public RHIPixelShader, public D3D11ShaderData
	{
	public:
		enum { StaticFrequency = SF_Pixel };
		ID3D11PixelShaderPtr mResource;
	};

	class D3D11ComputeShader : public RHIComputeShader, public D3D11ShaderData
	{
	public:
		enum{ StaticFrequency = SF_Compute };
		ID3D11ComputeShaderPtr mResource;
	};


	template<>
	struct TD3D11ResourceTraits<RHIVertexShader>
	{
		typedef D3D11VertexShader TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIPixelShader>
	{
		typedef D3D11PixelShader TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIHullShader>
	{
		typedef D3D11HullShader TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIDomainShader>
	{
		typedef D3D11DomainShader TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIGeometryShader>
	{
		typedef D3D11GeometryShader TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIComputeShader>
	{
		typedef D3D11ComputeShader TConcreteType;
	};

	class D3D11BoundShaderState : public RHIBoundShaderState
	{
	public:
		CachedBoundShaderStateLink mCachedLink;
		TRefCountPtr<ID3D11InputLayout> mInputLayout;
		TRefCountPtr<ID3D11VertexShader> mVertexShader;
		TRefCountPtr<ID3D11PixelShader> mPixelShader;
		TRefCountPtr<ID3D11HullShader> mHullShader;
		TRefCountPtr<ID3D11DomainShader> mDomainShader;
		TRefCountPtr<ID3D11GeometryShader> mGeometryShader;

		bool bShaderNeedsGlobalUniformBuffer[SF_NumFrequencies];
		D3D11BoundShaderState(
			RHIVertexDeclaration* inVertexDeclarationRHI,
			RHIVertexShader* inVertexShaderRHI,
			RHIPixelShader* inPixelShaderRHI,
			RHIHullShader* inHullShaderRHI,
			RHIDomainShader* inDomainShaderRHI,
			RHIGeometryShader* inGeometryShaderRHI,
			ID3D11Device* direct3DDevice
		);

		~D3D11BoundShaderState();

		FORCEINLINE D3D11VertexShader* getVertexShader() const { return (D3D11VertexShader*)mCachedLink.getVertexShader(); }

		FORCEINLINE D3D11PixelShader* getPixelShader() const { return (D3D11PixelShader*)mCachedLink.getPixelShader(); }

		FORCEINLINE D3D11HullShader* getHullShader() const { return (D3D11HullShader*)mCachedLink.getHullShader(); }

		FORCEINLINE D3D11DomainShader* getDomainShader() const { return (D3D11DomainShader*)mCachedLink.getDomainShader(); }

		FORCEINLINE D3D11GeometryShader* getGeometryShader() const { return (D3D11GeometryShader*)mCachedLink.getGeometryShader(); }



	};

	DECL_D3D11_RESOURCE_TRAITS(D3D11BoundShaderState, RHIBoundShaderState)
}