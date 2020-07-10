#include "util.h"

#include <windows.h>

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif
#include <psapi.h> 
#include <setupapi.h>
#pragma  comment(lib,"Psapi.lib")
#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"netapi32.lib")

DWORD deax;
DWORD debx;
DWORD decx;
DWORD dedx;

typedef struct _ASTAT_
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER NameBuff[30];
} ASTAT, *PASTAT;

wchar* PI_API get_user_name()
{
	DWORD size = 0;
	wchar* name;
	GetUserName(NULL, &size);
	name = pi_new0(wchar, size);
	if (GetUserName(name, &size)) 
	{
		return name;
	}
	return 0;
}

void PI_API get_mac(byte * mac)
{
	ASTAT Adapter;
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM lenum;
	int i = 0;
	byte tempMac[20] = { 0 };

	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR *)&lenum;
	Ncb.ncb_length = sizeof(lenum);

	uRetCode = Netbios(&Ncb);

	for (i = 0; i < lenum.length && i < 5; i++)
	{
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios(&Ncb);

		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		strcpy((char *)Ncb.ncb_callname, "* ");
		Ncb.ncb_buffer = (unsigned char *)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios(&Ncb);

		if (uRetCode == 0)
		{
			sprintf(tempMac, "%02X-%02X-%02X-%02X-%02X-%02X ",
				Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5]
				);

			memcpy(mac + 20 * i, &tempMac, 20);
		}
	}
}

void ExeCPUID(DWORD veax)
{
	__asm
	{
		mov eax, veax
			cpuid
			mov deax, eax
			mov debx, ebx
			mov decx, ecx
			mov dedx, edx
	}
}

void PI_API get_cpu_info(byte *data)
{
	DWORD id = 0x80000002;
	memset(data, 0, sizeof(data));

	for (int t = 0; t < 3; t++)
	{
		ExeCPUID(id++);
		//每次循环结束,保存信息到数组
		memcpy(data + 16 * t + 0, &deax, 4);
		memcpy(data + 16 * t + 4, &debx, 4);
		memcpy(data + 16 * t + 8, &decx, 4);
		memcpy(data + 16 * t + 12, &dedx, 4);
	}
}

void PI_API app_open_file_explorer(char* filePath)
{
	char cmd[200] = "explorer.exe /select,";
	if(pi_str_cat(cmd, 200, filePath) > 200)
	{
		return;
	}
	system(cmd);
}

uint64 PI_API app_get_memory()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;  
    if(GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
       return (uint64)pmc.PrivateUsage;
    }
    return 0;  
}

uint64 PI_API app_get_total_memory()
{
	MEMORYSTATUSEX mstx;
	mstx.dwLength = sizeof(mstx);
	GlobalMemoryStatusEx(&mstx);
	return mstx.ullTotalPhys;
}

void PI_API get_graphics_card_info(byte *data)
{
	GUID pGUID = { 0x4D36E968, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 };

	HDEVINFO hDevInfo = 0;
	SP_DEVINFO_DATA spDevinofData = { 0 };
	byte szBuf[260] = { 0 };

	DWORD i = 1;
	BOOL bRtn = FALSE;
	//get the specified class

	//这是就通过guid找到了类的信息。
	hDevInfo = SetupDiGetClassDevs(&pGUID, 0, NULL, DIGCF_PRESENT);

	//枚举设备信息
	spDevinofData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &spDevinofData) && i < 5; i++)
	{
		//通过设备类，以及枚举出的设备信息数据获的具体的名称。
		bRtn = SetupDiGetDeviceRegistryPropertyA(hDevInfo, &spDevinofData, SPDRP_DEVICEDESC, 0L, (PBYTE)szBuf, 260, 0);
		memcpy(data + i * 260, &szBuf, 260);
	}
}

int PI_API app_get_system_info(int infoKey)
{
	return GetSystemMetrics(infoKey);
}

PiHeightMap* PI_API create_height_map(float* data, int width, int height)
{
	PiHeightMap* heightMap = pi_new0(PiHeightMap, 1);
	heightMap->height = height;
	heightMap->width = width;
	heightMap->data = pi_new0(float, width * height);
	pi_memcpy(heightMap->data, data, sizeof(float) * width * height);
	return heightMap;
}

void PI_API destroy_height_map(PiHeightMap* heightMap)
{
	pi_free(heightMap->data);
	pi_free(heightMap);
}

float PI_API get_pos_height(PiHeightMap* heightMap, int posX, int posZ)
{
	if (heightMap == NULL || heightMap->data == NULL) 
	{
		return 0;
	}
	if (posX > heightMap->width || posX < 0 || posZ > heightMap->height || posZ < 0) 
	{
		return 0;
	}
	return heightMap->data[posZ * heightMap->width + posX];
}