#ifndef INCLUDE_APPCORE_UTIL_H
#define INCLUDE_APPCORE_UTIL_H
#include "pi_lib.h"

typedef struct  
{
	float* data;
	int width;
	int height;
} PiHeightMap;

void PI_API app_open_file_explorer(char* filePath);

uint64 PI_API app_get_memory();

PiHeightMap* PI_API create_height_map(float* data, int width, int height);

void PI_API destroy_height_map(PiHeightMap* heightMap);

float PI_API get_pos_height(PiHeightMap* heightMap, int posX, int posZ);

wchar* PI_API get_user_name();

int PI_API app_get_system_info(int infoKey);

#endif // !INCLUDE_APPCORE_UTIL_H
