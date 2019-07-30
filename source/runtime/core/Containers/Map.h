#pragma once
#include "CoreType.h"
#include "Serialization/Archive.h"
#include <unordered_map>
#include "Template/TypeHash.h"
#include "Template/RefCounting.h"
namespace Air
{
	template<typename T>
	inline size_t getTypeHash(const T& arg)
	{
		std::hash<T> h;
		return h(arg);
	}

	template<typename T>
	inline size_t getTypeHash(TRefCountPtr<T>& arg)
	{
		return getTypeHash(arg.getReference());
	}

	template<typename T>
	inline size_t getTypeHash(const TRefCountPtr<T>& arg)
	{
		return getTypeHash(arg.getReference());
	}

	template<typename T>
	inline size_t getTypeHash(const std::weak_ptr<T>& arg)
	{
		return getTypeHash(arg._Get());
	}


	template<typename KeyType>
	struct _MapKey
	{
		std::size_t operator()(const KeyType& key) const
		{
			return static_cast<std::size_t>(getTypeHash(key));
		}
	};




	template<typename KeyType, typename ValueType, typename KeyHash = _MapKey<KeyType>>
	class TMap :public std::unordered_map<KeyType, ValueType, KeyHash>
	{
	public:

		FORCEINLINE ValueType findRef(const KeyType& key) const
		{
			const auto it = find(key);
			if (it == end())
			{
				return ValueType();
			}
			return it->second;
		}

		FORCEINLINE ValueType findChecked(const KeyType& key) const
		{
			const auto it = find(key);
			BOOST_ASSERT(it != end());
			return it->second;
		}
		FORCEINLINE ValueType& findChecked(const KeyType& key)
		{
			const auto it = find(key);
			BOOST_ASSERT(it != end());
			return it->second;
		}

		const KeyType* findKey(ValueType value) const
		{
			for (auto& it : *this)
			{
				if (it.second == value)
				{
					return &it.first;
				}
			}
			return nullptr;
		}

		void append(TMap other)
		{
			for (auto& it : other)
			{
				(*this)[it.first] = it.second;
			}
		}

		FORCEINLINE friend Archive & operator << (Archive& ar, TMap<KeyType, ValueType>& map)
		{
			size_t s = map.size();
			ar << s;
			if (ar.isLoading())
			{
				KeyType key;
				ValueType value;
				for (size_t i = 0; i < s; i++)
				{
					ar << key;
					ar << value;
					map.emplace(key, value);
				}
			}
			else
			{
				for (TMap<KeyType, ValueType>::iterator it = map.begin(); it != map.end(); it++)
				{
					ar << const_cast<KeyType&>(it->first);
					ar << it->second;
				}
			}
			return ar;
		}
		template<typename ArgType>
		FORCEINLINE ValueType& findOrAddImpl(ArgType&& arg)
		{
			return emplace(arg, ValueType()).first->second;
		}


		FORCEINLINE ValueType& findOrAdd(const KeyType& key) 
		{
			return findOrAddImpl(key);
		}

		FORCEINLINE ValueType& findOrAdd(KeyType&& key)
		{
			return findOrAddImpl(std::move(key));
		}
	};

	

	
}