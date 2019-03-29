#pragma once
#include "CoreMinimal.h"
#include "Class.h"
namespace Air
{
	template<class TClass>
	class TSubclassOf
	{
		template<class TClassA>
		friend class TSubclassOf;

	public:
		FORCEINLINE TSubclassOf():
			mClass(nullptr)
		{}

		FORCEINLINE TSubclassOf(RClass* from)
			:mClass(from)
		{
		}

		template<class TClassA, class = typename std::enable_if<std::is_base_of<TClass, TClassA>::value>::type>
		FORCEINLINE TSubclassOf(const TSubclassOf<TClassA>& from)
			:mClass(*from)
		{

		}

		template<class TClassA, class = typename std::enable_if<std::is_base_of<TClassA, TClass>::value>::type>
		FORCEINLINE TSubclassOf& operator = (const TSubclassOf<TClassA>& from)
		{
			mClass = *from;
			return *this;
		}

		FORCEINLINE TSubclassOf& operator=(RClass* from)
		{
			mClass = from;
			return *this;
		}

		FORCEINLINE RClass* operator*() const
		{
			if (!mClass || !mClass->isChildOf(TClass::StaticClass()))
			{
				return nullptr;
			}
			return mClass;
		}

		FORCEINLINE RClass* get() const
		{
			return **this;
		}

		FORCEINLINE RClass* operator->() const
		{
			return **this;
		}


		FORCEINLINE operator RClass* () const
		{
			return **this;
		}

		FORCEINLINE TClass* getDefaultObject() const
		{
			return mClass ? mClass->getDefaultObject<TClass>() : nullptr;
		}

	private:
		RClass* mClass;
	};
}