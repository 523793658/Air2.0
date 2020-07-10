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
	// �ļ������ڴ�
	char* fileBuf;

	// �ļ������С
	int64 fileSize;

	// �ļ���������
	int errorType;

	// �ļ�������Ϣ
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

	// ��ȡmount�ļ�����
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