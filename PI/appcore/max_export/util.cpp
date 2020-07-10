
#include "util.h"

std::string extractRelPath(const TCHAR *fullPath)
{
	setlocale(LC_ALL,"");

	char mb[MAX_FILE_PATH];
	std::size_t l;
	wcstombs_s(&l, mb, MAX_FILE_PATH, fullPath, MAX_FILE_PATH);

	std::string strFullPath(mb);

	std::size_t pos1 = strFullPath.find("piassets");
	std::size_t pos2 = strFullPath.find_last_of('\\');

	if(pos1 == std::string::npos || pos2 == std::string::npos)
		return "";

	std::string strRet = strFullPath.substr(pos1, pos2 - pos1 + 1);

	for(std::size_t i = 0; i < strRet.length(); ++i){
		if(strRet[i] == '\\')
			strRet[i] = '/';
	}

	return strRet;
}

std::string extractAbsPath(const TCHAR *fullPath)
{
	setlocale(LC_ALL,"");

	char mb[MAX_FILE_PATH];
	std::size_t l;
	wcstombs_s(&l, mb, MAX_FILE_PATH, fullPath, MAX_FILE_PATH);

	std::string strFullPath(mb);

	std::size_t pos1 = strFullPath.find_last_of('\\');

	if(pos1 == std::string::npos)
		return "";

	std::string strRet = strFullPath.substr(0, pos1+1);

	return strRet;
}

std::string extractName(const TCHAR *fullPath, bool bSuffix)
{
	setlocale(LC_ALL,"");

	char mb[MAX_FILE_PATH];
	std::size_t l;
	wcstombs_s(&l, mb, MAX_FILE_PATH, fullPath, MAX_FILE_PATH);

	std::string strFullPath(mb);

	std::size_t pos1 = strFullPath.find_last_of('\\');
	std::size_t pos2 = strFullPath.find_last_of('.');

	if(pos1 == std::string::npos || pos2 == std::string::npos)
		return "";

	std::string strRet;
	if(bSuffix)
		strRet = strFullPath.substr(pos1+1, -1);
	else
		strRet = strFullPath.substr(pos1+1, pos2 - pos1 - 1);

	return strRet;
}

void createDir(std::string strDir)
{
	TCHAR wc[MAX_FILE_PATH];
	std::size_t l;
	mbstowcs_s(&l, wc, MAX_FILE_PATH, strDir.c_str(), MAX_FILE_PATH);
	CreateDirectory(wc, NULL);
}


