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

	//命令行匹配
	bool SplitCommand(LPWSTR arg, LPWSTR* name, LPWSTR* value, LPWSTR buffer, int buffer_size);
	//解析命令行
	void ParseCommandLine();

	//打开控制台
	void OpenConsole(const char *locale, int width, int height);
	//解析控制台参数
	void ParseConsole(const rapidjson::Value& console);
	//解析app设置参数
	void ParseAppSetting(const rapidjson::Value& app, wchar_t* cwd);
	//解析win设置参数
	void ParseWindowSetting(const rapidjson::Value& window);
	//解析debug设置参数
	void ParseDebugSetting(const rapidjson::Value& debug);
	//解析运行参数
	void ParseRun(const rapidjson::Value& run);
	//解析配置json
	void ParseJson(const char *jsonPath);
	//读配置json
	char* ParseJsonFromFile(LPWSTR jsonPath);

	//设置vfs路径规则
	int SetVfsMountPath(std::wstring mapPath, std::wstring modName, int modType, std::wstring realPath);
	void ApplyVfsMountPath();

public:

	// 构造函数
	SettingsService();

	// 析构函数
	virtual ~SettingsService();

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize();

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize();

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize();

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime);

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName();

	//获取命令行
	std::wstring getCommandLine() { return cmdline; }

	//获取命令行个数
	int getCommandSize() { return mnCmdSize; }

	//获取app设置
	AppSetting& getAppSetting() { return mAppSetting; }

	//获取win设置
	WinSetting& getWinSetting() { return mWinSetting; }

	//获取工作路径
	std::wstring& getWordDir() { return mAppSetting.workeDir; }

	// 获取debug设置
	DebugSetting getDebugSetting() { return mDebugSetting; }

	//添加命令开关
	void AppendSwith(wchar_t* swithKey, wchar_t* switchValue);

	//开关是否有效
	PiBool HasSwith(const wchar_t* swithKey);

	//获取开关数据
	wchar_t* GetSwithValue(const wchar_t* swithKey);

};

#endif