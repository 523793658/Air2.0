#pragma once
#include "CoreObject.h"
#include "ObjectMacros.h"
#include "ObjectGlobals.h"
#include "ObjectBase.h"

namespace Air
{
	class World;
	class AActor;
	class RClass;
	class COREOBJECT_API Object : public ObjectBase
	{
		GENERATED_RCLASS_BODY(Object, ObjectBase)

	public:

		Object(EStaticConstructor, EObjectFlags inFlags);

		virtual bool modify(bool bAlwaysMarkDirty = true);

		bool isA(const RClass* SomeBase) const;

		virtual void postInitProperties();

		template<class TRetureType>
		TRetureType* createDefaultSubObject(wstring subObjetName, bool bTransient = false)
		{
			RClass* returnType = TRetureType::StaticClass();
			return static_cast<TRetureType*>(createDefaultSubObject(subObjetName, returnType, returnType, true, false, bTransient));
		}

		template<typename TReturnType, typename TClassToConstructByDefault>
		TReturnType* createDefaultSubObject(wstring subObjectName, bool bTransient = false)
		{
			return static_cast<TReturnType*>(createDefaultSubObject(subObjectName, TReturnType::StaticClass(), TClassToConstructByDefault::StaticClass(), true, false, bTransient));
		}

		Object* createDefaultSubObject(wstring subObjectName, RClass* returnType, RClass* classToCreateByDefault, bool bIsRequired, bool bAbstract, bool bIsTransient);

		virtual void postLoad();

		void conditionalPostLoad();
	protected:

		template<typename TReturnType>
		TReturnType* createAbsoluteDefaultSubobject(wstring subobjectName, bool bTransient = false)
		{
			RClass* ReTurnType = TReturnType::StaticClass();
			return static_cast<TReturnType*>(createDefaultSubObject(subobjectName, ReTurnType, ReTurnType, true, true, bTransient));

		}
	public:
		virtual World* getWorld() const;
		
		virtual World* getWorldChecked(bool& bSupported) const;

	private:

	};

	FORCEINLINE bool isValid(const Object* test)
	{
		return test && !test->isPendingKill();
	}





}