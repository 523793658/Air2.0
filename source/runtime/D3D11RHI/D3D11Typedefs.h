#ifndef _Air_D3D11_Typedefs_H_
#define _Air_D3D11_Typedefs_H_
#pragma once
#include <CoreMinimal.h>
#include <dxgi1_5.h>
#include <d3d11_3.h>
#include <memory>
#include "Template/RefCounting.h"
#include "dxgiformat.h"
namespace Air
{
	typedef TRefCountPtr<IDXGIFactory1>				IDXGIFactory1Ptr;
	typedef TRefCountPtr<IDXGIFactory2>				IDXGIFactory2Ptr;
	typedef TRefCountPtr<IDXGIFactory3>				IDXGIFactory3Ptr;
	typedef TRefCountPtr<IDXGIFactory4>				IDXGIFactory4Ptr;
	typedef TRefCountPtr<IDXGIFactory5>				IDXGIFactory5Ptr;

	typedef TRefCountPtr<IDXGIAdapter>				IDXGIAdapterPtr;
	typedef TRefCountPtr<IDXGIAdapter1>				IDXGIAdapter1Ptr;
	typedef TRefCountPtr<IDXGIAdapter2>				IDXGIAdapter2Ptr;

	typedef TRefCountPtr<IDXGISwapChain>				IDXGISwapChainPtr;
	typedef TRefCountPtr<IDXGISwapChain1>			IDXGISwapChain1Ptr;
	typedef TRefCountPtr<IDXGISwapChain2>			IDXGISwapChain2Ptr;
	typedef TRefCountPtr<IDXGISwapChain3>			IDXGISwapChain3Ptr;
	typedef TRefCountPtr<IDXGISwapChain4>			IDXGISwapChain4Ptr;

	typedef TRefCountPtr<ID3D11Device>				ID3D11DevicePtr;
	typedef TRefCountPtr<ID3D11Device1>				ID3D11Device1Ptr;
	typedef TRefCountPtr<ID3D11Device2>				ID3D11Device2Ptr;
	typedef TRefCountPtr<ID3D11Device3>				ID3D11Device3Ptr;

	typedef TRefCountPtr<ID3D11DeviceContext>		ID3D11DeviceContextPtr;
	typedef TRefCountPtr<ID3D11DeviceContext1>		ID3D11DeviceContext1Ptr;
	typedef TRefCountPtr<ID3D11DeviceContext2>		ID3D11DeviceContext2Ptr;
	typedef TRefCountPtr<ID3D11DeviceContext3>		ID3D11DeviceContext3Ptr;

	typedef TRefCountPtr<ID3D11Resource>				ID3D11ResourcePtr;

	typedef TRefCountPtr<ID3D11Texture1D>			ID3D11Texture1DPtr;
	typedef TRefCountPtr<ID3D11Texture2D>			ID3D11Texture2DPtr;
	typedef TRefCountPtr<ID3D11Texture3D>			ID3D11Texture3DPtr;
	typedef TRefCountPtr<ID3D11Texture2D>			ID3D11TextureCubePtr;

	typedef TRefCountPtr<ID3D11Buffer>				ID3D11BufferPtr;
	typedef TRefCountPtr<ID3D11InputLayout>			ID3D11InputLayoutPtr;

	typedef TRefCountPtr<ID3D11Query>				ID3D11QueryPtr;
	typedef TRefCountPtr<ID3D11Predicate>			ID3D11PredicatePtr;
	typedef TRefCountPtr<ID3D11VertexShader>			ID3D11VertexShaderPtr;
	typedef TRefCountPtr<ID3D11PixelShader>			ID3D11PixelShaderPtr;
	typedef TRefCountPtr<ID3D11GeometryShader>		ID3D11GeometryShaderPtr;
	typedef TRefCountPtr<ID3D11ComputeShader>		ID3D11ComputeShaderPtr;
	typedef TRefCountPtr<ID3D11HullShader>			ID3D11HullShaderPtr;
	typedef TRefCountPtr<ID3D11DomainShader>			ID3D11DomainShaderPtr;
	typedef TRefCountPtr<ID3D11RenderTargetView>		ID3D11RenderTargetViewPtr;
	typedef TRefCountPtr<ID3D11DepthStencilView>		ID3D11DepthStencilViewPtr;
	typedef TRefCountPtr<ID3D11UnorderedAccessView>	ID3D11UnorderedAccessViewPtr;
	typedef TRefCountPtr<ID3D11RasterizerState>		ID3D11RasterizerStatePtr;
	typedef TRefCountPtr<ID3D11RasterizerState1>		ID3D11RasterizerState1Ptr;
	typedef TRefCountPtr<ID3D11DepthStencilState>	ID3D11DepthStencilStatePtr;
	typedef TRefCountPtr<ID3D11BlendState>			ID3D11BlendStatePtr;
	typedef TRefCountPtr<ID3D11BlendState1>			ID3D11BlendState1Ptr;
	typedef TRefCountPtr<ID3D11SamplerState>			ID3D11SamplerStatePtr;
	typedef TRefCountPtr<ID3D11ShaderResourceView>	ID3D11ShaderResourceViewPtr;




#undef min
#undef max







}




#endif