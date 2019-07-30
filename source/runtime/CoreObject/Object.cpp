#include "Object.h"
#include "ObjectThreadContext.h"
#include "SimpleReflection.h"
namespace Air
{
	Object::Object(const ObjectInitializer& objectInitializer)
		:ObjectBase(objectInitializer)
	{
	}
	Object::Object(EStaticConstructor, EObjectFlags inFlags)
		:ObjectBase(inFlags)
	{

	}

	bool bGetWorldOverridden = false;

	class World* Object::getWorld() const
	{
		bGetWorldOverridden = false;
		return nullptr;
	}

	class World* Object::getWorldChecked(bool& bSupported) const
	{
		bGetWorldOverridden = true;
		World* world = getWorld();
		bSupported = bGetWorldOverridden;
		return world;
	}

	bool Object::modify(bool bAlwaysMarkDirty /* = true */)
	{
		return false;
	}

	void Object::postLoad()
	{

	}

	void Object::conditionalPostLoad()
	{
		if (hasAnyFlags(RF_NeedPostLoad))
		{
			BOOST_ASSERT(isInGameThread() || hasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || isA(RClass::StaticClass()));
			clearFlags(RF_NeedPostLoad);
			if (hasAnyFlags(RF_ClassDefaultObject))
			{
				getClass()->postLoadDefaultObject(this);
			}
			else
			{
				postLoad();
			}
		}
	}

	bool Object::isA(const RClass* SomeBase) const
	{
		RClass* thisClass = getClass();
		bool bOldResult = false;
		for (RClass* tempClass = thisClass; tempClass; tempClass = tempClass->getSupperClass())
		{
			if (tempClass == SomeBase)
			{
				bOldResult = true;
				break;
			}
		}
		return bOldResult;
	}

	std::shared_ptr<Object> Object::createDefaultSubObject(wstring subObjectName, RClass* returnType, RClass* classToCreateByDefault, bool bIsRequired, bool bAbstract, bool bIsTransient)
	{
		ObjectInitializer* currentInitializer = ObjectThreadContext::get().topInitializer();
		return currentInitializer->createDefaultSubObject(this, subObjectName, returnType, classToCreateByDefault, bIsRequired, bAbstract, bIsTransient);
	}



	DECLARE_SIMPLER_REFLECTION(Object)
}