#include "platform.h"
#include "windows.h"
#include <string>
PI_BEGIN_DECLS

const wchar* app_get_platform_info()
{
	SYSTEM_INFO info;        //用SYSTEM_INFO结构判断64位AMD处理器   
	GetSystemInfo(&info);    //调用GetSystemInfo函数填充结构   
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	std::wstring str = L"unknown system";
	if (GetVersionEx((OSVERSIONINFO *)&os))
	{
		switch (os.dwMajorVersion)
		{
		case 5:
			switch (os.dwMinorVersion)
			{
			case 0:
				str = L"Windows 2000";
				if (os.wSuiteMask == VER_SUITE_ENTERPRISE)
					str += L"Advanced Server";
				break;
			case 1:
				str = L"Windows XP";
				if (os.wSuiteMask == VER_SUITE_EMBEDDEDNT)
					str += L" Embedded";
				else if (os.wSuiteMask == VER_SUITE_PERSONAL)
					str += L" Home Edition";
				else
					str += L" Professional";
				break;
			case 2:
				if ((os.wProductType == VER_NT_WORKSTATION) && (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
				{
					str = L"Windows XP Professional x64 Edition";
				}
				else if (GetSystemMetrics(SM_SERVERR2) == 0)
				{
					str = L"Windows Server 2003";
				}
				else if (os.wSuiteMask & VER_SUITE_WH_SERVER)
				{
					str = L"Windows Home Server";
				}
				else if (GetSystemMetrics(SM_SERVERR2) != 0)
				{
					str = L"Windows Server 2003 R2";
				}

				if (GetSystemMetrics(SM_SERVERR2) == 0
					&& os.wSuiteMask == VER_SUITE_BLADE)  //Windows Server 2003   
					str += L" Web Edition";
				else if (GetSystemMetrics(SM_SERVERR2) == 0
					&& os.wSuiteMask == VER_SUITE_COMPUTE_SERVER)
					str += L" Compute Cluster Edition";
				else if (GetSystemMetrics(SM_SERVERR2) == 0
					&& os.wSuiteMask == VER_SUITE_STORAGE_SERVER)
					str += L" Storage Server";
				else if (GetSystemMetrics(SM_SERVERR2) == 0
					&& os.wSuiteMask == VER_SUITE_DATACENTER)
					str += L" Datacenter Edition";
				else if (GetSystemMetrics(SM_SERVERR2) == 0
					&& os.wSuiteMask == VER_SUITE_ENTERPRISE)
					str += L" Enterprise Edition";
				else if (GetSystemMetrics(SM_SERVERR2) != 0
					&& os.wSuiteMask == VER_SUITE_STORAGE_SERVER)
					str += L" Storage Server";
				break;
			default:
				break;
			}
			break;
		case 6:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					str = L"Windows Vista";
				}
				else
				{
					str = L"Windows Server 2008";
					if (os.wSuiteMask == VER_SUITE_DATACENTER)
					{
						str += L" Datacenter Server";
					}
					else if (os.wSuiteMask == VER_SUITE_ENTERPRISE)
					{
						str += L" Enterprise";
					}
				}
				break;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					str = L"Windows 7";
				}
				else
				{
					str = L"Windows Server 2008 R2";
				}
				break;

			case 2:
				break;
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					str = L"Windows 8";
				}
				else
				{
					str = L"Windows Server 2012";
				}

			case 3:
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					str = L"Windows 8.1";
				}
				else
				{
					str = L"Windows Server 2012 R2";
				}
				break;
			default:
				break;
			}
			break;
		case 10:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					str = L"Windows 10";
				}
				else
				{
					str = L"Windows Server 2016";
				}
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
	if (!str.empty())
	{
		if (os.wSuiteMask == VER_SUITE_PERSONAL)
		{
			str += L" Home Basic";
		}
		else if (os.wSuiteMask == VER_SUITE_SINGLEUSERTS)
		{
			str += L" Professional";
		}
		else if (os.wSuiteMask == VER_SUITE_ENTERPRISE)
		{
			str += L" Enterprise";
		}
		str += L" ";
		str += os.szCSDVersion;
	}
	
	return pi_wstr_dup(str.c_str());
}

PI_END_DECLS
