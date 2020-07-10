#ifndef _GameTools_H_
#define _GameTools_H_

#include "pi_lib.h"
#include <string>
#include <Windows.h>
#include "GameTimer.h"

namespace pi
{
	class Task;
}

struct MountFileData {
	// 文件缓冲内存
	char* fileBuf;

	// 文件缓冲大小
	int64 fileSize;

	// 文件错误类型
	int errorType;

	// 文件错误信息
	wchar_t* errorInfo;

	MountFileData()
		: fileBuf(nullptr)
		, fileSize(0)
		, errorType(0)
		, errorInfo(nullptr)
	{
	}
};

namespace GameTools {

	// 读取mount文件数据
	MountFileData ReadMountFile(const wchar_t* fileName);
	std::string GetAppDir();

	BOOL openEXE(std::wstring workDir, std::wstring exePath, std::wstring cmd);

	void AsyncInvoke(pi::Task* task);

	class TimeState {
	private:
		GameTimer* mnTimer;
		std::wstring stateName;
	public:
		TimeState();
		TimeState(wchar_t* name);
		~TimeState();
	};

}

#endif