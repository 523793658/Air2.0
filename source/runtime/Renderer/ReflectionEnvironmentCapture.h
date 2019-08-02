#pragma once
#include "RHI.h"
#include "RHICommandList.h"
#include "Math/SHMath.h"
namespace Air
{

	extern void computeDiffuseIrradiance(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, TextureRHIRef lightingSource, int32 lightingSourceMipIndex, SHVectorRGB3* outIrradianceEnvironmentMap);
}