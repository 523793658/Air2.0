#pragma once
#include "CoreMinimal.h"
#include "Linker.h"
namespace Air
{




	class LinkerLoad
#if !WITH_EDITOR
		final
#endif
		:public Linker, public Archive
	{
	public:
		COREOBJECT_API static bool isKnownMissingPackage(wstring packageName);

		COREOBJECT_API static void addKnownMissingPackage(wstring packageName);
	};
}