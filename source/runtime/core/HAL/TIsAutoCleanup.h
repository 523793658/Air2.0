#pragma once
#include "CoreType.h"
namespace Air
{
	class CORE_API TLSAutoCleanup
	{
	public:
		virtual ~TLSAutoCleanup() {}

		void Register();
	};
 }