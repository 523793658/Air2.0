
#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <windows.h>

#define MAX_FILE_PATH 1024

std::string extractRelPath(const TCHAR *fullPath);

std::string extractName(const TCHAR *fullPath, bool bSuffix = false);

std::string extractAbsPath(const TCHAR *fullPath);

void createDir(std::string strDir);



#endif