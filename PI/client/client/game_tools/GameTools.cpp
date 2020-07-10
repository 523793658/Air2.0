#include "GameTools.h"
#include "game_service/BaseClass/Task.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Async/AsyncLoadResourceService.h"

namespace GameTools {

	MountFileData ReadMountFile(const wchar_t* fileName) {
		void* file;
		MountFileData result;
		file = pi_vfs_file_open(fileName, FILE_OPEN_READ);
		if (file == NULL)
		{
			result.errorType = pi_error_get_type();
			result.errorInfo = pi_error_get_message();
		}
		else
		{
			if (pi_vfs_file_size(file, &result.fileSize))
			{
				result.fileBuf = (char*)pi_malloc((uint)(result.fileSize + 1));
				if (pi_vfs_file_read(file, 0, 0, result.fileBuf, (uint)result.fileSize) == -1)
				{
					result.errorType = pi_error_get_type();
					result.errorInfo = pi_error_get_message();
					pi_free(result.fileBuf);
					result.fileBuf = nullptr;
				}
				else
				{
					result.fileBuf[result.fileSize] = '\0';
				}
			}
			else
			{
				result.errorType = pi_error_get_type();
				result.errorInfo = pi_error_get_message();
			}
			pi_vfs_file_close(file);
		}
		return result;
	}

	std::string GetAppDir()
	{
		char ret[MAX_PATH];
		GetModuleFileNameA(NULL, ret, MAX_PATH); 
		*(strrchr(ret, '\\')) = '\0';   
		return ret;
	}

	BOOL openEXE(std::wstring workDir, std::wstring exePath, std::wstring cmd)
	{
		STARTUPINFOW si = { sizeof(si) };
		si.wShowWindow = SW_NORMAL;
		si.dwFlags = STARTF_USESHOWWINDOW;

		PROCESS_INFORMATION pi;

		std::wstring cmdLine = workDir + L"\\" + exePath + L" " + cmd;

		BOOL bRet = ::CreateProcessW(
			NULL,
			const_cast<LPWSTR>(cmdLine.c_str()), //在Unicode版本中此参数不能为常量字符串，因为此参数会被修改	 
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si,
			&pi);
		return bRet;
	}

	void AsyncInvoke(pi::Task* task)
	{
		PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
		AsyncLoadResourceService *asyncService = reinterpret_cast<AsyncLoadResourceService*>(tpManager->GetServiceByName(L"AsyncLoadResource"));

		asyncService->AddTask(reinterpret_cast<pi::Task*>(task));
	}

	TimeState::TimeState()
	{
		mnTimer = new GameTimer();
		mnTimer->Reset();
		stateName = L"default";
	}
	TimeState::TimeState(wchar_t* name)
	{
		mnTimer = new GameTimer();
		mnTimer->Reset();
		stateName = name;
	}
	TimeState::~TimeState()
	{
		mnTimer->Tick();
		float nowTime = mnTimer->DeltaTime() * 1000;
		if (nowTime > 2.0f)
		{
			pi_log_print(LOG_INFO, "time state info: state name is %ws, duration is %f", stateName.c_str(), nowTime);
		}
	}
}
