#pragma once

namespace Air
{
	static wstring NAME_PCD3D_SM5(TEXT("PCD3D_SM5"));
	static wstring NAME_PCD3D_SM4(TEXT("PCD3D_SM4"));
	static wstring NAME_PCD3D_ES3_1(TEXT("PCD3D_ES31"));
	static wstring NAME_PCD3D_ES2(TEXT("PCD3D_ES2"));
	static wstring NAME_GLSL_150(TEXT("GLSL_150"));
	static wstring NAME_GLSL_150_MAC(TEXT("GLSL_150_MAC"));
	static wstring NAME_SF_PS4(TEXT("SF_PS4"));
	static wstring NAME_SF_XBOXONE_D3D12(TEXT("SF_XBOXONE_D3D12"));

	static wstring NAME_GLSL_430(TEXT("GLSL_430"));
	static wstring NAME_GLSL_150_ES2(TEXT("GLSL_150_ES2"));
	static wstring NAME_GLSL_150_ES2_NOUB(TEXT("GLSL_150_ES2_NOUB"));
	static wstring NAME_GLSL_150_ES31(TEXT("GLSL_150_ES31"));
	static wstring NAME_GLSL_ES2(TEXT("GLSL_ES2"));
	static wstring NAME_GLSL_ES2_WEBGL(TEXT("GLSL_ES2_WEBGL"));
	static wstring NAME_GLSL_ES2_IOS(TEXT("GLSL_ES2_IOS"));
	static wstring NAME_SF_METAL(TEXT("SF_MATAL"));
	static wstring NAME_SF_MATAL_MRT(TEXT("SF_MATAL_MRT"));
	static wstring NAME_GLSL_310_ES_EXT(TEXT("GLSL_310_ES_EXT"));
	static wstring NAME_GLSL_ES3_1_ANDROID(TEXT("GLSL_310_ES3_1_ANDROID"));
	static wstring NAME_SF_MATAL_SM5(TEXT("SF_MATAL_SM5"));
	static wstring NAME_VULKAN_ES3_1_ANDROID(TEXT("SF_VULKAN_ES31_ANDROID"));
	static wstring NAME_VULKAN_ES3_1(TEXT("SF_VULKAN_ES31"));
	static wstring NAME_VULKAN_SM4_UB(TEXT("SF_VULKAN_SM4_UB"));
	static wstring NAME_VULKAN_SM4(TEXT("SF_VULKAN_SM4"));
	static wstring NAME_VULKAN_SM5(TEXT("SF_VULKAN_SM5"));
	static wstring NAME_SF_METAL_SM4(TEXT("SF_METAL_SM4"));
	static wstring NAME_SF_METAL_MACES3_1(TEXT("SF_METAL_MACES3_1"));
	static wstring NAME_SF_METAL_MACES2(TEXT("SF_METAL_MACES2"));
	static wstring NAME_GLSL_SWITCH(TEXT("GLSL_SWITCH"));
	static wstring NAME_GLSL_SWITCH_FORWARD(TEXT("GLSL_SWITCH_FORWARD"));

	static EShaderPlatform shaderFormatNameToShaderPlatform(wstring shaderFormat)
	{
		if (shaderFormat == NAME_PCD3D_SM5) return SP_PCD3D_SM5;
		if (shaderFormat == NAME_PCD3D_SM4) return SP_PCD3D_SM4;
		if (shaderFormat == NAME_PCD3D_ES3_1) return SP_PCD3D_ES3_1;
		if (shaderFormat == NAME_PCD3D_ES2) return SP_PCD3D_ES2;
		
		if (shaderFormat == NAME_GLSL_150) return SP_OPENGL_SM4;
		if (shaderFormat == NAME_GLSL_430) return SP_OPENGL_SM5;
		if (shaderFormat == NAME_GLSL_150_ES2) return SP_OPENGL_PCES2;
		if (shaderFormat == NAME_GLSL_150_ES2_NOUB) return SP_OPENGL_PCES2;
		if (shaderFormat == NAME_GLSL_150_ES31) return SP_OPENGL_PCES3_1;
		if (shaderFormat == NAME_GLSL_ES2) return SP_OPENGL_ES2_ANDROID;
		if (shaderFormat == NAME_GLSL_ES2_WEBGL) return SP_OPENGL_ES2_WEBGL;
		if (shaderFormat == NAME_GLSL_ES2_IOS) return SP_OPENGL_ES2_IOS;
		if (shaderFormat == NAME_GLSL_310_ES_EXT) return SP_OPENGL_ES31_EXT;
		if (shaderFormat == NAME_GLSL_ES3_1_ANDROID) return SP_OPENGL_ES3_1_ANDROID;

		if (shaderFormat == NAME_SF_PS4)			return SP_PS4;

		if (shaderFormat == NAME_SF_XBOXONE_D3D12)	return SP_XBOXONE_D3D12;

		if (shaderFormat == NAME_VULKAN_ES3_1_ANDROID) return SP_VULKAN_ES3_1_ANDROID;
	
		if (shaderFormat == NAME_VULKAN_SM5)		return SP_VULKAN_SM5;
	}
}