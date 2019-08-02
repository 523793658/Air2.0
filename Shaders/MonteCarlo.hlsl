#pragma once

float2 hammersley(uint index, uint numSamples, uint2 random)
{
	float E1 = frac((float)index / numSamples + float(random.x & 0xffff) / (1 << 16));
	float E2 = float(reverseBits32(index) ^ random.y) * 2.3283064365386963e-10;
	return float2(E1, E2);
}

float4 importanceSampleGGX(float2 e, float roughness)
{
	float m = roughness * roughness;
	float m2 = m * m;

	float phi = 2 * PI * e.x;

	float cosTheta = sqrt((1 - e.y) / (1 + (m2 - 1) * e.y));
	float sinTheta = sqrt(1 - cosTheta * cosTheta);

	float3 h;
	h.x = sinTheta * cos(phi);
	h.y = sinTheta * sin(phi);
	h.z = cosTheta;
	
	float d = (cosTheta * m2 - cosTheta) * cosTheta + 1;
	float D = m2 / (PI * d * d);
	float pdf = D * cosTheta;
	return float4(h, pdf);
}

float3 tangentToWorld(float3 vec, float3 tangentY)
{
	float3 upVector = abs(tangentY.y) < 0.999 ? float3(0, 1, 0) : float3(0, 0, 1);
	float3 tangentZ = normalize(cross(tangentY, upVector));
	float3 tangentX = cross(tangentY, tangentZ);
	return tangentX * vec.x + tangentY * vec.y + tangentZ * vec.z;
}