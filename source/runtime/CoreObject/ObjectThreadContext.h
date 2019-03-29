#pragma once
#include "CoreObject.h"
#include "HAL/ThreadSingleton.h"
namespace Air
{
	class ObjectInitializer;
	class Object;
	class COREOBJECT_API ObjectThreadContext : public TThreadSingleton<ObjectThreadContext>
	{
		friend TThreadSingleton<ObjectThreadContext>;
		ObjectThreadContext();
		TArray<ObjectInitializer*> mInitializerStack;

	public:
		void popInitializer()
		{
			mInitializerStack.pop();
		}
		void pushInitializer(ObjectInitializer* initializer)
		{
			mInitializerStack.push(initializer);
		}
		ObjectInitializer* topInitializer()
		{
			return mInitializerStack.size() ? mInitializerStack.last() : nullptr;
		}

		ObjectInitializer& topInitializerChecked()
		{
			ObjectInitializer* objectIntializerPtr = topInitializer();
			BOOST_ASSERT(objectIntializerPtr);
			return *objectIntializerPtr;
		}

		int32 mImportCount{ 0 };
		int32 mForcedExportCount{ 0 };
		int32 mObjBeginLoadCount{ 0 };
		TArray<Object*> mObjLoaded;
		int32 mIsInConstructor{ 0 };
		Object* mConstructedObject{ nullptr };

		bool isRoutingPostLoad;
	};
}
