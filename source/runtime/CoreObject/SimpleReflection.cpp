#include "SimpleReflection.h"
namespace Air
{

	TArray<RClass*>& SimpleReflectionManager::getClassArray()
	{
		static TArray<RClass*> mClassArray;
		return mClassArray;
	}

	TMap<string, RClass*>& SimpleReflectionManager::getClassMap()
	{
		static TMap<string, RClass*> mClassNameMap;
		return mClassNameMap;
	}

	TMap<string, RClass*>& SimpleReflectionManager::getClassInfoMap()
	{
		static TMap<string, RClass*> mClassInfoName;
		return mClassInfoName;
	}
}
