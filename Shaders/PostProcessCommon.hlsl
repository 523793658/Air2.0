#pragma once
Texture2D PostprocessInput0;
SamplerState PostprocessInput0Sampler;

Texture2D PostprocessInput1;
SamplerState PostprocessInput1Sampler;

Texture2D PostprocessInput2;
SamplerState PostprocessInput2Sampler;

Texture2D PostprocessInput3;
SamplerState PostprocessInput3Sampler;

Texture2D PostprocessInput4;
SamplerState PostprocessInput4Sampler;

Texture2D PostprocessInput5;
SamplerState PostprocessInput5Sampler;

Texture2D PostprocessInput6;
SamplerState PostprocessInput6Sampler;

//width, height, 1/width, 1/height

float4 PostprocessInput0Size;
float4 PostprocessInput1Size;
float4 PostprocessInput2Size;
float4 PostprocessInput3Size;
float4 PostprocessInput4Size;
float4 PostprocessInput5Size;
float4 PostprocessInput6Size;


float4 PostprocessInput0MinMax;
float4 PostprocessInput1MinMax;
float4 PostprocessInput2MinMax;
float4 PostprocessInput3MinMax;
float4 PostprocessInput4MinMax;
float4 PostprocessInput5MinMax;
float4 PostprocessInput6MinMax;

// viewport width, height, 1/width, 1/height
float4 ViewportSize;

//in pixel
uint4 ViewportRect;

SamplerState BilinearTextureSampler0;
SamplerState BilinearTextureSampler1;

float4 ScreenPosToPixel;

float2 computePixelPosCenter(float2 screenPos, bool bPixelCenter)
{
	float2 pixelOffset = bPixelCenter ? 0.5f : 0.0f;
	return screenPos * ScreenPosToPixel.xy + ScreenPosToPixel.zw + pixelOffset;
}

float2 screenPosFromPixelPos(float2 pixelPos, bool bPixelCenter)
{
	float2 pixelOffset = bPixelCenter ? 0.5f : 0.0f;
	return (pixelPos - pixelOffset - ScreenPosToPixel.zw) / ScreenPosToPixel.xy;
}

float discMask(float2 screenPos)
{
	float x = saturate(1.0f - dot(screenPos, screenPos));
	return x * x;
}

float rectMask(float2 screenPos)
{
	float2 uv = saturate(screenPos * 0.5f - 0.5f);
	float2 mask2 = uv * (1 - uv);
	return mask2.x * mask2.y * 8.0f;
}

