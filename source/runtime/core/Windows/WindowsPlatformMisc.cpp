#include "Windows/WindowsPlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include "WindowsSystemIncludes.h"
#include "WindowsHWrapper.h"
#include "HAL/AirMemory.h"
#include "WindowsApplication.h"
#include "CoreGlobals.h"
#include <WinBase.h>
#include "HAL/ThreadHeartBeat.h"
#include "Misc/Paths.h"
extern "C"
{
	CORE_API Windows::HINSTANCE hInstance = NULL;
}
namespace Air
{



	static void WinPumpMessages()
	{
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}


	int32 WindowsPlatformMisc::numberOfCoresIncludingHyperthreads()
	{
		static int32 coreCount = 0;
		if (coreCount == 0)
		{
			SYSTEM_INFO SI;
			GetSystemInfo(&SI);
			coreCount = (int32)SI.dwNumberOfProcessors;
		}
		return coreCount;
	}

	int32 WindowsPlatformMisc::numberOfCores()
	{
		static int32 coreCount = 0;
		if (coreCount == 0)
		{
			//
			bool usehyperthreading = false;
			if (usehyperthreading)
			{
				coreCount = numberOfCoresIncludingHyperthreads();
			}
			else
			{
				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION infoBuffer = NULL;
				::DWORD bufferSize = 0;
				::BOOL result = GetLogicalProcessorInformation(infoBuffer, &bufferSize);
				infoBuffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)Memory::malloc(bufferSize);
				result = GetLogicalProcessorInformation(infoBuffer, &bufferSize);
				const int32 infoCount = (int32)(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
				for (int32 index = 0; index < infoCount; ++index)
				{
					SYSTEM_LOGICAL_PROCESSOR_INFORMATION* info = &infoBuffer[index];
					if (info->Relationship == RelationProcessorCore)
					{
						coreCount++;
					}
				}
				GMalloc->free(infoBuffer);
			}
		}
		return coreCount;
	}

	bool WindowsPlatformMisc::isDebuggerPresent()
	{
		return !!::IsDebuggerPresent();
	}

	bool WindowsPlatformMisc::getDiskTotalAndFreeSpace(const wstring& inPath, uint64 & totalNumOfBytes, uint64& numberOfFreeBytes)
	{
		bool bSuccess = false;
		wstring validatedPath = Paths::convertRelativePathToFull(inPath);
		boost::algorithm::replace_all(validatedPath, TEXT("/"), TEXT("\\"));
		if (validatedPath.length() >= 3 && validatedPath[1] == ':'&& validatedPath[2] == '\\')
		{
			bSuccess = !!::GetDiskFreeSpaceEx(validatedPath.c_str(), nullptr, reinterpret_cast<ULARGE_INTEGER*>(&totalNumOfBytes), reinterpret_cast<ULARGE_INTEGER*>(&numberOfFreeBytes));
		}
		return bSuccess;
	}

	GenericApplication* WindowsPlatformMisc::createApplication()
	{
		HICON appIconHandle = LoadIcon(hInstance, MAKEINTRESOURCE(NULL));
		if (appIconHandle == NULL)
		{
			appIconHandle = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
		}
		return WindowsApplication::createWindowsApplication(hInstance, appIconHandle);

	}

	void WindowsPlatformMisc::pumpMessages(bool bFromMainLoop)
	{
		if (!bFromMainLoop)
		{
			return;
		}
		GPumpingMessagesOutsideOfMainLoop = false;
		WinPumpMessages();
	}

	GPUDriverInfo WindowsPlatformMisc::getGPUDriverInfo(const wstring & deviceDescription)
	{
		GPUDriverInfo ret;
		ret.mInternalDriverVersion = TEXT("Unknown");
		ret.mUserDriverVersion = TEXT("Unknown");
		ret.mDriverDate = TEXT("Unknown");

		int32 method = 0;
		const bool bIterateAvailableAndChoose = method == 0;
		if (bIterateAvailableAndChoose)
		{
			for (int32 i = 0; i < 256; ++i)
			{
			
			}
		}



		wstring debugString;
		uint32 foundDriverCount = 0;
		return ret;
	}

	void WindowsPlatformMisc::requestExit(bool force)
	{
		if (force)
		{
			TerminateProcess(GetCurrentProcess(), GIsCriticalError ? 3 : 0);
		}
		else
		{
			PostQuitMessage(0);
			GIsRequestingExit = 1;
		}
	}

	uint32 WindowsPlatformMisc::getCharKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings)
	{
		return GenericPlatformMisc::getStandardPrintableKeyMap(keyCodes, keyNames, maxMappings, true, false);
	}
	uint32 WindowsPlatformMisc::getKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings)
	{
#define ADDKEYMAP(keyCode, keyName) if(numMappings < maxMappings){keyCodes[numMappings] = keyCode; keyNames[numMappings] = keyName; ++numMappings;}

		uint32 numMappings = 0;
		if (keyCodes && keyNames && (maxMappings > 0))
		{
			ADDKEYMAP(VK_LBUTTON, TEXT("LeftMouseButton"));
			ADDKEYMAP(VK_RBUTTON, TEXT("RightMouseButton"));
			ADDKEYMAP(VK_MBUTTON, TEXT("MiddleMouseButton"));

			ADDKEYMAP(VK_XBUTTON1, TEXT("ThumbMouseButton"));
			ADDKEYMAP(VK_XBUTTON2, TEXT("ThumbMouseButton2"));

			ADDKEYMAP(VK_BACK, TEXT("BackSpace"));
			ADDKEYMAP(VK_TAB, TEXT("Tab"));
			ADDKEYMAP(VK_RETURN, TEXT("Enter"));
			ADDKEYMAP(VK_PAUSE, TEXT("Pause"));

ADDKEYMAP(VK_CAPITAL, TEXT("CapsLock"));
ADDKEYMAP(VK_ESCAPE, TEXT("Escape"));
ADDKEYMAP(VK_SPACE, TEXT("SpaceBar"));
ADDKEYMAP(VK_PRIOR, TEXT("PageUp"));
ADDKEYMAP(VK_NEXT, TEXT("PageDown"));
ADDKEYMAP(VK_END, TEXT("End"));
ADDKEYMAP(VK_HOME, TEXT("Home"));

ADDKEYMAP(VK_LEFT, TEXT("Left"));
ADDKEYMAP(VK_RIGHT, TEXT("Right"));
ADDKEYMAP(VK_DOWN, TEXT("Down"));
ADDKEYMAP(VK_UP, TEXT("Up"));

ADDKEYMAP(VK_INSERT, TEXT("Insert"));
ADDKEYMAP(VK_DELETE, TEXT("Delete"));

ADDKEYMAP(VK_NUMPAD0, TEXT("NumPadZero"));
ADDKEYMAP(VK_NUMPAD1, TEXT("NumPadOne"));
ADDKEYMAP(VK_NUMPAD2, TEXT("NumPadTwo"));
ADDKEYMAP(VK_NUMPAD3, TEXT("NumPadThree"));
ADDKEYMAP(VK_NUMPAD4, TEXT("NumPadFour"));
ADDKEYMAP(VK_NUMPAD5, TEXT("NumPadFiv"));
ADDKEYMAP(VK_NUMPAD6, TEXT("NumPadSix"));
ADDKEYMAP(VK_NUMPAD7, TEXT("NumPadSeven"));
ADDKEYMAP(VK_NUMPAD8, TEXT("NumPadEight"));
ADDKEYMAP(VK_NUMPAD9, TEXT("NumPadNine"));

ADDKEYMAP(VK_MULTIPLY, TEXT("Multiply"));
ADDKEYMAP(VK_ADD, TEXT("Add"));
ADDKEYMAP(VK_SUBTRACT, TEXT("Subtract"));
ADDKEYMAP(VK_DIVIDE, TEXT("Divide"));
ADDKEYMAP(VK_DECIMAL, TEXT("Decimal"));

ADDKEYMAP(VK_F1, TEXT("F1"));
ADDKEYMAP(VK_F2, TEXT("F2"));
ADDKEYMAP(VK_F3, TEXT("F3"));
ADDKEYMAP(VK_F4, TEXT("F4"));
ADDKEYMAP(VK_F5, TEXT("F5"));
ADDKEYMAP(VK_F6, TEXT("F6"));
ADDKEYMAP(VK_F7, TEXT("F7"));
ADDKEYMAP(VK_F8, TEXT("F8"));
ADDKEYMAP(VK_F9, TEXT("F9"));
ADDKEYMAP(VK_F10, TEXT("F10"));
ADDKEYMAP(VK_F11, TEXT("F11"));
ADDKEYMAP(VK_F12, TEXT("F12"));

ADDKEYMAP(VK_NUMLOCK, TEXT("NumLock"));
ADDKEYMAP(VK_SCROLL, TEXT("ScrollLock"));

ADDKEYMAP(VK_LSHIFT, TEXT("LeftShift"));
ADDKEYMAP(VK_RSHIFT, TEXT("RightShift"));

ADDKEYMAP(VK_LCONTROL, TEXT("LeftControl"));
ADDKEYMAP(VK_RCONTROL, TEXT("RightControl"));
ADDKEYMAP(VK_LMENU, TEXT("LeftAlt"));
ADDKEYMAP(VK_RMENU, TEXT("RightAlt"));
ADDKEYMAP(VK_LWIN, TEXT("LeftCommand"));
ADDKEYMAP(VK_RWIN, TEXT("RightCommand"));

TMap<uint32, uint32> scanToVKMap;
#define MAP_ONE_VK_TO_SCAN(keyCode) {const uint32 charCode = MapVirtualKey(keyCode, 2); if(charCode != 0){scanToVKMap.emplace(charCode, keyCode);}}

MAP_ONE_VK_TO_SCAN(VK_OEM_1);
MAP_ONE_VK_TO_SCAN(VK_OEM_2);
MAP_ONE_VK_TO_SCAN(VK_OEM_3);
MAP_ONE_VK_TO_SCAN(VK_OEM_4);
MAP_ONE_VK_TO_SCAN(VK_OEM_5);
MAP_ONE_VK_TO_SCAN(VK_OEM_6);
MAP_ONE_VK_TO_SCAN(VK_OEM_7);
MAP_ONE_VK_TO_SCAN(VK_OEM_8);
MAP_ONE_VK_TO_SCAN(VK_OEM_PLUS);
MAP_ONE_VK_TO_SCAN(VK_OEM_COMMA);
MAP_ONE_VK_TO_SCAN(VK_OEM_MINUS);
MAP_ONE_VK_TO_SCAN(VK_OEM_PERIOD);
MAP_ONE_VK_TO_SCAN(VK_OEM_102);
#undef MAP_ONE_VK_TO_SCAN

			static const uint32 MAX_KEY_MAPPINGS(256);
			uint32 charCodes[MAX_KEY_MAPPINGS];

			wstring charKeyNames[MAX_KEY_MAPPINGS];
			const int32 charMappings = getCharKeyMap(charCodes, charKeyNames, MAX_KEY_MAPPINGS);
			for (int32 mappingIndex = 0; mappingIndex < charMappings; ++mappingIndex)
			{
				scanToVKMap.erase(charCodes[mappingIndex]);
			}

			for (auto it : scanToVKMap)
			{
				ADDKEYMAP(it.second, wstring(1, (wchar_t)it.first));
			}
		}
		BOOST_ASSERT(numMappings < maxMappings);
		return numMappings;
	}

	void WindowsPlatformMisc::getEnvironmentVariable(const TCHAR* variableName, TCHAR* result, int32 resultLength)
	{
		uint32 error = ::GetEnvironmentVariableW(variableName, result, resultLength);
		if( error <= 0)
		{
			*result = 0;
		}
	}

	void WindowsPlatformMisc::setEnvironmentVar(const TCHAR* variableName, const TCHAR* value)
	{
		uint32 error = ::SetEnvironmentVariable(variableName, value);
		if (error == 0)
		{

		}
	}

	bool WindowsPlatformMisc::verifyWindowsVersion(uint32 majorVersion, uint32 minorVersion)
	{
		OSVERSIONINFOEX version;
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		version.dwMajorVersion = majorVersion;
		version.dwMinorVersion = minorVersion;
		ULONGLONG conditionMask = 0;
		conditionMask = VerSetConditionMask(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
		conditionMask = VerSetConditionMask(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
		return !!VerifyVersionInfo(&version, VER_MAJORVERSION | VER_MINORVERSION, conditionMask);
	}

	static TCHAR* GMessageBoxText = NULL;

	static TCHAR* GMessageBoxCaption = NULL;

	static bool GCancelButtonEnabled = false;

	int messageBoxExtInternal(EAppMsgType::Type msgType, HWND handleWnd, const TCHAR* text, const TCHAR* caption)
	{
		GMessageBoxText = (TCHAR*)text;
		GMessageBoxCaption = (TCHAR*)caption;

		/*switch (msgType)
		{
		case Air::EAppMsgType::YesNoYesAllNoAll:
			GCancelButtonEnabled = false;
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_YESNO2ALL))
			break;
		case Air::EAppMsgType::YesNoYesAllNoAllCancel:
			break;
		case Air::EAppMsgType::YesNoYesAll:
			break;
		default:
			break;
		}*/
		return -1;
	}

	EAppReturnType::Type WindowsPlatformMisc::messageBoxExt(EAppMsgType::Type msgType, const TCHAR* text, const TCHAR* caption)
	{
		SlowHeartBeatScope suspendHeartBeat;
		HWND parentWindow = (HWND)NULL;
		switch (msgType)
		{
		case Air::EAppMsgType::Ok:
		{
			MessageBox(parentWindow, text, caption, MB_OK | MB_SYSTEMMODAL);
			return EAppReturnType::Ok;
		}
		case Air::EAppMsgType::YesNo:
		{
			int32 ret = MessageBox(parentWindow, text, caption, MB_YESNO | MB_SYSTEMMODAL);
			return ret == IDYES ? EAppReturnType::Yes : EAppReturnType::No;
		}
		case Air::EAppMsgType::OkCancel:
		{
			int32 ret = MessageBox(parentWindow, text, caption, MB_OKCANCEL | MB_SYSTEMMODAL);
			return ret == IDOK ? EAppReturnType::Ok : EAppReturnType::Cancel;
		}
		case Air::EAppMsgType::YesNoCancel:
		{
			int32 ret = MessageBox(parentWindow, text, caption, MB_YESNOCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
			return ret == IDYES ? EAppReturnType::Yes : (ret == IDNO ? EAppReturnType::No : EAppReturnType::Cancel);
		}
		case Air::EAppMsgType::CancelRetryContinue:
		{
			int32 ret = MessageBox(parentWindow, text, caption, MB_CANCELTRYCONTINUE | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SYSTEMMODAL);
			return ret == IDCANCEL ? EAppReturnType::Cancel : (ret == IDTRYAGAIN ? EAppReturnType::Retry : EAppReturnType::Continue);
		}
		case Air::EAppMsgType::YesNoYesAllNoAll:
		{
			return (EAppReturnType::Type)messageBoxExtInternal(EAppMsgType::YesNoYesAllNoAll, parentWindow, text, caption);
		}
		case Air::EAppMsgType::YesNoYesAllNoAllCancel:
			return (EAppReturnType::Type)messageBoxExtInternal(EAppMsgType::YesNoYesAllNoAllCancel, parentWindow, text, caption);
		case Air::EAppMsgType::YesNoYesAll:
			return (EAppReturnType::Type)messageBoxExtInternal(EAppMsgType::YesNoYesAll, parentWindow, text, caption);
		default:
			break;
		}
		return EAppReturnType::Cancel;
	}
}
