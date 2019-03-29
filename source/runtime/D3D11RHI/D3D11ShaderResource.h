#pragma once
#include "Template/RefCounting.h"
#include "D3D11Resource.h"

namespace Air
{


	class D3D11ShaderResourceView : public RHIShaderResourceView
	{
	public:
		TRefCountPtr<ID3D11ShaderResourceView> mView;
		TRefCountPtr<D3D11BaseShaderResource> mResource;

		D3D11ShaderResourceView(ID3D11ShaderResourceView* inView, D3D11BaseShaderResource* inResource)
			:mView(inView)
			,mResource(inResource)
		{}
	};
}