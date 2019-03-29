#pragma once
#include "CoreType.h"
namespace Air
{
	enum EResourceAlignment
	{
		VERTEXBUFFER_ALIGNMENT = DEFAULT_ALIGNMENT,
		INDEXBUFFER_ALIGNMENT = DEFAULT_ALIGNMENT
	};


	template<typename ElementType, uint32 Alignment = DEFAULT_ALIGNMENT>
	class TResourceArray : public ResourceArrayInterface, public TArray<ElementType, TAlignedHeapAllocator<Alignment>>
	{
	public:
		typedef TArray<ElementType, TAlignedHeapAllocator<Alignment>> Super;

		TResourceArray(bool inNeedsCPUAccess = false)
			:Super()
			,bNeedsCPUAccess(inNeedsCPUAccess)
		{}
#if PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS
		TResourceArray(TResourceArray&&) = default;
		TResourceArray(const TResourceArray&) = default;
		TResourceArray& operator = (TResourceArray&&) = default;
		TResourceArray& operator = (const TResourceArray&) = default;
#else

#endif
		virtual const void* getResourceData() const
		{
			return &(*this)[0];
		}

		virtual uint32 getResourceDataSize() const
		{
			return this->size() * sizeof(ElementType);
		}

		virtual void discard()
		{
			if (!bNeedsCPUAccess && !GIsEditor)
			{
				this->empty();
			}
		}

		virtual bool isStatic() const
		{
			return false;
		}

		virtual bool getAllowCPUAccess() const
		{
			return bNeedsCPUAccess;
		}

		virtual void setAllowCPUAccess(bool bInNeedsCPUAccess)
		{
			bNeedsCPUAccess = bInNeedsCPUAccess;
		}

		TResourceArray & operator = (const Super& other)
		{
			Super::operator =(other);
			return *this;
		}

		void bulkSerialize(Archive& ar)
		{

		}

		friend Archive & operator << (Archive& ar, TResourceArray<ElementType, Alignment>& resourceArray)
		{
			return ar << *(Super*)&resourceArray;
		}
	private:
		bool bNeedsCPUAccess;
	};
}