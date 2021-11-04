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
	static wstring NAME_SF_METAL_MRT(TEXT("SF_MATAL_MRT"));
	static wstring NAME_GLSL_310_ES_EXT(TEXT("GLSL_310_ES_EXT"));
	static wstring NAME_GLSL_ES3_1_ANDROID(TEXT("GLSL_310_ES3_1_ANDROID"));
	static wstring NAME_SF_MATAL_SM5(TEXT("SF_MATAL_SM5"));
	static wstring NAME_VULKAN_ES3_1_ANDROID(TEXT("SF_VULKAN_ES31_ANDROID"));
	static wstring NAME_VULKAN_ES3_1_ANDROID_NOUB(TEXT("SF_VULKAN_ES31_ANDROID_NOUB"));
	static wstring NAME_VULKAN_ES3_1(TEXT("SF_VULKAN_ES31"));
	static wstring NAME_VULKAN_ES3_1_NOUB(TEXT("SF_VULKAN_ES31_NOUB"));
	static wstring NAME_VULKAN_SM4_UB(TEXT("SF_VULKAN_SM4_UB"));
	static wstring NAME_VULKAN_SM5(TEXT("SF_VULKAN_SM5"));
	static wstring NAME_VULKAN_SM5_NOUB(TEXT("SF_VULKAN_SM5_NOUB"));
	static wstring NAME_SF_METAL_SM4(TEXT("SF_METAL_SM4"));
	static wstring NAME_SF_METAL_SM5(TEXT("SF_METAL_SM5"));
	static wstring NAME_SF_METAL_MACES3_1(TEXT("SF_METAL_MACES3_1"));
	static wstring NAME_SF_METAL_MACES2(TEXT("SF_METAL_MACES2"));
	static wstring NAME_GLSL_SWITCH(TEXT("GLSL_SWITCH"));
	static wstring NAME_GLSL_SWITCH_FORWARD(TEXT("GLSL_SWITCH_FORWARD"));

	static EShaderPlatform shaderFormatNameToShaderPlatform(wstring ShaderFormat)
	{
		if (ShaderFormat == NAME_PCD3D_SM5)					return SP_PCD3D_SM5;
		if (ShaderFormat == NAME_PCD3D_ES3_1)				return SP_PCD3D_ES3_1;

		if (ShaderFormat == NAME_GLSL_150_ES31)				return SP_OPENGL_PCES3_1;
		if (ShaderFormat == NAME_GLSL_ES3_1_ANDROID)		return SP_OPENGL_ES3_1_ANDROID;

		if (ShaderFormat == NAME_SF_PS4)					return SP_PS4;

		if (ShaderFormat == NAME_SF_XBOXONE_D3D12)			return SP_XBOXONE_D3D12;

		if (ShaderFormat == NAME_GLSL_SWITCH)				return SP_SWITCH;
		if (ShaderFormat == NAME_GLSL_SWITCH_FORWARD)		return SP_SWITCH_FORWARD;

		if (ShaderFormat == NAME_SF_METAL)					return SP_METAL;
		if (ShaderFormat == NAME_SF_METAL_MRT)				return SP_METAL_MRT;
		if (ShaderFormat == NAME_SF_METAL_SM5)				return SP_METAL_SM5;
		if (ShaderFormat == NAME_SF_METAL_MACES3_1)			return SP_METAL_MACES3_1;

		if (ShaderFormat == NAME_VULKAN_ES3_1_ANDROID)		return SP_VULKAN_ES3_1_ANDROID;
		if (ShaderFormat == NAME_VULKAN_ES3_1_ANDROID_NOUB)	return SP_VULKAN_ES3_1_ANDROID;
		if (ShaderFormat == NAME_VULKAN_ES3_1)				return SP_VULKAN_PCES3_1;
		if (ShaderFormat == NAME_VULKAN_ES3_1_NOUB)			return SP_VULKAN_PCES3_1;
		if (ShaderFormat == NAME_VULKAN_SM5_NOUB)			return SP_VULKAN_SM5;
		if (ShaderFormat == NAME_VULKAN_SM5)				return SP_VULKAN_SM5;

		for (int32 StaticPlatform = SP_StaticPlatform_First; StaticPlatform <= SP_StaticPlatform_Last; ++StaticPlatform)
		{
			if (ShaderFormat == StaticShaderPlatformNames::Get().GetShaderFormat(EShaderPlatform(StaticPlatform)))
			{
				return EShaderPlatform(StaticPlatform);
			}
		}

		return SP_NumPlatforms;
	}

	static wstring shaderPlatformToShaderFormatName(EShaderPlatform platform)
	{
		switch (platform)
		{
		case Air::SP_PCD3D_SM5:
			return NAME_PCD3D_SM5;
		case Air::SP_PS4:
			return NAME_SF_PS4;
		case Air::SP_XBOXONE_D3D12:
			return NAME_SF_XBOXONE_D3D12;
		case Air::SP_METAL:
			return NAME_SF_METAL;
		case Air::SP_METAL_MRT:
			return NAME_SF_METAL_MRT;
		case Air::SP_PCD3D_ES3_1:
			return NAME_PCD3D_ES3_1;
		case Air::SP_OPENGL_PCES3_1:
			return NAME_GLSL_150_ES31;
		case Air::SP_METAL_SM5:
			return NAME_SF_METAL_SM5;
		case Air::SP_VULKAN_PCES3_1:
		{

			static auto* CVar = IConsoleManager::get().FindTConsoleVariableDataInt(TEXT("r.Vulkan.UseRealUBs"));
			return (CVar && CVar->getValueOnAnyThread() == 0) ? NAME_VULKAN_ES3_1_NOUB : NAME_VULKAN_ES3_1;
		}
		case Air::SP_VULKAN_SM5:
		{

			static auto* CVar = IConsoleManager::get().FindTConsoleVariableDataInt(TEXT("r.Vulkan.UseRealUBs"));
			return (CVar && CVar->getValueOnAnyThread() == 0) ? NAME_VULKAN_SM5_NOUB : NAME_VULKAN_SM5;
		}
		case Air::SP_VULKAN_ES3_1_ANDROID:
			return NAME_VULKAN_ES3_1_ANDROID_NOUB;
		case Air::SP_METAL_MACES3_1:
			return NAME_SF_METAL_MACES3_1;
		case Air::SP_OPENGL_ES3_1_ANDROID:
			return NAME_GLSL_ES3_1_ANDROID;
		case Air::SP_SWITCH:
			return NAME_GLSL_SWITCH;
		case Air::SP_SWITCH_FORWARD:
			return NAME_GLSL_SWITCH_FORWARD;
		}
		return TEXT("");
	}
}