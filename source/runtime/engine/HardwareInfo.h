#pragma once
#include "EngineMininal.h"
namespace Air
{
	static const wstring NAME_RHI(L"RHI");
	static const wstring NAME_TextureFormat(L"TextureFormat");
	static const wstring NAME_DeviceType(L"DeviceType");

	struct ENGINE_API HardwareInfo
	{
		static void registerHardwareInfo(const wstring specIdentifier, const wstring & hardwareInfo);

		static wstring getHardwareInfo(const wstring specIdentifier);

		static const wstring getHardwareDetailsString();
	};
}