#pragma once

#include "Containers/LockFreeVoidPointerListBase.h"
#include "Containers/Array.h"
namespace Air
{
	template<class T>
	class TLockFreePointerListLIFOBase : public
		LockFreeVoidPointerListGeneric
	{
	public:

		void push(T* newItem)
		{
			LockFreeVoidPointerListGeneric::push(newItem);
		}

		T* pop()
		{
			return (T*)LockFreeVoidPointerListGeneric::pop();
		}

		void popAll(TArray<T*>& output)
		{
			LockFreeVoidPointerListGeneric::popAll<TArray<T*>, T*>(output);
		}

		FORCEINLINE bool isEmpty() const
		{
			return LockFreeVoidPointerListGeneric::isEmpty();
		}

	protected:
		FORCEINLINE bool replaceListIfEmpty(LockFreeVoidPointerListBase& notTreadSafeTempListToReplaceWith)
		{
			return LockFreeVoidPointerListGeneric::replaceListIfEmpty(notTreadSafeTempListToReplaceWith);
		}
	};


	template<class T>
	class TClosableLockFreePointerListLIFO : private LockFreeVoidPointerListGeneric
	{
	public:
		bool pushIfNotClosed(T* newItem)
		{
			return LockFreeVoidPointerListGeneric::pushIfNotClosed(newItem);
		}

		void popAllAndClose(TArray<T*>& output)
		{
			LockFreeVoidPointerListGeneric::popAllAndClose<TArray<T*>, T*>(output);
		}

		bool isClosed() const
		{
			return LockFreeVoidPointerListGeneric::isClosed();
		}
	};

	template<class T>
	class TReopenableLockFreePointerListLIFO : private LockFreeVoidPointerListGeneric
	{
	public:
		bool reopenIfClosedAndPush(T *newItem)
		{
			bool bWasReopenedByMe = LockFreeVoidPointerListGeneric::reopenIfClosedAndPush(newItem);
			return bWasReopenedByMe;
		}

		void popAll(TArray<T*>& output)
		{
			LockFreeVoidPointerListGeneric::popAll<TArray<T*>, T*>(output);
		}
	};

	template<class T, int TPaddingForCacheContention>
	class TLockFreePointerListUnordered : public
		TLockFreePointerListLIFOBase<T>
	{

	};

	template<class T>
	class TLockFreePointerListLIFO : public TLockFreePointerListLIFOBase<T>
	{
	public:
		FORCEINLINE bool replaceListIfEmpty(LockFreeVoidPointerListBase& notTreadSafeTempListToReplaceWith)
		{
			return TLockFreePointerListLIFOBase<T>::replaceListIfEmpty(notTreadSafeTempListToReplaceWith);
		}
	};

	template<class T, int TPaddingForCacheContention>
	class TClosableLockFreePointerListUnorderedSingleConsumer : public TClosableLockFreePointerListLIFO<T>
	{

	};
}