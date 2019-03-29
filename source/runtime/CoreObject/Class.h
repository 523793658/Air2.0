#pragma once
#include "Object.h"
#include "ObjectGlobals.h"
#include "boost/lexical_cast.hpp"
namespace Air
{
	class Object;

	template<class T>
	void internalConstructor(const ObjectInitializer& x)
	{
		T::__defaultConstructor(x);
	}

	class COREOBJECT_API RStruct : public Object
	{
	public:
		RStruct()
			:Object(EC_StaticConstructor, RF_NoFlags)
		{

		}
	};

	class COREOBJECT_API RClass :public RStruct
	{
		friend class SimpleReflectionManager;
	public:
		size_t mClassId;

		string mUniqueName;

		RClass* mSuperClass{ nullptr };

		Object* mClassDefaultObject{ nullptr };

		int32 mPropertiesSize;
	public:
		RClass* getSupperClass() const
		{
			return mSuperClass;
		}

		FORCEINLINE int32 getPropertiesSize() const
		{
			return mPropertiesSize;
		}

		virtual void postLoadDefaultObject(Object* object) { object->postLoad(); }

		void addDefaultSubobject(Object* newSubObject, RClass* baseClass)
		{
			BOOST_ASSERT(newSubObject->isA(baseClass));
			BOOST_ASSERT(isChildOf(newSubObject->getOuter()->getClass()));
		}

	public:

		typedef void(*ClassConstructorType) (const ObjectInitializer&);

		ClassConstructorType classConstructor;

		Object* getTypeOuter(RClass* target) const;

		FORCEINLINE int32 getMinAlignment() const
		{
			return 1;
		}

		//virtual Object* createInstance() = 0;

		template<typename T>
		T* getTypeOuter() const
		{
			return (T*)getTypeOuter(T::StaticClass());
		}

		Object* createDefaultObject();

		Object* getDefaultObject(bool bCreateIfNeed = true)
		{
			if (mClassDefaultObject == nullptr && bCreateIfNeed)
			{
				createDefaultObject();
			}
			return mClassDefaultObject;
		}

		template<class T>
		T* getDefaultObject()
		{
			Object* ret = getDefaultObject();
			return (T*)ret;
		}



		template<class T>
		bool isChildOf() const
		{
			return isChildOf(T::StaticClass());
		}

		bool isChildOf(const RClass* someBase) const
		{
			for (const RClass* Class = this; Class; Class = Class->getSupperClass())
			{
				if (Class == someBase)
				{
					return true;
				}
			}
			return false;
		}


	};

	template<typename Type>
	class TClass : public RClass
	{
		typedef Type ClassType;
	public:
		TClass()
		{
			const type_info & info = typeid(ClassType);
			mClassId = info.hash_code();
			classConstructor = &internalConstructor<Type>;
			mPropertiesSize = sizeof(ClassType);
			mUniqueName = string(info.name()) + boost::lexical_cast<string>(info.hash_code());
		}
	};
}

