
#include <windows.h>
#include <dbghelp.h>
#include <string>
#include <tchar.h>
#include "map"

#include "vector"
#include "map"

#include <Shlwapi.h>
#include <pi_lib.h> 
#include  <direct.h>
#include "chrono"
#include <memory>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "crash_dump.h"
extern "C"
{
#include "upload.h"
}


void myInvalidParameterHandler(const wchar_t* expression,
	const wchar_t* function,
	const wchar_t* file,
	unsigned int line,
	uintptr_t pReserved)
{
	__asm int 3;
}

void myPurecallHandler(void)
{
	__asm int 3;
}

upLoadInfo::upLoadInfo()
{
	::sprintf(m_account, "default");
	::sprintf(m_userName, "default");
	::sprintf(m_ftpServerUrl, "default");
	::sprintf(m_ftpUserName, "default");
	::sprintf(m_ftpPassword, "default");
	::sprintf(m_serverVersion, "default");
	::sprintf(m_uploadFileStr, "default");
}


upLoadInfo::~upLoadInfo()
{

}

void upLoadInfo::setServerVersion(char* serverVersion)
{
	::sprintf(m_serverVersion, "%s", serverVersion);
}

void upLoadInfo::setUserMsg(wchar_t* account, wchar_t* userName)
{
	::sprintf(m_account, ("%s"), pi_wstr_to_str(account, PI_CP_UTF8));
	::sprintf(m_userName, ("%s"), pi_wstr_to_str(userName, PI_CP_UTF8));
}

void upLoadInfo::setFtpMsg(char* ftpServerUrl, char* ftpUserName, char* ftpPassword)
{
	::sprintf(m_ftpServerUrl, ("%s"), ftpServerUrl);
	::sprintf(m_ftpUserName, ("%s"), ftpUserName);
	::sprintf(m_ftpPassword, ("%s"), ftpPassword);
}

//filePrefix 文件前缀
void upLoadInfo::upload_file(char* fileString, char* filePrefix)
{
// 	STARTUPINFO si;
// 	memset(&si, 0, sizeof(si));
// 	si.cb = sizeof(si);
// 	PROCESS_INFORMATION pi;
// 
// 	//获取exe所在目录路径
// 	TCHAR chExePath[MAX_PATH] = { 0 };
// 	::GetModuleFileName(NULL, chExePath, MAX_PATH);
// 	(_tcsrchr(chExePath, _T('\\')))[1] = 0;
// 
// 	//组装命令行
// 	TCHAR lpCommandLine[MAX_PATH * 2] = { 0 };
// 	::wsprintf(lpCommandLine, _T("%lsmw_dump.exe %s %s %s %s %s %s %s"), chExePath, m_account, m_userName,
// 		m_ftpServerUrl, m_ftpUserName, m_ftpPassword, m_serverVersion, filePrefix);
// 
// 	//加入需要上传的文件的路径
// 	::wsprintf(lpCommandLine, _T("%ls %s"), lpCommandLine, fileString);
// 
// 	//组装exe路径
// 	TCHAR lpExe[MAX_PATH] = { 0 };
// 	::wsprintf(lpExe, _T("%lsmw_dump.exe"), chExePath);
// 
// 	if (!CreateProcess(lpExe, lpCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
// 	{
// 		MessageBox(NULL, L"打开进程失败", L"错误", MB_OK);
// 	}
}

void upLoadInfo::upload_log_file(char* fileString)
{
	pi_log_write_in_main_thread();
	pi_log_copy();
	upLoadInfo::upload_file(fileString, "file");
}

//这个是预设的上传文件，就是每一次都会上传的
void upLoadInfo::setUploadFileStr(char* uploadFileStr)
{
	::sprintf(m_uploadFileStr, "%s", uploadFileStr);
}

//设置是否上传dump
void upLoadInfo::setUploadDump(bool uploadDump)
{
	m_uploadDump = uploadDump;
}

//获取是否上传dump
bool upLoadInfo::getUploadDump()
{
	return m_uploadDump;
}


template<typename T>
std::vector<T> split(T src, T s)
{
	boost::regex reg(s.c_str());
	std::vector<T> vec;
	boost::sregex_token_iterator it(src.begin(), src.end(), reg, -1);
	boost::sregex_token_iterator end;
	while (it != end)
	{
		vec.push_back(*it++);
	}
	return vec;
}

void upLoadInfo::execUpload(char* uploadFileStr)
{
	//m_uploadFileStr
	
	std::vector<std::string> paths = split(std::string(m_uploadFileStr), std::string(" "));
	char* p = pi_log_get_path();//getenv("log_path");
	std::string log_path = p;

	UploadTask* task = (UploadTask*)create_upload_task(m_ftpServerUrl, m_ftpUserName, m_ftpPassword, TRUE);
	std::string romote_path = std::string(m_serverVersion) + std::string("/") + std::string(m_account) + "/" + std::string(m_userName);
	for (auto it : paths)
	{
		
		boost::regex reg("%log_path%");
		boost::regex_replace(it, reg, log_path);
		add_file_to_upload_task(task, it.c_str(), romote_path.c_str(), "dump");
	}

	add_file_to_upload_task(task, uploadFileStr, romote_path.c_str(), "dump");

	upload_task_start(task);
	free_upload_task(task);
}

DWORD CCrashDump::m_DumpType = MiniDumpWithIndirectlyReferencedMemory;//MiniDumpWithFullMemory;
upLoadInfo CCrashDump::m_upLoad;


static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDisableFun(__in LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}
LPCSTR g_lpszKey = "";
CCrashDump::CCrashDump(LPCSTR lpKey)
{
	::SetUnhandledExceptionFilter(CCrashDump::TopLevelExceptionFilter);
	if (lpKey[0])
	{
		AddVectoredExceptionHandler(FALSE, CCrashDump::TopLevelExceptionFilterFirst);
	}

	unsigned char code[8];
	int size = 0;
	code[size++] = 0x33;
	code[size++] = 0xc0;
	code[size++] = 0xc2;
	code[size++] = 0x08;
	code[size++] = 0x00;
	DWORD dw = 0;
	DWORD dwOldFlag, dwTempFlag;

	if (lpKey[0])
	{
		VirtualProtectEx(INVALID_HANDLE_VALUE, AddVectoredExceptionHandler, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);
		WriteProcessMemory(INVALID_HANDLE_VALUE, AddVectoredExceptionHandler, code, size, &dw);
		VirtualProtectEx(INVALID_HANDLE_VALUE, AddVectoredExceptionHandler, size, dwOldFlag, &dwTempFlag);
	}

	code[3] = 0x04;
	VirtualProtectEx(INVALID_HANDLE_VALUE, SetUnhandledExceptionFilter, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);
	WriteProcessMemory(INVALID_HANDLE_VALUE, SetUnhandledExceptionFilter, code, size, &dw);
	VirtualProtectEx(INVALID_HANDLE_VALUE, SetUnhandledExceptionFilter, size, dwOldFlag, &dwTempFlag);

	//_set_invalid_parameter_handler(myInvalidParameterHandler);
	//_set_purecall_handler(myPurecallHandler);
	//CreateDirectory(DUMP_PATH, NULL);
	g_lpszKey = lpKey;
}


CCrashDump::~CCrashDump()
{

}

void CCrashDump::AddTime(LPTSTR lpStr)
{
	if (!lpStr)
	{
		return;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	//format dump2010-03-08-15:58
	::wsprintf(lpStr, _T("%s-%d-%02d-%02d-%02d-%02d.dmp"), lpStr, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

}

typedef BOOL(WINAPI* MINIDUMP_WRITE_DUMP)(
	IN HANDLE hProcess,
	IN DWORD ProcessId,
	IN HANDLE hFile,
	IN MINIDUMP_TYPE DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
	IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
	IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
	);

DWORD WINAPI CCrashDump::threadFunc(LPVOID lpParam)
{
	PEXCEPTION_POINTERS ExceptionInfo = (PEXCEPTION_POINTERS)lpParam;
	LONG ret = EXCEPTION_CONTINUE_SEARCH;
	pi_log_print(LOG_INFO, "system log: start dump");
	if (!PathIsDirectory(LOG_PATH))
	{
		CreateDirectory(LOG_PATH, NULL);
	}
	if (!PathIsDirectory(DUMP_PATH))
	{
		CreateDirectory(DUMP_PATH, NULL);
	}

	TCHAR chDumpModule[MAX_PATH] = { 0 };
	swprintf_s(chDumpModule, _T("%s%s-%x"), DUMP_PATH, L"dump", ExceptionInfo->ExceptionRecord->ExceptionAddress);
	AddTime(chDumpModule);
	HANDLE hFile = CreateFile(
		chDumpModule,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_WRITE_THROUGH*/,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		//MessageBox(NULL, _T("CREATE DUMP FILE ERROR"), _T("DUMP ERROR"), 0);
		pi_log_print(LOG_INFO, "system log: create dump file error");
		return ret;
	}
	//::MessageBoxA(NULL, "3", "3", 0);
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = ExceptionInfo;
	eInfo.ClientPointers = FALSE;
	BOOL bResult = MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		(MINIDUMP_TYPE)m_DumpType,
		&eInfo,
		NULL,
		NULL);

	if (bResult)
	{
		ret = EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		pi_log_print(LOG_INFO, "create dump error, reason = %d", GetLastError());
		//TCHAR  errorMsg[MAX_PATH] = { 0 };
		//swprintf_s(errorMsg, _T("%u"), GetLastError());
		//MessageBox(NULL, errorMsg, _T("DUMP ERROR"), 0);
	}

	CloseHandle(hFile);

	pi_log_print(LOG_INFO, "system log: finish dump file");

	//由于log是一直打开握着的，所以需要关闭，否则无法压缩
	pi_log_close();
	//上传文件
	if (CCrashDump::m_upLoad.getUploadDump())
	{
		char p[MAX_PATH] = { '\0' };
		char defaultC = '*';
		BOOL x;
		WideCharToMultiByte(CP_ACP, 0, chDumpModule, pi_wstrlen(chDumpModule), p, MAX_PATH, &defaultC, &x);
		CCrashDump::m_upLoad.execUpload(p);
	}

	//关闭进程
	BOOL back = ::TerminateProcess(GetCurrentProcess(), 0);
	if (!back)
	{
		//由于log已经关闭，所以不能使用pi_log_print,但是又需要一定的信息，所以使用messageBox
		TCHAR  errorMsg1[MAX_PATH] = { 0 };
		swprintf_s(errorMsg1, _T("%u"), GetLastError());
		MessageBox(NULL, errorMsg1, _T("TERMINATE PROCESS ERROR"), 0);
	}
	return 0;
}

DWORD WINAPI CCrashDump::threadFuncNotUpdate(LPVOID lpParam)
{
	PEXCEPTION_POINTERS ExceptionInfo = (PEXCEPTION_POINTERS)lpParam;
	LONG ret = EXCEPTION_CONTINUE_SEARCH;
	pi_log_print(LOG_INFO, "system log: start dump");
	if (!PathIsDirectory(LOG_PATH))
	{
		CreateDirectory(LOG_PATH, NULL);
	}
	if (!PathIsDirectory(DUMP_PATH))
	{
		CreateDirectory(DUMP_PATH, NULL);
	}

	TCHAR chDumpModule[MAX_PATH] = { 0 };
	swprintf_s(chDumpModule, _T("%s%s-%x"), DUMP_PATH, L"dump", ExceptionInfo->ExceptionRecord->ExceptionAddress);
	AddTime(chDumpModule);
	HANDLE hFile = CreateFile(
		chDumpModule,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_WRITE_THROUGH*/,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return ret;
	}
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = ExceptionInfo;
	eInfo.ClientPointers = FALSE;
	BOOL bResult = MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		(MINIDUMP_TYPE)m_DumpType,
		&eInfo,
		NULL,
		NULL);

	if (bResult)
	{
		ret = EXCEPTION_EXECUTE_HANDLER;
	}

	CloseHandle(hFile);
	return 0;
}

LONG WINAPI CCrashDump::TopLevelExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	static HANDLE hThread = NULL;
	static DWORD dwThreadID = 0;
	hThread = CreateThread(NULL, 0, threadFunc, (LPVOID)ExceptionInfo, 0, &dwThreadID);
	if (WAIT_TIMEOUT == WaitForSingleObject(hThread, 200000))
	{
		threadFunc((LPVOID)ExceptionInfo);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI CCrashDump::TopLevelExceptionFilterFirst(PEXCEPTION_POINTERS ExceptionInfo)
{
	bool NeedDump = false;
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
		//case STATUS_WAIT_0:
		//case STATUS_ABANDONED_WAIT_0:
		//case STATUS_USER_APC:
		//case STATUS_TIMEOUT: 
		//case STATUS_PENDING: 
		//case DBG_EXCEPTION_HANDLED:
		//case DBG_CONTINUE:   
		//case STATUS_SEGMENT_NOTIFICATION:        
		//case DBG_TERMINATE_THREAD:              
		//case DBG_TERMINATE_PROCESS:               
		//case DBG_CONTROL_C:  
		//case DBG_CONTROL_BREAK:                   
		//case DBG_COMMAND_EXCEPTION:               
		//case STATUS_GUARD_PAGE_VIOLATION:         
		//case STATUS_DATATYPE_MISALIGNMENT:        
		//case STATUS_BREAKPOINT:                  
		//case STATUS_SINGLE_STEP:                  
	case DBG_EXCEPTION_NOT_HANDLED:
	case STATUS_ACCESS_VIOLATION:
	case STATUS_IN_PAGE_ERROR:
	case STATUS_INVALID_HANDLE:
	case STATUS_NO_MEMORY:
		//case STATUS_ILLEGAL_INSTRUCTION:          
		//case STATUS_NONCONTINUABLE_EXCEPTION:     
		//case STATUS_INVALID_DISPOSITION:          
	case STATUS_ARRAY_BOUNDS_EXCEEDED:
	{
		NeedDump = true;
	}
	break;
	case STATUS_FLOAT_DENORMAL_OPERAND:
	case STATUS_FLOAT_DIVIDE_BY_ZERO:
	case STATUS_FLOAT_INEXACT_RESULT:
	case STATUS_FLOAT_INVALID_OPERATION:
	case STATUS_FLOAT_OVERFLOW:
	case STATUS_FLOAT_STACK_CHECK:
	case STATUS_FLOAT_UNDERFLOW:
	{
		//要产要minidump
	}
	break;
	case STATUS_INTEGER_DIVIDE_BY_ZERO:
		//case STATUS_INTEGER_OVERFLOW:      整形溢出要不要当机？       
		//case STATUS_PRIVILEGED_INSTRUCTION:       
	case STATUS_STACK_OVERFLOW:
		//case STATUS_CONTROL_C_EXIT:               
		//case STATUS_FLOAT_MULTIPLE_FAULTS:        
		//case STATUS_FLOAT_MULTIPLE_TRAPS:         
		//case STATUS_REG_NAT_CONSUMPTION:     
		//case STATUS_SXS_EARLY_DEACTIVATION:    
		//case STATUS_SXS_INVALID_DEACTIVATION: 
	{
		NeedDump = true;
	}
	}

	if (NeedDump)
	{
		static HANDLE hThread = NULL;
		hThread = CreateThread(NULL, 0, threadFunc, (LPVOID)ExceptionInfo, 0, NULL);
		WaitForSingleObject(hThread, INFINITE);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void CCrashDump::DumpFile(char* lpName)
{
	__try
	{
		__asm int 3;
	}
	__except (DumpFileByName(lpName, GetExceptionInformation(), MiniDumpWithFullMemory), EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

void CCrashDump::DumpMiniFile(char* lpName)
{
	__try
	{
		__asm int 3;
	}
	__except (DumpFileByName(lpName, GetExceptionInformation(), MiniDumpNormal), EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

int CCrashDump::DumpThread(PEXCEPTION_POINTERS ExceptionInfo)
{
	LONG ret = EXCEPTION_CONTINUE_SEARCH;

	TCHAR chDumpModule[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, chDumpModule, MAX_PATH);
	int len = lstrlen(chDumpModule);

	AddTime(chDumpModule);
	//chDumpModule
	HANDLE hFile = CreateFile(
		chDumpModule,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_WRITE_THROUGH*/,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return ret;
	}



	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = ExceptionInfo;
	eInfo.ClientPointers = FALSE;
	BOOL bResult = MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MiniDumpWithIndirectlyReferencedMemory,
		&eInfo,
		NULL,
		NULL);
	if (bResult)
	{
		ret = EXCEPTION_EXECUTE_HANDLER;
	}

	CloseHandle(hFile);
	return ret;
}

LONG CCrashDump::DumpFileByName(char* lpName, PEXCEPTION_POINTERS ExceptionInfo, int dumpType)
{
	HANDLE hThreadOther = NULL;
	hThreadOther = CreateThread(NULL, 0, threadFuncNotUpdate, (LPVOID)ExceptionInfo, 0, NULL);
	WaitForSingleObject(hThreadOther, INFINITE);
	return EXCEPTION_EXECUTE_HANDLER;
}


extern "C"
{


void __stdcall create_dump(DumpType type) {
	CCrashDump crash_dump;
	switch (type)
	{
	case INDIRECTLY_REFERENCED_MEMORY:
		crash_dump.SetDumpType(MiniDumpWithIndirectlyReferencedMemory);
		break;

	case FULL_MEMORY:
		crash_dump.SetDumpType(MiniDumpWithFullMemory);
		break;

	default:
		break;
	}
}

void __stdcall dump_file(char* filePath) 
{
	CCrashDump::DumpFile(filePath);
}

void __stdcall set_upload_file(char* fileString)
{
	CCrashDump::m_upLoad.setUploadFileStr(fileString);
}

void __stdcall set_upload_ftp_msg(char* ftpUrl, char* ftpUserName, char* ftpPassword)
{
	CCrashDump::m_upLoad.setFtpMsg(ftpUrl, ftpUserName, ftpPassword);
}

void __stdcall set_upload_info(wchar_t* account, wchar_t* userName)
{
	CCrashDump::m_upLoad.setUserMsg(account, userName);
}

void __stdcall set_upload_server_version(char* serverVersion)
{
	CCrashDump::m_upLoad.setServerVersion(serverVersion);
}

void __stdcall set_upload_dump(BOOL uploadDump)
{
	CCrashDump::m_upLoad.setUploadDump(uploadDump);
}

void __stdcall upload_file(char* fileString, char* filePrefix)
{
	CCrashDump::m_upLoad.upload_file(fileString, filePrefix);
}

void __stdcall upload_log_file(char* fileString)
{
	CCrashDump::m_upLoad.upload_log_file(fileString);
}


void* __stdcall create_upload_task(char* server_url, char* user_name, char* password, PiBool auto_delete)
{
	UploadTask* task = new UploadTask();
	task->server_url = server_url;
	task->auto_delete = auto_delete;
	task->password = password;
	task->user_name = user_name;
	return task;
}

void __stdcall add_file_to_upload_task(void* p, const char* local_path, const char* romote_path, const char* compressed_name)
{
	UploadTask* task = static_cast<UploadTask*>(p);
	int id = -1;
	if (compressed_name)
	{
		auto index = task->compressed_name_map.find(compressed_name);
		if (index == task->compressed_name_map.end())
		{
			id = task->compressed_name_map.size();
			task->compressed_name_map.emplace(std::make_pair(compressed_name, id));
		}
		else
		{
			id = index->second;
		}
	}
	task->file_list.push_back(UploadParams(local_path, romote_path, id));

}


void __stdcall free_upload_task(void* p)
{
	UploadTask* task = static_cast<UploadTask*>(p);
	delete task;
}


struct CStringBuffer
{
	char buffer[256];
};

struct BufferHead
{
	bool auto_delete;
	uint buffer_size;
	uint32_t item_count;
	uint32_t compressed_name_count;
	CStringBuffer server_url;
	CStringBuffer user_name;
	CStringBuffer password;
};



struct ItemData
{
	CStringBuffer local_path;
	CStringBuffer romote_path;
	int compress_id;
};

void  GetCurrentTimestamp(std::string &timeStringText)
{
	auto now = std::chrono::system_clock::now();
	time_t t = std::chrono::system_clock::to_time_t(now);
	auto nowTm = std::make_unique<tm>();
	localtime_s(nowTm.get(), &t);

	std::stringstream ss;
	ss << std::put_time(nowTm.get(), "%Y_%m_%d_%H_%M_%S");

	timeStringText = ss.str();
}

void _stdcall upload_task_start(void* pp)
{
	


	UploadTask* task = static_cast<UploadTask*>(pp);
	uint8_t *p;
	uint buffer_size = 0;
	std::map<std::string, int> compressed_name_map;
	buffer_size += sizeof(BufferHead);

	buffer_size += sizeof(CStringBuffer) * task->compressed_name_map.size();

	buffer_size += sizeof(ItemData) * task->file_list.size();

	

	std::string file_mapping_name;
	GetCurrentTimestamp(file_mapping_name);

	file_mapping_name = "Global" + file_mapping_name + "upLoad";
	HANDLE file_mapping = CreateFileMappingA(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		buffer_size,
		file_mapping_name.c_str());


	void *buffer = MapViewOfFile(file_mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	if (buffer == NULL)
	{
		DWORD error = GetLastError();
		error = error;
		
	}
	p = static_cast<uint8_t*>(buffer);
	BufferHead* head = static_cast<BufferHead*>((void*)p);
	head->auto_delete = task->auto_delete;
	head->item_count = task->file_list.size();
	head->buffer_size = buffer_size;
	head->compressed_name_count = task->compressed_name_map.size();
	pi_strcpy(head->password.buffer, task->password.c_str(), task->password.length());
	pi_strcpy(head->server_url.buffer, task->server_url.c_str(), task->server_url.length());
	pi_strcpy(head->user_name.buffer, task->user_name.c_str(), task->user_name.length());
	p += sizeof(BufferHead);
	for (auto it : task->compressed_name_map)
	{
		CStringBuffer* cstr = static_cast<CStringBuffer*>((void*)p);
		pi_strcpy(cstr->buffer, it.first.c_str(), it.first.length());
		p += sizeof(CStringBuffer);
	}


	for (auto it : task->file_list)
	{
		ItemData* item = static_cast<ItemData*>((void*)p);
		item->compress_id = it.compressed_name_id;
		pi_strcpy(item->local_path.buffer, it.local_path.c_str(), it.local_path.length());
		pi_strcpy(item->romote_path.buffer, it.romote_path.c_str(), it.romote_path.length());
		p += sizeof(ItemData);
	}


	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	//获取exe所在目录路径
	TCHAR chExePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, chExePath, MAX_PATH);
	(_tcsrchr(chExePath, _T('\\')))[1] = 0;

	//组装命令行
	TCHAR event_name[MAX_PATH];
	
	::wsprintf(event_name, _T("local_syn_event%d"), GetCurrentProcessId());
	TCHAR lpCommandLine[MAX_PATH * 2] = { 0 };
	::wsprintf(lpCommandLine, _T("%ls %ls"), pi_str_to_wstr(file_mapping_name.c_str(), PI_CP_UTF8), event_name);

	//组装exe路径
	TCHAR lpExe[MAX_PATH] = { 0 };
	::wsprintf(lpExe, _T("%lsmw_dump.exe"), chExePath);

	
	HANDLE m_event = CreateEventW(NULL, TRUE, FALSE, event_name);


	if (!CreateProcess(lpExe, lpCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		MessageBox(NULL, L"打开进程失败", L"错误", MB_OK);
		CloseHandle(m_event);
		UnmapViewOfFile(buffer);
		buffer = NULL;
		CloseHandle(file_mapping);
	}
	else
	{
		WaitForSingleObject(m_event, INFINITE);
		UnmapViewOfFile(buffer);

		buffer = NULL;

		CloseHandle(file_mapping);
		CloseHandle(m_event);
	}
}
}

