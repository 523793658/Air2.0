#pragma once
#include "CoreType.h"
namespace Air
{
	class DelegateHandle
	{
	public:
		enum EGenerateNewHandleType
		{
			GenerateNewHandle
		};

		DelegateHandle()
			: mID(0)
		{
		}
		explicit DelegateHandle(EGenerateNewHandleType)
			: mID(generateNewID())
		{
		}

		bool isValid() const
		{
			return mID != 0;
		}

		void reset()
		{
			mID = 0;
		}


	private:

		friend bool operator == (const DelegateHandle& lhs, const DelegateHandle & rhs)
		{
			return lhs.mID == rhs.mID;
		}

		friend bool operator != (const DelegateHandle & lhs, const DelegateHandle & rhs)
		{
			return lhs.mID != rhs.mID;
		}



		static CORE_API uint64 generateNewID();

		uint64 mID;

	};


}