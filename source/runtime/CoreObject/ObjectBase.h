#pragma once
#include "CoreObject.h"
#include "ObjectMacros.h"
namespace Air
{
	class RClass;
	class Object;
	class COREOBJECT_API ObjectBase
	{
	protected:
		ObjectBase()
		{}


	public:
		ObjectBase(RClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, Object* inOuter, wstring name);
		ObjectBase(EObjectFlags inFlags);
		virtual ~ObjectBase() {}

		FORCEINLINE RClass* getClass() const
		{
			return mClassPrivate;
		}

		FORCEINLINE bool hasAnyFlags(EObjectFlags flagsToCheck) const
		{
			BOOST_ASSERT(!(flagsToCheck & (RF_MarkAsNative | RF_MarkAsRootSet)) || flagsToCheck == RF_AllFlags);
			return (getFlags() & flagsToCheck) != 0;

		}

		FORCEINLINE EObjectFlags getFlags() const
		{
			BOOST_ASSERT((mObjectFlags & ~RF_AllFlags) == 0);
			return mObjectFlags;
		}

		FORCEINLINE void clearFlags(EObjectFlags newFlags)
		{
			BOOST_ASSERT(!(newFlags &(RF_MarkAsNative | RF_MarkAsRootSet)) || newFlags == RF_AllFlags);
			setFlagsTo(EObjectFlags(getFlags() & ~newFlags));
		}

		FORCEINLINE EObjectFlags getMaskedFlags(EObjectFlags mask = RF_AllFlags) const
		{
			return EObjectFlags(getFlags() & mask);
		}

		FORCEINLINE void setFlags(EObjectFlags newFlags)
		{
			BOOST_ASSERT(!(newFlags & (RF_MarkAsRootSet | RF_MarkAsNative)));
			setFlagsTo(getFlags() | newFlags);
		}

		FORCEINLINE Object* getOuter() const
		{
			return mOuterPrivate;
		}
		Object* getTypedOuter(RClass* target) const;

		template<typename T>
		T* getTypedOuter() const
		{
			return (T*)getTypedOuter(T::StaticClass());
		}

		FORCEINLINE bool isPendingKill() const
		{
			return false;
		}

		FORCEINLINE void markPendingKill()
		{

		}

		FORCEINLINE void clearPendingKill()
		{

		}

		wstring getName() const
		{
			return mNamePrivate;
		}

		wstring getPathName(const Object* stopOuter = nullptr) const;

		bool isTemplate(EObjectFlags templateTypes = RF_ArchetypeObject | RF_ClassDefaultObject) const;
	protected:
		FORCEINLINE void setFlagsTo(EObjectFlags newFlags)
		{
			BOOST_ASSERT((newFlags & ~RF_AllFlags) == 0);
			mObjectFlags = newFlags;
		}
	protected:
		EObjectFlags	mObjectFlags;
		int32 mInternalIndex;

	private:
		RClass* mClassPrivate;

		wstring mNamePrivate;

		Object*	mOuterPrivate;
	};
}