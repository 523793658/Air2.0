#pragma once
#include "CoreType.h"
namespace Air
{
	template <class ContainerType>
	class TLinkedListIteratorBase
	{
	public:
		explicit TLinkedListIteratorBase(ContainerType* firstLink)
			:mCurrentLink(firstLink)
		{}

		FORCEINLINE void next()
		{
			mCurrentLink = (ContainerType*)mCurrentLink->getNextLink();
		}

		FORCEINLINE TLinkedListIteratorBase& operator ++()
		{
			next();
			return *this;
		}
		FORCEINLINE TLinkedListIteratorBase operator ++(int)
		{
			auto tmp = *this;
			next();
			return tmp;
		}

		FORCEINLINE explicit operator bool() const
		{
			return mCurrentLink != nullptr;
		}

	protected:
		ContainerType* mCurrentLink;
		FORCEINLINE friend bool operator == (const TLinkedListIteratorBase& lhs, const TLinkedListIteratorBase& rhs)
		{
			return lhs.mCurrentLink == rhs.mCurrentLink;
		}

		FORCEINLINE friend bool operator != (const TLinkedListIteratorBase & lhs, const TLinkedListIteratorBase& rhs)
		{
			return lhs.mCurrentLink != rhs.mCurrentLink;
		}
	};




	template<class ContainerType, class ElementType>
	class TLinkedListIterator : public TLinkedListIteratorBase<ContainerType>
	{
		typedef TLinkedListIteratorBase<ContainerType> Supper;
	public:
		explicit TLinkedListIterator(ContainerType* first)
			:Supper(first)
		{

		}

		FORCEINLINE ElementType& operator ->()const
		{
			return **(this->mCurrentLink);
		}

		FORCEINLINE ElementType& operator*() const
		{
			return **(this->mCurrentLink);
		}
	};

	template<class ContainerType, class ElementType>
	class TIntrusiveLinkedListIterator : public TLinkedListIteratorBase<ElementType>
	{
		typedef TLinkedListIteratorBase<ElementType> Supper;
	public:	
		explicit TIntrusiveLinkedListIterator(ElementType* firstLink)
			:Supper(firstLink)
		{

		}
		FORCEINLINE ElementType& operator->() const
		{
			return*(this->mCurrentLink);
		}

		FORCEINLINE ElementType& operator*() const
		{
			return *(this->mCurrentLink);
		}
	};




	template<class ContainerType, class ElementType, template<class, class> class IteratorType>
	class TLinkedListBase
	{
	public:
		typedef IteratorType<ContainerType, ElementType> TIterator;
		typedef IteratorType<ContainerType, const ElementType> TConstIterator;

		TLinkedListBase()
			:mNextLink(nullptr)
			, mPrevLink(nullptr)
		{}

		FORCEINLINE void unLink()
		{
			if (mNextLink)
			{
				mNextLink->mPrevLink = mPrevLink;
			}
			if (mPrevLink)
			{
				*mPrevLink = mNextLink;
			}
			mNextLink = nullptr;
			mPrevLink = nullptr;
		}

		FORCEINLINE void linkBefore(ContainerType* before)
		{
			mPrevLink = before->mPrevLink;
			before->mPrevLink = &mNextLink;
			mNextLink = before;
			if (mPrevLink != nullptr)
			{
				*mPrevLink = (ContainerType*)this;
			}
		}

		FORCEINLINE void linkAfter(ContainerType* after)
		{
			mPrevLink = &after->mNextLink;
			mNextLink = *mPrevLink;
			*mPrevLink = (ContainerType*)this;
			if (mNextLink != nullptr)
			{
				mNextLink->mPrevLink = &mNextLink;
			}
		}


		FORCEINLINE void linkReplace(ContainerType* replace)
		{
			ContainerType** & replacePrev = replace->mPrevLink;
			ContainerType*&	replaceNext = replace->mNextLink;

			mPrevLink = replacePrev;
			mNextLink = replaceNext;
			if (mPrevLink != nullptr)
			{
				*mPrevLink = (ContainerType*)this;
			}
			if (mNextLink != nullptr)
			{
				mNextLink->mPrevLink = &mNextLink;
			}
			replaceNext = nullptr;
			replacePrev = nullptr;
		}

		FORCEINLINE void linkHead(ContainerType*& head)
		{
			if (head != nullptr)
			{
				head->mPrevLink = &mNextLink;
			}
			mNextLink = head;
			mPrevLink = &head;
			head = (ContainerType*)this;
		}

		FORCEINLINE bool isLinked()
		{
			return mPrevLink != nullptr;
		}

		FORCEINLINE ContainerType** getPrevLink() const
		{
			return mPrevLink;
		}

		FORCEINLINE ContainerType* getNextLink() const
		{
			return mNextLink;
		}

		FORCEINLINE ContainerType* next()
		{
			return mNextLink;
		}

	private:
		ContainerType* mNextLink;
		ContainerType** mPrevLink;

		FORCEINLINE friend TIterator	begin(ContainerType& list)
		{
			return TIterator(&list);
		}
		FORCEINLINE friend TConstIterator begin(const ContainerType& list)
		{
			return TConstIterator(const_cast<ContainerType*>(&list));
		}
		FORCEINLINE friend TIterator	end(ContainerType& list)
		{
			return TIterator(nullptr);
		}
		FORCEINLINE friend TConstIterator end(const ContainerType& list)
		{
			return TConstIterator(nullptr);
		}
	};



	template <class ElementType>
	class TLinkedList : public TLinkedListBase<TLinkedList<ElementType>, ElementType, TLinkedListIterator>
	{
		typedef TLinkedListBase<TLinkedList<ElementType>, ElementType, TLinkedListIterator> Super;
	public:
		TLinkedList()
			:Super()
		{}

		explicit TLinkedList(const ElementType& inElement)
			:Super()
			, mElement(inElement)
		{

		}

		FORCEINLINE ElementType* operator ->()
		{
			return &mElement;
		}

		FORCEINLINE const ElementType* operator ->() const
		{
			return &mElement;
		}

		FORCEINLINE ElementType& operator *()
		{
			return mElement;
		}

		FORCEINLINE const ElementType& operator*() const
		{
			return mElement;
		}
	private:
		ElementType mElement;
	};

	
}