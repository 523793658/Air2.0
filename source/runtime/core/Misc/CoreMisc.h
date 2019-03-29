#pragma once
#include "CoreType.h"
#include "Containers/Array.h"
#include "Interface/ITargetPlatformManagerModule.h"
namespace Air
{


	class CORE_API SelfRegisteringExec
	{
	public:
		SelfRegisteringExec()
		{

		}
		virtual ~SelfRegisteringExec()
		{

		}

		
	};

	struct CORE_API UrlConfig
	{
		wstring mDefaultProtocol;
		wstring mDefaultName;
		wstring mDefaultHost;
		wstring mDefaultPortal;
		wstring mDefaultSaveExt;
		int32 mDefaultPort;

		void init();

		void reset();
	};

	struct TCharArrayTester 
	{
		template<uint32 N>
		static char(&func(const TCHAR(&)[N]))[2];
		static char(&func(...))[1];
	};


	CORE_API class ITargetPlatformManagerModule& getTargetPlatformManagerRef();

	FORCEINLINE bool isRunningDedicatedServer()
	{
		return false;
	}

	CORE_API class DerivedDataCacheInterface&  getDerivedDataCacheRef();

#define IS_TCHAR_ARRAY(expr) (sizeof(TCharArrayTester::func(expr)) == 2)
}