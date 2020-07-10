#ifndef _SettingsService_H_
#define _SettingsService_H_

#include "game_service/BaseClass/PiGameService.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "pi_lib.h"

#include <wtypes.h>

#define LOG_PATH L"log\\"

typedef struct{
	std::wstring mapPath;
	std::wstring modName;
	int    modeType;
	std::wstring realPath;
}VfsMountItem;

//Window Settings
typedef struct {
	int width;
	int height;
	bool hideCursor;
	std::wstring title;
} WinSetting;

//App Settings
typedef struct {
	std::string startPath;
	std::vector<VfsMountItem>  vfsMountTable;
	std::wstring workeDir;
	bool enableBacktrace;
} AppSetting;

// Debug Settings
typedef struct
{
	bool enable;
	int port;
} DebugSetting;

class SettingsService : public PiGameService
{
private:
	AppSetting mAppSetting;
	WinSetting mWinSetting;
	DebugSetting mDebugSetting;
	std::wstring cmdline;
	int		mnCmdSize;
	PiHash *mpCmdSwith;

	//������ƥ��
	bool SplitCommand(LPWSTR arg, LPWSTR* name, LPWSTR* value, LPWSTR buffer, int buffer_size);
	//����������
	void ParseCommandLine();

	//�򿪿���̨
	void OpenConsole(const char *locale, int width, int height);
	//��������̨����
	void ParseConsole(const rapidjson::Value& console);
	//����app���ò���
	void ParseAppSetting(const rapidjson::Value& app, wchar_t* cwd);
	//����win���ò���
	void ParseWindowSetting(const rapidjson::Value& window);
	//����debug���ò���
	void ParseDebugSetting(const rapidjson::Value& debug);
	//�������в���
	void ParseRun(const rapidjson::Value& run);
	//��������json
	void ParseJson(const char *jsonPath);
	//������json
	char* ParseJsonFromFile(LPWSTR jsonPath);

	//����vfs·������
	int SetVfsMountPath(std::wstring mapPath, std::wstring modName, int modType, std::wstring realPath);
	void ApplyVfsMountPath();

public:

	// ���캯��
	SettingsService();

	// ��������
	virtual ~SettingsService();

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize();

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize();

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize();

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime);

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName();

	//��ȡ������
	std::wstring getCommandLine() { return cmdline; }

	//��ȡ�����и���
	int getCommandSize() { return mnCmdSize; }

	//��ȡapp����
	AppSetting& getAppSetting() { return mAppSetting; }

	//��ȡwin����
	WinSetting& getWinSetting() { return mWinSetting; }

	//��ȡ����·��
	std::wstring& getWordDir() { return mAppSetting.workeDir; }

	// ��ȡdebug����
	DebugSetting getDebugSetting() { return mDebugSetting; }

	//��������
	void AppendSwith(wchar_t* swithKey, wchar_t* switchValue);

	//�����Ƿ���Ч
	PiBool HasSwith(const wchar_t* swithKey);

	//��ȡ��������
	wchar_t* GetSwithValue(const wchar_t* swithKey);

};

#endif