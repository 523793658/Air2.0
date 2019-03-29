#pragma once

namespace ELogVerbosity
{
	enum Type
	{
		NoLogging = 0,
		Fatal,
		Error,
		Warning,
		Display,
		Log,
		Verbose,
		VeryVerbose,
		All	= VeryVerbose,
		NumVerbosity,
		VerbosityMask = 0xf,
		SetColor = 0x40,
		BreakOnLog = 0x80
	};
}

using namespace ELogVerbosity;

#define AIR_LOG(categoryName, verbosity, format, ...) \
{\
	if(verbosity == ELogVerbosity::Fatal){BOOST_ASSERT(false);}\
	::wprintf(TEXT("\n"));\
	::wprintf(format, ##__VA_ARGS__);\
}