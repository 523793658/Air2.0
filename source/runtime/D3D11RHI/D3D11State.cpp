#include "D3D11State.h"
#include "D3D11DynamicRHI.h"
#include "D3D11Util.h"
#include "Containers/Map.h"

namespace Air
{
	static TMap<ID3D11SamplerState*, D3D11SamplerState*> GSamplerStateCache;


	static D3D11_TEXTURE_ADDRESS_MODE translateAddressMode(ESamplerAddressMode mode)
	{
		switch (mode)
		{
		case Air::AM_Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case Air::AM_Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case Air::AM_Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		default:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	static D3D11_COMPARISON_FUNC translateSamplerCompareFunction(ESamplerCompareFunction func)
	{
		switch (func)
		{
		case Air::SCF_Less:
			return D3D11_COMPARISON_LESS;
		case Air::SCF_Never:
		default:
			return D3D11_COMPARISON_NEVER;
		}
	}

	static D3D11_CULL_MODE translateCullMode(ERasterizerCullMode mode)
	{
		switch (mode)
		{
		case Air::CM_CCW:
			return D3D11_CULL_FRONT;

		case Air::CM_CW:
			return D3D11_CULL_BACK;
		default: 
			return D3D11_CULL_NONE;
		}
	}

	static D3D11_BLEND_OP translateBlendOption(EBlendOperation op)
	{
		switch (op)
		{
		case Air::BO_Subtract:
			return D3D11_BLEND_OP_SUBTRACT;
		case Air::BO_Min:
			return D3D11_BLEND_OP_MIN;
		case Air::BO_Max:
			return D3D11_BLEND_OP_MAX;
		case Air::BO_ReverseSubtract:
			return D3D11_BLEND_OP_REV_SUBTRACT;
		default:
			return D3D11_BLEND_OP_ADD;
		}
	}

	static D3D11_BLEND translateBlendFactor(EBlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case Air::BF_Zero:
			return D3D11_BLEND_ZERO;
		case Air::BF_One:
			return D3D11_BLEND_ONE;
		case Air::BF_SourceColor:
			return D3D11_BLEND_SRC_COLOR;
		case Air::BF_InverseSourceColor:
			return D3D11_BLEND_INV_SRC_COLOR;
		case Air::BF_SourceAlpha:
			return D3D11_BLEND_SRC_ALPHA;
		case Air::BF_InverseSourceAlpha:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case Air::BF_DestAlpha:
			return D3D11_BLEND_DEST_ALPHA;
		case Air::BF_InverseDestAlpha:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case Air::BF_DestColor:
			return D3D11_BLEND_DEST_COLOR;
		case Air::BF_InverseDestColor:
			return D3D11_BLEND_INV_DEST_COLOR;
		case Air::BF_constantBlendFactor:
			return D3D11_BLEND_BLEND_FACTOR;
		case Air::BF_InverseConstantBlendFactor:
			return D3D11_BLEND_INV_BLEND_FACTOR;
		default:
			return D3D11_BLEND_ZERO;
		}
	}

	static D3D11_FILL_MODE translateFillMode(ERasterizerFillMode mode)
	{
		switch (mode)
		{
		case Air::FM_Wireframe:
			return D3D11_FILL_WIREFRAME;
		default:
			return D3D11_FILL_SOLID;
		}
	}

	static D3D11_STENCIL_OP translateStencilOp(EStencilOp op)
	{
		switch (op)
		{
		
		case Air::SO_Zero:
			return D3D11_STENCIL_OP_ZERO;
		case Air::SO_Replace:
			return D3D11_STENCIL_OP_REPLACE;
		case Air::SO_SaturatedIncrement:
			return D3D11_STENCIL_OP_INCR_SAT;
		case Air::SO_SaturatedDecrement:
			return D3D11_STENCIL_OP_DECR_SAT;
		case Air::SO_Invert:
			return D3D11_STENCIL_OP_INVERT;
		case Air::SO_Increment:
			return D3D11_STENCIL_OP_INCR;
		case Air::SO_Decrement:
			return D3D11_STENCIL_OP_DECR;
		default:
			return D3D11_STENCIL_OP_KEEP;
		}
	}

	static D3D11_COMPARISON_FUNC translateCompareFunction(ECompareFunction func)
	{
		switch (func)
		{
		case Air::CF_Less:
			return D3D11_COMPARISON_LESS;
		case Air::CF_LessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;
		case Air::CF_Greater:
			return D3D11_COMPARISON_GREATER;
		case Air::CF_GreateEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case Air::CF_Equal:
			return D3D11_COMPARISON_EQUAL;
		case Air::CF_NotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;
		case Air::CF_Never:
			return D3D11_COMPARISON_NEVER;
		default:
			return D3D11_COMPARISON_ALWAYS;
		}
	}

	SamplerStateRHIRef D3D11DynamicRHI::RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI)
	{
		D3D11_SAMPLER_DESC desc;
		Memory::memzero(desc);
		desc.AddressU = translateAddressMode(inInitializerRHI.mAddressU);
		desc.AddressV = translateAddressMode(inInitializerRHI.mAddressV);
		desc.AddressW = translateAddressMode(inInitializerRHI.mAddressW);
		const LinearColor linearBorderColor = Color(inInitializerRHI.mBorderColor);
		desc.BorderColor[0] = linearBorderColor.R;
		desc.BorderColor[1] = linearBorderColor.G;
		desc.BorderColor[2] = linearBorderColor.B;
		desc.BorderColor[3] = linearBorderColor.A;

		desc.ComparisonFunc = translateSamplerCompareFunction(inInitializerRHI.mSamplerComparisonFunction);

		const bool bComparisonEnable = inInitializerRHI.mSamplerComparisonFunction != SCF_Never;

		switch (inInitializerRHI.mFilter)
		{
		case SF_AnisotropicLinear:
		case SF_AnisotropicPoint:
			if (desc.MaxAnisotropy == 1)
			{
				desc.Filter = bComparisonEnable ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR :
					D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				desc.Filter = bComparisonEnable ? D3D11_FILTER_COMPARISON_ANISOTROPIC : D3D11_FILTER_ANISOTROPIC;
			}
			break;
		case SF_Trilinear:
			desc.Filter = bComparisonEnable ?
				D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR :
				D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;

		case SF_Bilinear:
			desc.Filter = bComparisonEnable ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT :
				D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case SF_Point:
			desc.Filter = bComparisonEnable ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT :
				D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		}

		desc.MaxAnisotropy = inInitializerRHI.mMaxAnisotropy;
		desc.MaxLOD = inInitializerRHI.mMaxMipLevel;
		desc.MinLOD = inInitializerRHI.mMinMipLevel;
		desc.MipLODBias = inInitializerRHI.mMipBias;
		TRefCountPtr<ID3D11SamplerState> samplerStateHandle;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateSamplerState(&desc, samplerStateHandle.getInitReference()), mD3D11Device);
		auto found = GSamplerStateCache.find(samplerStateHandle);
		if (found != GSamplerStateCache.end())
		{
			return found->second;
		}

		D3D11SamplerState* samplerState = new D3D11SamplerState();
		samplerState->mResource = samplerStateHandle;
		samplerState->AddRef();
		GSamplerStateCache.emplace(samplerStateHandle.getReference(), samplerState);
		return samplerState;
	}

	RasterizerStateRHIRef D3D11DynamicRHI::RHICreateRasterizerState(const RasterizerStateInitializerRHI& inInitializerRHI)
	{
		D3D11_RASTERIZER_DESC desc;
		Memory::memzero(desc);
		desc.CullMode = translateCullMode(inInitializerRHI.mCullMode);
		desc.FillMode = translateFillMode(inInitializerRHI.mFillMode);
		desc.DepthBias = floorl(inInitializerRHI.mDepthBias * (float)(1 << 24));
		desc.SlopeScaledDepthBias = inInitializerRHI.mSlopeScaleDepthBias;
		desc.FrontCounterClockwise = true;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = inInitializerRHI.bAllowMSAA;
		desc.ScissorEnable = true;
		D3D11RasterizerState* rasterizerState = new D3D11RasterizerState();
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateRasterizerState(&desc, rasterizerState->mResource.getInitReference()), mD3D11Device);
		return rasterizerState;
	}

	BlendStateRHIRef D3D11DynamicRHI::RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI)
	{
		D3D11_BLEND_DESC desc;
		Memory::memzero(desc);
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = inInitializerRHI.bUseIndependentRenderTargetBlendStates;
		static_assert(MaxSimultaneousRenderTargets <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "Too many MRTs");
		for (uint32 renderTargetIndex = 0; renderTargetIndex < MaxSimultaneousRenderTargets; ++renderTargetIndex)
		{
			const BlendStateInitializerRHI::RenderTarget& renderTargetInitializer = inInitializerRHI.mRenderTargets[renderTargetIndex];
			D3D11_RENDER_TARGET_BLEND_DESC& renderTarget = desc.RenderTarget[renderTargetIndex];
			renderTarget.BlendEnable = renderTargetInitializer.mColorBlendOp != BO_Add ||
				renderTargetInitializer.mColorDstBlend != BF_Zero ||
				renderTargetInitializer.mColorSrcBlend != BF_One ||
				renderTargetInitializer.mAlphaBlendOp != BO_Add ||
				renderTargetInitializer.mAlphaDstBlend != BF_Zero ||
				renderTargetInitializer.mAlphaSrcBlend != BF_One;
			renderTarget.BlendOp = translateBlendOption(renderTargetInitializer.mColorBlendOp);
			renderTarget.SrcBlend = translateBlendFactor(renderTargetInitializer.mColorSrcBlend);
			renderTarget.DestBlend = translateBlendFactor(renderTargetInitializer.mColorDstBlend);

			renderTarget.BlendOpAlpha = translateBlendOption(renderTargetInitializer.mAlphaBlendOp);
			renderTarget.DestBlendAlpha = translateBlendFactor(renderTargetInitializer.mAlphaDstBlend);
			renderTarget.SrcBlendAlpha = translateBlendFactor(renderTargetInitializer.mAlphaSrcBlend);
			renderTarget.RenderTargetWriteMask = ((renderTargetInitializer.mColorWriteMask & CW_RED) ? D3D11_COLOR_WRITE_ENABLE_RED : 0) | ((renderTargetInitializer.mColorWriteMask & CW_GREEN) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) | ((renderTargetInitializer.mColorWriteMask & CW_BLUE) ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) | ((renderTargetInitializer.mColorWriteMask & CW_ALPHA) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
		}
		D3D11BlendState* blendState = new D3D11BlendState();
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateBlendState(&desc, blendState->mResource.getInitReference()), mD3D11Device);
		return blendState;
	}

	DepthStencilStateRHIRef D3D11DynamicRHI::RHICreateDepthStencilState(const DepthStencilStateInitializerRHI& inInitializerRHI)
	{
		D3D11DepthStencilState* depthStencilState = new D3D11DepthStencilState;

		D3D11_DEPTH_STENCIL_DESC desc;
		Memory::memzero(desc);
		desc.DepthEnable = inInitializerRHI.mDepthTest != CF_Always || inInitializerRHI.bEnableDepthWrite;
		desc.DepthWriteMask = inInitializerRHI.bEnableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = translateCompareFunction(inInitializerRHI.mDepthTest);

		desc.StencilEnable = inInitializerRHI.bEnableFrontFaceStencil || inInitializerRHI.bEnableBackFaceStencil;
		desc.StencilReadMask = inInitializerRHI.mStencilReadMask;
		desc.StencilWriteMask = inInitializerRHI.mStencilWriteMask;
		desc.FrontFace.StencilFunc = translateCompareFunction(inInitializerRHI.mFrontFaceStencilTest);
		desc.FrontFace.StencilFailOp = translateStencilOp(inInitializerRHI.mFrontFaceStencilFailStencilOp);
		desc.FrontFace.StencilDepthFailOp = translateStencilOp(inInitializerRHI.mFrontFaceDepthFailStencilOp);
		desc.FrontFace.StencilPassOp = translateStencilOp(inInitializerRHI.mFrontFacePassStencilOp);

		if (inInitializerRHI.bEnableBackFaceStencil)
		{

			desc.BackFace.StencilDepthFailOp = translateStencilOp(inInitializerRHI.mBackFaceDepthFailStencilOp);
			desc.BackFace.StencilFailOp = translateStencilOp(inInitializerRHI.mBackFaceStencilFailStencilOp);
			desc.BackFace.StencilFunc = translateCompareFunction(inInitializerRHI.mBackFaceStencilTest);
			desc.BackFace.StencilPassOp = translateStencilOp(inInitializerRHI.mBackFacePassStencilOp);
		}
		else
		{
			desc.BackFace = desc.FrontFace;
		}

		const bool bStencilOpIsKeep = inInitializerRHI.mFrontFaceStencilFailStencilOp == SO_Keep && inInitializerRHI.mFrontFaceDepthFailStencilOp == SO_Keep&& inInitializerRHI.mFrontFacePassStencilOp == SO_Keep && inInitializerRHI.mBackFaceDepthFailStencilOp == SO_Keep&& inInitializerRHI.mBackFaceStencilFailStencilOp == SO_Keep && inInitializerRHI.mBackFacePassStencilOp == SO_Keep;

		const bool bMayWriteStencil = inInitializerRHI.mStencilWriteMask != 0 && !bStencilOpIsKeep;
		depthStencilState->mAccessType.SetDepthStencilWrite(inInitializerRHI.bEnableDepthWrite, bMayWriteStencil);
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateDepthStencilState(&desc, depthStencilState->mResource.getInitReference()), mD3D11Device);
		return depthStencilState;
	}

	void D3D11DynamicRHI::RHISetBlendState(RHIBlendState* newStateRHI, const LinearColor& blendFactory)
	{
		D3D11BlendState* newState = ResourceCast(newStateRHI);
		mStateCache.setBlendState(newState->mResource, (const float*)&blendFactory, 0xffffffff);
	}

	void D3D11DynamicRHI::RHISetDepthStencilState(RHIDepthStencilState* newStateRHI, uint32 inStencilRef)
	{
		D3D11DepthStencilState* newState = ResourceCast(newStateRHI);
		mStateCache.setDepthStencilState(newState->mResource, inStencilRef);
	}

	void D3D11DynamicRHI::RHISetRasterizerState(RHIRasterizerState* newStateRHI)
	{
		D3D11RasterizerState* newState = ResourceCast(newStateRHI);
		mStateCache.setRasterizerState(newState->mResource);
	}


}