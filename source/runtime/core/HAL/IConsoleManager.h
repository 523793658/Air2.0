#pragma once
#include "CoreType.h"
namespace Air
{
	class CORE_API AutoConsoleObject
	{
	protected:
		AutoConsoleObject();
	};

	template<class T>
	class AutoConsoleVariable : public AutoConsoleObject
	{

	};


}