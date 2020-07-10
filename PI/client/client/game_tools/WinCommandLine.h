#ifndef _WINCOMMANDLINE_H_
#define _WINCOMMANDLINE_H_

#include <windows.h>
#include <shellapi.h>
#include <string>

#include "pi_lib.h"

namespace GameTools
{
class WinCommandLine
{
public:
	static WinCommandLine* GetInstance()
	{
		static WinCommandLine cmd;
		return &cmd;
	}

	WinCommandLine() : argc_(0), argv_(nullptr)
	{
		LPWSTR *argvw = ::CommandLineToArgvW(::GetCommandLineW(), &argc_);
		if (argvw == nullptr)
		{
			return;
		}
		argv_ = new char *[argc_];
		for (int i = 0; i < argc_; ++i)
		{
			std::string str = pi_wstr_to_str(argvw[i], PI_CP_UTF8);
			argv_[i] = new char[str.size() + 1];
			::memcpy(argv_[i], str.data(), str.size());
			argv_[i][str.size()] = '\0';
		}
		LocalFree(argvw);
	}

	virtual ~WinCommandLine()
	{
		for (int i = 0; i < argc_; ++i)
		{
			delete[] argv_[i];
		}
		delete[] argv_;
		argv_ = nullptr;
	}
	int argc() const { return argc_; }
	char **argv() const { return argv_; }
private:
	int argc_;
	char **argv_;
};

}

#endif // !_WINCOMMANDLINE_H_
