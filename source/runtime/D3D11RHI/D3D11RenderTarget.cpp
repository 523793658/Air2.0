#include "D3D11DynamicRHI.h"
#include "ResolveShader.h"
#include "RenderUtils.h"
#include "RHIStaticStates.h"
#include "sceneRendering.h"
#include "ScreenRendering.h"
namespace Air
{
	static ResolveRect getDefaultRect(const ResolveRect& rect, uint32 defaultWidth, uint32 defaultHeight)
	{
		if (rect.X1 >= 0 && rect.X2 >= 0 && rect.Y1 >= 0 && rect.Y2 >= 0)
		{
			return rect;
		}
		else
		{
			return ResolveRect(0, 0, defaultWidth, defaultHeight);
		}
	}

	static inline DXGI_FORMAT convertTypelessToUnorm(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			default:
				return format;
		}
		
	}

	template<typename TPixelShader>
	void D3D11DynamicRHI::resolveTextureUsingShader(RHICommandList_RecursiveHazardous& RHICmdList, D3D11Texture2D* sourceTexture, D3D11Texture2D* destTexture, ID3D11RenderTargetView* destSurfaceRTV, ID3D11DepthStencilView* DestSurfaceDSV, const D3D11_TEXTURE2D_DESC& resolveTargetDesc, const ResolveRect& sourceRect, ResolveRect& destRect, ID3D11DeviceContext* direct3DDeviceContext, typename TPixelShader::Parameter pixelShaderParameter)
	{
		D3D11_VIEWPORT savedViewport;
		uint32 numSavedViewports = 1;
		mStateCache.getViewports(&numSavedViewports, &savedViewport);

		GraphicsPipelineStateInitializer graphicsPSOInit;
		RHICmdList.applyCachedRenderTargets(graphicsPSOInit);

		graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();
		graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();

		if (destTexture)
		{
			conditionalClearShaderResource(destTexture, false);
		}
		const bool bClearDestTexture = destRect.X1 == 0 && destRect.Y1 == 0 && destRect.X2 == resolveTargetDesc.Width && destRect.Y2 == resolveTargetDesc.Height;

		FExclusiveDepthStencil originalDSVAccessType = mCurrentDSVAccessType;

		TRefCountPtr<D3D11TextureBase> originalDepthTexture = mCurrentDepthTexture;

		if (resolveTargetDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			if (bClearDestTexture)
			{
				mD3D11Context->ClearDepthStencilView(DestSurfaceDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0, 0);
			}

			mCurrentDSVAccessType = FExclusiveDepthStencil::DepthWrite_StencilWrite;
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<true, CF_Always>::getRHI();

			ID3D11RenderTargetView* nullRTV = nullptr;
			mD3D11Context->OMSetRenderTargets(1, &nullRTV, DestSurfaceDSV);
		}
		else
		{
			if (bClearDestTexture)
			{
				LinearColor clearColor(0, 0, 0, 0);
				mD3D11Context->ClearRenderTargetView(destSurfaceRTV, (float*)&clearColor);
			}
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
			mD3D11Context->OMSetRenderTargets(1, &destSurfaceRTV, NULL);
		}

		RHICmdList.setViewport(0.0f, 0.0f, 0.0f, resolveTargetDesc.Width, resolveTargetDesc.Height, 1.0f);

		const float minU = sourceRect.X1;
		const float minV = sourceRect.Y1;
		const float maxU = sourceRect.X2;
		const float maxV = sourceRect.Y2;
		const float minX = -1.0f + destRect.X1 / ((float)resolveTargetDesc.Width * 0.5f);
		const float minY = 1.0f - destRect.Y1 / ((float)resolveTargetDesc.Height * 0.5f);
		const float maxX = -1.0f + destRect.X2 / ((float)resolveTargetDesc.Width * 0.5f);
		const float maxY = 1.0f - destRect.Y2 / ((float)resolveTargetDesc.Width * 0.5f);

		auto shaderMap = getGlobalShaderMap(GMaxRHIFeatureLevel);

		TShaderMapRef<ResolveVS> resolveVertexShader(shaderMap);
		TShaderMapRef<TPixelShader> resolvePixelShader(shaderMap);

		graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GScreenVertexDeclaration.mVertexDeclarationRHI;
		graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*resolveVertexShader);
		graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*resolvePixelShader);
		graphicsPSOInit.mPrimitiveType = PT_TriangleStrip;

		mCurrentDepthTexture = destTexture;
		setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
		RHICmdList.setBlendFactor(LinearColor::White);

		resolvePixelShader->setParameters(RHICmdList, pixelShaderParameter);

		RHICmdList.flush();

		const uint32 textureIndex = resolvePixelShader->mUnresolvedSurface.getBaseIndex();

		if (sourceTexture)
		{
			setShaderResourceView<SF_Pixel>(sourceTexture, sourceTexture->getShaderResourceView(), textureIndex, sourceTexture->getName());
		}
		RHIResourceCreateInfo createInfo;
		VertexBufferRHIRef vertexBufferRHI = RHICreateVertexBuffer(sizeof(ScreenVertex) * 4, BUF_Volatile, createInfo);
		void* voidPtr = RHILockVertexBuffer(vertexBufferRHI, 0, sizeof(ScreenVertex) * 4, RLM_WriteOnly);

		ScreenVertex* vertices = (ScreenVertex*)voidPtr;

		vertices[0].mPosition.x = maxX;
		vertices[0].mPosition.y = minY;
		vertices[0].mUV.x = maxU;
		vertices[0].mUV.y = minV;

		vertices[1].mPosition.x = maxX;
		vertices[1].mPosition.y = maxY;
		vertices[1].mUV.x = maxU;
		vertices[1].mUV.y = maxV;

		vertices[2].mPosition.x = minX;
		vertices[2].mPosition.y = minY;
		vertices[2].mUV.x = minU;
		vertices[2].mUV.y = minV;

		vertices[3].mPosition.x = minX;
		vertices[3].mPosition.y = maxY;
		vertices[3].mUV.x = minU;
		vertices[3].mUV.y = maxV;

		RHIUnlockVertexBuffer(vertexBufferRHI);

		RHICmdList.setStreamSource(0, vertexBufferRHI, 0);

		RHICmdList.drawPrimitive(0, 2, 1);

		RHICmdList.flush();

		if (sourceTexture)
		{
			conditionalClearShaderResource(sourceTexture, false);
		}

		commitRenderTargetsAndUAVs();

		RHISetMultipleViewports(1, (ViewportBounds*)&savedViewport);

		mCurrentDSVAccessType = originalDSVAccessType;
		mCurrentDepthTexture = originalDepthTexture;
	}

	void D3D11DynamicRHI::RHICopyToResolveTarget(RHITexture* sourceTextureRHI, RHITexture* destTextureRHI, const ResolveParams& resolveParams)
	{
		if (!sourceTextureRHI || !destTextureRHI)
		{
			return;
		}

		RHITransitionResources(EResourceTransitionAccess::EReadable, &sourceTextureRHI, 1);

		RHICommandList_RecursiveHazardous RHICmdList(this);

		D3D11Texture2D* sourceTexture2D = static_cast<D3D11Texture2D*> (sourceTextureRHI->getTexture2D());
		D3D11Texture2D* destTexture2D = static_cast<D3D11Texture2D*> (destTextureRHI->getTexture2D());

		D3D11TextureCube* sourceTextureCube = static_cast<D3D11TextureCube*>(sourceTextureRHI->getTextureCube());
		D3D11TextureCube* destTextureCube = static_cast<D3D11TextureCube*>(destTextureRHI->getTextureCube());

		D3D11Texture3D* sourceTexture3D = static_cast<D3D11Texture3D*>(sourceTextureRHI->getTexture3D());
		D3D11Texture3D* destTexture3D = static_cast<D3D11Texture3D*>(destTextureRHI->getTexture3D());

		if (sourceTexture2D && destTexture2D)
		{
			BOOST_ASSERT(!sourceTextureCube && !destTextureCube);
			if (sourceTexture2D != destTexture2D)
			{
				if ((mFeatureLevel == D3D_FEATURE_LEVEL_11_0 || mFeatureLevel == D3D_FEATURE_LEVEL_11_1) && destTexture2D->getDepthStencilView(FExclusiveDepthStencil::DepthWrite_StencilWrite) && sourceTextureRHI->isMultisampled() && !destTextureRHI->isMultisampled())
				{
					D3D11_TEXTURE2D_DESC resolveTargetDesc;
					destTexture2D->getResource()->GetDesc(&resolveTargetDesc);

					resolveTextureUsingShader<ResolveDepthPS>(
						RHICmdList,
						sourceTexture2D,
						destTexture2D,
						destTexture2D->getRenderTargetView(0, -1),
						destTexture2D->getDepthStencilView(FExclusiveDepthStencil::DepthWrite_StencilWrite),
						resolveTargetDesc,
						getDefaultRect(resolveParams.mRect, destTexture2D->getWidth(), destTexture2D->getHeight()),
						getDefaultRect(resolveParams.mRect, destTexture2D->getWidth(), destTexture2D->getHeight()),
						mD3D11Context,
						DummyResolveParameter()
						);
				}
				else if (mFeatureLevel == D3D_FEATURE_LEVEL_10_0
					&& destTexture2D->getDepthStencilView(FExclusiveDepthStencil::DepthWrite_StencilWrite))
				{
					D3D11_TEXTURE2D_DESC resolveTargetDesc;
					destTexture2D->getResource()->GetDesc(&resolveTargetDesc);
					resolveTextureUsingShader<ResolveDepthNonMSPS>(
						RHICmdList,
						sourceTexture2D,
						destTexture2D,
						nullptr,
						destTexture2D->getDepthStencilView(FExclusiveDepthStencil::DepthWrite_StencilWrite),
						resolveTargetDesc,
						getDefaultRect(resolveParams.mRect, destTexture2D->getWidth(), destTexture2D->getHeight()),
						getDefaultRect(resolveParams.mRect, destTexture2D->getWidth(), destTexture2D->getHeight()),
						mD3D11Context,
						DummyResolveParameter()
						);
				}
				else
				{
					DXGI_FORMAT srcFmt = (DXGI_FORMAT)GPixelFormats[sourceTextureRHI->getFormat()].PlatformFormat;
					DXGI_FORMAT dstFmt = (DXGI_FORMAT)GPixelFormats[destTexture2D->getFormat()].PlatformFormat;

					DXGI_FORMAT fmt = convertTypelessToUnorm((DXGI_FORMAT)GPixelFormats[destTexture2D->getFormat()].PlatformFormat);

					if (sourceTextureRHI->isMultisampled() && !destTexture2D->isMultisampled())
					{
						mD3D11Context->ResolveSubresource(
							destTexture2D->getResource(),
							resolveParams.mDestArrayIndex,
							sourceTexture2D->getResource(),
							resolveParams.mSourceArrayIndex,
							fmt
						);
					}
					else
					{
						if (resolveParams.mRect.isValid()
							&& !sourceTextureRHI->isMultisampled()
							&& !destTexture2D->getDepthStencilView(FExclusiveDepthStencil::DepthWrite_StencilWrite))
						{
							D3D11_BOX srcBox;

							srcBox.left = resolveParams.mRect.X1;
							srcBox.top = resolveParams.mRect.Y1;
							srcBox.front = 0;
							srcBox.right = resolveParams.mRect.X2;
							srcBox.bottom = resolveParams.mRect.Y2;
							srcBox.back = 1;

							const ResolveRect& destRect = resolveParams.mDestRect.isValid() ? resolveParams.mDestRect : resolveParams.mRect;
							mD3D11Context->CopySubresourceRegion(destTexture2D->getResource(), resolveParams.mDestArrayIndex, destRect.X1, destRect.Y1, 0, sourceTexture2D->getResource(), resolveParams.mSourceArrayIndex, &srcBox);
						}
						else
						{
							mD3D11Context->CopyResource(destTexture2D->getResource(), sourceTexture2D->getResource());
						}
					}
				}
			}
		}
		else if (sourceTextureCube && destTextureCube)
		{
			BOOST_ASSERT(!sourceTexture2D && !destTexture2D);
			if (sourceTextureCube != destTextureCube)
			{
				const uint32 d3dFace = getD3D11CubeFace(resolveParams.mCubeFace);
				const uint32 sourceSubresource = D3D11CalcSubresource(resolveParams.mMipIndex, resolveParams.mSourceArrayIndex * 6 + d3dFace, sourceTextureCube->getNumMips());
				const uint32 destSubresource = D3D11CalcSubresource(resolveParams.mMipIndex, resolveParams.mDestArrayIndex * 6 + d3dFace, destTextureCube->getNumMips());

				if (sourceTextureRHI->isMultisampled() && !destTextureCube->isMultisampled())
				{
					mD3D11Context->ResolveSubresource(
						destTextureCube->getResource(),
						destSubresource,
						sourceTextureCube->getResource(),
						sourceSubresource,
						(DXGI_FORMAT)GPixelFormats[destTextureCube->getFormat()].PlatformFormat
					);
				}
				else
				{
					if (resolveParams.mRect.isValid())
					{
						D3D11_BOX srcBox;
						srcBox.left = resolveParams.mRect.X1;
						srcBox.top = resolveParams.mRect.Y1;
						srcBox.front = 0;
						srcBox.right = resolveParams.mRect.X2;
						srcBox.bottom = resolveParams.mRect.Y2;
						srcBox.back = 1;
						mD3D11Context->CopySubresourceRegion(destTextureCube->getResource(), destSubresource, 0, 0, 0, sourceTextureCube->getResource(), sourceSubresource, &srcBox);

					}
					else
					{
						mD3D11Context->CopySubresourceRegion(destTextureCube->getResource(), destSubresource, 0, 0, 0, sourceTextureCube->getResource(), sourceSubresource, nullptr);
					}
				}
			}
		}
		else if (sourceTexture2D && destTextureCube)
		{
			const uint32 d3dFace = getD3D11CubeFace(resolveParams.mCubeFace);
			const uint32 subresource = D3D11CalcSubresource(0, d3dFace, 1);
			mD3D11Context->CopySubresourceRegion(destTextureCube->getResource(), subresource, 0, 0, 0, sourceTexture2D->getResource(), 0, nullptr);
		}
		else if (sourceTexture3D && destTexture3D)
		{
			BOOST_ASSERT(resolveParams.mSourceArrayIndex == 0);
			BOOST_ASSERT(sourceTexture3D == destTexture3D);
		}
	}
}