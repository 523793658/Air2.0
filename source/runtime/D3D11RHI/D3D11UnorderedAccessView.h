#pragma once
#include "D3D11RHI.h"
namespace Air
{
	class D3D11BaseShaderResource;

	class D3D11UnorderedAccessView : public RHIUnorderedAccessView
	{
	public:
		TRefCountPtr<ID3D11UnorderedAccessView> mView;
		TRefCountPtr<D3D11BaseShaderResource> mResource;

		void* mIHVResourceHandle;

		D3D11UnorderedAccessView(ID3D11UnorderedAccessView* inView, D3D11BaseShaderResource* inResource)
			:mView(inView)
			,mResource(inResource)
			,mIHVResourceHandle(nullptr)
		{}
	};

	DECL_D3D11_RESOURCE_TRAITS(D3D11UnorderedAccessView, RHIUnorderedAccessView)
}