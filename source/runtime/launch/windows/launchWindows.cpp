#include "LaunchMininal.h"
#include "Windows/WindowsSystemIncludes.h"
#include "Windows/WindowsHWrapper.h"
#include "CoreGlobals.h"
#ifdef _WIN64
bool GEnableInnerException = true;
#else
bool GEnableInnerException = false;
#endif
namespace Air
{


	extern int32 guardedMain(const TCHAR* cmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstatance, int32 nCmdShow);
}


LAUNCH_API int guardedMainWrapper(const TCHAR* cmdline, HINSTANCE hINInstatance, HINSTANCE hPrevInstance, int nCmdShow)
{
	int ErrorLevel = 0;
	if (GEnableInnerException)
	{
		{
			ErrorLevel = Air::guardedMain(cmdline, hINInstatance, hPrevInstance, nCmdShow);
		}

	}
	else
	{
		ErrorLevel = Air::guardedMain(cmdline, hINInstatance, hPrevInstance, nCmdShow);
	}
	return ErrorLevel;
}


void invalidParameterHandler(const TCHAR* expression,
	const TCHAR *function, const TCHAR* file, unsigned int line, uintptr_t reserved)
{
	//AIR_LOG()
}

void setupWindowsEnvironment(void)
{
	_set_invalid_parameter_handler(invalidParameterHandler);
}

LAUNCH_API int LaunchMain(int argc, char* argv[])
{
	_CrtDumpMemoryLeaks();
	setupWindowsEnvironment();
	int errorLevel = 0;
	hInstance = GetModuleHandle(NULL);

	const TCHAR* cmdLine = ::GetCommandLineW();
	//bool GAlwaysReportCrash = false;
	Air::GIsDemo = true;

	errorLevel = Air::guardedMain(cmdLine, hInstance, NULL, argc);

	return errorLevel;
}

int main(int argc, char* argv[])
{
	LaunchMain(argc, argv);
}
