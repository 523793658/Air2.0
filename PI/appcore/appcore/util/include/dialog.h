#ifndef INCLUDE_APPCORE_DIALOG_H
#define INCLUDE_APPCORE_DIALOG_H
#include "pi_lib.h"


wchar* PI_API app_open_file_dialog(wchar* title, wchar* initPath, wchar* filter);

wchar* PI_API app_open_files_dialog(wchar* title, wchar* initPath, wchar* ffilter);

wchar* PI_API app_save_file_dialog(wchar* title, wchar* initPath, wchar* filter);

uint PI_API app_color_dialog(uint);

wchar* PI_API app_browse_directory_dialog(wchar* title, wchar* initDir);


#endif // !INCLUDE_APPCORE_UTIL_H
