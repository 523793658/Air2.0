#include "backtrace.h"

#include <Windows.h>

#include <utility>
#include <algorithm>
#include <sstream>

#include "v8.h"
#include <pi_lib.h>
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Settings/SettingsService.h"


Backtrace::Backtrace()
{

}

Backtrace::~Backtrace()
{

}

// 对插件目录和文件名做个映射，用于报错堆栈的处理
void Backtrace::AddPlugin(const std::string& pluginName, const std::vector<std::string>& fileVec, const std::vector<std::string>& contentVec)
{
	if (fileVec.size() == contentVec.size())
	{
		FileArray fileArray;
		for (int i = 0; i < fileVec.size(); i++)
		{
			// 文件名与文件的行数
			BtFileInfo info;
			info.name = fileVec[i];
			info.line = std::count_if(contentVec[i].begin(), contentVec[i].end(), [](char ch) { return ch == '\n'; });
			fileArray.push_back(info);
		}
		pluginMap_.insert(std::make_pair(pluginName, fileArray));
	}
}

// 对堆栈进行过滤，替换插件名和行号，使得与真正的文件内容能够对应
std::string Backtrace::Filter(const std::string& bt)
{
	std::string newBt = bt;

	SettingsService *settingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	if (settingService->HasSwith(L"backtrace") == TRUE)
	{
		try
		{
			std::size_t jsIndex = newBt.find(".js");

			std::size_t blankBegin = newBt.rfind(' ', jsIndex);
			std::size_t bracketBegin = newBt.rfind('(', jsIndex);

			std::size_t blankEnd = newBt.find('\n', jsIndex);
			std::size_t bracketEnd = newBt.find(')', jsIndex);

			while (jsIndex != std::string::npos)
			{
				std::size_t lIndex = 0;
				std::size_t rIndex = 0;

				// 开始
				if (blankBegin != std::string::npos && bracketBegin != std::string::npos)
				{
					lIndex = blankBegin > bracketBegin ? blankBegin : bracketBegin;
				}
				else if (blankBegin == std::string::npos)
				{
					lIndex = bracketBegin;
				}
				else if (bracketBegin == std::string::npos)
				{
					lIndex = blankBegin;
				}

				// 结束
				if (blankEnd != std::string::npos && bracketEnd != std::string::npos)
				{
					rIndex = blankEnd < bracketEnd ? blankEnd : bracketEnd;
				}
				else if (blankEnd == std::string::npos)
				{
					rIndex = bracketEnd;
				}
				else if (bracketEnd == std::string::npos)
				{
					rIndex = blankEnd;
				}

				std::string fileInfo = newBt.substr(lIndex + 1, rIndex - 1 - lIndex);
				std::string newfileInfo = GetFixFileInfo(fileInfo);
				newBt = newBt.replace(lIndex + 1, fileInfo.size(), newfileInfo);

				jsIndex = newBt.find(".js", rIndex);
				blankBegin = newBt.rfind(' ', jsIndex);
				bracketBegin = newBt.rfind('(', jsIndex);
				blankEnd = newBt.find('\n', jsIndex);
				bracketEnd = newBt.find(')', jsIndex);
			}
		}
		catch (...)
		{
			pi_log_print(LOG_WARNING, "[%d] parse stack trace error \n%s", GetCurrentProcessId(), bt.c_str());
		}
	}

	return newBt;
}

std::string Backtrace::GetFixFileInfo(const std::string& fileInfo)
{
	std::string newFileInfo = fileInfo;
	std::vector<std::string> infoVec = Split(newFileInfo, ':');

	std::string fileName = infoVec[0];
	int lineNum = std::stod(infoVec[1]);
	int col = std::stod(infoVec[2]);
	
	auto pluginIter = pluginMap_.find(infoVec[0]);
	if (pluginIter != pluginMap_.end())
	{
		FileArray fileArray = pluginIter->second;
		for (int i = 0; i < fileArray.size(); i++)
		{
			int fileline = fileArray[i].line;
			if (lineNum > fileline)
			{
				lineNum -= fileline;
			}
			else
			{
				std::stringstream ss;
				ss << fileArray[i].name << ":" << lineNum  << ":" << col;
				newFileInfo = ss.str();
				break;
			}
		}
	}

	return newFileInfo;
}

void Backtrace::ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch)
{
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = *exception;
	v8::Local<v8::Message> message = try_catch->Message();
	if (message.IsEmpty())
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		pi_log_print(LOG_WARNING, "[%d] %s", GetCurrentProcessId(), exception_string);
	}
	else
	{
		v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		int linenum = message->GetLineNumber(context).FromJust();
		int endPos = message->GetEndColumn();

		std::stringstream ss;
		ss << *filename << ":" << linenum << ":" << endPos;
		std::string fileStr = Backtrace::GetInstance().GetFixFileInfo(ss.str());

		pi_log_print(LOG_WARNING, "[%d] %s: %s", GetCurrentProcessId(), fileStr.c_str(), exception_string);
		// Print line of source code.
		// 因为编码问题，暂时先屏蔽出错源文件的输出
// 		v8::String::Utf8Value sourceline(
// 			message->GetSourceLine(context).ToLocalChecked());
// 		pi_log_print(LOG_WARNING, "[%d] %s", GetCurrentProcessId(), *sourceline);

		v8::String::Utf8Value stack_trace(
			try_catch->StackTrace(context).ToLocalChecked());
		if (stack_trace.length() > 0)
		{
			std::string ss = Backtrace::GetInstance().Filter(std::string(*stack_trace));
			pi_log_print(LOG_WARNING, "[%d] %s", GetCurrentProcessId(), ss.c_str());

		}
	}
}

std::vector<std::string> Backtrace::Split(const std::string& str, char delimiter)
{
	std::vector<std::string> infoVec;
	std::istringstream f(str);
	std::string s;
	while (std::getline(f, s, delimiter))
	{
		infoVec.push_back(s);
	}
	return infoVec;
}

