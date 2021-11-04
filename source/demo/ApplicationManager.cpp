#include "ApplicationManager.h"
#include <iostream>
#include "Demos/initEngine/InitEngine.h"
namespace Air
{
	TArray<string> ApplicationManager::mApplicationList;


	void ApplicationManager::addApplication(string className)
	{
		mApplicationList.add(className);
	}

	void ApplicationManager::printInfo()
	{
		for (int i = 0; i < mApplicationList.size(); i++)
		{
			RClass* c = SimpleReflectionManager::getClass(mApplicationList[i]);
			Object* obj = c->getDefaultObject(true);
			Application* app = static_cast<Application*>(obj);
			std::cout << i << ":" << app->getTitle()<< std::endl;
		}
	}

	Application* ApplicationManager::createApplication(int index)
	{
		if (index < mApplicationList.size())
		{
			RClass* c = SimpleReflectionManager::getClass(mApplicationList[index]);
			return dynamic_cast<Application*>(c->getDefaultObject(true));
		}
		return nullptr;
	}

	Application* ApplicationManager::initApplication()
	{
		printInfo();
		int index;
		scanf("%d", &index);
		return createApplication(index);
	}
}