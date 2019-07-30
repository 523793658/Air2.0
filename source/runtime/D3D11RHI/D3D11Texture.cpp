#include "D3D11Texture.h"
#include "RenderUtils.h"
#include "Template/AlignmentTemplates.h"
#include "D3D11Typedefs.h"
#include "D3D11DynamicRHI.h"
#include "RHIDefinitions.h"
#include "HAL/PlatformProperties.h"
#include "Containers/StringConv.h"
#include "D3D11Util.h"
#include "RHI.h"
namespace Air
{
	int64 D3D11GlobalStats::GDedicatedSystemMemory = 0;
	int64 D3D11GlobalStats::GDedicatedVideoMemory = 0;
	int64 D3D11GlobalStats::GSharedSystemMemory = 0;
	int64 D3D11GlobalStats::GTotalGraphicsMemory = 0;

	struct PooledTexture2D
	{
		TRefCountPtr<ID3D11Texture2D> mResource;
	};

	static bool shouldCountAsTextureMemory(uint32 bindFlags)
	{
		return (bindFlags & (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS)) == 0;
	}

	void updateD3D11TextureStats(uint32 bindFlags, uint32 miscFlags, int64 textureSize, bool b3D)
	{
		if (textureSize == 0)
		{
			return;
		}
		int64 alignedSize = (textureSize > 0) ? align(textureSize, 1024) / 1024 : -(align(-textureSize, 1024) / 1024);

		if (shouldCountAsTextureMemory(bindFlags))
		{
			PlatformAtomics::interLockedAdd(&GCurrentTextureMemorySize, alignedSize);
		}
		else
		{
			PlatformAtomics::interLockedAdd(&GCurrentRendertargetMemorySize, alignedSize);
		}

		bool bCubeMap = (miscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0;

	}


	template<typename BaseResourceType>
	void D3D11TextureAllocated(TD3D11Texture2D<BaseResourceType>& texture)
	{
		ID3D11Texture2D* d3d11Texture2D = texture.getResource();
		if (d3d11Texture2D)
		{
			if ((texture.getFlags() & TexCreate_Virtual) == TexCreate_Virtual)
			{
				texture.setMemorySize(0);
			}
			else
			{
				D3D11_TEXTURE2D_DESC desc;
				d3d11Texture2D->GetDesc(&desc);
				int64 textureSize = calcTextureSize(desc.Width, desc.Height, texture.getFormat(), desc.MipLevels) * desc.ArraySize;
				texture.setMemorySize(textureSize);
				updateD3D11TextureStats(desc.BindFlags, desc.MiscFlags, textureSize, false);
			}
		}
	}


	void D3D11TextureAllocated2D(D3D11Texture2D& texture)
	{
		D3D11TextureAllocated(texture);
	}

	bool getPooledTexture2D(int32 mipCount, EPixelFormat pixelFormat, PooledTexture2D* outTexture)
	{
		return false;
	}

	void safeCreateTexture2D(ID3D11Device* direct3DDevice, const D3D11_TEXTURE2D_DESC* textureDesc, const D3D11_SUBRESOURCE_DATA* subResourceData, ID3D11Texture2D** outTexture2D)
	{
		VERIFYD3d11RESULT(direct3DDevice->CreateTexture2D(textureDesc, subResourceData, outTexture2D));
	}


	template<typename BaseResourceType>
	TD3D11Texture2D<BaseResourceType>* D3D11DynamicRHI::createD3D11Texture2D(uint32 width, uint32 height, uint32 size, bool bTextureArray, bool CubeTexture, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		BOOST_ASSERT(width > 0 && height > 0 && numMips > 0);
		if (CubeTexture)
		{
		}
		else
		{

		}

		bool bPooledTexture = true;

		if(GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1);
		{
			flags &= ~TexCreate_SRGB;
		}
		const bool bSRGB = (flags & TexCreate_SRGB) != 0;
		const DXGI_FORMAT platformResourceFormat = D3D11DynamicRHI::getPlatformTextureResourceFormat((DXGI_FORMAT)GPixelFormats[format].PlatformFormat, flags);
		const DXGI_FORMAT	platformShaderResourceFormat = findShaderResourceDXGIFormat(platformResourceFormat, bSRGB);
		const DXGI_FORMAT platformRenderTargetFormat = findShaderResourceDXGIFormat(platformResourceFormat, bSRGB);
		D3D11_DSV_DIMENSION depthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		D3D11_RTV_DIMENSION renderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		D3D11_SRV_DIMENSION shaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		uint32 CPUAccessFlags = 0;
		D3D11_USAGE textureUsage = D3D11_USAGE_DEFAULT;
		uint32 bindFlags = D3D11_BIND_SHADER_RESOURCE;
		bool bCreateShaderResource = true;
		uint32 actualMSAACount = numSamplers;
		uint32 actualMSAAQuality = getMaxMSAAQuality(actualMSAACount);
		if (actualMSAAQuality == 0xffffffff || (flags & TexCreate_Shared) != 0)
		{
			actualMSAACount = 1;
			actualMSAAQuality = 0;
		}
		if (actualMSAACount > 1)
		{
			depthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			renderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			shaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

			bPooledTexture = false;
		}
		if (numMips < 1 || width != height || (1 << (numMips - 1)) != width || (flags & TexCreate_Shared) != 0)
		{
			bPooledTexture = false;
		}

		if (flags & TexCreate_CPUReadback)
		{
			BOOST_ASSERT(!(flags& TexCreate_RenderTargetable));
			BOOST_ASSERT(!(flags& TexCreate_DepthStencilTargetable));
			BOOST_ASSERT(!(flags& TexCreate_ShaderResource));
			CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			textureUsage = D3D11_USAGE_STAGING;
			bindFlags = 0;
			bCreateShaderResource = false;
		}

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = numMips;
		textureDesc.ArraySize = size;
		textureDesc.Format = platformResourceFormat;
		textureDesc.SampleDesc.Count = actualMSAACount;
		textureDesc.SampleDesc.Quality = actualMSAAQuality;
		textureDesc.Usage = textureUsage;
		textureDesc.BindFlags = bindFlags;
		textureDesc.CPUAccessFlags = CPUAccessFlags;
		textureDesc.MiscFlags = CubeTexture ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
		if (flags & TexCreate_Shared)
		{
			textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
		}
		if (flags & TexCreate_GenerateMipCapable)
		{
			textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			bPooledTexture = false;
		}
		bool bCreateRTV = false;
		bool bCreateDSV = false;
		bool bCreateRTVPerSlice = false;
		if (flags & TexCreate_RenderTargetable)
		{
			BOOST_ASSERT(!(flags & TexCreate_DepthStencilTargetable));
			BOOST_ASSERT(!(flags & TexCreate_ResolveTargetable));
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}
		else if (flags & TexCreate_DepthStencilTargetable)
		{
			BOOST_ASSERT(!(flags & TexCreate_RenderTargetable));
			BOOST_ASSERT(!(flags & TexCreate_ResolveTargetable));
			textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			bCreateDSV = true;
		}
		else if (flags & TexCreate_ResolveTargetable)
		{
			BOOST_ASSERT(!(flags & TexCreate_RenderTargetable));
			BOOST_ASSERT(!(flags & TexCreate_DepthStencilTargetable));
			if (format == PF_DepthStencil || format == PF_ShadowDepth || format == PF_D24)
			{
				textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
				bCreateDSV = true;
			}
			else
			{
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				bCreateRTV = true;
			}
		}
		if (flags & TexCreate_UAV)
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			bPooledTexture = false;
		}
		if (bCreateDSV || bCreateRTV || CubeTexture || bTextureArray)
		{
			bPooledTexture = false;
		}
		VRamAllocation vRamAllocation;
		if (PlatformProperties::supportsFastVRAMMemory())
		{
			if (flags & TexCreate_FastVRAM)
			{
				vRamAllocation = FastVRAMAllocator::getFastVRAMAllocator()->allocTexture2D(textureDesc);
			}
		}
		TRefCountPtr<ID3D11Texture2D> textureResource;
		TRefCountPtr<ID3D11ShaderResourceView> shaderResourceView;
		TArray<TRefCountPtr<ID3D11RenderTargetView>> renderTargetViews;
		TRefCountPtr<ID3D11DepthStencilView> depthStencilView[FExclusiveDepthStencil::MaxIndex];

		flags &= ~TexCreate_Virtual;
		if (bPooledTexture)
		{
			PooledTexture2D pooledTexture;
			if (getPooledTexture2D(numMips, (EPixelFormat)format, &pooledTexture))
			{
				textureResource = pooledTexture.mResource;
			}
		}
		if (!isValidRef(textureResource))
		{
			TArray<D3D11_SUBRESOURCE_DATA> subResourceData;
			if (createInfo.mBulkData)
			{
				uint8* data = (uint8*)createInfo.mBulkData->getResourceBulkData();
				subResourceData.addZeroed(numMips * size);
				uint32 sliceOffset = 0;
				for (uint32 arraySliceIndex = 0; arraySliceIndex < size; ++arraySliceIndex)
				{
					uint32 mipOffset = 0;
					for (uint32 mipIndex = 0; mipIndex < numMips; ++mipIndex)
					{
						uint32 dataOffset = sliceOffset + mipOffset;
						uint32 subResourceIndex = arraySliceIndex * numMips + mipIndex;
						uint32 numBlocksX = std::max<uint32>(1, (width >> mipIndex) / GPixelFormats[format].BlockSizeX);
						uint32 numblocksY = std::max<uint32>(1, (height >> mipIndex) / GPixelFormats[format].BlockSizeY);
						subResourceData[subResourceIndex].pSysMem = &data[dataOffset];
						subResourceData[subResourceIndex].SysMemPitch = numBlocksX * GPixelFormats[format].BlockBytes;
						subResourceData[subResourceIndex].SysMemSlicePitch = numBlocksX * numblocksY * subResourceData[mipIndex].SysMemPitch;
						mipOffset += numblocksY * subResourceData[mipIndex].SysMemPitch;
					}
					sliceOffset += mipOffset;
				}
			}
			{
				safeCreateTexture2D(mD3D11Device, &textureDesc, createInfo.mBulkData != nullptr ? (const D3D11_SUBRESOURCE_DATA*)subResourceData.data() : NULL, textureResource.getInitReference());
			}
			if (bCreateRTV)
			{
				for (uint32 mipIndex = 0; mipIndex < numMips; mipIndex++)
				{
					if ((flags & TexCreate_TargetArraySlicesIndependently) && (bTextureArray || CubeTexture))
					{
						bCreateRTVPerSlice = true;
						for (uint32 sliceIndex = 0; sliceIndex < textureDesc.ArraySize; sliceIndex++)
						{
							D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
							Memory::memzero(&rtvDesc, sizeof(rtvDesc));
							rtvDesc.Format = platformRenderTargetFormat;
							rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
							rtvDesc.Texture2DArray.FirstArraySlice = sliceIndex;
							rtvDesc.Texture2DArray.ArraySize = 1;
							rtvDesc.Texture2DArray.MipSlice = mipIndex;
							TRefCountPtr<ID3D11RenderTargetView> renderTargetView;
							VERIFYD3D11RESULT_EX(mD3D11Device->CreateRenderTargetView(textureResource, &rtvDesc, renderTargetView.getInitReference()), mD3D11Device);
							renderTargetViews.push_back(renderTargetView);
						}
					}
					else
					{
						D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
						Memory::memzero(&rtvDesc, sizeof(rtvDesc));
						rtvDesc.Format = platformRenderTargetFormat;
						if (bTextureArray || CubeTexture)
						{
							rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
							rtvDesc.Texture2DArray.FirstArraySlice = 0;
							rtvDesc.Texture2DArray.ArraySize = textureDesc.ArraySize;
							rtvDesc.Texture2DArray.MipSlice = mipIndex;
						}
						else
						{
							rtvDesc.ViewDimension = renderTargetViewDimension;
							rtvDesc.Texture2D.MipSlice = mipIndex;
						}
						TRefCountPtr<ID3D11RenderTargetView> renderTargetView;
						VERIFYD3D11RESULT_EX(mD3D11Device->CreateRenderTargetView(textureResource, &rtvDesc, renderTargetView.getInitReference()), mD3D11Device);
						renderTargetViews.push_back(renderTargetView);
					}
				}
			}
			if (bCreateDSV)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				Memory::memzero(&dsvDesc, sizeof(dsvDesc));
				dsvDesc.Format = findDepthStencilDXGIFormat(platformResourceFormat);
				if (bTextureArray || CubeTexture)
				{
					dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.ArraySize = textureDesc.ArraySize;
					dsvDesc.Texture2DArray.FirstArraySlice = 0;
					dsvDesc.Texture2DArray.MipSlice = 0;
				}
				else
				{
					dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
				}
				for (uint32 accessType = 0; accessType < FExclusiveDepthStencil::MaxIndex; ++accessType)
				{
					if (mD3D11Device->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0)
					{
						dsvDesc.Flags = (accessType & FExclusiveDepthStencil::DepthRead_StencilWrite) ? D3D11_DSV_READ_ONLY_DEPTH : 0;
						if (hasStencilBits(dsvDesc.Format))
						{
							dsvDesc.Flags |= (accessType & FExclusiveDepthStencil::DepthWrite_StencilRead) ? D3D11_DSV_READ_ONLY_STENCIL : 0;
						}
					}
					VERIFYD3D11RESULT_EX(mD3D11Device->CreateDepthStencilView(textureResource, &dsvDesc, depthStencilView[accessType].getInitReference()), mD3D11Device);
				}
			}
		}
		BOOST_ASSERT(isValidRef(textureResource));
		if (bCreateShaderResource)
		{
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				SRVDesc.Format = platformShaderResourceFormat;
				if (CubeTexture && bTextureArray)
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
					SRVDesc.TextureCubeArray.First2DArrayFace = 0;
					SRVDesc.TextureCubeArray.MipLevels = 0;
					SRVDesc.TextureCubeArray.MipLevels = numMips;
					SRVDesc.TextureCubeArray.NumCubes = size / 6;
				}
				else if (CubeTexture)
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					SRVDesc.TextureCube.MipLevels = numMips;
					SRVDesc.TextureCube.MostDetailedMip = 0;
				}
				else if (bTextureArray)
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVDesc.Texture2DArray.MostDetailedMip = 0;
					SRVDesc.Texture2DArray.ArraySize = textureDesc.ArraySize;
					SRVDesc.Texture2DArray.FirstArraySlice = 0;
					SRVDesc.Texture2DArray.MipLevels = numMips;
				}
				else
				{
					SRVDesc.ViewDimension = shaderResourceViewDimension;
					SRVDesc.Texture2D.MostDetailedMip = 0;
					SRVDesc.Texture2D.MipLevels = numMips;
				}
				VERIFYD3D11RESULT_EX(mD3D11Device->CreateShaderResourceView(textureResource, &SRVDesc, shaderResourceView.getInitReference()), mD3D11Device);
			}
			BOOST_ASSERT(isValidRef(shaderResourceView));
		}
		TD3D11Texture2D<BaseResourceType>* texture2D = new TD3D11Texture2D<BaseResourceType>(this, textureResource, shaderResourceView, bCreateRTVPerSlice, textureDesc.ArraySize, renderTargetViews, depthStencilView, width, height, size, numMips, actualMSAACount, (EPixelFormat)format, CubeTexture, flags, bPooledTexture, createInfo.mClearValueBinding);
		texture2D->mResourceInfo.mVRamAllocation = vRamAllocation;
		if (flags & TexCreate_RenderTargetable)
		{
			texture2D->setCurrentGPUAccess(EResourceTransitionAccess::EWritable);
		}
		D3D11TextureAllocated(*texture2D);
		if (isRHIDeviceNVIDIA() && (flags & TexCreate_AFRManual))
		{

		}
		return texture2D;
	}


	Texture2DRHIRef D3D11DynamicRHI::RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return createD3D11Texture2D<D3D11BaseTexture2D>(width, height, 1, false, false, format, numMips, numSamplers, flags, createInfo);
	}


	template<typename RHIResourceType>
	void* TD3D11Texture2D<RHIResourceType>::lock(uint32 mipIndex, uint32 arrayIndex, EResourceLockMode lockMode, uint32 & destStride)
	{
		const uint32 subResource = D3D11CalcSubresource(mipIndex, arrayIndex, getNumMips());
		const uint32 blockSizeX = GPixelFormats[getFormat()].BlockSizeX;
		const uint32 blockSizeY = GPixelFormats[getFormat()].BlockSizeY;
		const uint32 blockBytes = GPixelFormats[getFormat()].BlockBytes;
		const uint32 mipSizeX = std::max<uint32>(getWidth() >> mipIndex, blockSizeX);
		const uint32 mipSizeY = std::max<uint32>(getHeight() >> mipIndex, blockSizeY);
		const uint32 numBlocksX = (mipSizeX + blockSizeX - 1) / blockSizeX;
		const uint32 numBlocksY = (mipSizeY + blockSizeY - 1) / blockSizeY;
		const uint32 mipBytes = numBlocksX * numBlocksY * blockBytes;
		
		D3D11LockedData lockedData;
		if (lockMode == RLM_WriteOnly)
		{
			lockedData.allocData(mipBytes);
			lockedData.mPitch = destStride = numBlocksX * blockBytes;
		}
		else
		{
			D3D11_TEXTURE2D_DESC stagingTextureDesc;
			getResource()->GetDesc(&stagingTextureDesc);
			stagingTextureDesc.Width = mipSizeX;
			stagingTextureDesc.Height = mipSizeY;
			stagingTextureDesc.MipLevels = 1;
			stagingTextureDesc.ArraySize = 1;
			stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
			stagingTextureDesc.BindFlags = 0;
			stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			stagingTextureDesc.MiscFlags = 0;
			TRefCountPtr<ID3D11Texture2D> stagingTexture;
			VERIFYD3D11CEATETEXTURERESULT(mD3DRHI->getDevice()->CreateTexture2D(&stagingTextureDesc, nullptr, stagingTexture.getInitReference()), getWidth(), getHeight(), getDepth(), stagingTextureDesc.Format, 1, 0, mD3DRHI->getDevice());
			lockedData.mStagingResource = stagingTexture;
			mD3DRHI->getDeviceContext()->CopySubresourceRegion(stagingTexture, 0, 0, 0, 0, getResource(), subResource, NULL);
			D3D11_MAPPED_SUBRESOURCE mappedTexture;
			VERIFYD3D11RESULT_EX(mD3DRHI->getDeviceContext()->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedTexture), mD3DRHI->getDevice());
			lockedData.setData(mappedTexture.pData);
			lockedData.mPitch = destStride = mappedTexture.RowPitch;
		}

		mD3DRHI->mOutstandingLocks.emplace(D3D11LockedKey(getResource(), subResource), lockedData);
		return (void*)lockedData.getData();
	}

	template<typename RHIResourceType>
	void TD3D11Texture2D<RHIResourceType>::unlock(uint32 mipIndex, uint32 arrayIndex)
	{
		const uint32 subResource = D3D11CalcSubresource(mipIndex, arrayIndex, getNumMips());
		const D3D11LockedKey lockedKey(getResource(), subResource);
		auto it = mD3DRHI->mOutstandingLocks.find(lockedKey);
		BOOST_ASSERT(it != mD3DRHI->mOutstandingLocks.end());
		D3D11LockedData& lockedData = it->second;
		if (!lockedData.mStagingResource)
		{
			mD3DRHI->getDeviceContext()->UpdateSubresource(getResource(), subResource, NULL, lockedData.getData(), lockedData.mPitch, 0);
			lockedData.freeData();
		}
		mD3DRHI->mOutstandingLocks.erase(it);
	}

	void* D3D11DynamicRHI::RHILockTexture2D(Texture2DRHIParamRef textureRHI, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
	{
		BOOST_ASSERT(textureRHI);
		D3D11Texture2D* texture = ResourceCast(textureRHI);
		conditionalClearShaderResource(texture);
		return texture->lock(mipIndex, 0, lockMode, destStride);
	}

	void D3D11DynamicRHI::RHIUnlockTexture2D(Texture2DRHIParamRef textureRHI, uint32 mipIndex, bool bLockWithinMiptail)
	{
		BOOST_ASSERT(textureRHI);
		D3D11Texture2D* texture = ResourceCast(textureRHI);
		texture->unlock(mipIndex, 0);
	}

	TextureCubeRHIRef D3D11DynamicRHI::RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return createD3D11Texture2D<D3D11BaseTextureCube>(size, size, 6, false, true, format, numMips, 1, flags, createInfo);
	}

	TextureReferenceRHIRef D3D11DynamicRHI::RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime)
	{
		return new D3D11TextureReference(this, lastRenderTime);
	}

	void* D3D11DynamicRHI::RHILockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)
	{
		D3D11TextureCube* textureCube = ResourceCast(texture);
		conditionalClearShaderResource(textureCube);
		uint32 d3dFace = getD3D11CubeFace((ECubeFace)faceIndex);
		return textureCube->lock(mipIndex, d3dFace + arrayIndex * 6, lockMode, destStride);
	}


	void D3D11DynamicRHI::RHIUnlockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)
	{
		D3D11TextureCube* textureCube = ResourceCast(texture);
		uint32 d3dFace = getD3D11CubeFace((ECubeFace)faceIndex);
		textureCube->unlock(mipIndex, d3dFace + arrayIndex * 6);
	}

	void D3D11DynamicRHI::RHIUpdateTextureReference(TextureReferenceRHIParamRef textureRHI, TextureRHIParamRef newTextureRHI)
	{
		D3D11TextureReference* textureRef = (D3D11TextureReference*)textureRHI;
		if (textureRef)
		{
			D3D11TextureBase* newTexture = nullptr;
			ID3D11ShaderResourceView* newSRV = nullptr;
			if (newTextureRHI)
			{
				newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
				if (newTexture)
				{
					newSRV = newTexture->getShaderResourceView();
				}
			}
			textureRef->setReferencedTexture(newTextureRHI, newTexture, newSRV);
		}
	}

	void D3D11DynamicRHI::RHIBindDebugLabelName(TextureRHIParamRef texture, const TCHAR* name)
	{
		texture->setName(name);

#if _DEBUG
		if (D3D11Texture2D* texture2D = (D3D11Texture2D*)texture->getTexture2D())
		{
			texture2D->getResource()->SetPrivateData(WKPDID_D3DDebugObjectName, CString::strlen(name) + 1, TCHAR_TO_ANSI(name));
		}
		else if (D3D11TextureCube* textureCube = (D3D11TextureCube*)texture->getTextureCube())
		{
			textureCube->getResource()->SetPrivateData(WKPDID_D3DDebugObjectName, CString::strlen(name) + 1, TCHAR_TO_ANSI(name));
		}
#endif
	}
	
}