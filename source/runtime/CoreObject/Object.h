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
		std::shared_ptr<TRetureType> createDefaultSubObject(wstring subObjetName, bool bTransient = false)
		{
			RClass* returnType = TRetureType::StaticClass();
			return std::static_pointer_cast<TRetureType>(createDefaultSubObject(subObjetName, returnType, returnType, true, false, bTransient));
		}

		template<typename TReturnType, typename TClassToConstructByDefault>
		std::shared_ptr<TReturnType> createDefaultSubObject(wstring subObjectName, bool bTransient = false)
		{
			return std::static_pointer_cast<TReturnType>(createDefaultSubObject(subObjectName, TReturnType::StaticClass(), TClassToConstructByDefault::StaticClass(), true, false, bTransient));
		}

		std::shared_ptr<Object> createDefaultSubObject(wstring subObjectName, RClass* returnType, RClass* classToCreateByDefault, bool bIsRequired, bool bAbstract, bool bIsTransient);

		virtual void postLoad();

		void conditionalPostLoad();
	protected:

		template<typename TReturnType>
		std::shared_ptr<TReturnType> createAbsoluteDefaultSubobject(wstring subobjectName, bool bTransient = false)
		{
			RClass* ReTurnType = TReturnType::StaticClass();
			return std::dynamic_pointer_cast<TReturnType>(createDefaultSubObject(subobjectName, ReTurnType, ReTurnType, true, true, bTransient));

		}
	public:
		virtual World* getWorld() const;
		
		virtual World* getWorldChecked(bool& bSupported) const;

	private:

	};

	FORCEINLINE bool isValid(Object* test)
	{
		return test && !test->isPendingKill();
	}





}