#pragma once
#include "CoreType.h"
#include "Template/AirTypeTraits.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Containers/SparseArray.h"
namespace Air
{
	template<typename ElementType, typename InKeyType, bool bInAllowDuplicateKeys = false>
	struct BaseKeyFuncs
	{
		typedef InKeyType KeyType;
		typedef typename std::call_traits<InKeyType>::param_type KeyInitType;
		typedef typename std::call_traits<ElementType>::param_type ElementInitType;
		enum {
			bAllowDuplicateKeys = bInAllowDuplicateKeys
		};
	};


	template<typename ElementType, bool bInAllowDuplicateKeys = false>
	struct DefualtKeyFuncs :
		BaseKeyFuncs<ElementType, ElementType, bInAllowDuplicateKeys>
	{
		typedef typename std::call_traits<ElementType>::param_type KeyInitType;
		typedef typename std::call_traits<ElementType>::param_type ElementInitType;

		static FORCEINLINE KeyInitType getSetKey(ElementInitType element)
		{
			return element;
		}

		static FORCEINLINE bool matches(KeyInitType A, KeyInitType B)
		{
			return A == B;
		}

		static FORCEINLINE uint32 getKeyHash(KeyInitType key)
		{
			return getTypeHash(key);
		}
	};


	template<typename InElementType, typename KeyFuncs = DefualtKeyFuncs<InElementType>, typename Allocator = DefaultSetAllocator>
	class TSet;


	template<typename T>
	FORCEINLINE void moveByRelocate(T& A, T& B)
	{
		A.~T();
		relocateConstructItems<T>(&A, &B, 1);
	}

	class SetElementId
	{
	public:
		template<typename, typename, typename>
		friend class TSet;

		friend class ScriptSet;

		FORCEINLINE SetElementId()
			:mIndex(INDEX_NONE)
		{}

		FORCEINLINE bool isValidId() const
		{
			return mIndex != INDEX_NONE;
		}

		FORCEINLINE friend bool operator == (const SetElementId& A, const SetElementId& B)
		{
			return A.mIndex == B.mIndex;
		}

		FORCEINLINE int32 asInteger() const
		{
			return mIndex;
		}

		FORCEINLINE static SetElementId fromInteger(int32 integer)
		{
			return SetElementId(integer);
		}



	private:
		int32 mIndex;
		
		FORCEINLINE SetElementId(int32 inIndex)
			:mIndex(inIndex)
		{}

		FORCEINLINE operator int32() const
		{
			return mIndex;
		}
	
	};

	template<typename InElementType>
	class TSetElement
	{
	public:
		typedef InElementType ElementType;
		ElementType mValue;

		mutable SetElementId mHashNextId;
		mutable int32 mHashIndex;
		FORCEINLINE TSetElement()
		{}

		template<typename InitType> FORCEINLINE TSetElement(const InitType& inValue) :mValue(inValue) {}

		template<typename InitType> FORCEINLINE TSetElement(InitType&& inValue) : mValue(std::move(inValue)) {}

		FORCEINLINE TSetElement(const TSetElement& rhs) : mValue(rhs.mValue), mHashNextId(rhs.mHashNextId), mHashIndex(rhs.mHashIndex) {}

		FORCEINLINE TSetElement(TSetElement&& rhs) : mValue(std::move(rhs.mValue)), mHashNextId(std::move(rhs.mHashNextId)), mHashIndex(rhs.mHashIndex) {}

		FORCEINLINE TSetElement& operator = (const TSetElement& rhs) 
		{
			mValue = rhs.mValue;
			mHashNextId = rhs.mHashNextId;
			mHashIndex = rhs.mHashIndex;
			return *this;
		}

		FORCEINLINE TSetElement& operator = (TSetElement&& rhs) {
			mValue = std::move(rhs.mValue);
			mHashNextId = std::move(rhs.mHashNextId);
			mHashIndex = rhs.mHashIndex;
			return *this;
		}

		FORCEINLINE friend Archive& operator << (Archive& ar, TSetElement element)
		{
			return ar << element.mValue;
		}

		FORCEINLINE bool operator == (const TSetElement& other) const
		{
			return mValue == other.mValue;
		}

		FORCEINLINE bool operator != (const TSetElement & other) const
		{
			return mValue != other.mValue;
		}


	};


	template<typename InElementType, typename KeyFuncs, typename Allocator>
	class TSet
	{
		friend class ScriptSet;
		typedef typename KeyFuncs::KeyInitType KeyInitType;
		typedef typename KeyFuncs::ElementInitType ElementInitType;

		typedef TSetElement<InElementType> SetElementType;

		typedef TSparseArray<SetElementType, typename Allocator::SparseArrayAllocator>  ElementArrayType;

	public:
		typedef InElementType ElementType;


		FORCEINLINE TSet()
			: mHashSize(0)
		{}

		FORCEINLINE TSet(const TSet& copy)
			: mHashSize(0)
		{
			*this = copy;
		}

		FORCEINLINE explicit TSet(const TArray<ElementType> & InArray)
			:mHashSize(0)
		{
			append(InArray);
		}

		FORCEINLINE explicit TSet(TArray<ElementType>&& inArray)
			:mHashSize(0)
		{
			append(std::move(inArray));
		}

		void reset()
		{
			mElements.reset();
			for (int32 hashIndex = 0, localHashSize = mHashSize; hashIndex < localHashSize; ++hashIndex)
			{
				getTypeHash(hashIndex) == SetElementId();
			}
		}

		FORCEINLINE ~TSet()
		{
			mHashSize = 0;
		}

		TSet& operator = (const TSet& copy)
		{
			if (this != &copy)
			{
				empty(copy.size());
				for (TConstIterator copyIt(copy); copyIt; ++copyIt)
				{
					add(*copyIt);
				}
			}
			return *this;
		}

		template<bool bConst>
		class TBaseIterator
		{
		private:
			friend class TSet;
			typedef typename TChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;
		public:
			typedef typename TChooseClass<bConst, typename ElementArrayType::TConstIterator, typename ElementArrayType::TIterator>::Result ElementItType;

			FORCEINLINE TBaseIterator(const ElementItType& inElementIt)
				:mElementIt(inElementIt)
			{

			}

			FORCEINLINE TBaseIterator& operator ++()
			{
				++mElementIt;
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{
				return !!mElementIt;
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE SetElementId getId() const
			{
				return TSet::indexToId(mElementIt.getIndex());
			}

			FORCEINLINE ItElementType* operator->() const
			{
				return &mElementIt->mValue;
			}
			FORCEINLINE ItElementType& operator*() const
			{
				return mElementIt->mValue;
			}

			FORCEINLINE friend bool operator == (const TBaseIterator& lhs, const TBaseIterator& rhs)
			{
				return lhs.mElementIt == rhs.mElementIt;
			}

			FORCEINLINE friend bool operator != (const TBaseIterator& lhs, const TBaseIterator & rhs)
			{
				return lhs.mElementIt != rhs.mElementIt;
			}

		

			ElementItType mElementIt;
		};

	public:
		class TConstIterator : public TBaseIterator<true>
		{
			friend class TSet;
		public:
			FORCEINLINE TConstIterator(const typename TBaseIterator<true>::ElementItType& inElementId)
				:TBaseIterator<true>(inElementId)
			{}

			FORCEINLINE TConstIterator(const TSet& inSet)
				: TBaseIterator<true>(begin(inSet.mElements))
			{}
		};

		class TIterator : public TBaseIterator<false>
		{
			friend class TSet;
		public:
			FORCEINLINE TIterator(TSet& inSet, const typename TBaseIterator<false>::ElementItType& inElementId)
				:TBaseIterator<false>(inElementId),
				mSet(inSet)
			{}

			FORCEINLINE TIterator(TSet& inSet)
				: TBaseIterator<false>(begin(inSet.mElements))
				, mSet(inSet)
			{}

			FORCEINLINE void removeCurrent()
			{
				mSet.remove(TBaseIterator<false>::getId());
			}

		private:
			TSet& mSet;
		};

		static FORCEINLINE SetElementId indexToId(int32 index)
		{
			return SetElementId(index);
		}

		FORCEINLINE int32 size() const
		{
			return mElements.size();
		}

		bool conditionalRehash(int32 numHashedElements, bool bAllowShrinking = false) const
		{
			const int32 desiredHashSize = Allocator::getNumberOfHashBuckets(numHashedElements);
			if (numHashedElements > 0 && (!mHashSize || mHashSize < desiredHashSize || (mHashSize > desiredHashSize && bAllowShrinking)))
			{
				mHashSize = desiredHashSize;
				rehash();
				return true;
			}
			else
			{
				return false;
			}
		}

		void rehash() const
		{
			mHash.resizeAllocation(0, 0, sizeof(SetElementId));
			int32 localHashSize = mHashSize;
			if (localHashSize)
			{
				BOOST_ASSERT(!(localHashSize & (mHashSize - 1)));
				mHash.resizeAllocation(0, localHashSize, sizeof(SetElementId));
				for (int32 hashIndex = 0; hashIndex < localHashSize; ++hashIndex)
				{
					getTypeHash(hashIndex) = SetElementId();
				}
				for (typename ElementArrayType::TConstIterator elementIt(mElements); elementIt; ++elementIt)
				{
					hashElement(SetElementId(elementIt.getIndex()), *elementIt);
				}
			}
		}

		void empty(int32 expectedNumElements = 0)
		{
			mElements.empty();
			if (!conditionalRehash(expectedNumElements, true))
			{
				for (int32 i = 0, localHashSize = mHashSize; i < localHashSize; i++)
				{
					getTypeHash(i) = SetElementId();
				}
			}
		}

		FORCEINLINE ElementType & operator[](SetElementId id)
		{
			return mElements[id].mValue;
		}

		FORCEINLINE const ElementType & operator[](SetElementId id) const
		{
			return mElements[id].mValue;
		}

		SetElementId findId(KeyInitType key) const
		{
			if (mElements.count())
			{
				for (SetElementId elementId = getTypeHash(KeyFuncs::getKeyHash(key));
					elementId.isValidId(); elementId = mElements[elementId].mHashNextId)
				{
					if (KeyFuncs::matches(KeyFuncs::getSetKey(mElements[elementId].mValue), key))
					{
						return elementId;
					}
				}
			}
			return SetElementId();
		}

		FORCEINLINE SetElementId& getTypeHash(int32 hashIndex) const
		{
			return((SetElementId*)mHash.getAllocation())[hashIndex&(mHashSize - 1)];
		}

		FORCEINLINE void hashElement(SetElementId elementId, const SetElementType& element) const
		{
			element.mHashIndex = KeyFuncs::getKeyHash(KeyFuncs::getSetKey(element.mValue)) & (mHashSize - 1);
			element.mHashNextId = getTypeHash(element.mHashIndex);
			getTypeHash(element.mHashIndex) = elementId;
		}

		template<typename ArgsType>
		SetElementId emplace(ArgsType&& args, bool* bIsAlreadyInSetPtr = nullptr)
		{
			SparseArrayAllocationInfo elementAllocation = mElements.addUninitialized();
			SetElementId elementId(elementAllocation.mIndex);
			auto& element = *new(elementAllocation)SetElementType(std::forward<ArgsType>(args));

			bool bIsAlreadyInSet = false;
			if (!KeyFuncs::bAllowDuplicateKeys)
			{
				if (mElements.count() != 1)
				{
					SetElementId existingId = findId(KeyFuncs::getSetKey(element.mValue));
					bIsAlreadyInSet = existingId.isValidId();
					if (bIsAlreadyInSet)
					{
						moveByRelocate(mElements[existingId].mValue, element.mValue);
						mElements.removeAtUninitialized(elementId);
						elementId = existingId;
					}
				}
			}
			if (!bIsAlreadyInSet)
			{
				if (!conditionalRehash(mElements.count()))
				{
					hashElement(elementId, element);
				}
			}
			if (bIsAlreadyInSetPtr)
			{
				*bIsAlreadyInSetPtr = bIsAlreadyInSet;
			}
			return elementId;
		}

		void remove(SetElementId elementId)
		{
			if (mElements.count())
			{
				const auto& elementBeingRemoved = mElements[elementId];
				for (SetElementId* nextElementId = &getTypeHash(elementBeingRemoved.mHashIndex); nextElementId->isValidId(); nextElementId = &mElements[*nextElementId].mHashNextId)
				{
					if (*nextElementId == elementId)
					{
						*nextElementId = elementBeingRemoved.mHashNextId;
						break;
					}
				}
			}
			mElements.removeAt(elementId);
		}

		int remove(KeyInitType key)
		{
			int32 numRemovedElements = 0;
			if (mElements.size())
			{
				SetElementId* nextElementId = &getTypeHash(KeyFuncs::getKeyHash(key));
				while (nextElementId->isValidId())
				{
					auto& element = mElements[*nextElementId];
					if (KeyFuncs::matches(KeyFuncs::getSetKey(element.mValue), key))
					{
						remove(*nextElementId);
						numRemovedElements++;
						if (!KeyFuncs::bAllowDuplicateKeys)
						{
							break;
						}
					}
					else
					{
						nextElementId = &element.mHashNextId;
					}
				}
			}
			return numRemovedElements;
		}

		FORCEINLINE bool contains(KeyInitType key) const
		{
			return findId(key).isValidId();
		}

		FORCEINLINE ElementType* find(KeyInitType key)
		{
			SetElementId elementId = findId(key);
			if (elementId.isValidId())
			{
				return &mElements[elementId].mValue;
			}
			else
			{
				return nullptr;
			}
		}

		FORCEINLINE SetElementId add(const InElementType& inElement, bool* bIsAreadyInSetPtr = nullptr)
		{
			return emplace(inElement, bIsAreadyInSetPtr);
		}

		FORCEINLINE SetElementId add(InElementType&& inElement, bool* bIsAlreadyInSetPtr = nullptr) {
			return emplace(std::move(inElement), bIsAlreadyInSetPtr);
		}

	private:
		FORCEINLINE friend TIterator begin(TSet& set) { return TIterator(set, begin(set.mElements)); }
		FORCEINLINE friend TConstIterator begin(const TSet& set) { return TConstIterator(begin(set.mElements)); }

		FORCEINLINE friend TIterator end(TSet& set) { return TIterator(set, end(set.mElements)); }
		FORCEINLINE friend TConstIterator end(const TSet& set) { return TConstIterator(end(set.mElements)); }
	private:

	

		typedef typename Allocator::HashAllocator::template ForElementType<SetElementId> HashType;

		ElementArrayType mElements;

		mutable int32 mHashSize;
		mutable HashType mHash;
	};
}