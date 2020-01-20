#pragma once
#include "D3D11RHI.h"
#include "RHIResource.h"
#include "D3D11Typedefs.h"
#include "BoundShaderStateCache.h"
#include "D3D11Typedefs.h"


namespace Air
{

	extern void updateBufferStats(TRefCountPtr<ID3D11Buffer> buffer, bool bAllocating);

	class D3D11BaseShaderResource : public IRefCountedObject
	{
	public:
		D3D11BaseShaderResource()
			:mCurrentGPUAccess(EResourceTransitionAccess::EReadable),
			mLastFrameWritten(-1),
			mDirty(false)
		{}
		void setCurrentGPUAccess(EResourceTransitionAccess access)
		{
			if (access == EResourceTransitionAccess::EReadable)
			{
				mDirty = false;
			}
			mCurrentGPUAccess = access;
		}

		EResourceTransitionAccess getCurrentGPUAccess() const
		{
			return mCurrentGPUAccess;
		}

		uint32 getLastFrameWritten()  const
		{
			return mLastFrameWritten;
		}

		void setDirty(bool bInDirty, uint32 mCurrentFrame);

		bool isDirty() const
		{
			return mDirty;
		}

	private:
		EResourceTransitionAccess mCurrentGPUAccess;
		uint32 mLastFrameWritten;
		bool mDirty;
	};

	struct RingAllocation
	{
		ID3D11Buffer* mBuffer;
		void* mDataPtr;
		uint32 mOffset;
		uint32 mSize;
		RingAllocation() : mBuffer(nullptr){}
		inline bool isValid() const { return mBuffer != nullptr; }
	};

	template<class T>
	struct TD3D11ResourceTraits
	{

	};

#define DECL_D3D11_RESOURCE_TRAITS(D3D11TypeName, RHITypeName) \
	template<>	\
	struct TD3D11ResourceTraits<RHITypeName>\
	{\
		typedef D3D11TypeName TConcreteType;\
	};

	typedef TArray<D3D11_INPUT_ELEMENT_DESC> D3D11VertexElements;
	
	class D3D11VertexDeclaration : public RHIVertexDeclaration
	{
	public:
		D3D11VertexElements mVertexElements;
		explicit D3D11VertexDeclaration(const D3D11VertexElements& inElements)
			:mVertexElements(inElements)
		{}
	};

	class D3D11VertexBuffer : public RHIVertexBuffer, public D3D11BaseShaderResource
	{
	public:	 
		TRefCountPtr<ID3D11Buffer> mResource;
		D3D11VertexBuffer(ID3D11Buffer* InResource, uint32 inSize, uint32 inUsage)
			:RHIVertexBuffer(inSize, inUsage)
			, mResource(InResource)
		{

		}

		virtual ~D3D11VertexBuffer()
		{
			updateBufferStats(mResource, false);
		}
		virtual uint32 AddRef() const
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const
		{
			return RHIResource::GetRefCount();
		}
	};

	class D3D11IndexBuffer : public RHIIndexBuffer, public D3D11BaseShaderResource
	{
	public:
		TRefCountPtr<ID3D11Buffer> mResource;
		D3D11IndexBuffer(ID3D11Buffer* inResource, uint32 inStride, uint32 inSize, uint32 inUsage)
			:RHIIndexBuffer(inStride, inSize, inUsage)
			,mResource(inResource)
		{}
		virtual ~D3D11IndexBuffer()
		{
			updateBufferStats(mResource, false);
		}

		virtual uint32 AddRef() const
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const
		{
			return RHIResource::GetRefCount();
		}
	};

	class D3D11StructuredBuffer : public RHIStructuredBuffer, public D3D11BaseShaderResource
	{
	public:
		TRefCountPtr<ID3D11Buffer> mResource;
		D3D11StructuredBuffer(ID3D11Buffer* inResource, uint32 inStride, uint32 inSize, uint32 inUsage)
			:RHIStructuredBuffer(inStride, inSize, inUsage)
			, mResource(inResource)
		{
			setCurrentGPUAccess(EResourceTransitionAccess::ERWBarrier);
		}

		virtual ~D3D11StructuredBuffer()
		{
			updateBufferStats(mResource, false);
		}

		virtual uint32 AddRef() const
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const
		{
			return RHIResource::GetRefCount();
		}
	};

	DECL_D3D11_RESOURCE_TRAITS(D3D11VertexDeclaration, RHIVertexDeclaration);

	DECL_D3D11_RESOURCE_TRAITS(D3D11VertexBuffer, RHIVertexBuffer);

	DECL_D3D11_RESOURCE_TRAITS(D3D11IndexBuffer, RHIIndexBuffer);

	DECL_D3D11_RESOURCE_TRAITS(D3D11StructuredBuffer, RHIStructuredBuffer);

}

namespace std
{
	template<>
	struct is_bytewise_comparable<D3D11_INPUT_ELEMENT_DESC>
	{
		enum { value = true };
	};

	
}