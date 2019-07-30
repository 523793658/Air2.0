#include "Class.h"
namespace Air
{
	Object* RClass::createDefaultObject()
	{
		if (!mClassDefaultObject)
		{
			RClass* parentClass = getSupperClass();
			Object* parentDefaultObject = nullptr;
			if (parentClass != nullptr)
			{
				parentDefaultObject = parentClass->getDefaultObject();
			}
			if ((parentDefaultObject != nullptr) ||  this == Object::StaticClass())
			{
				if (!mClassDefaultObject )
				{
					Object* obj = staticAllocateObject(this, getOuter(), TEXT(""), EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject));
					const bool bShouldInitialzedProperties = false;
					ObjectInitializer objectinitializer(obj, nullptr, false, bShouldInitialzedProperties);
					(*classConstructor)(objectinitializer);
					mClassDefaultObject = objectinitializer.getSharedPtr();
				}
			}
		}
		return mClassDefaultObject.get();
	}
}