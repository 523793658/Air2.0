/*	
	the head file for crash module
*/
#pragma once


using std::wstring;
#define LOG_PATH L"log\\"
#define DUMP_PATH L"log\\dump\\"

struct UploadParams
{
	std::string local_path;
	std::string romote_path;
	int compressed_name_id;
	UploadParams(std::string lp, std::string rp, int id)
	{
		local_path = lp;
		romote_path = rp;
		compressed_name_id = id;
	}
};

struct UploadTask
{
	std::vector<UploadParams> file_list;
	bool auto_delete;
	std::string server_url;
	std::string user_name;
	std::string password;
	std::map<std::string, int> compressed_name_map;
};


typedef enum
{
    INDIRECTLY_REFERENCED_MEMORY,
    FULL_MEMORY,
    TYPE_COUNT
} DumpType;

class upLoadInfo
{
public:
	upLoadInfo();
	~upLoadInfo();
	void setUserMsg(wchar_t* account, wchar_t* userName);
	void setFtpMsg(char* ftpServerUrl, char* ftpUserName, char* ftpPassword);
	void setServerVersion(char* serverVersion);
	void setUploadFileStr(char* uploadFileStr);
	void execUpload(char* dumpPath);
	void setUploadDump(bool uploadDump);
	bool getUploadDump();
	void upload_file(char* fileString, char* filePrefix);
	void upload_log_file(char* fileString);

private:
	bool m_uploadDump = true;
	char m_logPath[MAX_PATH];
	char m_account[MAX_PATH];
	char m_userName[MAX_PATH];
	char m_ftpServerUrl[MAX_PATH];
	char m_ftpUserName[MAX_PATH];
	char m_ftpPassword[MAX_PATH];
	char m_serverVersion[MAX_PATH];
	char m_uploadFileStr[MAX_PATH];
};

class CCrashDump
{
public:
	CCrashDump(LPCSTR lpKey = "");
	~CCrashDump();
	static upLoadInfo m_upLoad;
	static void DumpFile(char* lpName);
	static void DumpMiniFile(char* lpName);
	static int DumpThread(PEXCEPTION_POINTERS ExceptionInfo);
    static DWORD& DumpType() { return m_DumpType; }
    static void SetDumpType(DWORD type) { m_DumpType = type; }
protected:
	static LONG WINAPI TopLevelExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);
	static LONG WINAPI TopLevelExceptionFilterFirst(PEXCEPTION_POINTERS ExceptionInfo);
    static DWORD WINAPI threadFunc(LPVOID lpParam);
	static DWORD WINAPI threadFuncNotUpdate(LPVOID lpParam);

	static void AddTime(LPTSTR lpStr);

	static LONG DumpFileByName(char* lpName, PEXCEPTION_POINTERS ExceptionInfo, int dumpType);
    static DWORD m_DumpType;
private:
};

extern "C"
{

	void __stdcall create_dump(DumpType type);

}

