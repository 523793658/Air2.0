#pragma once
#include "CoreType.h"
#include "CoreFwd.h"

namespace Air
{
	class IPlatformFile;

	class CORE_API PlatformFileManager
	{
		IPlatformFile* mTopmostPlatformFile;
	public:

		IPlatformFile& getPlatformFile();

		static PlatformFileManager& get();
	};


}