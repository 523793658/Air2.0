#include "SceneManagement.h"
#include "SceneView.h"
#include "Texture2D.h"
#include "RHIStaticStates.h"
namespace Air
{
	ENGINE_API bool GDrawListsLocked = false;

	ViewConstantShaderParameters::ViewConstantShaderParameters()
	{
		Memory::memzero(*this);
		SkyBoxTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::getRHI();
	}

	SharedSamplerState* Wrap_WorldGroupSettings = nullptr;
	SharedSamplerState* Clamp_WorldGroupSettings = nullptr;

	void initializeSharedSamplerStates()
	{
		Wrap_WorldGroupSettings = new SharedSamplerState(true);
		Clamp_WorldGroupSettings = new SharedSamplerState(false);
		beginInitResource(Wrap_WorldGroupSettings);
		beginInitResource(Clamp_WorldGroupSettings);
	}

	void SharedSamplerState::initRHI()
	{
		const float mipmapBias = RTexture2D::getGlobalMipMapLODBias();
		SamplerStateInitializerRHI samplerStateInitializer(
			ESamplerFilter::SF_Bilinear, bWrap ? AM_Wrap : AM_Clamp,
			bWrap ? AM_Wrap : AM_Clamp, bWrap ? AM_Wrap : AM_Clamp, mipmapBias
		);
		mSamplerStateRHI = RHICreateSamplerState(samplerStateInitializer);
	}


	float computeBoundsScreenSize(const float4& boundsOrigin, const float sphereRadius, const float4& viewOrigin, const Matrix&projMatrix)
	{
		const float dist = float3::dist(boundsOrigin, viewOrigin);
		const float screenMultiple = Math::max(0.5f * projMatrix.M[0][0], 0.5f * projMatrix.M[1][1]);
		const float screenRadius = screenMultiple * sphereRadius / Math::max(1.0f, dist);
		return screenRadius * 2.0f;
	}
}