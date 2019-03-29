#pragma once
#include "CoreType.h"
namespace Air
{
	template<typename OptionalType>
	struct TOptional
	{
	public:
		TOptional(const OptionalType& inValue)
		{
			new (&mValue) OptionalType(inValue);
			bIsSet = true;
		}
		TOptional(OptionalType&& inValue)
		{
			new (&mValue) OptionalType(std::move(inValue));
			bIsSet = true;
		}

		TOptional()
			:bIsSet(false)
		{

		}
		~TOptional()
		{
			reset();
		}

		TOptional(const TOptional& inValue)
			:bIsSet(false)
		{
			if (inValue.bIsSet)
			{
				new (&mValue)OptionalType(*(const OptionalType*)&inValue.mValue);
				bIsSet = true;
			}
		}
		TOptional(TOptional&& inValue)
			:bIsSet(false)
		{
			if (inValue.bIsSet)
			{
				new (&mValue)OptionalType(std::move(*(OptionalType*)&inValue.mValue));
				bIsSet = true;
			}
		}

		TOptional& operator = (const TOptional& inValue)
		{
			if (&inValue != this)
			{
				reset();
				if (inValue.bIsSet)
				{
					new(&mValue)OptionalType(*(const OptionalType*)&inValue.mValue);
					bIsSet = true;
				}
			}
			return *this;
		}
		TOptional& operator = (TOptional&& inValue)
		{
			if(&inValue != this)
			{
				reset();
				if (inValue.bIsSet)
				{
					new(&mValue)OptionalType(std::move(*(OptionalType*)&inValue.mValue));
					bIsSet = true;
				}
			}
			return *this;

		}

		TOptional & operator = (const OptionalType& inValue)
		{
			if (&inValue != (OptionalType*)&mValue)
			{
				reset();
				new(&mValue)OptionalType(inValue);
				bIsSet = true;
			}
			return *this;
		}
		TOptional & operator = (OptionalType&& inValue)
		{
			if (&inValue != (OptionalType*)&mValue)
			{
				reset();
				new(&mValue)OptionalType(std::move(inValue));
				bIsSet = true;
			}
			return *this;
		}

		void reset()
		{
			if (bIsSet)
			{
				bIsSet = false;
				typedef OptionalType OptionalDestructOptionalType;
				((OptionalType*)&mValue)->OptionalDestructOptionalType::~OptionalDestructOptionalType();
			}
		}

		template <typename... ArgsType>
		void emplace(ArgsType&&...args)
		{
			reset();
			new(&mValue)OptionalType(std::forward<ArgsType>(args)...);
			bIsSet = true;
		}

		friend bool operator == (const TOptional& lhs, const TOptional& rhs)
		{
			if (lhs.bIsSet != rhs.bIsSet)
			{
				return false;
			}
			if (!lhs.bIsSet)
			{
				return true;
			}
			return (*(OptionalType*)&lhs.mValue) == (*(OptionalType*)&rhs.mValue);
		}

		friend bool operator != (const TOptional & lhs, const TOptional& rhs)
		{
			return !(lhs == rhs);
		}

		bool isSet() const { return bIsSet; }
		FORCEINLINE explicit operator bool() const { return bIsSet; }

		const OptionalType& getValue() const 
		{
			BOOST_ASSERT(isSet()); 
			return *(OptionalType*)&mValue;
		}

		OptionalType& getValue()
		{
			BOOST_ASSERT(isSet());
			return *(OptionalType*)&mValue;
		}
		const OptionalType* operator->() const { return &getValue(); }
		OptionalType* operator->() { return getValue(); }

		const OptionalType& get(const OptionalType& defalutValue) const
		{
			return isSet() ? *(OptionalType*)&mValue : defalutValue;
		}



	private:
		bool bIsSet;
		TTypeCompatibleBytes<OptionalType> mValue;
	};
}