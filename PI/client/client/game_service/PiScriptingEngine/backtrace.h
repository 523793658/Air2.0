#ifndef _PI_BACKTRACE_H_
#define _PI_BACKTRACE_H_

#include <unordered_map>
#include <vector>
#include <string>

#include <DinoUIV8ScriptingEngine/DinoUIV8Backtrace.h>

struct BtFileInfo
{
	std::string name;
	int line;
};

namespace v8
{
	class Isolate;
	class TryCatch;
}

class Backtrace : public DinoUI::DinoUIV8Backtrace
{
public:
	static Backtrace& GetInstance()
	{
		static Backtrace bt;
		return bt;
	}

	Backtrace();
	~Backtrace();

	void AddPlugin(const std::string& pluginName, const std::vector<std::string>& fileVec, const std::vector<std::string>& contentVec);
	std::string Filter(const std::string& bt);

	std::string GetFixFileInfo(const std::string& fileInfo);
	virtual void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);


private:
	std::vector<std::string> Split(const std::string& str, char delimiter);

private:
	using FileArray = std::vector<BtFileInfo>;
	std::unordered_map<std::string, FileArray> pluginMap_;
};

#endif