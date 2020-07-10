/*
This file created by python script
Author: G:\code\cef\chromium\src\cef\libcef\pi_bindings\pi_vcall\vcall_function_creator.py
Date  : 2015-05-04 14:37:30.977000
*/
#include "pi_vcall_fun.h"

int VCALL_INT(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	int ret = 0;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_INT_0 vcall = (VCALL_INT_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_INT_1 vcall = (VCALL_INT_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_INT_2 vcall = (VCALL_INT_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_INT_3 vcall = (VCALL_INT_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_INT_4 vcall = (VCALL_INT_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_INT_5 vcall = (VCALL_INT_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_INT_6 vcall = (VCALL_INT_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_INT_7 vcall = (VCALL_INT_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_INT_8 vcall = (VCALL_INT_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_INT_9 vcall = (VCALL_INT_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_INT_10 vcall = (VCALL_INT_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_INT_11 vcall = (VCALL_INT_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_INT_12 vcall = (VCALL_INT_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_INT_13 vcall = (VCALL_INT_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_INT_14 vcall = (VCALL_INT_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_INT_15 vcall = (VCALL_INT_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_INT_16 vcall = (VCALL_INT_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_INT_17 vcall = (VCALL_INT_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_INT_18 vcall = (VCALL_INT_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_INT_19 vcall = (VCALL_INT_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_INT_20 vcall = (VCALL_INT_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

uint VCALL_UINT(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	uint ret = 0;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_UINT_0 vcall = (VCALL_UINT_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_UINT_1 vcall = (VCALL_UINT_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_UINT_2 vcall = (VCALL_UINT_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_UINT_3 vcall = (VCALL_UINT_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_UINT_4 vcall = (VCALL_UINT_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_UINT_5 vcall = (VCALL_UINT_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_UINT_6 vcall = (VCALL_UINT_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_UINT_7 vcall = (VCALL_UINT_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_UINT_8 vcall = (VCALL_UINT_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_UINT_9 vcall = (VCALL_UINT_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_UINT_10 vcall = (VCALL_UINT_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_UINT_11 vcall = (VCALL_UINT_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_UINT_12 vcall = (VCALL_UINT_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_UINT_13 vcall = (VCALL_UINT_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_UINT_14 vcall = (VCALL_UINT_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_UINT_15 vcall = (VCALL_UINT_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_UINT_16 vcall = (VCALL_UINT_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_UINT_17 vcall = (VCALL_UINT_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_UINT_18 vcall = (VCALL_UINT_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_UINT_19 vcall = (VCALL_UINT_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_UINT_20 vcall = (VCALL_UINT_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

int64 VCALL_INT64(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	int64 ret = 0ll;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_INT64_0 vcall = (VCALL_INT64_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_INT64_1 vcall = (VCALL_INT64_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_INT64_2 vcall = (VCALL_INT64_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_INT64_3 vcall = (VCALL_INT64_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_INT64_4 vcall = (VCALL_INT64_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_INT64_5 vcall = (VCALL_INT64_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_INT64_6 vcall = (VCALL_INT64_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_INT64_7 vcall = (VCALL_INT64_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_INT64_8 vcall = (VCALL_INT64_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_INT64_9 vcall = (VCALL_INT64_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_INT64_10 vcall = (VCALL_INT64_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_INT64_11 vcall = (VCALL_INT64_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_INT64_12 vcall = (VCALL_INT64_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_INT64_13 vcall = (VCALL_INT64_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_INT64_14 vcall = (VCALL_INT64_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_INT64_15 vcall = (VCALL_INT64_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_INT64_16 vcall = (VCALL_INT64_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_INT64_17 vcall = (VCALL_INT64_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_INT64_18 vcall = (VCALL_INT64_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_INT64_19 vcall = (VCALL_INT64_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_INT64_20 vcall = (VCALL_INT64_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

uint64 VCALL_UINT64(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	uint64 ret = 0ull;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_UINT64_0 vcall = (VCALL_UINT64_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_UINT64_1 vcall = (VCALL_UINT64_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_UINT64_2 vcall = (VCALL_UINT64_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_UINT64_3 vcall = (VCALL_UINT64_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_UINT64_4 vcall = (VCALL_UINT64_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_UINT64_5 vcall = (VCALL_UINT64_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_UINT64_6 vcall = (VCALL_UINT64_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_UINT64_7 vcall = (VCALL_UINT64_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_UINT64_8 vcall = (VCALL_UINT64_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_UINT64_9 vcall = (VCALL_UINT64_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_UINT64_10 vcall = (VCALL_UINT64_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_UINT64_11 vcall = (VCALL_UINT64_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_UINT64_12 vcall = (VCALL_UINT64_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_UINT64_13 vcall = (VCALL_UINT64_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_UINT64_14 vcall = (VCALL_UINT64_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_UINT64_15 vcall = (VCALL_UINT64_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_UINT64_16 vcall = (VCALL_UINT64_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_UINT64_17 vcall = (VCALL_UINT64_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_UINT64_18 vcall = (VCALL_UINT64_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_UINT64_19 vcall = (VCALL_UINT64_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_UINT64_20 vcall = (VCALL_UINT64_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

float VCALL_FLOAT(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	float ret = 0.0f;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_FLOAT_0 vcall = (VCALL_FLOAT_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_FLOAT_1 vcall = (VCALL_FLOAT_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_FLOAT_2 vcall = (VCALL_FLOAT_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_FLOAT_3 vcall = (VCALL_FLOAT_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_FLOAT_4 vcall = (VCALL_FLOAT_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_FLOAT_5 vcall = (VCALL_FLOAT_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_FLOAT_6 vcall = (VCALL_FLOAT_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_FLOAT_7 vcall = (VCALL_FLOAT_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_FLOAT_8 vcall = (VCALL_FLOAT_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_FLOAT_9 vcall = (VCALL_FLOAT_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_FLOAT_10 vcall = (VCALL_FLOAT_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_FLOAT_11 vcall = (VCALL_FLOAT_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_FLOAT_12 vcall = (VCALL_FLOAT_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_FLOAT_13 vcall = (VCALL_FLOAT_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_FLOAT_14 vcall = (VCALL_FLOAT_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_FLOAT_15 vcall = (VCALL_FLOAT_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_FLOAT_16 vcall = (VCALL_FLOAT_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_FLOAT_17 vcall = (VCALL_FLOAT_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_FLOAT_18 vcall = (VCALL_FLOAT_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_FLOAT_19 vcall = (VCALL_FLOAT_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_FLOAT_20 vcall = (VCALL_FLOAT_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

double VCALL_DOUBLE(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	double ret = 0.0f;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_DOUBLE_0 vcall = (VCALL_DOUBLE_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_DOUBLE_1 vcall = (VCALL_DOUBLE_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_DOUBLE_2 vcall = (VCALL_DOUBLE_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_DOUBLE_3 vcall = (VCALL_DOUBLE_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_DOUBLE_4 vcall = (VCALL_DOUBLE_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_DOUBLE_5 vcall = (VCALL_DOUBLE_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_DOUBLE_6 vcall = (VCALL_DOUBLE_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_DOUBLE_7 vcall = (VCALL_DOUBLE_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_DOUBLE_8 vcall = (VCALL_DOUBLE_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_DOUBLE_9 vcall = (VCALL_DOUBLE_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_DOUBLE_10 vcall = (VCALL_DOUBLE_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_DOUBLE_11 vcall = (VCALL_DOUBLE_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_DOUBLE_12 vcall = (VCALL_DOUBLE_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_DOUBLE_13 vcall = (VCALL_DOUBLE_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_DOUBLE_14 vcall = (VCALL_DOUBLE_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_DOUBLE_15 vcall = (VCALL_DOUBLE_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_DOUBLE_16 vcall = (VCALL_DOUBLE_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_DOUBLE_17 vcall = (VCALL_DOUBLE_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_DOUBLE_18 vcall = (VCALL_DOUBLE_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_DOUBLE_19 vcall = (VCALL_DOUBLE_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_DOUBLE_20 vcall = (VCALL_DOUBLE_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

char* VCALL_STR(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	char* ret = { 0 };
	switch (argsCount)
	{
	case 0:
	{
		VCALL_STR_0 vcall = (VCALL_STR_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_STR_1 vcall = (VCALL_STR_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_STR_2 vcall = (VCALL_STR_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_STR_3 vcall = (VCALL_STR_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_STR_4 vcall = (VCALL_STR_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_STR_5 vcall = (VCALL_STR_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_STR_6 vcall = (VCALL_STR_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_STR_7 vcall = (VCALL_STR_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_STR_8 vcall = (VCALL_STR_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_STR_9 vcall = (VCALL_STR_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_STR_10 vcall = (VCALL_STR_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_STR_11 vcall = (VCALL_STR_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_STR_12 vcall = (VCALL_STR_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_STR_13 vcall = (VCALL_STR_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_STR_14 vcall = (VCALL_STR_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_STR_15 vcall = (VCALL_STR_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_STR_16 vcall = (VCALL_STR_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_STR_17 vcall = (VCALL_STR_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_STR_18 vcall = (VCALL_STR_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_STR_19 vcall = (VCALL_STR_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_STR_20 vcall = (VCALL_STR_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

wchar* VCALL_WSTR(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	wchar* ret = { 0 };
	switch (argsCount)
	{
	case 0:
	{
		VCALL_WSTR_0 vcall = (VCALL_WSTR_0)pFunc;
		ret = vcall();
		break;
	}
	case 1:
	{
		VCALL_WSTR_1 vcall = (VCALL_WSTR_1)pFunc;
		ret = vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_WSTR_2 vcall = (VCALL_WSTR_2)pFunc;
		ret = vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_WSTR_3 vcall = (VCALL_WSTR_3)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_WSTR_4 vcall = (VCALL_WSTR_4)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_WSTR_5 vcall = (VCALL_WSTR_5)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_WSTR_6 vcall = (VCALL_WSTR_6)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_WSTR_7 vcall = (VCALL_WSTR_7)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_WSTR_8 vcall = (VCALL_WSTR_8)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_WSTR_9 vcall = (VCALL_WSTR_9)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_WSTR_10 vcall = (VCALL_WSTR_10)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_WSTR_11 vcall = (VCALL_WSTR_11)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_WSTR_12 vcall = (VCALL_WSTR_12)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_WSTR_13 vcall = (VCALL_WSTR_13)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_WSTR_14 vcall = (VCALL_WSTR_14)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_WSTR_15 vcall = (VCALL_WSTR_15)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_WSTR_16 vcall = (VCALL_WSTR_16)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_WSTR_17 vcall = (VCALL_WSTR_17)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_WSTR_18 vcall = (VCALL_WSTR_18)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_WSTR_19 vcall = (VCALL_WSTR_19)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_WSTR_20 vcall = (VCALL_WSTR_20)pFunc;
		ret = vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}
	return ret;
}

void VCALL_VOID(void* pFunc, void* argsData, int argsDataLength)
{
	int argsCount = argsDataLength / sizeof(int);
	int* arguments = (int*)argsData;
	switch (argsCount)
	{
	case 0:
	{
		VCALL_VOID_0 vcall = (VCALL_VOID_0)pFunc;
		vcall();
		break;
	}
	case 1:
	{
		VCALL_VOID_1 vcall = (VCALL_VOID_1)pFunc;
		vcall(arguments[0]);
		break;
	}
	case 2:
	{
		VCALL_VOID_2 vcall = (VCALL_VOID_2)pFunc;
		vcall(arguments[0], arguments[1]);
		break;
	}
	case 3:
	{
		VCALL_VOID_3 vcall = (VCALL_VOID_3)pFunc;
		vcall(arguments[0], arguments[1], arguments[2]);
		break;
	}
	case 4:
	{
		VCALL_VOID_4 vcall = (VCALL_VOID_4)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	}
	case 5:
	{
		VCALL_VOID_5 vcall = (VCALL_VOID_5)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	}
	case 6:
	{
		VCALL_VOID_6 vcall = (VCALL_VOID_6)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	}
	case 7:
	{
		VCALL_VOID_7 vcall = (VCALL_VOID_7)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	}
	case 8:
	{
		VCALL_VOID_8 vcall = (VCALL_VOID_8)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	}
	case 9:
	{
		VCALL_VOID_9 vcall = (VCALL_VOID_9)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	}
	case 10:
	{
		VCALL_VOID_10 vcall = (VCALL_VOID_10)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);
		break;
	}
	case 11:
	{
		VCALL_VOID_11 vcall = (VCALL_VOID_11)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);
		break;
	}
	case 12:
	{
		VCALL_VOID_12 vcall = (VCALL_VOID_12)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);
		break;
	}
	case 13:
	{
		VCALL_VOID_13 vcall = (VCALL_VOID_13)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
		break;
	}
	case 14:
	{
		VCALL_VOID_14 vcall = (VCALL_VOID_14)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);
		break;
	}
	case 15:
	{
		VCALL_VOID_15 vcall = (VCALL_VOID_15)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
		break;
	}
	case 16:
	{
		VCALL_VOID_16 vcall = (VCALL_VOID_16)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);
		break;
	}
	case 17:
	{
		VCALL_VOID_17 vcall = (VCALL_VOID_17)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);
		break;
	}
	case 18:
	{
		VCALL_VOID_18 vcall = (VCALL_VOID_18)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);
		break;
	}
	case 19:
	{
		VCALL_VOID_19 vcall = (VCALL_VOID_19)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);
		break;
	}
	case 20:
	{
		VCALL_VOID_20 vcall = (VCALL_VOID_20)pFunc;
		vcall(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);
		break;
	}
	default:
	{
		break;
	}
	}

}

