#pragma once
#include "Common.hlsl"
//基于物理的着色模型
//基于为表面的specular分量的参数公式如下：
//Microfacet specular = D * G * F / (4 * NoL * NoV) = D * Vis * F
//Vis = G / (4 * NoL * NoV)
//其中D是法线分布函数， G：几何衰减因子， F(fresnel):菲尼尔方程



//BRDF 主流的几种Diffuse计算公式--------------------------------------------------------------

float3 Diffuse_Lambert(float3 diffuseColor)
{
	return diffuseColor * (1 / PI);
}

//[Burley 2012, "Physically-Based Shading at Disney"]
float3 Diffuse_Burley(float3 diffuseColor, float roughness, float NoV, float NoL, float VoH)
{
	float FD90 = 0.5 + 2 * VoH * VoH * roughness;
	float FdV = 1 + (FD90 - 1) * pow5(1 - NoV);
	float FdL = 1 + (FD90 - 1) * pow5(1 - NoL);
	return diffuseColor * ((1 / PI) * FdV * FdL);
}

//BRDF 几种法线分布函数 D(h)项--------------------------------------------------------

//[Blinn 1977, "Models of light reflection for computer synthesized pictures"]
float D_Blinn(float roughness, float NoH)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float n = 2 / a2 - 2;
	return (n + 2) / (2 * PI) * phongShadingPow(NoH, n);
}

//[Beckmann 1963, "The scattering of electormagnetic waves from rough surfaces"]
float D_Beckmann(float roughness, float NoH)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NoH2 = NoH * NoH;
	return exp((NoH2 - 1) / (a2 * NoH2)) / (PI * a2 * NoH2 * NoH2);
}

// [Walter et el. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float roughness, float NoH)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float d = (NoH * a2 - NoH) * NoH + 1;
	return a2 / (PI * d * d);
}

//[Burley 2012, "Physically-Based Shading at Disney"]
float D_GGXaniso(float roughnessX, float roughnessY, float NoH, float3 H, float3 X, float3 Y)
{
	float ax = roughnessX * roughnessX;
	float ay = roughnessY * roughnessY;
	float XoH = dot(X, H);
	float YoH = dot(Y, H);
	float d = XoH * XoH / (ax * ax) + YoH * YoH / (ay * ay) + NoH * NoH;
	return 1 / (PI * ax * ay * d * d);
}


//Vis项-------------------------------------------------------------------

float Vis_Implicit()
{
	return 0.25;
}

//[Neumann et al. 1999, "Compat metallic reflectance models"]
float Vis_Neumann(float NoV, float NoL)
{
	return 1 / (4 * max(NoL, NoV));
}

//[Kelemen 2001, "A microfacet based coupled specular-matte brdf model with importance sampling"]
float Vis_Kelemen(float VoH)
{
	return rcp(4 * VoH * VoH + 1e-5);
}

//[Schlick_1994, "An inexpensive BRDF Model for Physically-Based Rendering"]
float Vis_Schlick(float roughness, float NoV, float NoL)
{
	float k = square(roughness) * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

//[Simth 1967, "Geometrical shadowing of a random rough surface"]
float Vis_Smith(float roughness, float NoV, float NoL)
{
	float a = square(roughness);
	float a2 = a * a;
	float Vis_SmithV = NoV + sqrt(NoV * (NoV - NoV * a2) + a2);
	float Vis_SmithL = NoL + sqrt(NoL * (NoL - NoL * a2) + a2);
	return rcp(Vis_SmithV * Vis_SmithL);
}

//[Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox(float roughness, float NoV, float NoL)
{
	float a = square(roughness);
	float Vis_SmithV = NoL * (NoV * (1 - a) + a);
	float Vis_SmithL = NoV * (NoL * (1 - a) + a);
	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
}

//F项-------------------------------------------------------------------
float3 F_None(float3 specularColor)
{
	return specularColor;
}

//[Schlick 1994, "An Inexpensive BRDF Model For Physically-Based Rendering"]
float3 F_Schlick(float3 specularColor, float VoH)
{
	float fc = pow5(1 - VoH);
	return saturate(50.0 * specularColor.g) * fc + (1 - fc) * specularColor;
}

float3 F_Fresnel(float3 specularColor, float VoH)
{
	float3 specularColorSqrt = sqrt(clamp(float3(0, 0, 0), float3(0.99, 0.99, 0.99), specularColor));
	float3 n = (1 + specularColorSqrt) / (1 - specularColorSqrt);
	float3 g = sqrt(n * n + VoH * VoH - 1);
	return 0.5 * square((g - VoH) / (g + VoH)) * (1 + square((((g + VoH) * VoH - 1) / ((g - VoH) * VoH) + 1)));
}

//------
//EnvBRDF 环境光的BRDF-----------------------------------------------------------
//------
