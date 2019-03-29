#pragma once
#include "CoreObject.h"
namespace Air
{
	class LinkerTables
	{

	};

	namespace ELinkerType
	{
		enum Type
		{
			None,
			Load,
			Save
		};
	}

	class Linker : public LinkerTables
	{
	private:
		ELinkerType::Type mLinkerType;


	};
}