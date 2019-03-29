#pragma once
#include "RHIResource.h"
#include "D3D11Typedefs.h"
#include "D3D11Resource.h"
namespace Air
{
	class D3D11SamplerState : public RHISamplerState
	{
	public:
		TRefCountPtr<ID3D11SamplerState> mResource;
	};

	class D3D11BlendState : public RHIBlendState
	{
	public:
		TRefCountPtr<ID3D11BlendState> mResource;
	};

	class D3D11RasterizerState : public RHIRasterizerState
	{
	public:
		TRefCountPtr<ID3D11RasterizerState> mResource;
	};

	class D3D11DepthStencilState : public RHIDepthStencilState
	{
	public:
		TRefCountPtr<ID3D11DepthStencilState> mResource;

		FExclusiveDepthStencil mAccessType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIBlendState>
	{
		typedef D3D11BlendState TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIRasterizerState>
	{
		typedef D3D11RasterizerState TConcreteType;
	};

	template<>
	struct TD3D11ResourceTraits<RHIDepthStencilState>
	{
		typedef D3D11DepthStencilState TConcreteType;
	};


	template<>
	struct TD3D11ResourceTraits<RHISamplerState>
	{
		typedef D3D11SamplerState TConcreteType;
	};


}