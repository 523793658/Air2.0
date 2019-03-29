#include "Class.h"
namespace Air
{
	Object* RClass::createDefaultObject()
	{
		if (mClassDefaultObject == nullptr)
		{
			//if (this == Object::StaticClass())
			{
				if (mClassDefaultObject == nullptr)
				{
					mClassDefaultObject = staticAllocateObject(this, getOuter(), TEXT(""), EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject));
					const bool bShouldInitialzedProperties = false;
					(*classConstructor)(ObjectInitializer(mClassDefaultObject, nullptr, false, bShouldInitialzedProperties));
				}
			}
		}
		return mClassDefaultObject;
	}
}