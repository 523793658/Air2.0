#pragma once
#include "Containers/String.h"
#include "Containers/Map.h"
#include "Class.h"
#include <functional>
#include "Containers/StringConv.h"

namespace Air
{
	class RClass;
	class COREOBJECT_API SimpleReflectionManager
	{
	public:
		typedef std::function<void*(void)> ConstructorType;
	private:
		
	public:
		static TArray<RClass*>& getClassArray();

		static TMap<string, RClass*>& getClassMap();

		static TMap<string, RClass*>& getClassInfoMap();
	public:
		static void registerClass(string className, RClass* inClass, const type_info& parentType)
		{
			static TMap<size_t, TArray<RClass*>> temp;

			auto& classMap = getClassMap();
			auto& classInfoMap = getClassInfoMap();
			string parentKey = string(parentType.name()) + boost::lexical_cast<string>(parentType.hash_code());
			BOOST_ASSERT(classMap.find(className) == classMap.end() && classInfoMap.find(inClass->mUniqueName) == classInfoMap.end());
			classMap.emplace(className, inClass);
			classInfoMap.emplace(inClass->mUniqueName, inClass);
			auto it = classInfoMap.find(parentKey);
			if (it != classInfoMap.end())
			{
				inClass->mSuperClass = it->second;
			}
			else
			{
				auto& l = temp.findOrAdd(parentType.hash_code());
				l.emplace(inClass);
			}
			auto& t = temp.find(inClass->mClassId);
			if (t != temp.end())
			{
				for (RClass* child : t->second)
				{
					child->mSuperClass = inClass;
				}
				temp.erase(t);
			}
		}

		static RClass* getClass(string className)
		{
			auto& classMap = getClassMap();
			auto it = classMap.find(className);
			if (it == classMap.end())
			{
				return nullptr;
			}
			return it->second;
		}

		static RClass* getClass(wstring className)
		{
			char* ch = TCHAR_TO_ANSI(className.c_str());
			string s = string(ch);
			return getClass(s);
		}

		template<typename T = void>
		static T* createInstantce(int id)
		{
			if (getClassArray().isValidIndex(id))
			{
				return getClassArray()[id].
			}
			else
			{
				return nullptr;
			}
		}
	};

#define DECLARE_SIMPLER_REFLECTION(ClassName)	\
	class ClassName##Factory : public TClass<ClassName>{\
	private:	\
		friend class ClassName;\
		ClassName##Factory()	\
		{\
			SimpleReflectionManager::registerClass(#ClassName, this, typeid(ClassName::ParentType));\
		}	\
	private:	\
		static ClassName##Factory mInstance;	\
	};	\
	ClassName##Factory ClassName##Factory::mInstance; \
	RClass* ClassName::mClassInstance = &ClassName##Factory::mInstance;
}