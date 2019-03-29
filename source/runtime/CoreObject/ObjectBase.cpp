#include "ObjectBase.h"
#include "Object.h"
namespace Air
{
	ObjectBase::ObjectBase(EObjectFlags inFlags)
		:mObjectFlags(inFlags)
	{

	}

	ObjectBase::ObjectBase(RClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, Object* inOuter, wstring name)
		:mObjectFlags(inFlags)
		,mInternalIndex(INDEX_NONE)
		,mClassPrivate(inClass)
		,mNamePrivate(name)
		,mOuterPrivate(inOuter)
	{

	}

	Object* ObjectBase::getTypedOuter(RClass* target) const
	{
		Object* result = nullptr;
		for (Object* nextOuter = getOuter(); result == nullptr && nextOuter != nullptr; nextOuter = nextOuter->getOuter())
		{
			if (nextOuter->isA(target))
			{
				result = nextOuter;
			}
		}
		return result;
	}

	bool ObjectBase::isTemplate(EObjectFlags templateTypes /* = RF_ArchetypeObject | RF_ClassDefaultObject */) const
	{
		for (const ObjectBase* testOuter = this; testOuter; testOuter = testOuter->getOuter())
		{
			if (testOuter->hasAnyFlags(templateTypes))
			{
				return true;
			}
		}
		return false;
	}

	wstring ObjectBase::getPathName(const Object* stopOuter /* = nullptr */) const
	{
		return TEXT("");
	}
}