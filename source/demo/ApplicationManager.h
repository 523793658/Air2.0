#pragma once
#include "core/Application.h"
#include "SimpleReflection.h"
namespace Demo
{
	using namespace Air;
	class ApplicationManager
	{
	public:
		static void addApplication(string className);

		static void printInfo();

		static Application* createApplication(int index);

		static Application* initApplication();
	private:
		static TArray<string> mApplicationList;
	};


#define REGISTER_DEMO(ClassName) \
	class Demo##ClassName##Reg{\
	private:\
		static Demo##ClassName##Reg mDemoReg;\
		Demo##ClassName##Reg(){\
			ApplicationManager::addApplication(#ClassName);\
		}\
	};\
	Demo##ClassName##Reg Demo##ClassName##Reg::mDemoReg;

#define DECALRE_DEMO(ClassName) DECLARE_SIMPLER_REFLECTION(ClassName);  \
	REGISTER_DEMO(ClassName)
}