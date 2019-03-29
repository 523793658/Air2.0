#pragma once
#include "CoreType.h"
#include "boost/assert.hpp"
#include "Serialization/Archive.h"
namespace Air
{

	template<typename ReferencedType>
	class TRefCountPtr
	{
		typedef ReferencedType* ReferenceType;

	public:
		FORCEINLINE TRefCountPtr():
		mReference(nullptr)
		{}

		TRefCountPtr(ReferenceType InReference, bool bAddRef = true)
		{
			mReference = InReference;
			if (mReference && bAddRef)
			{
				mReference->AddRef();
			}
		}

		TRefCountPtr(const TRefCountPtr& copy)
		{
			mReference = copy.mReference;
			if (mReference)
			{
				mReference->AddRef();
			}
		}

		FORCEINLINE TRefCountPtr(TRefCountPtr&& copy)
		{
			mReference = copy.mReference;
			copy.mReference = nullptr;
		}

		~TRefCountPtr()
		{
			if (mReference)
			{
				mReference->Release();
			}
		}

		TRefCountPtr& operator=(ReferenceType inReference)
		{
			ReferenceType oldReference = mReference;
			mReference = inReference;
			if (mReference)
			{
				mReference->AddRef();
			}
			if (oldReference)
			{
				oldReference->Release();
			}
			return *this;
		}

		FORCEINLINE TRefCountPtr& operator =(const TRefCountPtr& inPtr)
		{
			return *this = inPtr.mReference;
		}

		TRefCountPtr& operator = (TRefCountPtr&& inPtr)
		{
			if (this != &inPtr)
			{
				ReferencedType* oldReference = mReference;
				mReference = inPtr.mReference;
				inPtr.mReference = nullptr;
				if (oldReference)
				{
					oldReference->Release();
				}
			}
			return *this;
		}
		

		FORCEINLINE ReferenceType operator->() const
		{
			return mReference;
		}
		FORCEINLINE operator ReferenceType() const
		{
			return mReference;
		}

		FORCEINLINE ReferencedType** getInitReference()
		{
			*this = nullptr;
			return &mReference;
		}

		FORCEINLINE ReferenceType getReference() const
		{
			return mReference;
		}

		FORCEINLINE friend bool isValidRef(const TRefCountPtr& inReference)
		{
			return inReference.mReference != nullptr;
		}

		FORCEINLINE bool isValid() const
		{
			return mReference != nullptr;
		}

		FORCEINLINE void safeRelease()
		{
			*this = nullptr;
		}

		uint32 GetRefCount()
		{
			uint32 result = 0;
			if (mReference)
			{
				result = mReference->GetRefCount();
			}
			return result;
		}

		FORCEINLINE void swap(TRefCountPtr& inptr)
		{
			ReferenceType oldReference = mReference;
			mReference = inptr.mReference;
			inptr.mReference = oldReference;
		}

		friend Archive& operator<<(Archive& ar, TRefCountPtr& ptr)
		{
			ReferenceType ptrReference = ptr.mReference;
			ar << ptrReference;
			if (ar.isLoading())
			{
				ptr = ptrReference;
			}
			return ar;
		}

	private:
		ReferencedType*	mReference;
	};


	class IRefCountedObject
	{
	public:
		virtual uint32 AddRef() const = 0;
		virtual uint32 Release() const = 0;
		virtual uint32 GetRefCount() const = 0;
	};

	class CORE_API RefCountedObject
	{
	public:
		RefCountedObject() :mNumRefs(0) {}
		virtual ~RefCountedObject() { BOOST_ASSERT(!mNumRefs); }

		uint32 AddRef() const
		{
			return uint32(++mNumRefs);
		}
		uint32 Release() const
		{
			uint32 refs = uint32(--mNumRefs);
			if (refs == 0)
			{
				delete this;
			}
			return refs;
		}

		uint32 GetRefCount() const
		{
			return uint32(mNumRefs);
		}
	private:
		mutable int32 mNumRefs;
	};

}