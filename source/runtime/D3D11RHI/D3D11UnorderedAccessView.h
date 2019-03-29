#pragma once
#include "D3D11RHI.h"
namespace Air
{
	class D3D11UnorderedAccessView : public RHIUnorderedAccessView
	{
	public:
		TRefCountPtr<ID3D11UnorderedAccessView> mView;
		TRefCountPtr<D3D11BaseShaderResource> mResource;
	};
}