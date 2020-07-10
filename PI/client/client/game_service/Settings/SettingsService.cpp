#include "SettingsService.h"
#include <processenv.h> 
#include <io.h>
#include <fcntl.h>
#include <locale.h>
#include <Shlwapi.h>


using namespace std;
using namespace rapidjson;

#define JSON_KEY_RUN "run"
#define JSON_KEY_CONSOLE "console"
#define JSON_KEY_RUN_ARGUMENT "argument"
#define JSON_KEY_APP_SETTINGS "app-settings"
#define JSON_KEY_WINDOW_SETTINGS "window-settings"
#define JSON_KEY_DEBUG_SETTINGS "debug-settings"

#define KEY_LOG_PATH L"log-file"
#define KEY_LOAD_FILE L"-l"
#define KEY_CONSOLE L"console"

// console各字段的默认值
#define DEFAULT_CONSOLE_LOCALE "chs"
#define DEFAULT_CONSOLE_WIDTH 80
#define DEFAULT_CONSOLE_HEIGHT 2000
// console对象的属性
#define CONSOLE_LOCALE "locale"
#define CONSOLE_WIDTH  "width"
#define CONSOLE_HEIGHT "height"

#define DEFAULT_DEBUG_PORT 42000

/*
To simply the conditional judging when using rapid json object.
source: JSON_TRY_PARSE_VALUE(browser, "width", IsInt)
out   : browser.HasMember("width") && browser["width"].IsInt()
*/
#define TRY_PARSE(object, key, type_function) object##.HasMember(key) && object##[key].##type_function##()

static PiSelectR PI_API _delete_key(void *user_data, PiKeyValue  *hash_node)
{
	pi_free(hash_node->key);
	pi_free(hash_node->value);
	return SELECT_NEXT;
}

static BOOL WINAPI _consoleCtrlhandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		pi_log_close();
	}
	return TRUE;
}

// 构造函数
SettingsService::SettingsService()
{

}

// 析构函数
SettingsService::~SettingsService()
{

}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult SettingsService::PreInitialize()
{
	return SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult SettingsService::Initialize()
{
	if (!PathIsDirectoryW(LOG_PATH))
	{
		CreateDirectory(LOG_PATH, NULL);
	}
	mpCmdSwith = pi_hash_new(0.75f, (PiHashFunc)pi_wstr_hash, pi_wstring_equal);
	ParseCommandLine();
	ApplyVfsMountPath();
	return AsyncResult_Complete;
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult SettingsService::Finalize()
{
	if (mpCmdSwith != NULL)
	{
		pi_hash_foreach(mpCmdSwith, (PiSelectFunc)_delete_key, NULL);
		pi_hash_clear(mpCmdSwith, TRUE);
		pi_free(mpCmdSwith);
		mpCmdSwith = NULL;
	}
	SetConsoleCtrlHandler(_consoleCtrlhandler, FALSE);
	return AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult SettingsService::Update(const float vfElaspedTime)
{
	return AsyncResult_Complete;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring SettingsService::GetDisplayName()
{
	return L"Settings";
}

//设置开关数据
void SettingsService::AppendSwith(wchar_t* swithKey, wchar_t* switchValue)
{
	PiKeyValue old;
	if (mpCmdSwith == NULL)
	{
		return;
	}
	wchar_t *key = pi_wstr_dup(swithKey);
	wchar_t *value = pi_wstr_dup(switchValue);
	if (pi_hash_enter(mpCmdSwith, key, value, &old))
	{
		pi_free(old.key);
		pi_free(old.value);
	}
}

//开关是否有效
PiBool SettingsService::HasSwith(const wchar_t* swithKey)
{
	return mpCmdSwith == NULL ? FALSE : pi_hash_lookup(mpCmdSwith, swithKey, NULL);
}

//获取开关数据
wchar_t* SettingsService::GetSwithValue(const wchar_t* swithKey)
{
	wchar_t* value = NULL;
	if (mpCmdSwith != NULL && pi_hash_lookup(mpCmdSwith, swithKey, (void **)&value))
	{
		return value;
	}
	return value;
}

//读取json数据，给后续解析做依据
char* SettingsService::ParseJsonFromFile(LPWSTR jsonPath)
{
	int size;
	DWORD high = 0;
	char *json = NULL;
	HANDLE file;

	file = CreateFileW(jsonPath, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		pi_log_print(LOG_WARNING, "load file %s error, file isn't exist!", jsonPath);
		return NULL;
	}

	size = GetFileSize(file, &high);
	json = (char *)malloc(size + 1);

	ReadFile(file, json, size, &high, NULL);
	json[size] = 0;

	CloseHandle(file);
	return json;
}

//打开控制台，如果有控制台参数的话
void SettingsService::OpenConsole(const char *locale, int width, int height)
{
	/* 控制台，-1表示创建失败，0表示未创建，1表示附加到父进程的控制台，2表示新建控制台 */
	static int _s_console;

	int fd;
	HANDLE std;
	FILE* file;
	COORD size;

	if (_s_console)
		return;
	_s_console = -1;
	if (AttachConsole((DWORD)-1))
	{
		_s_console = 1;
	}
	else if (AllocConsole())
	{
		char title[MAX_PATH];
		GetModuleFileNameA(NULL, title, MAX_PATH);
		SetConsoleTitleA(title);

		setlocale(LC_ALL, locale);
		_s_console = 2;
	}
	else
	{
		pi_log_print(LOG_WARNING, "Allocate Console Error!");
		return;
	}

	size.X = (SHORT)width;
	size.Y = (SHORT)height;

	// 重定向标准输出流句柄到新的控制台窗口
	std = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleScreenBufferSize(std, size);

	// 文本模式
	fd = _open_osfhandle((intptr_t)std, _O_TEXT);
	file = _fdopen(fd, "w");

	// 无缓冲
	setvbuf(file, NULL, _IONBF, 0);
	*stdout = *file;

	// 重定向标准错误流句柄到新的控制台窗口
	std = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleScreenBufferSize(std, size);

	// 文本模式
	fd = _open_osfhandle((intptr_t)std, _O_TEXT);
	file = _fdopen(fd, "w");

	// 无缓冲
	setvbuf(file, NULL, _IONBF, 0);
	*stderr = *file;

	SetConsoleCtrlHandler(_consoleCtrlhandler, TRUE);
}

//解析控制台参数，主要有缓冲区大小等
void SettingsService::ParseConsole(const Value& console)
{
	int width = DEFAULT_CONSOLE_WIDTH;
	int height = DEFAULT_CONSOLE_HEIGHT;
	char *locale = DEFAULT_CONSOLE_LOCALE;

	if (console.HasMember(CONSOLE_LOCALE) && console[CONSOLE_LOCALE].IsString())
	{
		locale = (char *)console[CONSOLE_LOCALE].GetString();
	}

	if (console.HasMember(CONSOLE_WIDTH) && console[CONSOLE_WIDTH].IsInt())
	{
		width = console[CONSOLE_WIDTH].GetInt();
	}

	if (console.HasMember(CONSOLE_HEIGHT) && console[CONSOLE_HEIGHT].IsInt())
	{
		height = console[CONSOLE_HEIGHT].GetInt();
	}

	OpenConsole(locale, width, height);
}

//解析app数据，主js,vfs路径，工作环境
void SettingsService::ParseAppSetting(const Value& app, wchar_t* cwd)
{
	wchar_t *map_path, *mod_name, *real_path;
	if (TRY_PARSE(app, "start-js", IsString))
	{
		mAppSetting.startPath = app["start-js"].GetString();
	}
	if (TRY_PARSE(app, "vfs-mount", IsArray))
	{
		const Value& array = app["vfs-mount"];
		for (SizeType i = 0; i < array.Size(); ++i)
		{
			VfsMountItem item;
			const Value& json_item = array[i];
			if (json_item.IsArray())
			{
				map_path = pi_str_to_wstr(json_item[0].GetString(), PI_CP_UTF8);
				mod_name = pi_str_to_wstr(json_item[1].GetString(), PI_CP_UTF8);
				item.modeType = json_item[2].GetInt();
				real_path = pi_str_to_wstr(json_item[3].GetString(), PI_CP_UTF8);
				item.mapPath = map_path;
				item.modName = mod_name;
				item.modeType = json_item[2].GetInt();
				item.realPath = real_path;
				pi_free(map_path);
				pi_free(mod_name);
				pi_free(real_path);
			}
			mAppSetting.vfsMountTable.push_back(item);
		}
	}
	if (TRY_PARSE(app, "enable-backtrace", IsBool)) 
	{
		mAppSetting.enableBacktrace = app["enable-backtrace"].GetBool();
	}

	mAppSetting.workeDir = cwd;
}

//解析窗口参数 窗口title， 窗口大小
void SettingsService::ParseWindowSetting(const Value& window)
{
	wchar_t* title;
	if (TRY_PARSE(window, "width", IsInt)){
		mWinSetting.width = window["width"].GetInt();
	}

	if (TRY_PARSE(window, "height", IsInt)){
		mWinSetting.height = window["height"].GetInt();
	}

	if (TRY_PARSE(window, "hide-cursor", IsBool)){
		mWinSetting.hideCursor = window["hide-cursor"].GetBool();
	}


	if (TRY_PARSE(window, "title", IsString)){
		title = pi_str_to_wstr(window["title"].GetString(), PI_CP_UTF8);
		mWinSetting.title = title;
		pi_free(title);
	}
}

void SettingsService::ParseDebugSetting(const rapidjson::Value& debug)
{
	mDebugSetting.enable = true;
	if (TRY_PARSE(debug, "port", IsInt))
	{
		mDebugSetting.port = debug["port"].GetInt();
	}
	else
	{
		mDebugSetting.port = DEFAULT_DEBUG_PORT;
	}
}

//解析运行数据，app数据、window数据
void SettingsService::ParseRun(const Value& run)
{
	wchar_t cwd[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, cwd);
	if (run.HasMember(JSON_KEY_APP_SETTINGS) && run[JSON_KEY_APP_SETTINGS].IsObject()){
		ParseAppSetting(run[JSON_KEY_APP_SETTINGS], cwd);
	}
	else {
		mAppSetting.workeDir = cwd;
	}
	if (run.HasMember(JSON_KEY_WINDOW_SETTINGS) && run[JSON_KEY_WINDOW_SETTINGS].IsObject()){
		ParseWindowSetting(run[JSON_KEY_WINDOW_SETTINGS]);
	}
	else {
		pi_log_print(LOG_WARNING, "not fount init fun cfg !");
	}
	if (run.HasMember(JSON_KEY_DEBUG_SETTINGS) && run[JSON_KEY_DEBUG_SETTINGS].IsObject())
	{
		ParseDebugSetting(run[JSON_KEY_DEBUG_SETTINGS]);
	}
	else
	{
		mDebugSetting.enable = false;
		mDebugSetting.port = DEFAULT_DEBUG_PORT;
	}
}

void SettingsService::ParseJson(const char *json)
{
	Document doc;
	doc.Parse(json);
	if (doc.HasParseError()) {
		return;
	}
	if (doc.HasMember(JSON_KEY_CONSOLE) && doc[JSON_KEY_CONSOLE].IsObject()) {
		ParseConsole(doc[JSON_KEY_CONSOLE]);
	}
	if (doc.HasMember(JSON_KEY_RUN) && doc[JSON_KEY_RUN].IsObject()) {
		ParseRun(doc[JSON_KEY_RUN]);
	}
}

//匹配命令行， 有--前缀的为有效命令，如--log-file="game.txt"或--dev
bool SettingsService::SplitCommand(LPWSTR arg, LPWSTR* name, LPWSTR* value, LPWSTR buffer, int buffer_size)
{
	*name = NULL;
	*value = NULL;
	if (*arg == '-')
	{
		arg++;
		if (*arg == '-') {
			arg++;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	*name = arg;
	while (*arg != '\0' && *arg != '=')
	{
		arg++;
		if (*arg == '=')
		{
			int n = static_cast<int>(arg - *name);
			if (n >= buffer_size)
				return false;
			memcpy(buffer, *name, n * sizeof(wchar_t));
			buffer[n] = '\0';
			*name = buffer;
			*value = arg + 1;
		}
	}
	return true;
}

//解析命令行 添加命令开关、解析配置json
void SettingsService::ParseCommandLine()
{
	int command_size, i;
	char *json = NULL;
	wchar_t aplit_buffer[1024];
	LPWSTR split_name, split_value;

	cmdline = GetCommandLineW();
	LPWSTR* command_line = CommandLineToArgvW(cmdline.c_str(), &command_size);
	mnCmdSize = command_size;
	if (command_size != NULL) {
		for (i = 0; i < command_size; i++) {
			if (wcscmp(command_line[i], KEY_LOAD_FILE) == 0) {
				json = ParseJsonFromFile(command_line[i + 1]);
				AppendSwith(L"init-json", command_line[i + 1]);
			} else if (SplitCommand(command_line[i], &split_name, &split_value, aplit_buffer, sizeof aplit_buffer) && split_name != NULL) {
				if (wcscmp(split_name, KEY_LOG_PATH) == 0) {
					pi_log_setting(split_value, TRUE, TRUE);
				}
				AppendSwith(split_name, split_value);
			}
		}
		if (json != NULL) {
			ParseJson(json);
			free(json);
		}
		else {
			pi_log_print(LOG_WARNING, "not found init json file error");
		}
		//只能在解析控制台信息后进行初始化，否则重定向的流不对
		pi_log_init();
		LocalFree(command_line);
	}
}

//设置vfs配置，主要用于vfs文件读取
int SettingsService::SetVfsMountPath(wstring mapPath, wstring modName, int modType, wstring realPath)
{
	void* mode = (void*)modType;
	wstring path(L"");

	if (path.size() > 0 && path[path.size() - 1] != L'\\')
	{
		path = path.append(L"\\");
	}

	if (realPath != L".") {
		path = path.append(realPath);
	}
	return pi_vfs_mount(mapPath.c_str(), modName.c_str(), mode, path.c_str());
}

void SettingsService::ApplyVfsMountPath()
{
	vector<VfsMountItem>& mount_table = mAppSetting.vfsMountTable;
	vector<VfsMountItem>::const_iterator it = mount_table.begin();
	for (; it != mount_table.end(); ++it){
		int ret = SetVfsMountPath(it->mapPath, it->modName, it->modeType, it->realPath);
		pi_log_print(LOG_INFO, "vfs mount:%d, reason: %d - %ls %ls %d %ls", ret, pi_error_get_code(), it->mapPath.c_str(), it->modName.c_str(), it->modeType, it->realPath.c_str());
	}
}